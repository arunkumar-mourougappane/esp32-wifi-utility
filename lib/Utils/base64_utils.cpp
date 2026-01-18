/**
 * @file base64_utils.cpp
 * @brief Base64 encoding/decoding implementation
 * 
 * Implements base64 encoding and decoding for password storage.
 * Uses ESP32's built-in mbedtls library for efficient encoding/decoding.
 * Prevents special characters in passwords from corrupting NVS storage.
 * 
 * @author Arunkumar Mourougappane
 * @version 4.2.0
 * @date 2026-01-17
 */

#include "base64_utils.h"
#include <mbedtls/base64.h>

String base64Encode(const String& input) {
    if (input.length() == 0) {
        return "";
    }
    
    // Calculate required buffer size
    size_t outputLen = 0;
    mbedtls_base64_encode(nullptr, 0, &outputLen, 
                         (const unsigned char*)input.c_str(), 
                         input.length());
    
    if (outputLen == 0) {
        return "";
    }
    
    // Allocate buffer and encode
    unsigned char* buffer = new unsigned char[outputLen + 1];
    if (buffer == nullptr) {
        return "";
    }
    
    int ret = mbedtls_base64_encode(buffer, outputLen, &outputLen,
                                    (const unsigned char*)input.c_str(),
                                    input.length());
    
    String result;
    if (ret == 0) {
        buffer[outputLen] = '\0';
        result = String((char*)buffer);
    }
    
    delete[] buffer;
    return result;
}

String base64Decode(const String& input) {
    if (input.length() == 0) {
        return "";
    }
    
    // Calculate required buffer size
    size_t outputLen = 0;
    mbedtls_base64_decode(nullptr, 0, &outputLen,
                         (const unsigned char*)input.c_str(),
                         input.length());
    
    if (outputLen == 0) {
        return "";
    }
    
    // Allocate buffer and decode
    unsigned char* buffer = new unsigned char[outputLen + 1];
    if (buffer == nullptr) {
        return "";
    }
    
    int ret = mbedtls_base64_decode(buffer, outputLen, &outputLen,
                                    (const unsigned char*)input.c_str(),
                                    input.length());
    
    String result;
    if (ret == 0) {
        buffer[outputLen] = '\0';
        result = String((char*)buffer);
    }
    
    delete[] buffer;
    return result;
}
