#include "command_task.h"

#ifdef USE_RTOS

#include "command_interface.h"
#include "mutex_manager.h"
#include "rtos_manager.h"
#include <stdarg.h>

// ==========================================
// GLOBAL INSTANCE
// ==========================================
CommandTask* commandTask = nullptr;

// ==========================================
// SERIAL INPUT BUFFER IMPLEMENTATION
// ==========================================
SerialInputBuffer::SerialInputBuffer() 
    : writeIndex(0), lineStartIndex(0), lineReady(false) {
    memset(buffer, 0, SERIAL_INPUT_BUFFER_SIZE);
    memset(lineBuffer, 0, MAX_COMMAND_LENGTH);
}

bool SerialInputBuffer::readAvailable() {
    while (Serial.available()) {
        char c = Serial.read();
        processCharacter(c);
        
        if (lineReady) {
            return true;
        }
    }
    return lineReady;
}

void SerialInputBuffer::processCharacter(char c) {
    if (c == '\n' || c == '\r') {
        handleEnter();
    } else if (c == 127 || c == 8) { // Backspace or DEL
        handleBackspace();
    } else if (c >= 32 && c <= 126) { // Printable characters
        uint16_t currentLength = (writeIndex >= lineStartIndex) 
            ? (writeIndex - lineStartIndex) 
            : (SERIAL_INPUT_BUFFER_SIZE - lineStartIndex + writeIndex);
        
        if (currentLength < MAX_COMMAND_LENGTH - 1) {
            buffer[writeIndex] = c;
            writeIndex = (writeIndex + 1) % SERIAL_INPUT_BUFFER_SIZE;
            Serial.print(c); // Echo character
        }
    }
}

void SerialInputBuffer::handleBackspace() {
    if (writeIndex != lineStartIndex) {
        writeIndex = (writeIndex == 0) ? (SERIAL_INPUT_BUFFER_SIZE - 1) : (writeIndex - 1);
        buffer[writeIndex] = 0;
        Serial.print("\b \b"); // Erase character on terminal
    }
}

void SerialInputBuffer::handleEnter() {
    Serial.println(); // Move to new line
    
    // Extract the line
    uint16_t idx = lineStartIndex;
    uint16_t lineIdx = 0;
    
    while (idx != writeIndex && lineIdx < MAX_COMMAND_LENGTH - 1) {
        lineBuffer[lineIdx++] = buffer[idx];
        idx = (idx + 1) % SERIAL_INPUT_BUFFER_SIZE;
    }
    lineBuffer[lineIdx] = '\0';
    
    lineReady = true;
}

bool SerialInputBuffer::getLine(char* output, size_t maxLength) {
    if (!lineReady) {
        return false;
    }
    
    strncpy(output, lineBuffer, maxLength - 1);
    output[maxLength - 1] = '\0';
    
    // Reset for next line
    lineStartIndex = writeIndex;
    lineReady = false;
    memset(lineBuffer, 0, MAX_COMMAND_LENGTH);
    
    return true;
}

void SerialInputBuffer::clear() {
    writeIndex = 0;
    lineStartIndex = 0;
    lineReady = false;
    memset(buffer, 0, SERIAL_INPUT_BUFFER_SIZE);
    memset(lineBuffer, 0, MAX_COMMAND_LENGTH);
}

// ==========================================
// COMMAND HISTORY IMPLEMENTATION
// ==========================================
CommandHistory::CommandHistory() 
    : count(0), writeIndex(0), navigationIndex(-1) {
}

void CommandHistory::add(const String& command) {
    if (command.length() == 0) {
        return;
    }
    
    // Don't add duplicate of last command
    if (count > 0) {
        uint8_t lastIdx = (writeIndex == 0) ? (COMMAND_HISTORY_SIZE - 1) : (writeIndex - 1);
        if (history[lastIdx] == command) {
            return;
        }
    }
    
    history[writeIndex] = command;
    writeIndex = (writeIndex + 1) % COMMAND_HISTORY_SIZE;
    
    if (count < COMMAND_HISTORY_SIZE) {
        count++;
    }
    
    navigationIndex = -1; // Reset navigation
}

