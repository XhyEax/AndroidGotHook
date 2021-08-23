//
// Created by Xhy on 2021/8/23.
//

#ifndef ANDROIDGOTHOOK_GOTUTIL_H
#define ANDROIDGOTHOOK_GOTUTIL_H

#include <cstdint>

void getGOTBase(uintptr_t &GOTBase, int &GOTSize, const char *module_path);

void replaceGOT(uintptr_t GOTBase, int GOTSize, uintptr_t replace, uintptr_t ori);

#endif //ANDROIDGOTHOOK_GOTUTIL_H
