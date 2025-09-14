#include <stdlib.h>
#include <string.h>
#include "torpedo.h"

// Generic image structure for web canvas data
typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t* rgba_data;  // RGBA format from canvas
    size_t data_size;
} canvas_image_t;

// Convert RGBA to RGB for steganography
static uint8_t* rgba_to_rgb(const uint8_t* rgba_data, size_t width, size_t height) {
    size_t rgb_size = width * height * 3;
    uint8_t* rgb_data = malloc(rgb_size);
    if (!rgb_data) return NULL;
    
    for (size_t i = 0; i < width * height; i++) {
        rgb_data[i * 3] = rgba_data[i * 4];     // R
        rgb_data[i * 3 + 1] = rgba_data[i * 4 + 1]; // G
        rgb_data[i * 3 + 2] = rgba_data[i * 4 + 2]; // B
        // Skip alpha channel
    }
    
    return rgb_data;
}

// Convert RGB back to RGBA
static uint8_t* rgb_to_rgba(const uint8_t* rgb_data, size_t width, size_t height) {
    size_t rgba_size = width * height * 4;
    uint8_t* rgba_data = malloc(rgba_size);
    if (!rgba_data) return NULL;
    
    for (size_t i = 0; i < width * height; i++) {
        rgba_data[i * 4] = rgb_data[i * 3];     // R
        rgba_data[i * 4 + 1] = rgb_data[i * 3 + 1]; // G
        rgba_data[i * 4 + 2] = rgb_data[i * 3 + 2]; // B
        rgba_data[i * 4 + 3] = 255;             // A (opaque)
    }
    
    return rgba_data;
}

torpedo_image_t* torpedo_load_canvas(uint8_t* rgba_data, uint32_t width, uint32_t height) {
    torpedo_image_t* image = malloc(sizeof(torpedo_image_t));
    if (!image) return NULL;
    
    image->width = width;
    image->height = height;
    image->data_size = width * height * 3;
    image->header_size = 0;
    
    image->data = rgba_to_rgb(rgba_data, width, height);
    if (!image->data) {
        free(image);
        return NULL;
    }
    
    return image;
}

uint8_t* torpedo_save_canvas(torpedo_image_t* image) {
    return rgb_to_rgba(image->data, image->width, image->height);
}