String CommandHistory::getPrevious(const String& current) {
    if (count == 0) {
        return "";
    }
    
    if (navigationIndex == -1) {
        // Starting navigation from current
        navigationIndex = (writeIndex == 0) ? (count - 1) : (writeIndex - 1);
    } else {
        // Move backwards in history
        if (navigationIndex > 0 || count == COMMAND_HISTORY_SIZE) {
            navigationIndex = (navigationIndex == 0) ? (COMMAND_HISTORY_SIZE - 1) : (navigationIndex - 1);
        }
    }
    
    return history[navigationIndex];
}

String CommandHistory::getNext() {
    if (navigationIndex == -1) {
        return "";
    }
    
    navigationIndex = (navigationIndex + 1) % COMMAND_HISTORY_SIZE;
    
    // Check if we've reached the end
    if (navigationIndex == writeIndex) {
        navigationIndex = -1;
        return "";
    }
    
    return history[navigationIndex];
}

void CommandHistory::resetNavigation() {
    navigationIndex = -1;
}

void CommandHistory::clear() {
    count = 0;
    writeIndex = 0;
    navigationIndex = -1;
    for (int i = 0; i < COMMAND_HISTORY_SIZE; i++) {
        history[i] = "";
    }
}

void CommandHistory::print() {
    MutexLock lock(serialMutex, "CommandHistory::print");
    
    Serial.println("\n=== Command History ===");
    if (count == 0) {
        Serial.println("(empty)");
    } else {
        uint8_t startIdx = (count < COMMAND_HISTORY_SIZE) ? 0 : writeIndex;
        for (uint8_t i = 0; i < count; i++) {
            uint8_t idx = (startIdx + i) % COMMAND_HISTORY_SIZE;
            Serial.printf("%d: %s\n", i + 1, history[idx].c_str());
        }
    }
    Serial.println("======================\n");
}

// ==========================================
// COMMAND TASK IMPLEMENTATION
// ==========================================
CommandTask::CommandTask() 
    : TaskBase("CommandTask", 4096, TaskPriority::PRIORITY_HIGH, 1), // Pin to Core 1 (Application Core)
      promptShown(false),
      lastActivity(0),
      commandStartTime(0),
      commandInProgress(false) {
}

CommandTask::~CommandTask() {
}

void CommandTask::setup() {
    MutexLock lock(serialMutex, "CommandTask::setup");
    
    Serial.println("\n==========================================");
    Serial.println("   Command Task Initialized (RTOS Mode)");
    Serial.println("==========================================");
    
    inputBuffer.clear();
    commandHistory.clear();
    
    showInitialPrompt();
}

void CommandTask::loop() {
    // Process serial input (non-blocking)
    processSerialInput();
    
    // Process commands from queue
    processCommandQueue();
    
    // Check for command timeout
    if (commandInProgress) {
        checkCommandTimeout();
    }
    
    // Show prompt if needed
    if (!promptShown && !commandInProgress && (millis() - lastActivity > 100)) {
        showPrompt();
    }
    
    // Small delay to prevent tight loop
    vTaskDelay(pdMS_TO_TICKS(10));
}

void CommandTask::cleanup() {
    MutexLock lock(serialMutex, "CommandTask::cleanup");
    Serial.println("\nCommand Task shutting down...");
}

