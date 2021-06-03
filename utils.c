#include <stdio.h>
#include "utils.h"

void write_file(FILE *fptr, char *data, long pos, size_t size) {
    fseek(fptr, pos, SEEK_SET);
    fwrite(data, size, 1, fptr);
}

char *p64(int64_t n) {
    static char s[8];
    for (size_t i=0; i<8; ++i) {
        s[i] = n&0xff;
        n>>=8;
    }
    return s;
}

char *p32(int32_t n) {
    static char s[4];
    for (size_t i=0; i<4; ++i) {
        s[i] = n&0xff;
        n>>=8;
    }
    return s;
}
