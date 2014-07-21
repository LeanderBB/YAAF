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
#include "YAAF_LZ4.h"
#include "YAAF_Internal.h"
#include "3rdparty/lz4/lz4.h"
#include "3rdparty/lz4/lz4hc.h"
#include "3rdparty/xxhash/xxhash.h"

/* --------------------------------------------------------------------------*/
#define YAAF_LZ4_BLOCK_UNCOMRESSED_BIT 0x80000000
#define YAAF_LZ4_BLOCK_SIZE_MASK 0x7FFFFFFF
#define YAAF_LZ4_MAGIC 0x184D2204
#define YAAF_64KB (65536)
#define YAAF_LZ4_MIN_STREAM_BUFFER_SIZE ((2U << 20) + YAAF_64KB)
/* --------------------------------------------------------------------------*/
#pragma pack(push)
#pragma pack(1)

struct YAAF_SLZ4Header
{
  uint32_t magic;
  uint8_t maxBlockSize;
};

#pragma pack(0)

typedef struct YAAF_SLZ4Header YAAF_LZ4Header;

enum
{
  YAAF_LZ4_BLOCK_DEPENENT = 1 << 0,
  YAAF_LZ4_BLOCK_SIZE_5 = 1 << 1,
  YAAF_LZ4_BLOCK_SIZE_6 = 1 << 2,
  YAAF_LZ4_BLOCK_SIZE_7 = 1 << 3
};