void CommandTask::processSerialInput() {
    if (inputBuffer.readAvailable()) {
        char lineBuffer[MAX_COMMAND_LENGTH];
        if (inputBuffer.getLine(lineBuffer, MAX_COMMAND_LENGTH)) {
            String command = String(lineBuffer);
            command.trim();
            
            if (command.length() > 0) {
                // Add to history
                commandHistory.add(command);
                
                // Send to command queue for async execution
                CommandRequest request;
                request.type = CommandRequest::UNKNOWN; // Will be determined by parser
                request.commandString = command;
                request.argument = "";
                request.secondArgument = "";
                request.requestId = millis();
                request.timestamp = millis();
                
                if (sendCommand(request, 100)) {
                    commandInProgress = true;
                    commandStartTime = millis();
                    currentCommandText = command;
                } else {
                    MutexLock lock(serialMutex, "CommandTask::processSerialInput");
                    Serial.println("✗ Command queue full, please wait...");
                    showPrompt();
                }
            } else {
                showPrompt();
            }
            
            lastActivity = millis();
        }
    }
}

void CommandTask::processCommandQueue() {
    CommandRequest request;
    
    // Non-blocking check for commands
    if (receiveCommand(request, 0)) {
        commandInProgress = true;
        commandStartTime = millis();
        
        // Execute the command
        String cmdStr = request.commandString;
        executeCommand(cmdStr);
        
        commandInProgress = false;
        showPrompt();
    }
}

void CommandTask::executeCommand(const String& command) {
    // Delegate to existing command parser
    parseAndExecute(command);
}

void CommandTask::parseAndExecute(const String& command) {
    // This calls the existing executeCommand function from command_interface.cpp
    // which will be wrapped with mutex protection
    ::executeCommand(command);
}

void CommandTask::showPrompt() {
    if (!promptShown && !commandInProgress) {
        MutexLock lock(serialMutex, "CommandTask::showPrompt");
        Serial.print("> ");
        promptShown = true;
    }
}

void CommandTask::showInitialPrompt() {
    Serial.println("\n==========================================");
    Serial.println("🟡 Device in IDLE mode - Ready for commands");
    Serial.println("Type 'help' for available commands");
    Serial.println("==========================================\n");
    
    showPrompt();
}

void CommandTask::checkCommandTimeout() {
    if (millis() - commandStartTime > COMMAND_TIMEOUT_MS) {
        MutexLock lock(serialMutex, "CommandTask::checkCommandTimeout");
        Serial.println("\n✗ Command timeout!");
        Serial.printf("Command '%s' exceeded %d ms timeout\n", 
                     currentCommandText.c_str(), COMMAND_TIMEOUT_MS);
        
        commandInProgress = false;
        currentCommandText = "";
        showPrompt();
    }
}

// ==========================================
// PUBLIC API IMPLEMENTATION
// ==========================================
bool initializeCommandTask() {
    if (commandTask != nullptr) {
        return true; // Already initialized
    }
    
    commandTask = new CommandTask();
    if (commandTask == nullptr) {
        Serial.println("✗ Failed to allocate CommandTask");
        return false;
    }
    
    if (!commandTask->start()) {
        Serial.println("✗ Failed to start CommandTask");
        delete commandTask;
        commandTask = nullptr;
        return false;
    }
    
    return true;
}

void shutdownCommandTask() {
    if (commandTask != nullptr) {
        commandTask->stop(5000);
        delete commandTask;
        commandTask = nullptr;
    }
}

bool isCommandTaskRunning() {
    if (commandTask == nullptr) {
        return false;
    }
    
    TaskStatistics stats = commandTask->getStatistics();
    return (stats.state == TaskState::RUNNING);
}

void safePrintCommand(const String& message) {
    MutexLock lock(serialMutex, "safePrintCommand");
    Serial.print(message);
}

void safePrintlnCommand(const String& message) {
    MutexLock lock(serialMutex, "safePrintlnCommand");
    Serial.println(message);
}

void safePrintfCommand(const char* format, ...) {
    MutexLock lock(serialMutex, "safePrintfCommand");
    
    va_list args;
    va_start(args, format);
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.print(buffer);
    
    va_end(args);
}

#endif // USE_RTOS
