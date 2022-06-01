//
// Created by Xhy on 2021/8/23.
//

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cinttypes>
#include <unistd.h>
#include <sys/mman.h>
#include <android/log.h>
#include <elf.h>

#include "elfutil.h"
#include "mylog.h"

void getFullPath(const char *src, char *dest) {
    while (*src != '/') {
        *src++;
    }
    strncpy(dest, src, strlen(src) - 1);
}

uintptr_t getModuleBase(const char *module_name, char *moduleFullPath) {
    uintptr_t addr = 0;
    char buff[256] = "\n";

    FILE *fp = fopen("/proc/self/maps", "r");
    while (fgets(buff, sizeof(buff), fp)) {
        if (strstr(buff, "r-xp") && strstr(buff, module_name) &&
            sscanf(buff, "%" SCNxPTR, &addr) == 1) {
            getFullPath(buff, moduleFullPath);
            LOGE("[%s] moduleBase: %" SCNxPTR, moduleFullPath, addr);
            return addr;
        }
    }
//    LOGE("buff: %s", buff);
    LOGE("[%s] moduleBase not found!\n", module_name);
    fclose(fp);
    return 0;
}

// 解析Section
int getGOTOffsetAndSize(const char *modulePath, int &GOTSize) {
    int GOTOffset = 0;
    FILE *fp = fopen(modulePath, "r");
    if (!fp) {
        LOGE("[%s] open failed!", modulePath);
        return 0;
    }
    ELFW(Ehdr) elf_header;
    ELFW(Shdr) elf_section_header;
    memset(&elf_header, 0, sizeof(elf_header));
    memset(&elf_section_header, 0, sizeof(elf_section_header));
    // 解析elf_header
    fseek(fp, 0, SEEK_SET);
    fread(&elf_header, sizeof(elf_header), 1, fp);
//    LOGE("elf_header e_shoff: %" SCNxPTR ", e_shstrndx: %d", elf_header.e_shoff, elf_header.e_shstrndx);
//    LOGE("elf_header e_shnum: %d", elf_header.e_shnum);
    // 获取字符串表在section header中的偏移
    fseek(fp, elf_header.e_shoff + elf_header.e_shstrndx * elf_header.e_shentsize, SEEK_SET);
    fread(&elf_section_header, sizeof(elf_section_header), 1, fp);
    int string_table_size = elf_section_header.sh_size;
    char *string_table = (char *) (malloc(string_table_size));
    // 获取字符串表
    fseek(fp, elf_section_header.sh_offset, SEEK_SET);
    fread(string_table, string_table_size, 1, fp);
    // 遍历section header table, 查找.got
    fseek(fp, elf_header.e_shoff, SEEK_SET);
    for (int i = 0; i < elf_header.e_shnum; ++i) {
        fread(&elf_section_header, elf_header.e_shentsize, 1, fp);
        if (elf_section_header.sh_type == SHT_PROGBITS
            && 0 == strcmp(".got", string_table + elf_section_header.sh_name)) {
            GOTOffset = elf_section_header.sh_addr;
            GOTSize = elf_section_header.sh_size;
            break;
        }
    }
    free(string_table);
    fclose(fp);
    return GOTOffset;
}

