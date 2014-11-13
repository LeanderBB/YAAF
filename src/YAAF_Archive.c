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

#include "YAAF_Archive.h"
#include "YAAF_File.h"
#include "YAAF_Internal.h"
#include "YAAF_MemFile.h"
#include "xxhash.h"


/* Aux functions */
int YAAF_ArchiveParse(YAAF_Archive* pArchive,
                      const char* path);

uint32_t YAAF_ArchiveLocateFile(const YAAF_Archive* pArchive,
                                const char* file);

YAAF_FORCE_INLINE const char*
YAAF_ManifestEntryName(const YAAF_ManifestEntry* pEntry)
{
    const char* ptr = (const char*)pEntry;
    ptr += sizeof(struct YAAF_ManifestEntry);
    return ptr + pEntry->extraLen;
}

YAAF_Archive*
YAAF_ArchiveOpen(const char* path)
{
    YAAF_Archive* p_archive = NULL;
    if (path)
    {
        p_archive = (YAAF_Archive*) YAAF_malloc(sizeof(YAAF_Archive));
        p_archive->pEntries = NULL;
        p_archive->pManifest = NULL;

        if (YAAF_ArchiveParse(p_archive, path))
        {
            YAAF_ArchiveClose(p_archive);
            p_archive = NULL;
        }
    }
    return p_archive;
}

void
YAAF_ArchiveClose(YAAF_Archive* pArchive)
{
    if (pArchive)
    {
        if (pArchive->pEntries)
        {
            YAAF_free(pArchive->pEntries);
        }
        YAAF_MemFileClose(&pArchive->memFile);
        YAAF_free(pArchive);
    }
}

const char**
YAAF_ArchiveListAll(YAAF_Archive* pArchive)
{
    const char ** p_result = (const char**)YAAF_malloc(sizeof(char*) * (pArchive->pManifest->nEntries + 1));
    if (p_result)
    {
        size_t i;
        for (i = 0; i < pArchive->pManifest->nEntries; ++i)
        {
            p_result[i] = YAAF_ManifestEntryName(pArchive->pEntries[i]);
        }
        p_result[i] = NULL;
    }
    else
    {
        YAAF_SetError("Failed to allocate memory for list");
    }
    return p_result;
}

const char**
YAAF_ArchiveListDir(YAAF_Archive* pArchive, const char* dir)
{
    const char ** p_result = (const char**)YAAF_malloc(sizeof(char*) * (pArchive->pManifest->nEntries + 1));
    size_t dir_len = strlen(dir);
    if (p_result)
    {
        size_t i, result_i = 0;
        if (strcmp(dir,".") != 0)
        {
            for (i = 0; i < pArchive->pManifest->nEntries; ++i)
            {
                const char* entry_name = YAAF_ManifestEntryName(pArchive->pEntries[i]);
                if (strncmp(entry_name, dir, dir_len) == 0 &&
                        (entry_name[dir_len] == YAAF_ARCHIVE_SEP_CHR || dir[dir_len - 1] == YAAF_ARCHIVE_SEP_CHR))
                {
                    p_result[result_i] = entry_name;
                    ++result_i;
                }
            }
        }
        else
        {
            for (i = 0; i < pArchive->pManifest->nEntries; ++i)
            {
                const char* entry_name = YAAF_ManifestEntryName(pArchive->pEntries[i]);
                if (!YAAF_StrContainsChr(entry_name, YAAF_ARCHIVE_SEP_CHR))
                {
                    p_result[result_i] = entry_name;
                    ++result_i;
                }
            }
        }
        p_result[result_i] = NULL;
    }
    else
    {
        YAAF_SetError("Failed to allocate memory for list");
    }
    return p_result;
}

void
YAAF_ArchiveFreeList(const char** pList)
{
    if (pList)
    {
        YAAF_free((void*)pList);
    }
}

int
YAAF_ArchiveContains(const YAAF_Archive* pArchive, const char* file)
{
    YAAF_ASSERT(pArchive);
    return YAAF_ArchiveLocateFile(pArchive,file) == YAAF_ARCHIVE_FILE_NOT_FOUND ? YAAF_SUCCESS : YAAF_FAIL;
}

