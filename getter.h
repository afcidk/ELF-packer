#ifndef GETTER_H
#define GETTER_H
#include <stdio.h>
#include <elf.h>
#include "structs.h"

void get_ehdr(FILE *fptr, Ehdr *e_hdr);
void get_phdrs(FILE *fptr, Ehdr *e_hdr, Phdrs *p_hdrs);
void get_shdrs(FILE *fptr, Ehdr *e_hdr, Shdrs *s_hdrs);
void get_strtab(FILE *fptr, Shdrs *s_hdrs, Section_data *strtab);
int get_text(FILE *fptr, Shdrs *s_hdrs, Section_data *text, Section_data *strtab);
//void get_bss(FILE *fptr, Shdrs *s_hdrs, Section_data *bss, Section_data *strtab);

#endif
