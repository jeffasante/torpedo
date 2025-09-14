#include <stdio.h>
#include <stdlib.h>
#include "../src/torpedo.h"

int main() {
    // Load image
    torpedo_image_t* image = torpedo_load_bmp("input.bmp");
    if (!image) {
        fprintf(stderr, "Failed to load image\n");
        return 1;
    }
    
    // Check capacity
    size_t capacity = torpedo_get_capacity(image);
    printf("Image capacity: %zu bytes\n", capacity);
    
    // Encode message
    const char* message = "This is a secret message!";
    torpedo_error_t result = torpedo_encode(image, (uint8_t*)message, strlen(message), "mypassword");
    
    if (result == TORPEDO_OK) {
        // Save result
        result = torpedo_save_bmp(image, "output.bmp");
        if (result == TORPEDO_OK) {
            printf("Message encoded successfully!\n");
        } else {
            fprintf(stderr, "Failed to save image: %s\n", torpedo_error_string(result));
        }
    } else {
        fprintf(stderr, "Encoding failed: %s\n", torpedo_error_string(result));
    }
    
    torpedo_free_image(image);
    return (result == TORPEDO_OK) ? 0 : 1;
}