#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "torpedo.h"

static void print_usage(const char* prog_name) {
    printf("Torpedo - Simple Steganography Tool v%s\n\n", TORPEDO_VERSION);
    printf("Usage: %s [OPTIONS] COMMAND\n\n", prog_name);
    printf("Commands:\n");
    printf("  encode -i INPUT.bmp -o OUTPUT.bmp -d DATA_FILE [-p PASSWORD]\n");
    printf("  decode -i INPUT.bmp -o OUTPUT_FILE [-p PASSWORD]\n");
    printf("  info   -i INPUT.bmp\n\n");
    printf("Options:\n");
    printf("  -i, --input FILE     Input BMP image\n");
    printf("  -o, --output FILE    Output file\n");
    printf("  -d, --data FILE      Data file to hide (for encode)\n");
    printf("  -m, --message TEXT   Text message to hide (for encode)\n");
    printf("  -p, --password PASS  Encryption password (optional)\n");
    printf("  -h, --help           Show this help\n\n");
    printf("Examples:\n");
    printf("  %s encode -i photo.bmp -o stego.bmp -m \"Hello World\"\n", prog_name);
    printf("  %s decode -i stego.bmp -o message.txt\n", prog_name);
    printf("  %s info -i photo.bmp\n", prog_name);
}

typedef enum {
    CMD_NONE,
    CMD_ENCODE,
    CMD_DECODE,
    CMD_INFO
} command_t;

int main(int argc, char* argv[]) {
    command_t command = CMD_NONE;
    char* input_file = NULL;
    char* output_file = NULL;
    char* data_file = NULL;
    char* message = NULL;
    char* password = NULL;
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "encode") == 0) {
        command = CMD_ENCODE;
    } else if (strcmp(argv[1], "decode") == 0) {
        command = CMD_DECODE;
    } else if (strcmp(argv[1], "info") == 0) {
        command = CMD_INFO;
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return 1;
    }
    
    static struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"data", required_argument, 0, 'd'},
        {"message", required_argument, 0, 'm'},
        {"password", required_argument, 0, 'p'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc - 1, argv + 1, "i:o:d:m:p:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'i': input_file = optarg; break;
            case 'o': output_file = optarg; break;
            case 'd': data_file = optarg; break;
            case 'm': message = optarg; break;
            case 'p': password = optarg; break;
            case 'h': print_usage(argv[0]); return 0;
            default: fprintf(stderr, "Use -h for help\n"); return 1;
        }
    }
    
    if (!input_file) {
        fprintf(stderr, "Error: Input file (-i) is required\n");
        return 1;
    }
    
    switch (command) {
        case CMD_INFO: {
            torpedo_image_t* image = torpedo_load_bmp(input_file);
            if (!image) {
                fprintf(stderr, "Error: Failed to load BMP file '%s'\n", input_file);
                return 1;
            }
            
            printf("File: %s\n", input_file);
            printf("Dimensions: %ux%u\n", image->width, image->height);
            printf("Data size: %zu bytes\n", image->data_size);
            printf("Max capacity: %zu bytes\n", torpedo_get_capacity(image));
            
            torpedo_free_image(image);
            break;
        }
        
        case CMD_ENCODE: {
            if (!output_file) {
                fprintf(stderr, "Error: Output file (-o) is required for encode\n");
                return 1;
            }
            
            if (!data_file && !message) {
                fprintf(stderr, "Error: Either data file (-d) or message (-m) is required\n");
                return 1;
            }
            
            torpedo_image_t* image = torpedo_load_bmp(input_file);
            if (!image) {
                fprintf(stderr, "Error: Failed to load BMP file '%s'\n", input_file);
                return 1;
            }
            
            uint8_t* data = NULL;
            size_t data_size = 0;
            
            if (message) {
                data = (uint8_t*)message;
                data_size = strlen(message);
            } else {
                FILE* f = fopen(data_file, "rb");
                if (!f) {
                    fprintf(stderr, "Error: Failed to open data file '%s'\n", data_file);
                    torpedo_free_image(image);
                    return 1;
                }
                
                fseek(f, 0, SEEK_END);
                data_size = ftell(f);
                fseek(f, 0, SEEK_SET);
                
                data = malloc(data_size);
                if (!data || fread(data, 1, data_size, f) != data_size) {
                    fprintf(stderr, "Error: Failed to read data file\n");
                    if (data) free(data);
                    fclose(f);
                    torpedo_free_image(image);
                    return 1;
                }
                fclose(f);
            }
            
            torpedo_error_t result = torpedo_encode(image, data, data_size, password);
            
            if (result == TORPEDO_OK) {
                result = torpedo_save_bmp(image, output_file);
                if (result == TORPEDO_OK) {
                    printf("Success: Encoded %zu bytes into '%s'\n", data_size, output_file);
                } else {
                    fprintf(stderr, "Error: Failed to save output file: %s\n", 
                           torpedo_error_string(result));
                }
            } else {
                fprintf(stderr, "Error: Encoding failed: %s\n", torpedo_error_string(result));
            }
            
            if (data != (uint8_t*)message) free(data);
            torpedo_free_image(image);
            return (result == TORPEDO_OK) ? 0 : 1;
        }
        
        case CMD_DECODE: {
            torpedo_image_t* image = torpedo_load_bmp(input_file);
            if (!image) {
                fprintf(stderr, "Error: Failed to load BMP file '%s'\n", input_file);
                return 1;
            }
            
            uint8_t* decoded_data = NULL;
            size_t decoded_size = 0;
            
            torpedo_error_t result = torpedo_decode(image, &decoded_data, &decoded_size, password);
            
            if (result == TORPEDO_OK) {
                if (output_file) {
                    FILE* f = fopen(output_file, "wb");
                    if (f) {
                        fwrite(decoded_data, 1, decoded_size, f);
                        fclose(f);
                        printf("Success: Decoded %zu bytes to '%s'\n", decoded_size, output_file);
                    } else {
                        fprintf(stderr, "Error: Failed to create output file '%s'\n", output_file);
                        result = TORPEDO_ERROR_FILE;
                    }
                } else {
                    fwrite(decoded_data, 1, decoded_size, stdout);
                }
                
                free(decoded_data);
            } else {
                fprintf(stderr, "Error: Decoding failed: %s\n", torpedo_error_string(result));
            }
            
            torpedo_free_image(image);
            return (result == TORPEDO_OK) ? 0 : 1;
        }
        
        default:
            fprintf(stderr, "Internal error: Unknown command\n");
            return 1;
    }
    
    return 0;
}