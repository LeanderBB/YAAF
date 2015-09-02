/*
 * YAAF - Yet Another Archive Format
 * Copyright (C) 2014-2015, Leander Beernaert
 * BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You can contact the author at :
 * - YAAF source repository : http://www.github.com/LeanderBB/YAAF
 */
#ifndef __YAAF_HASHMAP_H__
#define __YAAF_HASHMAP_H__

#include "YAAF.h"


struct YAAF_HashMapEntry;
typedef struct YAAF_HashMapEntry YAAF_HashMapEntry;

/**
 * Implementation of an Open Addresing HashMap for YAAF.
 *
 * The hashmap only holds pointers, it does not allocate any data
 * besides the array in which the entries are stored.
 *
 * The capacity of the hashmap is maitained at 75% capacity to
 * increase the efficiency of linear probing method.
 */

typedef struct
{
    YAAF_HashMapEntry* pEntries;
    uint32_t count;
    uint32_t capacity;
} YAAF_HashMap;


void YAAF_HashMapInit(YAAF_HashMap*  pHashMap,
                      const uint32_t initialCount);

void YAAF_HashMapInitNoAlloc(YAAF_HashMap* pHashMap);

void YAAF_HashMapDestroy(YAAF_HashMap* pHashMap);

const void *YAAF_HashMapGet(const YAAF_HashMap *pHashMap,
                            const char*         key);

int YAAF_HashMapPut(YAAF_HashMap* pHashMap,
                    const char*   key,
                    const void*   pData);

int YAAF_HashMapPutWithHash(YAAF_HashMap*  pHashMap,
                            const uint32_t hash,
                            const char*    key,
                            const void*    pData);

int YAAF_HashMapRemove(YAAF_HashMap* pHashMap,
                       const char*   key);

const YAAF_HashMapEntry* YAAF_HashMapItBegin(const YAAF_HashMap* pHashMap);

void YAAF_HashMapItNext(const YAAF_HashMap*     pHashMap,
                        const YAAF_HashMapEntry **pIter);

const YAAF_HashMapEntry* YAAF_HashMapItEnd(const YAAF_HashMap* pHashMap);

const void* YAAF_HashMapItGet(const YAAF_HashMapEntry *pIter);


#endif
