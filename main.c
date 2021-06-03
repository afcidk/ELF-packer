#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <elf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include "getter.h"
#include "utils.h"

extern void unpack_asm(void);
extern int payload_size;
extern int data_size;

void pack(char *data, size_t size) {
    unsigned char rotate = 0;
    for (size_t i=0; i<size; ++i){
        data[i] += rotate;
        ++rotate;
    }
}

void unpack(char *data, size_t size) {
    unsigned char rotate = 0;
    for (size_t i=0; i<size; ++i) {
        data[i] -= rotate;
        ++rotate;
    }
}

int32_t find_inject_offset(FILE *fptr, int should_unpack) {
    size_t filesz;
    char buf;
    fseek(fptr, 0, SEEK_END);
    filesz = ftell(fptr);

    if (!should_unpack) {
        size_t inject_size = 0;

        for (size_t i=0; i<filesz; ++i) {
            fseek(fptr, i, SEEK_SET);
            fread(&buf, 1, 1, fptr);
            if (buf == 0) {
                ++inject_size;
                if (inject_size == payload_size) {
                    return i;
                }
            }
            else inject_size = 0;
        }

        assert("Cannot find space to inject" && 0);
        return 0;
    }
    else {
        const char magic[] = {0xde, 0xad, 0xbe, 0xef};
        size_t magic_idx = 0;
        for (size_t i=0; i<filesz; ++i) {
            fseek(fptr, i, SEEK_SET);
            fread(&buf, 1, 1, fptr);
            if (buf == magic[magic_idx]) {
                ++magic_idx;
                if (magic_idx == 4) {
                    uint32_t ret = 0;
                    fread(&ret, 4, 1, fptr);
                    printf("ret: %x\n", ret);
                    return ret;
                }
            }
            else magic_idx = 0;
        }
        assert("Cannot find magic number" && magic_idx == 4);
        return 0;
    }
}

int main(int argc, char **argv) {
    int should_unpack = 0;
    if (argc < 2) {
        printf("usage: packer <binary> [-d]\n");
        exit(1);
    }
    else if (argc >= 3) {
        should_unpack = 1;
    }

    char out_filename[32], old_filename[32];
    snprintf(out_filename, 32, "%s.new", argv[1]);
    snprintf(old_filename, 32, "%s.old", argv[1]);

    FILE *fptr = fopen(argv[1], "rb");
    FILE *fptr_out = fopen(out_filename, "wb");
    char buf[1024];
    int bytes;
    while ((bytes=fread(buf, sizeof(char), 1024, fptr))) {
        fwrite(buf, sizeof(char), 1024, fptr_out);
    }

    Ehdr e_hdr;
    Phdrs p_hdrs;
    Shdrs s_hdrs;
    Section_data strtab;
    Section_data text;

    // File Header
    get_ehdr(fptr, &e_hdr);

    // Program Header
    p_hdrs.size = e_hdr.hdr->e_phnum;
    p_hdrs.hdrs = calloc(p_hdrs.size, sizeof(Phdr));
    get_phdrs(fptr, &e_hdr, &p_hdrs);

    // Section Header
    s_hdrs.size = e_hdr.hdr->e_shnum;
    s_hdrs.hdrs = calloc(s_hdrs.size, sizeof(Shdr));
    get_shdrs(fptr, &e_hdr, &s_hdrs);
    get_strtab(fptr, &s_hdrs, &strtab);
    get_text(fptr, &s_hdrs, &text, &strtab);

    // Make LOAD executable
    for (size_t i=0; i<p_hdrs.size; ++i) {
        if (p_hdrs.hdrs[i].hdr->p_type == PT_LOAD) {
            write_file(fptr_out, p32(PF_R|PF_W|PF_X), p_hdrs.hdrs[i].pos + offsetof(Elf64_Phdr, p_flags), member_size(Elf64_Phdr, p_flags));
            break;
        }
    }

    int32_t entry = (int32_t)e_hdr.hdr->e_entry;
    int64_t base = 0x400000 & entry;
    int64_t text_start = base + text.pos;
    int64_t pack_size = text.size;
    int64_t inject_offset = find_inject_offset(fptr, should_unpack);
    int64_t new_ep = base + inject_offset;
    int32_t ep_fileoff = entry - base;

    printf("text_start: 0x%lx\n", text_start);
    printf("pack_size: 0x%lx\n", pack_size);
    printf("payload_size: 0x%x\n", payload_size);
    printf("New entry point: 0x%lx\n", new_ep);

    /* Encode .text section */
    if (!should_unpack) pack(text.data, text.size);
    else unpack(text.data, text.size);
    write_file(fptr_out, text.data, text.pos, text.size);

    /* Set entry point */
    write_file_struct(fptr_out, p64(new_ep), e_hdr.pos, Elf64_Ehdr, e_entry);

    /* Prepare payload */
    if (!should_unpack) {
        char payload[payload_size];
        memcpy(payload, unpack_asm, payload_size);
        memcpy(payload + payload_size - data_size, &text_start, sizeof(text_start));
        memcpy(payload + payload_size - data_size + 8, &pack_size, sizeof(pack_size));
        memcpy(payload + payload_size - data_size + 20, &ep_fileoff, sizeof(int32_t));
        write_file(fptr_out, payload, inject_offset, payload_size);
    }
    else {
        char payload[payload_size];
        memset(payload, 0, sizeof(payload));
        write_file(fptr_out, payload, ep_fileoff, payload_size);
    }

    fclose(fptr_out);
    fclose(fptr);

    /* Copy permission */
    struct stat stat_buf;
    stat(argv[1], &stat_buf);
    int perm = stat_buf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    chmod(out_filename, perm);

    /* Rename */
    rename(argv[1], old_filename);
    rename(out_filename, argv[1]);

    return 0;
}
