//
// Created by Xhy on 2021/8/23.
//

#ifndef ANDROIDGOTHOOK_GOTUTIL_H
#define ANDROIDGOTHOOK_GOTUTIL_H

#include <cstdint>

uintptr_t getGOTBase(int &GOTSize, const char *modulePath);

uintptr_t getSymAddrInGOT(uintptr_t GOTBase, int GOTSize, uintptr_t ori);

void replaceFunction(uintptr_t addr, uintptr_t replace, uintptr_t ori);


#endif //ANDROIDGOTHOOK_GOTUTIL_H
