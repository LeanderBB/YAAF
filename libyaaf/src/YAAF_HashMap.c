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
#include "YAAF_HashMap.h"
#include "YAAF_Internal.h"
#include "YAAF_Hash.h"

/* us the address of the pointer of the hashmap structure as an empty value*/
#define YAAF_HASHMAP_ENTRY_DELETED(hashmap) ((void*)(hashmap))

struct YAAF_HashMapEntry
{
    const void* pData;
    uint32_t hash;
};

void
YAAF_HashMapInit(YAAF_HashMap* pHashMap,
                 const uint32_t initialCount)
{

    pHashMap->count = 0;
    /* Use 0.75 load factor */
    pHashMap->capacity = initialCount * 4/3;
    pHashMap->pEntries = (YAAF_HashMapEntry*)YAAF_calloc(pHashMap->capacity, sizeof(YAAF_HashMapEntry));
}

void
YAAF_HashMapInitNoAlloc(YAAF_HashMap* pHashMap)
{
    pHashMap->count = 0;
    pHashMap->capacity = 0;
    pHashMap->pEntries = NULL;
}

void
YAAF_HashMapDestroy(YAAF_HashMap* pHashMap)
{
    if (pHashMap->pEntries)
    {
        YAAF_free(pHashMap->pEntries);
        pHashMap->pEntries = NULL;
    }
    pHashMap->capacity = 0;
    pHashMap->count = 0;
}

static uint32_t
YAAF_HashMapCalculateIdx(const uint32_t hash,
                         const uint32_t n,
                         const uint32_t size)
{
    /* use linear probing to find the best  */
    return (hash + n) % size;
}

static YAAF_HashMapEntry*
YAAF_HashMapFindEntry(const YAAF_HashMap* pHashMap,
                      const uint32_t hash)
{
    uint32_t hk = 0;
    uint32_t i = 0;
    YAAF_HashMapEntry* p_result = NULL;

    for(i = 0; i < pHashMap->capacity; ++i)
    {
        hk =  YAAF_HashMapCalculateIdx(hash, i, pHashMap->capacity);
        YAAF_HashMapEntry* p_cur_entry = &pHashMap->pEntries[hk];

        if (!p_cur_entry->pData)
        {
            /* No key found when data is NULL. exit loop */
            break;
        }

        if (p_cur_entry->pData == YAAF_HASHMAP_ENTRY_DELETED(pHashMap))
        {
            /* Element was deleted, find next element */
            continue;
        }

        if (p_cur_entry->pData)
        {
            /* check if the hashes match */
            if (pHashMap->pEntries[hk].hash == hash)
            {
                p_result = &pHashMap->pEntries[hk];
                break;
            }
        }
        /* continue loop */
    }
    return p_result;
}

const void*
YAAF_HashMapGet(const YAAF_HashMap* pHashMap,
                const char* key)
{
    uint32_t hash = YAAF_OnceAtATimeHashNoCase(key);
    YAAF_HashMapEntry* p_entry = NULL;

    p_entry = YAAF_HashMapFindEntry(pHashMap, hash);
    return (p_entry) ? p_entry->pData : NULL;
}

static int
YAAF_HashMapResizeIfNecessary(YAAF_HashMap* pHashMap)
{
    /* If the load factor exceeds 0.75... */
    if (pHashMap->count > (pHashMap->capacity * 3 / 4))
    {
        /* Add an additional with a 0.33 load factor */
        uint32_t new_capacity = pHashMap->capacity << 1;
        YAAF_HashMapEntry* new_entries = NULL;
        uint32_t i;

        /* check overflow */
        if (new_capacity < pHashMap->capacity)
        {
            return YAAF_FAIL;
        }

        /* create new entries */
        new_entries = YAAF_calloc(1, sizeof(YAAF_HashMapEntry) * new_capacity);
        if (!new_entries)
        {
            return YAAF_FAIL;
        }

        /* copy over existing entries*/
        for (i = 0; i < pHashMap->capacity; i++)
        {
            YAAF_HashMapEntry* cur_entry = &pHashMap->pEntries[i];
            if (cur_entry->pData != NULL && cur_entry->pData != YAAF_HASHMAP_ENTRY_DELETED(pHashMap))
            {
                uint32_t j=0;
                YAAF_HashMapEntry* new_entry = NULL;
                uint32_t idx =0;

                for(j = 0; j < pHashMap->capacity; ++j)
                {
                    idx = YAAF_HashMapCalculateIdx(cur_entry->hash, j, new_capacity);
                    new_entry = &new_entries[idx];
                    /* insert new element if the location is empty */
                    if (!new_entry->pData)
                    {
                        new_entry->pData = cur_entry->pData;
                        new_entry->hash = cur_entry->hash;
                        break;
                    }
                }
            }
        }

        /* release old data */
        YAAF_free(pHashMap->pEntries);
        pHashMap->pEntries = new_entries;
        pHashMap->capacity = new_capacity;
    }

    return YAAF_SUCCESS;
}


