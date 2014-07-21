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
  void (*free)(void*);
  void*(*calloc)(size_t, size_t);
} YAAF_Allocator;

YAAF_EXPORT void YAAF_CALL YAAF_SetAllocator(const YAAF_Allocator* pAlloc);

/* Use this struct to replace the behaviour of the default fopen.
 * The functions requested are expected to behave in the same manner as
 * C's FILE.
 */
typedef struct YAAF_Stream
{
  size_t (*read)(void*, size_t, void*);
  size_t (*write)(const void*, size_t, void*);
  size_t (*tell)(void*);
  int (*seek)(YAAF_signed_size_t, int, void*); /* 2nd arg expects flags of fseek function */
  void (*close)(void*);
  void* pStreamData;
} YAAF_Stream;

#define YAAF_STREAM_READ(s,b,z) s->read(b, z, s->pStreamData)
#define YAAF_STREAM_WRITE(s,b,z) s->write(b, z, s->pStreamData)
#define YAAF_STREAM_TELL(s) s->tell(s->pStreamData)
#define YAAF_STREAM_SEEK(s,p,f) s->seek(p, f, s->pStreamData)
#define YAAF_STREAM_CLOSE(s) s->close(s->pStreamData)

/* Prepare a YAAF_Stream_t to wrap around a FILE ptr.
 * returns YAAF_SUCCESS on success, YAAF_FAIL otherwise
 */
YAAF_EXPORT int  YAAF_CALL YAAF_StreamFromFOpen(YAAF_Stream* pStream,
                                                const char* filePath,
                                                const char* flags);

YAAF_EXPORT int  YAAF_CALL YAAF_StreamFromFile(YAAF_Stream* pStream,
                                               FILE* pFile);
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

/* YAAF Entry flags */
enum
{
  YAAF_MANIFEST_FLAGS_COMPRESSION_LZ4_BIT = 1 << 0,

  YAAF_SUPPORTED_COMPRESSIONS = YAAF_MANIFEST_FLAGS_COMPRESSION_LZ4_BIT
};


/* File Info */

typedef struct
{
  struct YAAF_DateTime lastModification;
  uint64_t sizeCompressed;
  uint64_t sizeUncompressed;
} YAAF_FileInfo;

/* YAAF Archive */
struct YAAF_Archive;
typedef struct YAAF_Archive YAAF_Archive;

/* YAAF Archive file handle */
struct YAAF_File;
typedef struct YAAF_File YAAF_File;

#define YAAF_ARCHIVE_SEP_CHR '/'
#define YAAF_ARCHIVE_SEP_STR "/"

YAAF_EXPORT YAAF_Archive* YAAF_CALL YAAF_ArchiveOpen(YAAF_Stream* pStream);

YAAF_EXPORT void YAAF_CALL YAAF_ArchiveClose(YAAF_Archive* pArchive);

YAAF_EXPORT const char** YAAF_CALL YAAF_ArchiveListAll(YAAF_Archive* pArchive);

YAAF_EXPORT const char** YAAF_CALL YAAF_ArchiveListDir(YAAF_Archive* pArchive,
                                                       const char* dir);

YAAF_EXPORT void YAAF_CALL YAAF_ArchiveFreeList(YAAF_Archive* pArchive
                                                ,const char** pList);

YAAF_EXPORT YAAF_File* YAAF_CALL YAAF_ArchiveFile(YAAF_Archive* pArchive,
                                                  YAAF_Stream* pStream,
                                                  const char* filePath);

YAAF_EXPORT int YAAF_CALL YAAF_ArchiveFileInfo(YAAF_Archive* pArchive,
                                               const char* filePath,
                                               YAAF_FileInfo* pInfo);

YAAF_EXPORT int YAAF_CALL YAAF_ArchiveContains(const YAAF_Archive* pArchive,
                                               const char* file);

YAAF_EXPORT size_t YAAF_CALL YAAF_FileRead(YAAF_File* pFile, void* pBuffer,
                                           const size_t size);

YAAF_EXPORT int YAAF_CALL YAAF_FileSeek(YAAF_File* pFile, YAAF_signed_size_t offset,
                                        int flags);

YAAF_EXPORT int YAAF_CALL YAAF_FileEOF(const YAAF_File* pFile);

YAAF_EXPORT size_t YAAF_CALL YAAF_FileTell(const YAAF_File* pFile);

YAAF_EXPORT void YAAF_CALL YAAF_FileClose(YAAF_File* pFile);

#endif
