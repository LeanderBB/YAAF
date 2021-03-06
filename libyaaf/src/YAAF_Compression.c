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

#include "YAAF_Compression.h"
#include "YAAF_Internal.h"
#include "YAAF_Compression_lz4.h"

int
YAAF_CompressorCreate(YAAF_Compressor* pCompressor,
                      const int type)
{
    switch(type)
    {
    case YAAF_COMPRESSION_LZ4_BIT:
        YAAF_CompressorCreateLZ4(pCompressor);
        return YAAF_SUCCESS;
    default:
        break;
    }
    return YAAF_FAIL;
}

int
YAAF_DecompressorCreate(YAAF_Decompressor* pDecompressor,
                        const int type)
{
    switch(type)
    {
    case YAAF_COMPRESSION_LZ4_BIT:
        YAAF_DecompressorCreateLZ4(pDecompressor);
        return YAAF_SUCCESS;
    default:
        break;
    }
    return YAAF_FAIL;
}


void
YAAF_CompressorDestroy(YAAF_Compressor* pCompressor)
{
    if(pCompressor->state)
    {
        YAAF_free(pCompressor->state);
    }
    memset(pCompressor, 0, sizeof(YAAF_Compressor));
}

void YAAF_DecompressorDestroy(YAAF_Decompressor* pDecompressor)
{
    if(pDecompressor->state)
    {
        YAAF_free(pDecompressor->state);
    }
    memset(pDecompressor, 0, sizeof(YAAF_Decompressor));
}


int
YAAF_CompressBlock(YAAF_Compressor* pCompressor,
                   const void * input,
                   const uint32_t input_size,
                   void * output,
                   const uint32_t output_size,
                   YAAF_BlockHeader *compresResult)
{
    return pCompressor->compress(pCompressor->state, input, input_size,
                                 output, output_size, compresResult);
}

int
YAAF_DecompressBlock(YAAF_Decompressor* pDecompressor,
                     const void * input,
                     const uint32_t input_size,
                     void * output,
                     const uint32_t output_size,
                     uint32_t* bytesWritten)
{
    return pDecompressor->decompress(pDecompressor->state, input, input_size,
                                     output, output_size, bytesWritten);
}
