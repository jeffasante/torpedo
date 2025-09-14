#include <stdio.h>
#include <stdlib.h>
#include "../src/torpedo.h"

int main() {
    // Load stego image
    torpedo_image_t* image = torpedo_load_bmp("stego.bmp");
    if (!image) {
        fprintf(stderr, "Failed to load image\n");
        return 1;
    }
    
    // Decode message
    uint8_t* decoded_data = NULL;
    size_t decoded_size = 0;
    
    torpedo_error_t result = torpedo_decode(image, &decoded_data, &decoded_size, "mypassword");
    
    if (result == TORPEDO_OK) {
        printf("Decoded message (%zu bytes): ", decoded_size);
        fwrite(decoded_data, 1, decoded_size, stdout);
        printf("\n");
        free(decoded_data);
    } else {
        fprintf(stderr, "Decoding failed: %s\n", torpedo_error_string(result));
    }
    
    torpedo_free_image(image);
    return (result == TORPEDO_OK) ? 0 : 1;
}