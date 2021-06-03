#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

#define member_size(type, member) sizeof(((type *)0)->member)
#define write_file_struct(ptr, data, base, type, member) \
        write_file(ptr, data, base+offsetof(type, member), member_size(type, member))

void write_file(FILE *fptr, char *data, long pos, size_t size);
char *p64(int64_t n);
char *p32(int32_t n);

#endif
