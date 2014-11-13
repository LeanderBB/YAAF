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

#include "YAAF_File.h"
#include "YAAF_Archive.h"
#include "YAAF_Internal.h"
#include "YAAF_Archive.h"

YAAF_File*
YAAF_FileCreate(const void *ptr,
                const struct YAAF_ManifestEntry * pManifestEntry)
{

    YAAF_File* p_result = NULL;
    if (!ptr)
    {
        return NULL;
    }

    const char* chr_ptr = (const char*) ptr;
    chr_ptr += pManifestEntry->offset;

    /* Read file header */

    const YAAF_FileHeader* p_hdr = (const YAAF_FileHeader*)chr_ptr;

    if (YAAF_LITTLE_E32(p_hdr->magic) != YAAF_FILE_HEADER_MAGIC)
    {
        YAAF_SetError("[YAAF_FileCreate] File header magic mismatch");
        return NULL;
    }

    chr_ptr += sizeof(YAAF_FileHeader);

    p_result = (YAAF_File*)YAAF_malloc(sizeof(YAAF_File));
    if (!p_result)
    {
        YAAF_SetError("[YAAF_FileCreate] Failed to allocate memory");
    }
    else
    {
        memset(p_result, 0, sizeof(YAAF_File));
        p_result->ptr = chr_ptr;
        p_result->nBytesUncompressed = pManifestEntry->sizeUncompressed;
        p_result->nBytesCompressed = pManifestEntry->sizeCompressed;
        p_result->nBytesRead  = 0;

        /* create decompressor */
        if (YAAF_DecompressorCreate(&p_result->decompressor, pManifestEntry->flags & YAAF_SUPPORTED_COMPRESSIONS_MASK)
                != YAAF_SUCCESS)
        {
            YAAF_free(p_result);
            p_result = NULL;
        }
    }
    return p_result;
}

static int
YAAF_FileDecompressNextBlock(YAAF_File* pFile)
{
    const uint32_t block_size = *(uint32_t*) YAAF_PTR_OFFSET(pFile->ptr, pFile->nBytesRead);
    const uint32_t data_size = YAAF_BLOCK_SIZE_GET(block_size);
    pFile->nBytesRead += sizeof(uint32_t);
    pFile->cacheOffset = 0;
    /* check if there are more blocks available */
    if (data_size != 0)
    {
        /* decompress only if the block has been compressed */
        if (YAAF_BLOCK_SIZE_COMPRESSED(block_size))
        {
            int res =YAAF_DecompressBlock(&pFile->decompressor,
                                          YAAF_PTR_OFFSET(pFile->ptr, pFile->nBytesRead),
                                          data_size,
                                          pFile->cacheBlock,
                                          YAAF_BLOCK_CACHE_SIZE_RD,
                                          &pFile->cacheSize);
            pFile->cachePtr = &pFile->cacheBlock[0];
            if (res == YAAF_COMPRESSION_OK)
            {
                pFile->nBytesRead += data_size;
            }
            return res;
        }
        else
        {
            /* do not copy any memory, simply point directly to the memory
               mapped file */
            pFile->cachePtr = YAAF_PTR_OFFSET(pFile->ptr, pFile->nBytesRead);
            pFile->cacheSize = data_size;
            pFile->nBytesRead += data_size;

            return YAAF_COMPRESSION_OK;
        }
    }
    else
    {
        pFile->cacheSize = 0;
        return YAAF_COMPRESSION_OK;
    }
}

uint32_t
YAAF_FileRead(YAAF_File* pFile,
              void* pBuffer,
              const uint32_t bufferSize)
{
    uint32_t bytes_written = 0;
    uint32_t size_to_copy;

    if (YAAF_FileEOF(pFile))
    {
        /* EOF */
        return 0;
    }

    while (bytes_written < bufferSize)
    {
        /* Decode a new block */
        if (pFile->cacheOffset >= pFile->cacheSize)
        {
            const int result = YAAF_FileDecompressNextBlock(pFile);
            if (result == YAAF_COMPRESSION_OK)
            {
                if( pFile->cacheSize > 0)
                {
                    pFile->nBytesDecoded += pFile->cacheSize;
                    pFile->cacheOffset = 0;
                }
                else
                {
                    /* EOF */
                    break;
                }
            }
            else
            {
                YAAF_SetError("[YAAF File] Failed to decode next block");
                return 0;
            }
        }

        /* Copy reaming into buffer */
        if (bytes_written + (pFile->cacheSize - pFile->cacheOffset) < bufferSize)
        {
            size_to_copy = pFile->cacheSize - pFile->cacheOffset;
        }
        else
        {
            size_to_copy =  bufferSize - bytes_written;
        }

        if (pBuffer) /* only copy if valid output buffer */
        {
            memcpy((char*)pBuffer + bytes_written,
                   YAAF_PTR_OFFSET(pFile->cachePtr, pFile->cacheOffset),
                   size_to_copy);
        }
        bytes_written += size_to_copy;
        pFile->cacheOffset += size_to_copy;
    }
    return bytes_written;
}


