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

#ifndef __YAAF_H__
#define __YAAF_H__

#include "YAAF_Setup.h"

#define YAAF_FAIL (-1)
#define YAAF_SUCCESS (0)

YAAF_EXPORT const char* YAAF_CALL YAAF_GetError();

/* Use this to replace the default allocator with an allocator
 * of your choosing
 */
typedef struct YAAF_Allocator
{
    void* (*malloc)(size_t);
    void  (*free)(void*);
    void* (*calloc)(size_t, size_t);
} YAAF_Allocator;

#pragma pack(push)
#pragma pack(1)
/* representation of date time in the archive */
struct YAAF_DateTime
{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
};
#pragma pack(pop)

/* File Info */

typedef struct
{
    struct YAAF_DateTime lastModification;
    uint32_t sizeCompressed;
    uint32_t sizeUncompressed;
} YAAF_FileInfo;

/* YAAF Archive */
struct YAAF_Archive;
typedef struct YAAF_Archive YAAF_Archive;

/* YAAF Archive file handle */
struct YAAF_File;
typedef struct YAAF_File YAAF_File;

#define YAAF_ARCHIVE_SEP_CHR '/'
#define YAAF_ARCHIVE_SEP_STR "/"

YAAF_EXPORT int YAAF_CALL YAAF_Init(const YAAF_Allocator* pAlloc);

YAAF_EXPORT void YAAF_CALL YAAF_Shutdown();

YAAF_EXPORT YAAF_Archive* YAAF_CALL YAAF_ArchiveOpen(const char* path);

YAAF_EXPORT void YAAF_CALL YAAF_ArchiveClose(YAAF_Archive* pArchive);

YAAF_EXPORT const char** YAAF_CALL YAAF_ArchiveListAll(YAAF_Archive* pArchive);

YAAF_EXPORT const char** YAAF_CALL YAAF_ArchiveListDir(YAAF_Archive* pArchive,
                                                       const char* dir);

YAAF_EXPORT void YAAF_CALL YAAF_ArchiveFreeList(YAAF_Archive* pArchive,
                                                const char** pList);

YAAF_EXPORT YAAF_File* YAAF_CALL YAAF_ArchiveFile(YAAF_Archive* pArchive,
                                                  const char* filePath);

YAAF_EXPORT int YAAF_CALL YAAF_ArchiveFileInfo(YAAF_Archive* pArchive,
                                               const char* filePath,
                                               YAAF_FileInfo* pInfo);

YAAF_EXPORT int YAAF_CALL YAAF_ArchiveContains(const YAAF_Archive* pArchive,
                                               const char* file);

YAAF_EXPORT uint32_t YAAF_CALL YAAF_FileRead(YAAF_File* pFile,
                                             void* pBuffer,
                                             const uint32_t size);

YAAF_EXPORT int YAAF_CALL YAAF_FileSeek(YAAF_File* pFile,
                                        int offset,
                                        int flags);

YAAF_EXPORT int YAAF_CALL YAAF_FileEOF(const YAAF_File* pFile);

YAAF_EXPORT uint32_t YAAF_CALL YAAF_FileTell(const YAAF_File* pFile);

YAAF_EXPORT void YAAF_CALL YAAF_FileDestroy(YAAF_File* pFile);


#endif
