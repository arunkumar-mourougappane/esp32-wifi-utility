/**
 * @file base64_utils.h
 * @brief Base64 encoding/decoding utilities for password storage
 * 
 * Provides base64 encoding and decoding for secure password storage in NVS.
 * Prevents special characters in passwords from corrupting NVS storage.
 * 
 * @author Arunkumar Mourougappane
 * @version 4.2.0
 * @date 2026-01-17
 */

#ifndef BASE64_UTILS_H
#define BASE64_UTILS_H

#include <Arduino.h>

/**
 * @brief Encode a string to base64
 * @param input Input string to encode
 * @return Base64 encoded string
 */
String base64Encode(const String& input);

/**
 * @brief Decode a base64 string
 * @param input Base64 encoded string
 * @return Decoded string
 */
String base64Decode(const String& input);

#endif // BASE64_UTILS_H
