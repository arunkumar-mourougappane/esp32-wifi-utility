#pragma once
#include <Arduino.h>
#include "config.h"

// ==========================================
// COMMAND INTERFACE
// ==========================================
extern String currentInput;
extern bool promptShown;
extern unsigned long lastActivity;

void initializeSerial();
void handleSerialCommands();
void processCharacter(char c);
void executeCommand(String command);
void showPrompt();
void clearConsole();
void printHelp();
void printStatus();
