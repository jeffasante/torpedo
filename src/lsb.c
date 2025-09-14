#include <stdlib.h>
#include <string.h>
#include "torpedo.h"

static void encode_byte(uint8_t* pixels, uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        pixels[i] = (pixels[i] & 0xFE) | ((byte >> i) & 1);
    }
}

static uint8_t decode_byte(const uint8_t* pixels) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte |= (pixels[i] & 1) << i;
    }
    return byte;
}

torpedo_error_t torpedo_encode_lsb(torpedo_image_t* image, const uint8_t* data, size_t data_size) {
    if (data_size > torpedo_get_capacity(image) - 4) {
        return TORPEDO_ERROR_CAPACITY;
    }
    
    uint8_t* pixels = image->data;
    size_t pixel_idx = 0;
    
    // Encode size first (4 bytes)
    uint32_t size = (uint32_t)data_size;
    for (int i = 0; i < 4; i++) {
        encode_byte(&pixels[pixel_idx], (size >> (i * 8)) & 0xFF);
        pixel_idx += 8;
    }
    
    // Encode data
    for (size_t i = 0; i < data_size; i++) {
        encode_byte(&pixels[pixel_idx], data[i]);
        pixel_idx += 8;
    }
    
    return TORPEDO_OK;
}

torpedo_error_t torpedo_decode_lsb(torpedo_image_t* image, uint8_t** data_out, size_t* data_size_out) {
    uint8_t* pixels = image->data;
    size_t pixel_idx = 0;
    
    // Decode size first
    uint32_t size = 0;
    for (int i = 0; i < 4; i++) {
        size |= decode_byte(&pixels[pixel_idx]) << (i * 8);
        pixel_idx += 8;
    }
    
    if (size == 0 || size > torpedo_get_capacity(image)) {
        return TORPEDO_ERROR_CORRUPTED;
    }
    
    uint8_t* data = malloc(size);
    if (!data) return TORPEDO_ERROR_MEMORY;
    
    // Decode data
    for (uint32_t i = 0; i < size; i++) {
        data[i] = decode_byte(&pixels[pixel_idx]);
        pixel_idx += 8;
    }
    
    *data_out = data;
    *data_size_out = size;
    return TORPEDO_OK;
}