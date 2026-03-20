#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    char name[64];
    uint32_t offset;
    uint32_t length;
} initrd_file_header_t;

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <output file> <input files...>\n", argv[0]);
        return 1;
    }

    FILE *out = fopen(argv[1], "wb");
    if (!out) {
        perror("Failed to open output file");
        return 1;
    }

    uint32_t num_files = argc - 2;
    fwrite(&num_files, sizeof(uint32_t), 1, out);

    initrd_file_header_t *headers = calloc(num_files, sizeof(initrd_file_header_t));
    uint32_t current_offset = sizeof(uint32_t) + (sizeof(initrd_file_header_t) * num_files);
    
    // We will read all file contents into memory, since this is a simple tool
    unsigned char **file_data = malloc(num_files * sizeof(unsigned char *));

    for (uint32_t i = 0; i < num_files; i++) {
        const char *in_name = argv[i + 2];
        FILE *in = fopen(in_name, "rb");
        if (!in) {
            printf("Error: Could not open input file %s\n", in_name);
            return 1;
        }

        fseek(in, 0, SEEK_END);
        headers[i].length = ftell(in);
        fseek(in, 0, SEEK_SET);

        file_data[i] = malloc(headers[i].length);
        fread(file_data[i], 1, headers[i].length, in);
        fclose(in);

        // Extract just the filename without path
        const char *base_name = strrchr(in_name, '/');
        if (base_name) base_name++; // skip '/'
        else base_name = in_name;

        strncpy(headers[i].name, base_name, 63);
        headers[i].offset = current_offset;
        current_offset += headers[i].length;
    }

    // Write headers
    fwrite(headers, sizeof(initrd_file_header_t), num_files, out);

    // Write file data
    for (uint32_t i = 0; i < num_files; i++) {
        fwrite(file_data[i], 1, headers[i].length, out);
        free(file_data[i]);
    }

    free(headers);
    free(file_data);
    fclose(out);

    printf("Created initrd %s with %d files.\n", argv[1], num_files);
    return 0;
}
