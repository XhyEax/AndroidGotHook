//
// Created by Xhy on 2021/8/23.
//

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/mman.h>
#include <android/log.h>

#include "elfutil.h"
#include <elf32.h>

#define LOG_TAG   "NInject"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void getGOTOffsetAndSize(const char *module_path, int &GOTOffset, int &GOTSize) {
    FILE *fp = fopen(module_path, "r");
    Elf32_Ehdr elf_header;
    Elf32_Shdr elf_section_header;
    memset(&elf_header, 0, sizeof(elf_header));
    memset(&elf_section_header, 0, sizeof(elf_section_header));
    // 解析elf_header
    fseek(fp, 0, SEEK_SET);
    fread(&elf_header, sizeof(elf_header), 1, fp);
//    LOGI("elf_header e_shoff: %X, e_shstrndx: %d", elf_header.e_shoff, elf_header.e_shstrndx);
//    LOGI("elf_header e_shnum: %d", elf_header.e_shnum);
    // 获取字符串表在section header中的偏移
    fseek(fp, elf_header.e_shoff + elf_header.e_shstrndx * elf_header.e_shentsize, SEEK_SET);
    fread(&elf_section_header, sizeof(elf_section_header), 1, fp);
    int string_table_size = elf_section_header.sh_size;
    char *string_table = (char *) (malloc(string_table_size));
    // 获取字符串表
    fseek(fp, elf_section_header.sh_offset, SEEK_SET);
    fread(string_table, string_table_size, 1, fp);
    // 遍历section header
    fseek(fp, elf_header.e_shoff, SEEK_SET);
    for (int i = 0; i < elf_header.e_shnum; ++i) {
        fread(&elf_section_header, elf_header.e_shentsize, 1, fp);
        if (elf_section_header.sh_type == SHT_PROGBITS
            && 0 == strcmp(".got", string_table + elf_section_header.sh_name)) {
            //返回GOT表偏移及大小
            GOTOffset = elf_section_header.sh_addr;
            GOTSize = elf_section_header.sh_size;
            break;
        }
    }
    free(string_table);
    fclose(fp);
}