#include <stdlib.h>
#include <string.h>
#include "torpedo.h"

// External function declarations
torpedo_error_t torpedo_encode_lsb(torpedo_image_t* image, const uint8_t* data, size_t data_size);
torpedo_error_t torpedo_decode_lsb(torpedo_image_t* image, uint8_t** data_out, size_t* data_size_out);
torpedo_error_t torpedo_encrypt_data(uint8_t* data, size_t size, const char* password);
torpedo_error_t torpedo_decrypt_data(uint8_t* data, size_t size, const char* password);

torpedo_error_t torpedo_encode(torpedo_image_t* image, const uint8_t* data, size_t data_size, const char* password) {
    if (!image || !data || data_size == 0) {
        return TORPEDO_ERROR_MEMORY;
    }
    
    // Copy data for encryption
    uint8_t* encrypted_data = malloc(data_size);
    if (!encrypted_data) return TORPEDO_ERROR_MEMORY;
    
    memcpy(encrypted_data, data, data_size);
    
    // Encrypt if password provided
    if (password) {
        torpedo_encrypt_data(encrypted_data, data_size, password);
    }
    
    // Encode using LSB
    torpedo_error_t result = torpedo_encode_lsb(image, encrypted_data, data_size);
    
    free(encrypted_data);
    return result;
}

torpedo_error_t torpedo_decode(torpedo_image_t* image, uint8_t** data_out, size_t* data_size_out, const char* password) {
    if (!image || !data_out || !data_size_out) {
        return TORPEDO_ERROR_MEMORY;
    }
    
    // Decode using LSB
    uint8_t* decoded_data;
    size_t decoded_size;
    torpedo_error_t result = torpedo_decode_lsb(image, &decoded_data, &decoded_size);
    
    if (result != TORPEDO_OK) {
        return result;
    }
    
    // Decrypt if password provided
    if (password) {
        torpedo_decrypt_data(decoded_data, decoded_size, password);
    }
    
    *data_out = decoded_data;
    *data_size_out = decoded_size;
    return TORPEDO_OK;
}

const char* torpedo_error_string(torpedo_error_t error) {
    switch (error) {
        case TORPEDO_OK: return "Success";
        case TORPEDO_ERROR_MEMORY: return "Memory allocation failed";
        case TORPEDO_ERROR_FILE: return "File I/O error";
        case TORPEDO_ERROR_FORMAT: return "Invalid file format";
        case TORPEDO_ERROR_CAPACITY: return "Data too large for image";
        case TORPEDO_ERROR_CORRUPTED: return "Corrupted or no hidden data";
        default: return "Unknown error";
    }
}