int
YAAF_HashMapPut(YAAF_HashMap* pHashMap,
                const char* key,
                const void* pData)
{
    uint32_t hash = YAAF_OnceAtATimeHashNoCase(key);
    return YAAF_HashMapPutWithHash(pHashMap, hash, pData);
}

int YAAF_HashMapPutWithHash(YAAF_HashMap*  pHashMap,
                            const uint32_t hash,
                            const void*    pData)
{
    uint32_t i = 0;
    YAAF_HashMapEntry* new_entry = NULL;
    uint32_t idx = 0;

    /* Reize if necessary, also checks against overflow */
    if(YAAF_HashMapResizeIfNecessary(pHashMap) == YAAF_FAIL)
    {
        return YAAF_FAIL;
    }

    for(i = 0; i < pHashMap->capacity; ++i)
    {
        /* get index */
        idx = YAAF_HashMapCalculateIdx(hash, i, pHashMap->capacity);
        new_entry = &(pHashMap->pEntries[idx]);
        /* insert new element if the location is empty */
        if (!new_entry->pData || new_entry->pData == YAAF_HASHMAP_ENTRY_DELETED(pHashMap))
        {
            new_entry->pData = pData;
            new_entry->hash = hash;
            pHashMap->count++;
            return YAAF_SUCCESS;
        }

        /* replace contents when hashes match */
        if (new_entry->hash == hash)
        {
            new_entry->pData = pData;
            return YAAF_SUCCESS;
        }
        /* continue loop */
    }

    return YAAF_FAIL;
}

int
YAAF_HashMapRemove(YAAF_HashMap* pHashMap,
                   const char* key)
{
    YAAF_HashMapEntry* p_entry = NULL;
    uint32_t hash = YAAF_OnceAtATimeHashNoCase(key);
    p_entry = YAAF_HashMapFindEntry(pHashMap, hash);

    if (p_entry)
    {
        p_entry->hash = 0;
        p_entry->pData = YAAF_HASHMAP_ENTRY_DELETED(pHashMap);
        pHashMap->count--;
        return YAAF_SUCCESS;
    }

    return YAAF_FAIL;
}

const YAAF_HashMapEntry*
YAAF_HashMapItBegin(const YAAF_HashMap* pHashMap)
{
    const YAAF_HashMapEntry* ptr, *ptr_end;
    YAAF_ASSERT(pHashMap->pEntries);

    ptr= pHashMap->pEntries;
    ptr_end = pHashMap->pEntries + pHashMap->capacity;

    while(ptr < ptr_end)
    {
        if (ptr->pData)
        {
            return ptr;
        }
    }
    return ptr_end;
}

void
YAAF_HashMapItNext(const YAAF_HashMap* pHashMap,
                   const YAAF_HashMapEntry** pIter)
{
    const YAAF_HashMapEntry* ptr_end;
    const YAAF_HashMapEntry* ptr;
    YAAF_ASSERT(pIter && *pIter);
    YAAF_ASSERT(pHashMap->pEntries);

    ptr = *pIter;
    ptr_end = pHashMap->pEntries + pHashMap->capacity;
    ++ptr;
    while(ptr < ptr_end)
    {
        if (ptr->pData && ptr->pData != YAAF_HASHMAP_ENTRY_DELETED(pHashMap))
        {
            *pIter = ptr;
            return;
        }
        ++ptr;
    }
    *pIter = ptr_end;
}

const YAAF_HashMapEntry*
YAAF_HashMapItEnd(const YAAF_HashMap* pHashMap)
{
    YAAF_ASSERT(pHashMap->pEntries);
    return pHashMap->pEntries + pHashMap->capacity;
}

const void*
YAAF_HashMapItGet(const YAAF_HashMapEntry* pIter)
{
    return (pIter) ? pIter->pData : NULL;
}
