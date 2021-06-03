#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "getter.h"
#include "structs.h"

void get_ehdr(FILE *fptr, Ehdr *hdr) {
    fseek(fptr, 0, SEEK_SET); 
    hdr->pos = ftell(fptr);
    hdr->hdr = calloc(1, sizeof(Elf64_Ehdr));
    fread(hdr->hdr, 1, sizeof(Elf64_Ehdr), fptr);
}

void get_shdrs(FILE *fptr, Ehdr *e_hdr, Shdrs *s_hdrs) {
    // Header
    fseek(fptr, e_hdr->hdr->e_shoff, SEEK_SET);
    for (size_t i=0; i<s_hdrs->size; ++i) {
        s_hdrs->hdrs[i].hdr = calloc(1, sizeof(Elf64_Shdr));
        s_hdrs->hdrs[i].pos = ftell(fptr);
        fread(s_hdrs->hdrs[i].hdr, 1, sizeof(Elf64_Shdr), fptr);
    }
}

void get_phdrs(FILE *fptr, Ehdr *e_hdr, Phdrs *p_hdrs) {
    // Header
    fseek(fptr, e_hdr->hdr->e_phoff, SEEK_SET);
    for (size_t i=0; i<p_hdrs->size; ++i) {
        p_hdrs->hdrs[i].hdr = calloc(1, sizeof(Elf64_Phdr));
        p_hdrs->hdrs[i].pos = ftell(fptr);
        fread(p_hdrs->hdrs[i].hdr, 1, sizeof(Elf64_Phdr), fptr);
    }
}

void get_strtab(FILE *fptr, Shdrs *s_hdrs, Section_data *strtab) {
    for (size_t i = 0; i<s_hdrs->size; ++i) {
        Elf64_Shdr *hdr = s_hdrs->hdrs[i].hdr;
        if (hdr->sh_type != SHT_STRTAB) continue;

        fseek(fptr, hdr->sh_offset, SEEK_SET);
        strtab->data = calloc(hdr->sh_size, sizeof(char));
        strtab->size = hdr->sh_size;
        strtab->pos = ftell(fptr);
        fread(strtab->data, 1, hdr->sh_size, fptr);
    }
}

char *get_section_name(char *base, int offset) {
    return (char *)(base + offset);
}

int get_text(FILE *fptr, Shdrs *s_hdrs, Section_data *text, Section_data *strtab) {
    for (size_t i=0; i<s_hdrs->size; ++i) {
        Elf64_Shdr *hdr = s_hdrs->hdrs[i].hdr;
        if (strncmp(get_section_name(strtab->data, hdr->sh_name), ".text", 5)) continue;

        fseek(fptr, hdr->sh_offset, SEEK_SET);
        text->data = calloc(hdr->sh_size, sizeof(char));
        text->size = hdr->sh_size;
        text->pos = ftell(fptr);
        fread(text->data, 1, hdr->sh_size, fptr);
        return i;
    }
    return -1;
}

