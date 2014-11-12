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
#include "YAAF_Compression_lz4.h"
#include "YAAF.h"
#include "YAAF_Internal.h"

#include "lz4.h"
#include "lz4hc.h"

static int YAAF_CompressLZ4(void* pState,
                            const void* inbuffer,
                            const uint32_t insize,
                            void* outbuffer,
                            const uint32_t outsize,
                            uint32_t *outWritten)
{
    (void) pState;
    if (outsize >= (uint32_t) LZ4_MAX_INPUT_SIZE)
    {
        return YAAF_COMPRESSION_FAILED;
    }

    int lz4_outsize = LZ4_compressBound((int) insize);
    if ((uint32_t)lz4_outsize > outsize)
    {
        return YAAF_COMPRESSION_OUTPUT_INSUFFICIENT;
    }

    int bytes_compressed = LZ4_compressHC(inbuffer, outbuffer, (int) insize);

    if (bytes_compressed <0)
    {
        return YAAF_COMPRESSION_FAILED;
    }

    if (bytes_compressed == 0)
    {
        /* no compression */
        memcpy(outbuffer, inbuffer, insize);
        *outWritten = YAAF_BLOCK_SIZE_BUILD(0, insize);
        return YAAF_COMPRESSION_OK;
    }
    else
    {
        *outWritten = YAAF_BLOCK_SIZE_BUILD(1, bytes_compressed);
        return YAAF_COMPRESSION_OK;
    }
}

static int YAAF_DecompressLZ4(void* pState,
                              const void* inbuffer,
                              const uint32_t insize,
                              void* outbuffer,
                              const uint32_t outsize,
                              uint32_t *outWritten)
{

    (void) pState;
    int bytes_decompressed = LZ4_decompress_safe(inbuffer, outbuffer,
                                                 (int) insize, (int) outsize);

    if (bytes_decompressed <= 0)
    {
        return YAAF_COMPRESSION_FAILED;
    }

    *outWritten = (uint32_t) bytes_decompressed;

    return YAAF_COMPRESSION_OK;
}


void
YAAF_CompressorCreateLZ4(YAAF_Compressor* pCompressor)
{
    pCompressor->state = NULL;
    pCompressor->compress = YAAF_CompressLZ4;
}

void
YAAF_DecompressorCreateLZ4(YAAF_Decompressor* pDecompressor)
{
    pDecompressor->state = NULL;
    pDecompressor->decompress = YAAF_DecompressLZ4;
}
