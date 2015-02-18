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
#ifndef __YAAF_COMPRESSION_H__
#define __YAAF_COMPRESSION_H__

#include "YAAF.h"
enum
{
    YAAF_COMPRESSION_OK,
    YAAF_COMPRESSION_FAILED,
    YAAF_COMPRESSION_REQUIRES_MORE_INPUT,
    YAAF_COMPRESSION_OUTPUT_INSUFFICIENT
};

#define YAAF_MAX_BLOCK_SIZE 0x7FFFFFFF
#define YAAF_BLOCK_SIZE_BUILD(compressed, size) ((compressed << 31) | (size & YAAF_MAX_BLOCK_SIZE))
#define YAAF_BLOCK_SIZE_COMPRESSED(size) (size >> 31)
#define YAAF_BLOCK_SIZE_GET(size) (size & YAAF_MAX_BLOCK_SIZE)

typedef struct
{
    uint32_t size;
    uint32_t hash;
}YAAF_BlockHeader;


typedef struct
{
    void* state;
    int (*compress)(void*,
                    const void*,
                    const uint32_t,
                    void*,
                    const uint32_t,
                    YAAF_BlockHeader*);
} YAAF_Compressor;

typedef struct
{
    void* state;
    int (*decompress)(void*,
                      const void*,
                      const uint32_t,
                      void*,
                      const uint32_t,
                      uint32_t*);
} YAAF_Decompressor;

int YAAF_CompressorCreate(YAAF_Compressor* pCompressor,
                          const int type);

int YAAF_DecompressorCreate(YAAF_Decompressor* pDecompressor,
                            const int type);

void YAAF_CompressorDestroy(YAAF_Compressor* pCompressor);

void YAAF_DecompressorDestroy(YAAF_Decompressor* pDecompressor);

int YAAF_CompressBlock(YAAF_Compressor* pCompressor,
                       const void * input,
                       const uint32_t input_size,
                       void * output,
                       const uint32_t output_size,
                       YAAF_BlockHeader* compresResult);

int YAAF_DecompressBlock(YAAF_Decompressor* pDecompressor,
                         const void * input,
                         const uint32_t input_size,
                         void * output,
                         const uint32_t output_size,
                         uint32_t* bytesWritten);

#endif
