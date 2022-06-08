//
// Created by Xhy on 2021/8/23.
//

#ifndef ANDROIDGOTHOOK_GOTUTIL_H
#define ANDROIDGOTHOOK_GOTUTIL_H

uintptr_t getGOTBase(int &GOTSize, const char *modulePath);

int getSymAddrInGOT(uintptr_t GOTBase, int GOTSize, uintptr_t ori, uintptr_t *addrArray);

void replaceFunction(uintptr_t addr, uintptr_t replace, uintptr_t ori);

uintptr_t hackGOT(const char *module_name, const char *target_lib, const char *target_func,
                        uintptr_t replace);

uintptr_t hackBySection(const char *module_name, const char *target_lib, const char *target_func,
                        uintptr_t replace);

uintptr_t hackBySegment(const char *module_name, const char *target_lib, const char *target_func,
                        uintptr_t replace);

#endif //ANDROIDGOTHOOK_GOTUTIL_H