static int
YAAF_FileSeekSet(YAAF_File* pFile,
                 uint32_t bytesRead,
                 const int offset)
{
    fflush(stdout);
    const void* ptr = YAAF_PTR_OFFSET(pFile->ptr, bytesRead);
    uint32_t block_size = YAAF_BLOCK_SIZE_GET(*((uint32_t*) ptr));
    uint32_t ptr_offset = 0;
    const uint32_t skip_blocks = (uint32_t) offset / YAAF_BLOCK_SIZE;
    const uint32_t skip_bytes = (uint32_t) offset % YAAF_BLOCK_SIZE;
    uint32_t i;


    /* reset status */
    pFile->nBytesRead = bytesRead;
    pFile->nBytesDecoded = 0;
    pFile->cacheSize = 0;
    pFile->cacheOffset = 0;

    /* Skip the first n skip_blocks */
    for (i = 0; i < skip_blocks && block_size != 0; ++i)
    {
        /* update ptr offset */
        ptr_offset = sizeof(uint32_t) + block_size;
        pFile->nBytesRead += ptr_offset;
        ptr = YAAF_PTR_OFFSET(ptr, ptr_offset);

        /* next block size */
        block_size = YAAF_BLOCK_SIZE_GET(*((uint32_t*) ptr));

        /* updated decode bytes */
        pFile->nBytesDecoded += YAAF_BLOCK_SIZE;

    }

    if (block_size != 0 && i >= skip_blocks)
    {
        /* decode next block */
        const int result = YAAF_FileDecompressNextBlock(pFile);
        if (result == YAAF_COMPRESSION_OK)
        {
            if( pFile->cacheSize > 0)
            {
                pFile->nBytesDecoded += pFile->cacheSize;
                pFile->cacheOffset = skip_bytes;
            }
        }
        else
        {
            return YAAF_FAIL;
        }
    }
    return YAAF_SUCCESS;
}


int
YAAF_FileSeek(YAAF_File* pFile,
              int offset,
              int flags)
{
    switch(flags)
    {
    case SEEK_SET:

        if(offset < 0)
        {
            return YAAF_FAIL;
        }

        if ((uint32_t) offset > pFile->nBytesUncompressed)
        {
            pFile->nBytesRead = pFile->nBytesCompressed;
            pFile->nBytesDecoded = pFile->nBytesUncompressed;
            pFile->cacheSize = 0;
            pFile->cacheOffset = 0;
            return YAAF_SUCCESS;
        }

        return YAAF_FileSeekSet(pFile, 0, offset);
    case SEEK_CUR:

        if (offset < 0)
        {
            /* check if the offset is still in the cach */
            int new_offset = offset + (int)pFile->cacheOffset;
            if ( new_offset >= 0)
            {
                pFile->cacheOffset = new_offset;
            }
            else
            {
                /* offset not in cache, need to start from begining again */
                return YAAF_FileSeek(pFile, pFile->nBytesDecoded - offset, SEEK_SET);
            }
        }
        else if (offset > 0)
        {

            /* check if the offset is still in the cache */
            if ((int)(pFile->cacheSize - pFile->cacheOffset) < offset )
            {
                pFile->cacheOffset += (uint32_t) offset;
            }
            else
            {
                int remaining_in_cache = (int)(pFile->cacheSize - pFile->cacheOffset);
                int offset_to_reach = offset - remaining_in_cache;
                /* read forward untill offset in cache */
                return YAAF_FileSeekSet(pFile, pFile->nBytesRead, offset_to_reach);
            }
        }
        return YAAF_SUCCESS;
    case SEEK_END:
        pFile->nBytesRead = pFile->nBytesCompressed;
        pFile->nBytesDecoded = pFile->nBytesUncompressed;
        pFile->cacheOffset = 0;
        pFile->cacheSize = 0;
        return YAAF_SUCCESS;
    default:
        return YAAF_FAIL;
    }
}


int
YAAF_FileEOF(const YAAF_File* pFile)
{
    return pFile->nBytesRead >= pFile->nBytesCompressed &&
            pFile->cacheOffset >= pFile->cacheSize;
}

uint32_t
YAAF_FileTell(const YAAF_File* pFile)
{
    return pFile->nBytesDecoded;
}

void
YAAF_FileDestroy(YAAF_File* pFile)
{
    YAAF_DecompressorDestroy(&pFile->decompressor);
    YAAF_free(pFile);
}

