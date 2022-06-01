//
// Created by Xhy on 2021/8/23.
//

#ifndef ANDROIDGOTHOOK_ELFUTIL_H
#define ANDROIDGOTHOOK_ELFUTIL_H

#if defined(__LP64__)
#define ELFW(what) Elf64_ ## what
#define ELF_R_TYPE(what) ELF64_R_TYPE(what)
#define ELF_R_SYM(what) ELF64_R_SYM(what)
#else
#define ELFW(what) Elf32_ ## what
#define ELF_R_TYPE(what) ELF32_R_TYPE(what)
#define ELF_R_SYM(what) ELF32_R_SYM(what)
#endif

#if defined(__arm__)
#define ELF_R_JUMP_SLOT R_ARM_JUMP_SLOT     //.rel.plt
#define ELF_R_GLOB_DAT  R_ARM_GLOB_DAT      //.rel.dyn
#define ELF_R_ABS       R_ARM_ABS32         //.rel.dyn
#elif defined(__aarch64__)
#define ELF_R_JUMP_SLOT R_AARCH64_JUMP_SLOT
#define ELF_R_GLOB_DAT  R_AARCH64_GLOB_DAT
#define ELF_R_ABS       R_AARCH64_ABS64
#endif

uintptr_t getModuleBase(const char *modulePath, char *moduleFullPath);

int getGOTOffsetAndSize(const char *modulePath, int &GOTSize);

uintptr_t getSymAddrDynamic(const char *moduleBase, const char *symName);

#endif //ANDROIDGOTHOOK_ELFUTIL_H
