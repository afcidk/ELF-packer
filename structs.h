#ifndef STRUCTS_H
#define STRUCTS_H
#include <elf.h>

typedef struct {
    long pos;
    Elf64_Phdr *hdr;
} Phdr;

typedef struct {
    long pos;
    Elf64_Shdr *hdr;
} Shdr;

typedef struct {
    long size;
    Phdr *hdrs;
} Phdrs;

typedef struct {
    size_t size;
    Shdr *hdrs;
} Shdrs;

typedef struct {
    long pos;
    Elf64_Ehdr *hdr;
} Ehdr;

typedef struct {
    long pos;
    size_t size;
    char *data;
} Section_data;

#endif
