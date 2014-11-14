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

/**
 * YAAF - Yet Another Archive Format
 *
 * YAAF is a compressed archive format designed to with one specific goal:
 * High Speed Rutime Decompression of compressed packed data.
 *
 * YAAF uses LZ4 to achieve high decompression speeds whilst sacrificing some
 * compression rate. You can find more information on LZ4 here
 * http://fastcompression.blogspot.de/p/lz4.html and
 * https://code.google.com/p/lz4/.
 *
 * Furthermore, YAAF use block based compression to allow for faster seek
 * operations on the compressed data and the archive is accessed through memory
 * mapped files. This way the OS's memory manager handles the data transfer,
 * resulting in less data copies, easier multi-threaded access and an
 * overall improved performance.
 */

/**
 * This struct holds all the functions required to replace the default system
 * allocator used by YAAF.
 */
#include "YAAF_Setup.h"

#define YAAF_FAIL (-1)
#define YAAF_SUCCESS (0)

/**
 * This struct holds all the functions required to replace the default system
 * allocator used by YAAF.
 */
typedef struct YAAF_Allocator
{
    void* (*malloc)(size_t);
    void  (*free)(void*);
    void* (*calloc)(size_t, size_t);
} YAAF_Allocator;


/**
 * YAAF_FileInfo holds information about a file in the archive. Currently we
 * provide information about the file's last modification date, its compressed
 * size and uncompressed size.
 */
typedef struct
{
    time_t lastModification;
    uint32_t sizeCompressed;
    uint32_t sizeUncompressed;
} YAAF_FileInfo;

/**
 * YAAF_Archive holds all the information regarding the archive.
 * It is provided as a forwad declaration in order to handle future abstractions
 * (e.g.: 32bit and 64bit versions).
 */
struct YAAF_Archive;
typedef struct YAAF_Archive YAAF_Archive;

/**
 * YAAF_File is a representation of a file in the archive
 * It is also provided as a forward declaration in order to abstract different
 * implentations.
 */
struct YAAF_File;
typedef struct YAAF_File YAAF_File;

/**
 * YAAF archives use the slasch character as a path separator. Note also that
 * there is no root separator. If , for instance, in the root of the archive
 * there was a folder name Foo and insied a file named Bar, the full path to
 * access Bar in the archive would be "Foo/Bar".
 */
#define YAAF_ARCHIVE_SEP_CHR '/'
#define YAAF_ARCHIVE_SEP_STR "/"

/* YAAF API */

/**
 * Intialize the internal state of YAAF.
 * @param pAlloc Ptr to a YAAF_Allocator struct to replace the default system
 * allocator. Pass NULL to use the default allocator.
 * @note Function points of YAAF_Allocator are copied, so the original struct
 * can be released after this call.
 * @return YAAF_FAIL on failure, otheriwse YAAF_SUCCESS.
 */
YAAF_EXPORT int YAAF_CALL YAAF_Init(const YAAF_Allocator* pAlloc);

/**
 * Destroy the interal state of YAAF.
 * @note Be sure to call this after all archives have been closed. Failing to do
 * so will result in errors.
 */
YAAF_EXPORT void YAAF_CALL YAAF_Shutdown();

/**
 * Get the current error message. The error message is stored locally to each.
 * Use this call to get more information about a failure in the YAAF API.
 * @return Error message or NULL when no error message is available.
 */
YAAF_EXPORT const char* YAAF_CALL YAAF_GetError();

/* YAAF Arhcive API */

/**
 * Open an archive at a given path.
 * @return NULL on failure, otherwise a pointer to the loaded archive.
 */
YAAF_EXPORT YAAF_Archive* YAAF_CALL YAAF_ArchiveOpen(const char* path);

/**
 * Close an archive and free all resources associated to it.
 */
YAAF_EXPORT void YAAF_CALL YAAF_ArchiveClose(YAAF_Archive* pArchive);

/**
 * List all files in an archive.
 * @return An array of string pointers with the last entry being a NULL ptr. Be
 * sure to free this allocated list with YAAF_ArchiveFreeList();
 */
YAAF_EXPORT const char** YAAF_CALL YAAF_ArchiveListAll(YAAF_Archive* pArchive);

/**
 * List a directory in the archive.
 *  * @return An array of string pointers with the last entry being a NULL ptr. Be
 * sure to free this allocated list with YAAF_ArchiveFreeList();
 */
YAAF_EXPORT const char** YAAF_CALL YAAF_ArchiveListDir(YAAF_Archive* pArchive,
                                                       const char* dir);
/**
 * Free a list allocated by the YAAF_ArchiveList* functions.
 */
YAAF_EXPORT void YAAF_CALL YAAF_ArchiveFreeList(const char** pList);

/**
 * Open a File stream for a file in the archive.
 * @return NULL if file was not found or on failure.
 */
YAAF_EXPORT YAAF_File* YAAF_CALL YAAF_ArchiveFile(YAAF_Archive* pArchive,
                                                  const char* filePath);

/**
 * Retrieve information for a file in the archive.
 * @return YAAF_FAIL if the files was not found, YAAF_SUCCESS otherwise.
 */
YAAF_EXPORT int YAAF_CALL YAAF_ArchiveFileInfo(YAAF_Archive* pArchive,
                                               const char* filePath,
                                               YAAF_FileInfo* pInfo);
/**
 * Check whether a file exists or not in the archive.
 * @return YAAF_FAIL if the files was not found, YAAF_SUCCESS otherwise.
 */
YAAF_EXPORT int YAAF_CALL YAAF_ArchiveContains(const YAAF_Archive* pArchive,
                                               const char* file);

/**
 * Check the archive's contents and see if they match the stored hashes.
 * For each entry this will check the hash for the compressed blocks as well
 * as the uncompressed data.
 * @note This is a slow operation, every file needs to be checked individually.
 * @return YAAF_SUCCESS if everthing checks out, YAAF_FAIL otherwise.
 */
YAAF_EXPORT int YAAF_CALL YAAF_ArchiveCheck(const YAAF_Archive* pArchive);

/**
 * Check wether the file in the archive's matches the stored hashes.
 * @return YAAF_SUCCESS if everthing checks out, YAAF_FAIL otherwise or if
 * the file does not exist.
 */
YAAF_EXPORT int YAAF_CALL YAAF_ArchiveCheckFile(const YAAF_Archive* pArchive,
                                                const char* file);

/* YAAF File API */
/**
 * Read up to size bytes into pBuffer.
 * @return Number of bytes read from the file.
 */
YAAF_EXPORT uint32_t YAAF_CALL YAAF_FileRead(YAAF_File* pFile,
                                             void* pBuffer,
                                             const uint32_t size);

/**
 * Seek to a position in the file stream. This function behaves the same ways
 * as libc's fseek().
 * @param flags Similar to C's file api, SEEK_SET, SEEK_CUR or SEEK_END
 * @return YAAF_FAIL on failure. YAAF_SUCCESS ohtherwise.
 */
YAAF_EXPORT int YAAF_CALL YAAF_FileSeek(YAAF_File* pFile,
                                        int offset,
                                        int flags);

/**
 * Check whether we have reached the end of the file.
 * @return 1 on EOF, 0 otherwise.
 */
YAAF_EXPORT int YAAF_CALL YAAF_FileEOF(const YAAF_File* pFile);

/**
 * Get the current position in the file.
 */
YAAF_EXPORT uint32_t YAAF_CALL YAAF_FileTell(const YAAF_File* pFile);

/**
 * Close the file stream.
 */
YAAF_EXPORT void YAAF_CALL YAAF_FileDestroy(YAAF_File* pFile);


#endif
