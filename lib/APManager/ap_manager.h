/**
 * @file ap_manager.h
 * @brief Access Point client management and deauthentication declarations
 * 
 * This header defines the interface for ESP32 Access Point client management
 * functionality. It provides declarations for client listing, connection
 * monitoring, and deauthentication operations when the ESP32 operates as
 * a WiFi Access Point.
 * 
 * The module integrates with ESP-IDF WiFi APIs for low-level client management
 * and provides high-level functions for user interaction and administration.
 * 
 * @author Arunkumar Mourougappane
 * @version 3.0.0
 * @date 2025-10-13
 */

#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include "config.h"

// ==========================================
// AP CLIENT MANAGEMENT
// ==========================================

/**
 * @brief Station list structure for connected client information
 * 
 * External reference to the global station list populated by ESP-IDF API.
 * Contains detailed information about clients connected to the Access Point.
 */
extern wifi_sta_list_t stationList;

/**
 * @brief Current number of connected clients
 * 
 * External reference to the global counter tracking connected clients.
 * Updated by updateClientList() and used throughout the module.
 */
extern uint8_t connectedClients;

// ==========================================
// FUNCTION DECLARATIONS
// ==========================================

/**
 * @brief Updates the internal client list with current connected clients
 * @see ap_manager.cpp for detailed implementation documentation
 */
void updateClientList();

/**
 * @brief Displays a formatted list of all connected Access Point clients  
 * @see ap_manager.cpp for detailed implementation documentation
 */
void listAPClients();

/**
 * @brief Deauthenticates a specific client from the Access Point
 * @param input Client ID number or MAC address for disconnection
 * @see ap_manager.cpp for detailed implementation documentation
 */
void deauthClient(String input);

/**
 * @brief Deauthenticates all connected clients from the Access Point
 * @see ap_manager.cpp for detailed implementation documentation  
 */
void deauthAllClients();

/**
 * @brief Displays comprehensive Access Point configuration and status
 * @see ap_manager.cpp for detailed implementation documentation
 */
void printAPInfo();