int
YAAF_ArchiveParse(YAAF_Archive* pArchive,
                  const char* path)
{
    (void) path;
    size_t manifest_offset = 0;
    size_t entries_offset = 0;
    size_t cur_offset = 0;
    const void* tmp_ptr = NULL;
    uint32_t i;


    if (YAAF_MemFileOpen(&pArchive->memFile, path) == YAAF_FAIL)
    {
        return YAAF_FAIL;
    }


    manifest_offset = pArchive->memFile.size - sizeof(YAAF_Manifest);
    pArchive->pManifest = ( const YAAF_Manifest*) YAAF_PTR_OFFSET(pArchive->memFile.ptr, manifest_offset);

    /* Validate manifest */
    if (pArchive->pManifest->magic != YAAF_MANIFEST_MAGIC)
    {
        YAAF_SetError("Invalid Manifest magic");
        return YAAF_FAIL;
    }

    /* Check version */
    if (YAAF_VERSION < pArchive->pManifest->versionRequired)
    {
        YAAF_SetError("Current version is not compatible with the archive");
        return YAAF_FAIL;
    }

    /* Go to the manifest entry start */
    entries_offset = manifest_offset - pArchive->pManifest->manifestEntriesSize;
    tmp_ptr = (const YAAF_ManifestEntry*) YAAF_CONST_PTR_OFFSET(pArchive->memFile.ptr, entries_offset);

    /* check entries hash */
    if (pArchive->pManifest->entriesHash != XXH32(tmp_ptr, pArchive->pManifest->manifestEntriesSize, 0))
    {
        YAAF_SetError("Manifest Entry list corrupted");
        return YAAF_FAIL;
    }

    pArchive->pEntries = YAAF_malloc(sizeof(void*) * pArchive->pManifest->nEntries);

    /* Validate entries */
    for (i = 0; i < pArchive->pManifest->nEntries; ++i)
    {
        const YAAF_ManifestEntry* pManifEntry = (const YAAF_ManifestEntry*) YAAF_CONST_PTR_OFFSET(tmp_ptr, cur_offset);

        /* validate entry magic */
        if (pManifEntry->magic != YAAF_MANIFEST_ENTRY_MAGIC)
        {
            YAAF_SetError("Invalid Manifest Entry");
            return YAAF_FAIL;
        }

        /* check compression */
        if (!(pManifEntry->flags & YAAF_SUPPORTED_COMPRESSIONS_MASK))
        {
            YAAF_SetError("Unsupported compression");
            return YAAF_FAIL;
        }

        /* register entry */
        pArchive->pEntries[i] = pManifEntry;

        /* calculate offset for the next entry */
        cur_offset += sizeof(struct YAAF_ManifestEntry) + pManifEntry->nameLen +
                pManifEntry->extraLen;
    }

    /* Everything succeeded */
    return YAAF_SUCCESS;
}

uint32_t
YAAF_ArchiveLocateFile(const YAAF_Archive* pArchive, const char* file)
{
    uint32_t start = 0, curr = 0, end = pArchive->pManifest->nEntries;
    const char* p_entry_name;
    while (end - start >= 1)
    {
        curr = start + ((end - start) / 2);
        p_entry_name = YAAF_ManifestEntryName(pArchive->pEntries[curr]);
        int cmp = YAAF_StrCompareNoCase(file, p_entry_name);
        if (cmp < 0)
        {
            /* search right */
            end = curr;
        }
        else if (cmp > 0)
        {
            /* search left */
            start = curr + 1;
        }
        else
        {
            /* found the file */
            return curr;
        }
    }
    return YAAF_ARCHIVE_FILE_NOT_FOUND;
}

YAAF_File*
YAAF_ArchiveFile(YAAF_Archive* pArchive,
                 const char* filePath)
{

    uint32_t index;


    /* locate file in archive */
    index = YAAF_ArchiveLocateFile(pArchive, filePath);
    if (index == YAAF_ARCHIVE_FILE_NOT_FOUND)
    {
        return NULL;
    }

    /* Open the file */

    return  YAAF_FileCreate(pArchive->memFile.ptr, pArchive->pEntries[index]);
}

