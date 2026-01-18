/**
 * @file command_interface.h
 * @brief Serial command interface declarations for ESP32 WiFi Utility
 * 
 * This header defines the command interface for interacting with the ESP32
 * WiFi Utility through serial communication. It provides function declarations
 * for command parsing, execution, and user interaction through the serial console.
 * 
 * The interface supports commands for WiFi scanning, network connection,
 * configuration management, network analysis, and system control.
 * 
 * @author Arunkumar Mourougappane
 * @version 1.0.0
 * @date 2026-01-17
 */

#pragma once
#include <Arduino.h>
#include "config.h"

// ==========================================
// COMMAND INTERFACE
// ==========================================

/**
 * @brief Current input buffer for serial command processing
 */
extern String currentInput;

/**
 * @brief Flag indicating if the command prompt has been shown
 */
extern bool promptShown;

/**
 * @brief Timestamp of last serial activity (milliseconds)
 */
extern unsigned long lastActivity;

/**
 * @brief Initialize serial communication interface
 * @details Sets up serial port at 115200 baud and displays startup banner
 */
void initializeSerial();

/**
 * @brief Display the initial command prompt on startup
 */
void showInitialPrompt();

/**
 * @brief Main handler for serial commands
 * @details Call this function in the main loop to process incoming serial data
 */
void handleSerialCommands();

/**
 * @brief Process a single character from serial input
 * @param c Character to process
 */
void processCharacter(char c);

/**
 * @brief Display the command prompt
 */
void showPrompt();

// Command execution

/**
 * @brief Execute a parsed command string
 * @param command Complete command string to execute
 */
void executeCommand(String command);

/**
 * @brief Execute system reset command
 * @details Restarts the ESP32 device
 */
void executeResetCommand();

/**
 * @brief Clear the serial console
 */
void clearConsole();

/**
 * @brief Print general help information
 * @details Displays available commands and usage instructions
 */
void printHelp();

/**
 * @brief Print current system status
 * @details Shows WiFi mode, connection state, and configuration
 */
void printStatus();

// Station configuration command handlers

/**
 * @brief Print Station mode help information
 * @details Displays station commands including security preferences
 */
void printStationHelp();

// Latency test command handlers

/**
 * @brief Execute latency testing commands
 * @param command Latency command string with parameters
 */
void executeLatencyCommand(String command);

/**
 * @brief Execute jitter analysis test
 * @details Performs statistical analysis of network latency variation
 */
void executeJitterAnalysis();

/**
 * @brief Print latency testing help information
 */
void printLatencyHelp();

// Channel analysis command handlers

/**
 * @brief Execute channel analysis commands
 * @param command Channel command string with parameters
 */
void executeChannelCommand(String command);

/**
 * @brief Execute channel congestion scan
 * @details Analyzes WiFi channel usage and interference
 */
void executeChannelCongestionScan();

/**
 * @brief Execute spectrum analysis
 * @details Provides detailed RF spectrum information
 */
void executeSpectrumAnalysis();

/**
 * @brief Print channel analysis help information
 */
void printChannelHelp();

// Signal monitoring command handlers

/**
 * @brief Execute signal monitoring commands
 * @param command Signal command string with parameters
 */
void executeSignalCommand(String command);

/**
 * @brief Print signal monitoring help information
 */
void printSignalHelp();

#ifdef USE_WEBSERVER
// Web server command handlers

/**
 * @brief Print web server help information
 * @details Displays web server commands and port information
 */
void printWebServerHelp();
#endif