static uint32_t elf_sysv_hash(const uint8_t *name) {
    uint32_t h = 0, g;

    while (*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

// 解析Segment
uintptr_t getSymAddrDynamic(const char *module_name, const char *symName) {
    char moduleFullPath[256] = {0};
    uintptr_t moduleBase = getModuleBase(module_name, moduleFullPath);
    if (moduleBase == 0) {
        return 0;
    }
    ELFW(Ehdr) elf_header;
    ELFW(Phdr) elf_program_header;
    memset(&elf_header, 0, sizeof(elf_header));
    memset(&elf_program_header, 0, sizeof(elf_program_header));
    // 解析elf_header
    memcpy(&elf_header, (const void *) moduleBase, sizeof(elf_header));
//    LOGE("elf_header e_phoff: %" SCNxPTR ", e_phentsize: %d", elf_header.e_phoff, elf_header.e_phentsize);
//    LOGE("elf_header e_phnum: %d", elf_header.e_phnum);
    // 遍历program header table, 查找.dynamic
    int DYNOffset = 0;
    int DYNSize = 0;
    for (int i = 0; i < elf_header.e_phnum; ++i) {
        memcpy(&elf_program_header,
               (const void *) (moduleBase + elf_header.e_phoff + i * elf_header.e_phentsize),
               elf_header.e_phentsize);
        if (elf_program_header.p_type == PT_DYNAMIC) {
            DYNOffset = elf_program_header.p_vaddr;
            DYNSize = elf_program_header.p_memsz;
            break;
        }
    }
    if (DYNOffset == 0 || DYNSize == 0) {
        LOGE(".dynamic not found!");
        return 0;
    }
    uintptr_t DYNBase = moduleBase + DYNOffset;
    LOGE("DYNOffset: %" SCNxPTR " DYNBase: %" SCNxPTR " DYNSize: %" SCNxPTR "", DYNOffset, DYNBase,
         DYNSize);

    // 保存各表
    ELFW(Sym) *dynsym;
    const char *dynstr;
    ELFW(Rel) *rel_dyn;
    ELFW(Rel) *rel_plt;
    int rel_dyn_cnt, rel_plt_cnt;
    //.hash
    const uint32_t *buckets;
    uint32_t buckets_cnt;
    const uint32_t *chains;
    uint32_t chains_cnt;
    //遍历.dynamic 解析需要的表 (.rel.plt .rel.dyn .dynsym .dynstr和.hash)
    ELFW(Dyn) dyn;
    int dyn_entsize = sizeof(ELFW(Dyn));
    for (int i = 0; i < DYNSize / dyn_entsize; ++i) {
        memcpy(&dyn, (const void *) (DYNBase + i * dyn_entsize), dyn_entsize);
        switch (dyn.d_tag) {
            // .rel.plt
            case DT_JMPREL:
                rel_plt = (ELFW(Rel) *) (moduleBase + dyn.d_un.d_ptr);
                break;
            case DT_PLTRELSZ:
                rel_plt_cnt = dyn.d_un.d_val / sizeof(ELFW(Rel));
                break;
                // .rel.dyn
            case DT_REL:
                rel_dyn = (ELFW(Rel) *) (moduleBase + dyn.d_un.d_ptr);
                break;
            case DT_RELSZ:
                rel_dyn_cnt = dyn.d_un.d_val / sizeof(ELFW(Rel));
                break;
                // .dynsym
            case DT_SYMTAB:
                dynsym = (ELFW(Sym) *) (moduleBase + dyn.d_un.d_ptr);
                break;
                // .dynstr
            case DT_STRTAB:
                dynstr = (const char *) (moduleBase + dyn.d_un.d_ptr);
                break;
                // .hash
            case DT_HASH:
                auto rawdata = (const uint32_t *) (moduleBase + dyn.d_un.d_ptr);
                buckets_cnt = rawdata[0];
                chains_cnt = rawdata[1];
                buckets = &(rawdata[2]);
                chains = &(buckets[buckets_cnt]);
                break;
        }
    }
    //查找符号 .hash -> .dynsym -> .dynstr O(x) + O(1) + O(1)
    ELFW(Sym) *target = nullptr;
    uint32_t hash = elf_sysv_hash((const uint8_t *) symName);
    for (uint32_t i = buckets[hash % buckets_cnt];
         0 != i; i = chains[i]) {
        ELFW(Sym) *sym = dynsym + i;
        unsigned char type = ELF_ST_TYPE(sym->st_info);
        if (STT_FUNC != type && STT_GNU_IFUNC != type && STT_NOTYPE != type)
            continue; // find function only, allow no-type
        if (0 == strcmp(dynstr + sym->st_name, symName)) {
            target = sym;
            break;
        }
    }
    if (!target) {
        LOGE("target sym not found!");
        return 0;
    }
    //遍历 .rel.plt 和 .rel.dyn，获取偏移，计算内存地址
    for (int i = 0; i < rel_plt_cnt; i++) {
        ELFW(Rel) &rel = rel_plt[i];
        if (&(dynsym[ELF_R_SYM(rel.r_info)]) == target &&
            ELF_R_TYPE(rel.r_info) == ELF_R_JUMP_SLOT) {
//            LOGE("target r_offset: %" SCNxPTR "", rel.r_offset);
            return moduleBase + rel.r_offset;
        }
    }
    for (int i = 0; i < rel_dyn_cnt; i++) {
        ELFW(Rel) &rel = rel_dyn[i];
        if (&(dynsym[ELF_R_SYM(rel.r_info)]) == target &&
            (ELF_R_TYPE(rel.r_info) == ELF_R_ABS
             || ELF_R_TYPE(rel.r_info) == ELF_R_GLOB_DAT)) {
//            LOGE("target r_offset: %" SCNxPTR "", rel.r_offset);
            return moduleBase + rel.r_offset;
        }
    }
    return 0;
}