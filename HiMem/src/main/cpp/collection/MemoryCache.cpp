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

#include "MemoryCache.h"
#include "../log.h"

MemoryCache::MemoryCache(bool allowRepeat) {
    alloc_cache = new AllocPool<AllocNode>(ALLOC_CACHE_SIZE);
    this->allowRepeat = allowRepeat;
    init();
}

MemoryCache::~MemoryCache() {
    delete alloc_cache;
}

void MemoryCache::init() {
    alloc_cache->reset();
    for (uint i = 0; i < ALLOC_INDEX_SIZE; i++) {
        alloc_table[i] = nullptr;
    }
}

bool MemoryCache::insert(uintptr_t address) {
    uint16_t alloc_hash = (address >> ADDR_HASH_OFFSET) & 0xFFFF;
    AllocNode *head = alloc_table[alloc_hash];
    if (allowRepeat || head == nullptr) {
        AllocNode *p = alloc_cache->apply();
        if (p == nullptr) return false;
        p->addr = address;
        p->next = head;
        alloc_table[alloc_hash] = p;
        return true;
    } else {
        AllocNode *node = head;
        while (node != nullptr && node->addr != address) node = node->next;
        if (node == nullptr) {
            AllocNode *p = alloc_cache->apply();
            if (p == nullptr) return false;
            p->addr = address;
            // 没有重复，insert 到链表头部
            p->next = head;
            alloc_table[alloc_hash] = p;
            return true;
        } else
            // 有重复，不允许 insert
            return false;
    }
}

inline AllocNode *remove_alloc(AllocNode **header, uintptr_t address) {
    AllocNode *hptr = *header;
    if (hptr == nullptr) {
        return nullptr;
    } else if (hptr->addr == address) {
        AllocNode *p = hptr;
        *header = p->next;
        return p;
    } else {
        AllocNode *p = hptr;
        while (p->next != nullptr && p->next->addr != address) p = p->next;
        AllocNode *t = p->next;
        if (t != nullptr) {
            p->next = t->next;
        }
        return t;
    }
}

bool MemoryCache::remove(uintptr_t address) {
    uint16_t alloc_hash = (address >> ADDR_HASH_OFFSET) & 0xFFFF;
    if (alloc_table[alloc_hash] == nullptr) return false;
    AllocNode *p = remove_alloc(&alloc_table[alloc_hash], address);
    bool founded = p != nullptr;
    if (founded)
        alloc_cache->recycle(p);
    return founded;
}