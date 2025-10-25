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
void showInitialPrompt();
void handleSerialCommands();
void processCharacter(char c);
void showPrompt();

// Command execution
void executeCommand(String command);
void executeResetCommand();
void clearConsole();
void printHelp();
void printStatus();

// Latency test command handlers
void executeLatencyCommand(String command);
void executeJitterAnalysis();
void printLatencyHelp();

// Channel analysis command handlers
void executeChannelCommand(String command);
void executeChannelCongestionScan();
void executeSpectrumAnalysis();
void printChannelHelp();

#ifdef USE_WEBSERVER
// Web server command handlers
void printWebServerHelp();
#endif
