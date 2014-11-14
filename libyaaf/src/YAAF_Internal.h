/*
 * YAAF - Yet Another Archive Format
 * Copyright (C) 2014, Leander Beernaert
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

#ifndef __YAAF_ERRORINTERNAL_H__
#define __YAAF_ERRORINTERNAL_H__

#include <time.h>
#define YAAF_BLOCK_SIZE (128 * 1024)
#define YAAF_BLOCK_CACHE_SIZE_RD YAAF_BLOCK_SIZE
#define YAAF_BLOCK_CACHE_SIZE_WR (YAAF_BLOCK_SIZE + (8 * 1024))


#define YAAF_PTR_OFFSET(ptr, offset) (((char*)ptr) + offset)
#define YAAF_CONST_PTR_OFFSET(ptr, offset) (((const char*)ptr) + offset)

#define YAAF_MAX_FILE_SIZE (0xF0000000)
#define YAAF_MAX_ARCHIVE_SIZE (0xFFFF0000)

struct YAAF_DateTime;

enum
{
    YAAF_COMPRESSION_LZ4_BIT = 1 << 0
};

enum
{
    YAAF_SUPPORTED_COMPRESSIONS_MASK = YAAF_COMPRESSION_LZ4_BIT,
    YAAF_DEFAULT_COMPRESSION_BIT = YAAF_COMPRESSION_LZ4_BIT
};



#pragma pack(push)
#pragma pack(1)

/* representation of date time in the archive */
struct YAAF_DateTime
{
    unsigned sec   :5;
    unsigned min   :5;
    unsigned hour  :6;
    unsigned day   :5;
    unsigned month :4;
    unsigned year  :7;
};
#pragma pack(pop)



void YAAF_SetError(const char* error);

void* YAAF_malloc(size_t size);

void YAAF_free(void* ptr);

void* YAAF_calloc(size_t nmb,
                  size_t size);

int YAAF_StrCompareNoCase(const char* str1,
                          const char* str2);

int YAAF_StrContainsChr(const char* str,
                        const char chr);

int YAAF_GetFileSize(size_t* out,
                     const char* path);

int YAAF_TimeToArchiveTime(const time_t time,
                           struct YAAF_DateTime* pDateTime);

time_t YAAF_ArchiveTimeToTime(const struct YAAF_DateTime* pDateTime);

#endif
