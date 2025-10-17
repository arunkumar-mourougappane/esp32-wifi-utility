#pragma once

#include <Arduino.h>
#include "task_base.h"
#include "queue_manager.h"

#ifdef USE_RTOS

// ==========================================
// CONSTANTS
// ==========================================
#define SERIAL_INPUT_BUFFER_SIZE 256
#define COMMAND_HISTORY_SIZE 10
#define MAX_COMMAND_LENGTH 128
#define COMMAND_TIMEOUT_MS 30000  // 30 seconds

// ==========================================
// SERIAL INPUT BUFFER
// ==========================================
/**
 * @brief Non-blocking circular buffer for serial input
 * 
 * Provides character-by-character input without blocking,
 * supports backspace, echo, and line buffering.
 */
class SerialInputBuffer {
private:
    char buffer[SERIAL_INPUT_BUFFER_SIZE];
    uint16_t writeIndex;
    uint16_t lineStartIndex;
    bool lineReady;
    char lineBuffer[MAX_COMMAND_LENGTH];
    
public:
    SerialInputBuffer();
    
    /**
     * @brief Read available characters from serial without blocking
     * @return True if a complete line is ready
     */
    bool readAvailable();
    
    /**
     * @brief Get the complete line if ready
     * @param output Buffer to store the line
     * @param maxLength Maximum length to copy
     * @return True if line was retrieved
     */
    bool getLine(char* output, size_t maxLength);
    
    /**
     * @brief Clear the input buffer
     */
    void clear();
    
    /**
     * @brief Check if a line is ready for processing
     */
    bool isLineReady() const { return lineReady; }
    
private:
    void processCharacter(char c);
    void handleBackspace();
    void handleEnter();
};

// ==========================================
// COMMAND HISTORY
// ==========================================
/**
 * @brief Command history with navigation support
 * 
 * Stores recent commands and allows navigation with arrow keys.
 */
class CommandHistory {
private:
    String history[COMMAND_HISTORY_SIZE];
    uint8_t count;
    uint8_t writeIndex;
    int8_t navigationIndex;  // -1 means not navigating
    
public:
    CommandHistory();
    
    /**
     * @brief Add a command to history
     * @param command Command string to add
     */
    void add(const String& command);
    
    /**
     * @brief Get previous command in history
     * @param current Current input buffer
     * @return Previous command, or empty if at start
     */
    String getPrevious(const String& current);
    
    /**
     * @brief Get next command in history
     * @return Next command, or empty if at end
     */
    String getNext();
    
    /**
     * @brief Reset navigation to end of history
     */
    void resetNavigation();
    
    /**
     * @brief Clear all history
     */
    void clear();
    
    /**
     * @brief Get number of commands in history
     */
    uint8_t getCount() const { return count; }
    
    /**
     * @brief Print all history entries
     */
    void print();
};

// ==========================================
// COMMAND TASK
// ==========================================
/**
 * @brief Command Interface Task
 * 
 * Manages serial command input and execution asynchronously.
 * Provides non-blocking serial I/O, command queuing, and
 * thread-safe output.
 */
class CommandTask : public TaskBase {
private:
    SerialInputBuffer inputBuffer;
    CommandHistory commandHistory;
    bool promptShown;
    unsigned long lastActivity;
    unsigned long commandStartTime;
    bool commandInProgress;
    String currentCommandText;
    
public:
    /**
     * @brief Construct a new Command Task
     */
    CommandTask();
    
    /**
     * @brief Destroy the Command Task
     */
    virtual ~CommandTask();
    
protected:
    /**
     * @brief Setup function called once when task starts
     */
    void setup() override;
    
    /**
     * @brief Main task loop - handles serial input and command processing
     */
    void loop() override;
    
    /**
     * @brief Cleanup function called when task stops
     */
    void cleanup() override;
    
private:
    /**
     * @brief Process non-blocking serial input
     */
    void processSerialInput();
    
    /**
     * @brief Process commands from the command queue
     */
    void processCommandQueue();
    
    /**
     * @brief Execute a command string
     * @param command Command to execute
     */
    void executeCommand(const String& command);
    
    /**
     * @brief Show command prompt
     */
    void showPrompt();
    
    /**
     * @brief Show initial prompt after startup
     */
    void showInitialPrompt();
    
    /**
     * @brief Check for command timeout
     */
    void checkCommandTimeout();
    
    /**
     * @brief Parse and execute command (delegates to existing handlers)
     * @param command Command string to parse
     */
    void parseAndExecute(const String& command);
};

// ==========================================
// GLOBAL INSTANCE
// ==========================================
extern CommandTask* commandTask;

// ==========================================
// PUBLIC API
// ==========================================
/**
 * @brief Initialize and start the command task
 * @return True if initialization successful
 */
bool initializeCommandTask();

/**
 * @brief Stop and cleanup the command task
 */
void shutdownCommandTask();

/**
 * @brief Check if command task is running
 */
bool isCommandTaskRunning();

/**
 * @brief Thread-safe print to serial (uses mutex)
 * @param message Message to print
 */
void safePrintCommand(const String& message);

/**
 * @brief Thread-safe println to serial (uses mutex)
 * @param message Message to print with newline
 */
void safePrintlnCommand(const String& message);

/**
 * @brief Thread-safe printf to serial (uses mutex)
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void safePrintfCommand(const char* format, ...);

#endif // USE_RTOS
