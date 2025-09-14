#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "torpedo.h"

#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} bmp_header_t;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
} bmp_info_t;
#pragma pack(pop)

torpedo_image_t* torpedo_load_bmp(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;
    
    bmp_header_t header;
    bmp_info_t info;
    
    if (fread(&header, sizeof(header), 1, file) != 1 ||
        fread(&info, sizeof(info), 1, file) != 1) {
        fclose(file);
        return NULL;
    }
    
    if (header.type != 0x4D42 || info.bits_per_pixel != 24) {
        fclose(file);
        return NULL;
    }
    
    torpedo_image_t* image = malloc(sizeof(torpedo_image_t));
    if (!image) {
        fclose(file);
        return NULL;
    }
    
    image->width = info.width;
    image->height = abs(info.height);
    image->header_size = header.offset;
    image->data_size = image->width * image->height * 3;
    
    image->data = malloc(image->data_size);
    if (!image->data) {
        free(image);
        fclose(file);
        return NULL;
    }
    
    fseek(file, header.offset, SEEK_SET);
    if (fread(image->data, 1, image->data_size, file) != image->data_size) {
        torpedo_free_image(image);
        fclose(file);
        return NULL;
    }
    
    fclose(file);
    return image;
}

torpedo_error_t torpedo_save_bmp(torpedo_image_t* image, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) return TORPEDO_ERROR_FILE;
    
    uint32_t row_size = ((image->width * 3 + 3) / 4) * 4;
    uint32_t image_size = row_size * image->height;
    
    bmp_header_t header = {
        .type = 0x4D42,
        .size = sizeof(bmp_header_t) + sizeof(bmp_info_t) + image_size,
        .reserved1 = 0,
        .reserved2 = 0,
        .offset = sizeof(bmp_header_t) + sizeof(bmp_info_t)
    };
    
    bmp_info_t info = {
        .size = sizeof(bmp_info_t),
        .width = image->width,
        .height = image->height,
        .planes = 1,
        .bits_per_pixel = 24,
        .compression = 0,
        .image_size = image_size,
        .x_pixels_per_meter = 2835,
        .y_pixels_per_meter = 2835,
        .colors_used = 0,
        .colors_important = 0
    };
    
    if (fwrite(&header, sizeof(header), 1, file) != 1 ||
        fwrite(&info, sizeof(info), 1, file) != 1) {
        fclose(file);
        return TORPEDO_ERROR_FILE;
    }
    
    for (uint32_t y = 0; y < image->height; y++) {
        uint8_t* row = image->data + y * image->width * 3;
        if (fwrite(row, 1, image->width * 3, file) != image->width * 3) {
            fclose(file);
            return TORPEDO_ERROR_FILE;
        }
        
        // Padding
        uint32_t padding = row_size - image->width * 3;
        for (uint32_t p = 0; p < padding; p++) {
            fputc(0, file);
        }
    }
    
    fclose(file);
    return TORPEDO_OK;
}

void torpedo_free_image(torpedo_image_t* image) {
    if (image) {
        free(image->data);
        free(image);
    }
}

size_t torpedo_get_capacity(torpedo_image_t* image) {
    return image->data_size / 8;
}