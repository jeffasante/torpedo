#include <emscripten.h>
#include <stdlib.h>
#include <string.h>
#include "torpedo.h"

EMSCRIPTEN_KEEPALIVE
int torpedo_encode_js(uint8_t* rgba_data, int width, int height, 
                      const char* message, const char* password,
                      uint8_t* output_rgba) {
    torpedo_image_t* image = torpedo_load_canvas(rgba_data, width, height);
    if (!image) return TORPEDO_ERROR_MEMORY;
    
    torpedo_error_t result = torpedo_encode(image, (uint8_t*)message, strlen(message), password);
    if (result == TORPEDO_OK) {
        uint8_t* encoded_rgba = torpedo_save_canvas(image);
        if (encoded_rgba) {
            memcpy(output_rgba, encoded_rgba, width * height * 4);
            free(encoded_rgba);
        } else {
            result = TORPEDO_ERROR_MEMORY;
        }
    }
    
    torpedo_free_image(image);
    return result;
}

EMSCRIPTEN_KEEPALIVE
int torpedo_decode_js(uint8_t* rgba_data, int width, int height,
                      const char* password, char* output_message, int max_size) {
    torpedo_image_t* image = torpedo_load_canvas(rgba_data, width, height);
    if (!image) return TORPEDO_ERROR_MEMORY;
    
    uint8_t* decoded_data = NULL;
    size_t decoded_size = 0;
    
    torpedo_error_t result = torpedo_decode(image, &decoded_data, &decoded_size, password);
    if (result == TORPEDO_OK && decoded_size < max_size) {
        memcpy(output_message, decoded_data, decoded_size);
        output_message[decoded_size] = '\0';
        free(decoded_data);
    }
    
    torpedo_free_image(image);
    return result;
}

EMSCRIPTEN_KEEPALIVE
int torpedo_get_capacity_js(int width, int height) {
    return (width * height * 3) / 8;
}