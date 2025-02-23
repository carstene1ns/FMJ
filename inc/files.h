
#ifndef FILES_H
#define FILES_H

#include <stdio.h>

FILE *open_file(const char *filename, const char *mode);
void close_file(FILE *handle);

int read_file(FILE *handle, void *buffer, size_t size);
int write_file(FILE *handle, const void *data, size_t size);

int move_file_pointer(FILE *handle, long offset, int whence);

int load_file(const char *filename, void *buffer, size_t size);
int save_file(const char *filename, const void *data, size_t size);

int exists_file(const char *filename);

#endif