int
YAAF_ArchiveFileInfo(YAAF_Archive* pArchive, const char* filePath,
                     YAAF_FileInfo* pInfo)
{
    uint32_t index;

    /* locate file in archive */
    index = YAAF_ArchiveLocateFile(pArchive, filePath);
    if (index == YAAF_ARCHIVE_FILE_NOT_FOUND)
    {
        return YAAF_FAIL;
    }

    /* copy info */
    pInfo->lastModification = YAAF_ArchiveTimeToTime(&pArchive->pEntries[index]->lastModDateTime);
    pInfo->sizeCompressed = pArchive->pEntries[index]->sizeCompressed;
    pInfo->sizeUncompressed = pArchive->pEntries[index]->sizeUncompressed;

    return YAAF_SUCCESS;
}

static int
YAAF_ArchiveCheckEntry(const YAAF_Archive* pArchive,
                       const YAAF_ManifestEntry* pEntry)
{
    int result = YAAF_SUCCESS;
    uint32_t offset = pEntry->offset + sizeof(YAAF_FileHeader);
    const void* ptr = YAAF_CONST_PTR_OFFSET(pArchive->memFile.ptr, offset);
    uint32_t hash_block, hash_uncompressed;
    XXH32_state_t hash_state;
    const YAAF_BlockHeader* block_header = (const YAAF_BlockHeader*)ptr;
    YAAF_Decompressor dc;
    char tmp_buffer[YAAF_BLOCK_SIZE];

    /* create decompressor */
    if (YAAF_DecompressorCreate(&dc, pEntry->flags & YAAF_SUPPORTED_COMPRESSIONS_MASK) == YAAF_FAIL)
    {
        YAAF_SetError("Failed to create decompressor");
        return YAAF_FAIL;
    }

    XXH32_reset(&hash_state, 0);

    while(block_header->size != 0)
    {
        uint32_t block_size = YAAF_BLOCK_SIZE_GET(block_header->size);
        uint32_t uncompressed_size = 0;


        offset += sizeof(YAAF_BlockHeader);
        ptr = YAAF_CONST_PTR_OFFSET(pArchive->memFile.ptr, offset);

        /* hash block */
        hash_block = XXH32(ptr, block_size, 0);

        /* check hash */
        if (hash_block != block_header->hash)
        {
            YAAF_SetError("Block hash does not match");
            result = YAAF_FAIL;
            break;
        }

        /* if block hash matches, check uncompressed */
        if (YAAF_BLOCK_SIZE_COMPRESSED(block_header->size))
        {
            if (YAAF_DecompressBlock(&dc, ptr, block_size, tmp_buffer, YAAF_BLOCK_SIZE,
                                     &uncompressed_size) != YAAF_COMPRESSION_OK)
            {
                YAAF_SetError("Failed to decompress block");
                result = YAAF_FAIL;
                break;
            }

            /* update uncompressed hash */
            if (XXH32_update(&hash_state, tmp_buffer, uncompressed_size) != XXH_OK)
            {
                YAAF_SetError("Failed to update uncompressed hash");
                result = YAAF_FAIL;
                break;
            }
        }
        else
        {
            /* update uncompressed hash */
            if (XXH32_update(&hash_state, ptr, block_size) != XXH_OK)
            {
                YAAF_SetError("Failed to update uncompressed hash");
                result = YAAF_FAIL;
                break;
            }
        }

        /* update ptr */
        offset += block_size;
        ptr = YAAF_CONST_PTR_OFFSET(pArchive->memFile.ptr, offset);
        block_header = (const YAAF_BlockHeader*)ptr;
    }

    hash_uncompressed = XXH32_digest(&hash_state);

    if (hash_uncompressed != pEntry->hashUncompressed)
    {
        YAAF_SetError("Uncompressed hash does not match");
        result = YAAF_FAIL;
    }

    YAAF_DecompressorDestroy(&dc);
    return result;
}

int
YAAF_ArchiveCheck(const YAAF_Archive* pArchive)
{
    int result = YAAF_SUCCESS;
    uint32_t i = 0;

    for (i = 0; i < pArchive->pManifest->nEntries && result == YAAF_SUCCESS; ++i)
    {
        result = YAAF_ArchiveCheckEntry(pArchive, pArchive->pEntries[i]);
    }
    return result;
}

