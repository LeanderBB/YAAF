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
#ifndef __YAAF_LZ4ENCODE_H__
#define __YAAF_LZ4ENCODE_H__

#include "YAAF.h"



typedef struct
{
  uint32_t hash;
  uint32_t maxBlockSize;
  uint32_t nBlocks;
  uint64_t fileSize;
  uint64_t compressedSize;
} YAAF_LZ4EncodeInfo;

int YAAF_LZ4Encode(YAAF_Stream* pInput, YAAF_Stream* pOutput, YAAF_LZ4EncodeInfo* pInfo);


typedef struct
{
  uint32_t hash;
  uint64_t nBytesDecoded;
} YAAF_LZ4DecodeInfo;

int YAAF_LZ4Decode(YAAF_Stream* pInput, YAAF_Stream* pOutput, YAAF_LZ4DecodeInfo* pInfo);

int YAAF_LZ4DecodeByBlock(YAAF_Stream* pInput, YAAF_Stream* pOutput, YAAF_LZ4DecodeInfo* pInfo);

typedef struct
{
  YAAF_Stream* pInputStream;
  char* outBuffer;
  char* inBuffer;
  char* outStart;
  uint32_t maxBlockSize;
  uint32_t inputSize;
  uint32_t outputSize;
} YAAF_LZ4DecodeBlockState;

int YAAF_LZ4DecodeBlockStateInit(YAAF_LZ4DecodeBlockState* pState, int blockSizeId,
                                 YAAF_Stream* pInputStream);

void YAAF_LZ4DecodeBlockStateDestroy(YAAF_LZ4DecodeBlockState* pState);

uint32_t YAAF_LZ4HeaderSize();

#define YAAF_LZ4_DECODE_BLOCK_FAIL 0xFFFFFFFF

/*   0                          - End Of Stream
 *   YAAF_LZ4_DECODE_BLOCK_FAIL - Error in stream
 *   > 0                        - Number of decoded bytes
 */
uint32_t YAAF_LZ4DecodeBlock(YAAF_LZ4DecodeBlockState* pState, char* pResult,
                             const size_t resultSize, size_t *pBytesRead);

/* Read LZ4 header from stream and validate it
 * return YAAF_FAIL on error
 * return BlockId (> 0) on success
 */
int YAAF_LZ4ReadAndValidateHeader(YAAF_Stream* pStream);

#endif
