#include <stdio.h>
#include <stdlib.h>

const char *message01 = "file reading error";
const char *message02 = "file saving error";
const char *message03 = "file saving ok";
const char *message04 = "error moving file pointer";

FILE *open_file(const char *filename, const char *mode) {
    FILE *handle = fopen(filename, mode);
    if (handle == NULL) {
        fprintf(stderr, "error opening file %s with mode %s\n", filename, mode);
    }
    return handle;
}

void close_file(FILE *handle) {
    if (handle == NULL) {
        fprintf(stderr, "error closing file, no handle\n");
        return;
    }
    fclose(handle);
}

int read_file(FILE *handle, void *buffer, size_t size) {
    size_t bytesRead;

    if (handle == NULL) {
        fprintf(stderr, "%s, no handle\n", message01);
        return -1;
    }
    bytesRead = fread(buffer, 1, size, handle);
    if (bytesRead < size) {
        //fprintf(stderr, "maybe %s?\n", message01);
        return -1;
    }
    return bytesRead;
}

int write_file(FILE *handle, const void *data, size_t size) {
    size_t bytesWritten;

    if (handle == NULL) {
        fprintf(stderr, "%s, no handle\n", message02);
        return -1;
    }
    bytesWritten = fwrite(data, 1, size, handle);
    if (bytesWritten < size) {
        //fprintf(stderr, "maybe %s?\n", message02);
        return -1;
    }
    return bytesWritten;
}

int move_file_pointer(FILE *handle, long offset, int whence) {
    if (handle == NULL) {
        fprintf(stderr, "%s, no handle\n", message04);
        return 0;
    }
    if (fseek(handle, offset, whence) != 0) {
        fprintf(stderr, "%s\n", message04);
        return 0;
    }
    return 1;
}

int load_file(const char *filename, void *buffer, size_t size) {
    FILE *handle = open_file(filename, "rb");
    if (handle == NULL) {
        fprintf(stderr, "%s, no handle: %s\n", message01, filename);
        return 0;
    }
    if (read_file(handle, buffer, size) < 0) {
        fprintf(stderr, "%s: %s\n", message01, filename);
        close_file(handle);
        return 0;
    }
    close_file(handle);

    fprintf(stderr, "%s: %s\n", "read ok", filename); // disable in non-debug
    return 1;
}

int save_file(const char *filename, const void *data, size_t size) {
    FILE *handle = open_file(filename, "rb+");
    if (handle == NULL) {
        handle = open_file(filename, "wb"); // Try to create file
        if (handle == NULL) {
            fprintf(stderr, "%s, no handle: %s\n", message02, filename);
            return 0;
        }
    }
    if (write_file(handle, data, size) < 0) {
        fprintf(stderr, "%s: %s\n", message02, filename);
        close_file(handle);
        return 0;
    }
    close_file(handle);

    fprintf(stderr, "%s: %s\n", message03, filename); // disable in non-debug
    return 1;
}

int exists_file(const char *filename) {
   FILE *handle = open_file(filename, "rb");
    if (handle == NULL) {
        fprintf(stderr, "%s: %s\n", "does not exist", filename);
        return 0;
    }
    close_file(handle);
    return 1;
}
