/*
 * Copyright (C) 2021 ByteDance Inc
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MEMORF_CACHE_H
#define MEMORF_CACHE_H

#include "AllocPool.hpp"

#ifdef __arm__
#define ADDR_HASH_OFFSET 4
#else
#define ADDR_HASH_OFFSET 6
#endif

#define ALLOC_INDEX_SIZE 1 << 16
#define ALLOC_CACHE_SIZE 1 << 15

using namespace std;

struct AllocNode {
    uintptr_t addr;
    AllocNode *next;
};

class MemoryCache {
public:
    MemoryCache(bool allowRepeat = true);

    ~MemoryCache();

public:
    void init();

    bool insert(uintptr_t address);

    bool remove(uintptr_t address);

private:
    AllocNode *alloc_table[ALLOC_INDEX_SIZE];
    AllocPool<AllocNode> *alloc_cache;
    bool allowRepeat;
};

#endif //MEMORF_CACHE_H