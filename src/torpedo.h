#ifndef TORPEDO_H
#define TORPEDO_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define TORPEDO_VERSION "0.1.0"

typedef enum {
    TORPEDO_OK = 0,
    TORPEDO_ERROR_MEMORY = -1,
    TORPEDO_ERROR_FILE = -2,
    TORPEDO_ERROR_FORMAT = -3,
    TORPEDO_ERROR_CAPACITY = -4,
    TORPEDO_ERROR_CORRUPTED = -5
} torpedo_error_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t* data;
    size_t data_size;
    size_t header_size;
} torpedo_image_t;

// Core functions
torpedo_image_t* torpedo_load_bmp(const char* filename);
torpedo_error_t torpedo_save_bmp(torpedo_image_t* image, const char* filename);
torpedo_image_t* torpedo_load_canvas(uint8_t* rgba_data, uint32_t width, uint32_t height);
uint8_t* torpedo_save_canvas(torpedo_image_t* image);
void torpedo_free_image(torpedo_image_t* image);
size_t torpedo_get_capacity(torpedo_image_t* image);

// Steganography functions
torpedo_error_t torpedo_encode(torpedo_image_t* image, const uint8_t* data, size_t data_size, const char* password);
torpedo_error_t torpedo_decode(torpedo_image_t* image, uint8_t** data_out, size_t* data_size_out, const char* password);

// Utility functions
const char* torpedo_error_string(torpedo_error_t error);

#endif