/* --------------------------------------------------------------------------*/
YAAF_FORCE_INLINE uint32_t YAAF_LZ4GetBlockSizeForId(const unsigned int id)
{
  return (1 << (8 + (2 * id)));
}
/* --------------------------------------------------------------------------*/
int YAAF_LZ4Encode(YAAF_Stream* pInput, YAAF_Stream* pOutput, YAAF_LZ4EncodeInfo *pInfo)
{

  uint32_t blockSize = YAAF_LZ4GetBlockSizeForId(6);
  uint32_t maxCompressedBlockSize = 0;
  uint64_t compressed_size = 0;
  uint64_t file_size = 0;
  int result = YAAF_SUCCESS;
  char *p_input_block, *p_output_block, *input_start, *input_end;
  size_t input_size = blockSize + (64 * 1024);
  size_t output_size = blockSize + 64;
  uint32_t bytes_written;
  YAAF_LZ4Header hdr;
  void* p_checksumstate = NULL;
  void* lz4_ctx = NULL;


  /* Prepare LZ4 Header */
  memset(&hdr, 0, sizeof(hdr));
  hdr.magic = YAAF_LITTLE_E32(YAAF_LZ4_MAGIC);
  hdr.maxBlockSize = 6;
  /* Write header */
  bytes_written = YAAF_STREAM_WRITE(pOutput, &hdr, sizeof(hdr));
  if (bytes_written != sizeof(hdr))
  {
    YAAF_SetError("LZ4Encode: Failed to write lz4 header");
    return YAAF_FAIL;
  }
  compressed_size += 2;


  /* Allocate Buffers */
  if (input_size < YAAF_LZ4_MIN_STREAM_BUFFER_SIZE)
  {
    input_size = YAAF_LZ4_MIN_STREAM_BUFFER_SIZE;
  }
  p_input_block = YAAF_malloc(input_size);
  p_output_block = YAAF_malloc(output_size);
  if (pInfo)
  {
    p_checksumstate = XXH32_init(0);
  }

  if (!p_input_block || !p_output_block)
  {
    YAAF_SetError("LZ4Encode: Failed to allocate memory for buffers");
    if (p_input_block)
    {
      YAAF_free(p_input_block);
    }
    if (p_output_block)
    {
      YAAF_free(p_output_block);
    }
    return YAAF_FAIL;
  }

  /* Create LZ4 High Compression Context */
  lz4_ctx = LZ4_createHC(p_input_block);
  input_start = p_input_block;
  input_end = input_start + input_size;

  while (1)
  {
    uint32_t bytes_read, output_size, final_ouput;

    /* Adjust compression window for previous 64kb */
    if ((input_start + blockSize) > input_end)
    {
      input_start = LZ4_slideInputBufferHC(lz4_ctx);
    }
    /* Read uncompressed input */
    bytes_read = (uint32_t) pInput->read(input_start, blockSize, pInput->pStreamData);

    /* EOF */
    if (bytes_read == 0)
    {
      break;   /* No more input : end of compression */
    }

    /* Compress Block */
    output_size = LZ4_compressHC_limitedOutput_continue(lz4_ctx, input_start, p_output_block + 4, bytes_read, bytes_read - 1);

    /* if poutut equals 0 it means there was no compression */
    if (output_size > 0)
    {
      final_ouput = output_size + 4;
    }
    else if (!output_size)
    {
      final_ouput = bytes_read;
    }
    else
    {
      YAAF_SetError("LZ4Enconde: Compression error");
      result = YAAF_FAIL;
      break;
    }

    file_size += bytes_read;
    compressed_size += final_ouput;

    /* Update checksum */
    if (pInfo)
    {
      XXH32_update(p_checksumstate, input_start, bytes_read);
    }


    if (output_size > 0) /* Write compressed block to output*/
    {
      /* Embed size in ouput block */
      * (uint32_t*) p_output_block = YAAF_LITTLE_E32(output_size);
      if (output_size > maxCompressedBlockSize)
      {
        maxCompressedBlockSize = output_size;
      }
      bytes_written= (uint32_t) pOutput->write(p_output_block, final_ouput, pOutput->pStreamData);
      if (bytes_written != final_ouput)
      {
        YAAF_SetError("LZEncode: Failed to write compressed block");
        result = YAAF_FAIL;
        break;
      }
    }
    else /* Write uncompressed data */
    {
      /* Add Uncompressed bit */
      * (uint32_t*) p_output_block = YAAF_LITTLE_E32((final_ouput|YAAF_LZ4_BLOCK_UNCOMRESSED_BIT));

      /* write block header */
      bytes_written = (uint32_t) pOutput->write(p_output_block, 4, pOutput->pStreamData);
      if (bytes_written != 4)
      {
        YAAF_SetError("LZEncode: Failed to write uncompressed block header");
        result = YAAF_FAIL;
        break;
      }
      /* write content */
      bytes_written = (uint32_t) pOutput->write(input_start, final_ouput, pOutput->pStreamData);
      if (bytes_written != final_ouput)
      {
        YAAF_SetError("LZEncode: Failed to write uncompressed block");
        result = YAAF_FAIL;
        break;
      }
    }
    input_start += bytes_read;
    if (pInfo)
    {
      pInfo->nBlocks++;
    }
  }

  /* Write End of Stream mark */
  * (uint32_t*) p_output_block = 0;
  bytes_written = (uint32_t) pOutput->write(p_output_block, 4, pOutput->pStreamData);
  if (bytes_written != 4)
  {
    YAAF_SetError("LZEncode: Failed to write end of stream");
    result = YAAF_FAIL;
  }

  /* Update Info */
  if (pInfo)
  {
    pInfo->hash = XXH32_digest(p_checksumstate);
    pInfo->maxBlockSize = maxCompressedBlockSize;
    pInfo->fileSize = file_size;
    pInfo->compressedSize = compressed_size + 4;
  }

  YAAF_free(p_input_block);
  YAAF_free(p_output_block);
  LZ4_freeHC(lz4_ctx);
  return result;
}
/* --------------------------------------------------------------------------*/
int YAAF_LZ4Decode(YAAF_Stream* pInput, YAAF_Stream* pOutput, YAAF_LZ4DecodeInfo* pInfo)
{
  int result = YAAF_SUCCESS;
  size_t bytes_read, bytes_written;
  char* p_input_buf, *p_output_buf, *out_start, *out_end;
  uint32_t max_block_size, input_buffer_size, output_buffer_size;
  uint64_t file_size = 0;
  void* hash_ctx = (pInfo) ? XXH32_init(0) : NULL;

  int block_id = YAAF_LZ4ReadAndValidateHeader(pInput);

  if (block_id == YAAF_FAIL)
  {
    return YAAF_FAIL;
  }

  max_block_size = YAAF_LZ4GetBlockSizeForId(block_id);
  input_buffer_size = max_block_size;
  output_buffer_size = max_block_size + (64 * 1024);

  if (output_buffer_size < YAAF_LZ4_MIN_STREAM_BUFFER_SIZE)
  {
    output_buffer_size = YAAF_LZ4_MIN_STREAM_BUFFER_SIZE;
  }

  p_input_buf = YAAF_malloc(input_buffer_size);
  p_output_buf = YAAF_malloc(output_buffer_size);

  if (!p_input_buf || !p_output_buf)
  {
    YAAF_SetError("LZ4Decode: Failed to allocate buffers");
    if (p_input_buf)
    {
      YAAF_free(p_input_buf);
    }
    if (p_output_buf)
    {
      YAAF_free(p_output_buf);
    }
    return YAAF_FAIL;
  }

  out_start = p_output_buf;
  out_end = p_output_buf + output_buffer_size;

  uint32_t cur_block_size, is_block_uncompressed;
  int decoded_bytes;
  while (1)
  {
    /* Read block size */
    bytes_read = YAAF_STREAM_READ(pInput, &cur_block_size, sizeof(cur_block_size));
    if (bytes_read != sizeof(cur_block_size))
    {
      YAAF_SetError("LZ4Decode: Unknown block size detected");
      result = YAAF_FAIL;
      break;
    }

    /* If size is 0, we reached the end of the stream */
    if (cur_block_size == 0)
    {
      break;
    }

    cur_block_size = YAAF_LITTLE_E32(cur_block_size);

    /* Check if it is compressed */
    is_block_uncompressed = (cur_block_size & YAAF_LZ4_BLOCK_UNCOMRESSED_BIT);
    cur_block_size &= YAAF_LZ4_BLOCK_SIZE_MASK;

    /* Validate block size */
    if (cur_block_size > max_block_size)
    {
      YAAF_SetError("LZ4Decode: Invalid block size");
      result = YAAF_FAIL;
      break;
    }

    /* Read Block */
    bytes_read = YAAF_STREAM_READ(pInput, p_input_buf, cur_block_size);
    if (bytes_read != cur_block_size)
    {
      YAAF_SetError("LZ4Decode: Failed to read block");
      result = YAAF_FAIL;
      break;
    }

    if (!is_block_uncompressed)
    {
      /* Decompress block */
      decoded_bytes = LZ4_decompress_safe_withPrefix64k(p_input_buf, out_start,cur_block_size, max_block_size);
      /* Check if valid */
      if (decoded_bytes < 0)
      {
        YAAF_SetError("LZ4Decode: Corrupt input");
        result = YAAF_FAIL;
        break;
      }
      file_size += decoded_bytes;
      /* Update hash */
      if (pInfo)
      {
        XXH32_update(hash_ctx, out_start, decoded_bytes);
      }

      /* Write decompressed data to output */
      bytes_written = YAAF_STREAM_WRITE(pOutput, out_start, decoded_bytes);
      if (bytes_written != (size_t)decoded_bytes)
      {
        YAAF_SetError("LZ4Decode: Failed to write compressed block");
        result = YAAF_FAIL;
        break;
      }
    }
    else
    {
      /* Write uncompressed block */
      file_size += cur_block_size;
      bytes_written = YAAF_STREAM_WRITE(pOutput, p_input_buf, cur_block_size);
      if (bytes_written != cur_block_size)
      {
        YAAF_SetError("LZ4Decode: Failed to write uncompressed block");
        result = YAAF_FAIL;
        break;
      }
      /* Update hash */
      if (pInfo)
      {
        XXH32_update(hash_ctx, p_input_buf, cur_block_size);
      }

      /* Update window for next block*/
      if (cur_block_size >= 64 * 1024)
      {
        memcpy(p_output_buf, p_input_buf + (cur_block_size - (64 * 1024)), (64 * 1024));   // Required for reference for next blocks
        out_start = p_output_buf + (64 * 1024);
        continue;
      }
      else
      {
        memcpy(out_start, p_input_buf, cur_block_size);
        decoded_bytes = cur_block_size;
      }
    }

    /* Update window for next block*/
    out_start += decoded_bytes;
    if ((size_t)(out_end - out_start) < (size_t)max_block_size)
    {
      memcpy(p_output_buf, out_start - (64 * 1024), (64 * 1024));
      out_start = p_output_buf + (64 * 1024);
    }

  }

  /* finish up */
  if (pInfo)
  {
    pInfo->hash = XXH32_digest(hash_ctx);
    pInfo->nBytesDecoded = file_size;
  }
  YAAF_free(p_input_buf);
  YAAF_free(p_output_buf);
  return result;
}
/* --------------------------------------------------------------------------*/
uint32_t YAAF_LZ4DecodeBlock(YAAF_LZ4DecodeBlockState *pState,
                             char *pResult, const size_t resultSize,
                             size_t* pBytesRead)
{
  size_t bytes_read;
  uint32_t cur_block_size, is_block_uncompressed;
  int decoded_bytes;

  if (resultSize < (pState->maxBlockSize))
  {
    YAAF_SetError("LZ4DecodeBlock: Outputbuffer buffer size should be greater or equal than maxBlockSize + 64KB");
    return YAAF_LZ4_DECODE_BLOCK_FAIL;
  }


  /* Read block size */
  bytes_read = YAAF_STREAM_READ(pState->pInputStream, &cur_block_size, sizeof(cur_block_size));
  if (bytes_read != sizeof(cur_block_size))
  {
    YAAF_SetError("LZ4DecodeBlock: Unknown block size detected");
    return YAAF_LZ4_DECODE_BLOCK_FAIL;
  }

  cur_block_size = YAAF_LITTLE_E32(cur_block_size);

  if(pBytesRead)
  {
    *pBytesRead = sizeof(cur_block_size);
  }

  /* If size is 0, we reached the end of the stream */
  if (cur_block_size == 0)
  {
    return YAAF_SUCCESS;
  }

  /* Check if it is compressed */
  is_block_uncompressed = (cur_block_size & YAAF_LZ4_BLOCK_UNCOMRESSED_BIT);
  cur_block_size &= YAAF_LZ4_BLOCK_SIZE_MASK;

  /* Validate block size */
  if (cur_block_size > pState->maxBlockSize)
  {
    YAAF_SetError("LZ4DecodeBlock: Invalid block size");
    return YAAF_LZ4_DECODE_BLOCK_FAIL;
  }

  /* Read Block */
  bytes_read = YAAF_STREAM_READ(pState->pInputStream, pState->inBuffer, cur_block_size);
  if (bytes_read != cur_block_size)
  {
    YAAF_SetError("LZ4DecodeBlock: Failed to read block");
    return YAAF_LZ4_DECODE_BLOCK_FAIL;
  }

  if(pBytesRead)
  {
    *pBytesRead += (cur_block_size & ~YAAF_LZ4_BLOCK_UNCOMRESSED_BIT);
  }

  if (!is_block_uncompressed)
  {
    /* Decompress block */
    decoded_bytes = LZ4_decompress_safe_withPrefix64k(pState->inBuffer, pState->outStart, cur_block_size, pState->maxBlockSize);
    /* Check if valid */
    if (decoded_bytes < 0)
    {
      YAAF_SetError("LZ4DecodeBlock: Corrupt input");
      return YAAF_LZ4_DECODE_BLOCK_FAIL;
    }

    /* Write compressed block */
    memcpy(pResult, pState->outStart, decoded_bytes);
  }
  else
  {
    /* Write uncompressed block */
    memcpy(pResult, pState->inBuffer, cur_block_size);

    /* Update window for next block*/
    if (cur_block_size >= 64 * 1024)
    {
      memcpy(pState->outBuffer, pState->inBuffer + (cur_block_size - (64 * 1024)), (64 * 1024));   /*Required for reference for next blocks */
      pState->outStart = pState->outBuffer + (64 * 1024);
      return cur_block_size;
    }
    else
    {
      memcpy(pState->outStart, pState->inBuffer, cur_block_size);
      decoded_bytes = cur_block_size;
    }
  }

  /* Update window for next block*/
  pState->outStart += decoded_bytes;
  if ((size_t)((pState->outBuffer + pState->outputSize) - pState->outStart) < (size_t)pState->maxBlockSize)
  {
    memcpy(pState->outBuffer, pState->outStart - (64 * 1024), (64 * 1024));
    pState->outStart = pState->outBuffer + (64 * 1024);
  }
  return (uint32_t)decoded_bytes;
}
/* --------------------------------------------------------------------------*/
int YAAF_LZ4DecodeBlockStateInit(YAAF_LZ4DecodeBlockState* pState, int blockSizeId,
                                 YAAF_Stream* pInputStream)
{
  pState->maxBlockSize = YAAF_LZ4GetBlockSizeForId(blockSizeId);
  pState->inputSize = pState->maxBlockSize;
  pState->outputSize = pState->maxBlockSize + YAAF_64KB;
  pState->inBuffer = YAAF_malloc(pState->inputSize);
  pState->outBuffer = YAAF_malloc(pState->outputSize);
  pState->pInputStream = pInputStream;
  if (pState->inBuffer && pState->outBuffer)
  {
    pState->outStart = pState->outBuffer;
    return YAAF_SUCCESS;
  }
  else
  {
    YAAF_SetError("LZ4DecodeBlockStateInit: Failed to allocate buffers");
    YAAF_LZ4DecodeBlockStateDestroy(pState);
    return YAAF_FAIL;
  }
}
/* --------------------------------------------------------------------------*/
void YAAF_LZ4DecodeBlockStateDestroy(YAAF_LZ4DecodeBlockState* pState)
{
  if (pState->outBuffer)
  {
    YAAF_free(pState->outBuffer);
  }
  if (pState->inBuffer)
  {
    YAAF_free(pState->inBuffer);
  }
}
/* --------------------------------------------------------------------------*/
uint32_t YAAF_LZ4HeaderSize()
{
  return sizeof(YAAF_LZ4Header);
}
/* --------------------------------------------------------------------------*/
int YAAF_LZ4DecodeByBlock(YAAF_Stream* pInput, YAAF_Stream* pOutput,
                          YAAF_LZ4DecodeInfo* pInfo)
{
  int result = YAAF_SUCCESS;
  size_t bytes_written;
  char *decoded_buffer;
  uint64_t file_size = 0;
  void* hash_ctx = (pInfo) ? XXH32_init(0) : NULL;
  YAAF_LZ4DecodeBlockState state;

  int max_block_size = YAAF_LZ4ReadAndValidateHeader(pInput);

  if (max_block_size == YAAF_FAIL)
  {
    return YAAF_FAIL;
  }

  if (YAAF_LZ4DecodeBlockStateInit(&state, max_block_size, pInput) != 0)
  {
    YAAF_SetError("LZ4DecodeByBlock: Failed to init DecodeBlockState");
    return YAAF_FAIL;
  }

  decoded_buffer = YAAF_malloc(state.maxBlockSize);

  if (!decoded_buffer)
  {
    YAAF_SetError("LZ4DecodeByBlock: Failed to allocate result buffer");
    if (decoded_buffer)
    {
      YAAF_free(decoded_buffer);
    }
    return YAAF_FAIL;
  }


  while (1)
  {
    uint32_t decoded_bytes;
    decoded_bytes = YAAF_LZ4DecodeBlock(&state, decoded_buffer, state.maxBlockSize, NULL);

    if (decoded_bytes == YAAF_LZ4_DECODE_BLOCK_FAIL)
    {
      result = YAAF_FAIL;
      break;
    }
    else if(decoded_bytes > 0)
    {
      /* Write uncompressed block */
      file_size += decoded_bytes;
      bytes_written = YAAF_STREAM_WRITE(pOutput, decoded_buffer, decoded_bytes);
      if (bytes_written != decoded_bytes)
      {
        YAAF_SetError("LZ4Decode: Failed to write uncompressed block");
        result = YAAF_FAIL;
        break;
      }
      /* Update hash */
      if (pInfo)
      {
        XXH32_update(hash_ctx, decoded_buffer, decoded_bytes);
      }
    }
    else
    {
      /* EOF */
      break;
    }
  }

  /* finish up */
  if (pInfo)
  {
    pInfo->hash = XXH32_digest(hash_ctx);
    pInfo->nBytesDecoded = file_size;
  }
  YAAF_LZ4DecodeBlockStateDestroy(&state);
  YAAF_free(decoded_buffer);
  return result;
}
/* --------------------------------------------------------------------------*/
int YAAF_LZ4ReadAndValidateHeader(YAAF_Stream* pStream)
{
  YAAF_LZ4Header lz4_header;
  size_t bytes_read = YAAF_STREAM_READ(pStream, &lz4_header, sizeof(lz4_header));
  if (bytes_read != sizeof(lz4_header))
  {
    YAAF_SetError("LZ4Decode: Failed to read LZ4 header");
    return YAAF_FAIL;
  }

  if (YAAF_LITTLE_E32(lz4_header.magic) != YAAF_LZ4_MAGIC)
  {
    YAAF_SetError("LZ4DecodeByBlock: Invalid LZ4 header");
    return YAAF_FAIL;
  }

  /*
  if (!(lz4_header.flags.bits.isBlockDependant))
  {
    YAAF_SetError("LZ4DecodeByBlock: Block indepent compression not supported yet");
    return YAAF_FAIL;
  }*/


  if (lz4_header.maxBlockSize < 6 ||
      lz4_header.maxBlockSize > 7)
  {
    YAAF_SetError("LZ4DecodeByBlock: Invalid block size detected");
    return YAAF_FAIL;
  }

  return lz4_header.maxBlockSize;
}
/* --------------------------------------------------------------------------*/
