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


/* --- C FILE Wrapper -------------------------------------------------------*/
static size_t YAAF_fwrite(const void* pBuffer, size_t size, void* pData)
{
  FILE* p_file = (FILE*) pData;
  return fwrite(pBuffer, 1, size, p_file);
}
/* --------------------------------------------------------------------------*/
static size_t YAAF_fread(void* pBuffer, size_t size, void* pData)
{
  FILE* p_file = (FILE*) pData;
  return fread(pBuffer, 1, size, p_file);
}
/* --------------------------------------------------------------------------*/
static size_t YAAF_ftell(void* pData)
{
  FILE* p_file = (FILE*) pData;
  return ftell(p_file);
}
/* --------------------------------------------------------------------------*/
static int YAAF_fseek(YAAF_signed_size_t offset, int flags, void* pData)
{
  FILE* p_file = (FILE*) pData;
  return fseek(p_file, offset, flags);
}
/* --------------------------------------------------------------------------*/
static void YAAF_fclose(void* pData)
{
  FILE* p_file = (FILE*) pData;
  fclose(p_file);
}
/* --------------------------------------------------------------------------*/
int YAAF_StreamFromFOpen(YAAF_Stream* pStream, const char* filePath,
                         const char* flags)
{
  int result = YAAF_FAIL;
  FILE* p_file = fopen(filePath, flags);
  if (p_file)
  {
    pStream->pStreamData = p_file;
    pStream->read = YAAF_fread;
    pStream->tell = YAAF_ftell;
    pStream->seek = YAAF_fseek;
    pStream->close = YAAF_fclose;
    pStream->write = YAAF_fwrite;
    result = YAAF_SUCCESS;
  }
  return result;
}
/* --------------------------------------------------------------------------*/
int  YAAF_StreamFromFile(YAAF_Stream* pStream, FILE* pFile)
{
  pStream->pStreamData = pFile;
  pStream->read = YAAF_fread;
  pStream->tell = YAAF_ftell;
  pStream->seek = YAAF_fseek;
  pStream->close = YAAF_fclose;
  pStream->write = YAAF_fwrite;
  return  YAAF_SUCCESS;
}
/* --------------------------------------------------------------------------*/
YAAF_File* YAAF_FileCreate(YAAF_Stream* pStream,  const size_t offset,
                           const size_t sizeCompressed, const size_t fileSize)
{
  int block_id;
  size_t bytes_read;
   YAAF_File* p_result = NULL;
  if (!pStream)
  {
    return NULL;
  }


  if (YAAF_STREAM_SEEK(pStream, offset, SEEK_SET) == YAAF_FAIL)
  {
    return NULL;
  }

  /* Read file header */

  YAAF_FileHeader hdr;
  bytes_read = YAAF_STREAM_READ(pStream, &hdr, sizeof(hdr));
  if (bytes_read != sizeof(hdr))
  {
    YAAF_SetError("[YAAF_FileCreate] Failed to read file header");
    return NULL;
  }

  if (YAAF_LITTLE_E32(hdr.magic) != YAAF_FILE_HEADER_MAGIC)
  {
    YAAF_SetError("[YAAF_FileCreate] File header magic mismatch");
    return NULL;
  }

  /* Check LZ4 header */
  block_id = YAAF_LZ4ReadAndValidateHeader(pStream);

  if (block_id == YAAF_FAIL)
  {
    return NULL;
  }

  p_result = (YAAF_File*)YAAF_malloc(sizeof(YAAF_File));
  if (!p_result)
  {
    YAAF_SetError("[YAAF_FileCreate] Failed to allocate memory");
  }
  else
  {
    memset(p_result, 0, sizeof(YAAF_File));
    p_result->nBytesUncompressed = fileSize;
    p_result->nBytesCompressed = sizeCompressed;
    p_result->nBytesRead += YAAF_LZ4HeaderSize();
    if (YAAF_LZ4DecodeBlockStateInit(&p_result->lz4State,block_id, pStream)
        == YAAF_FAIL)
    {
      YAAF_FileClose(p_result);
      p_result = NULL;
    }
    else
    {
      p_result->offsetAfterHeader = YAAF_STREAM_TELL(pStream);
      p_result->pCache = YAAF_malloc(p_result->lz4State.maxBlockSize);
      if (!p_result->pCache)
      {
        YAAF_FileClose(p_result);
        p_result = NULL;
      }

    }
  }
  return p_result;
}
/* --------------------------------------------------------------------------*/
size_t YAAF_FileRead(YAAF_File* pFile, void* pBuffer, const size_t bufferSize)
{
  size_t bytes_written = 0, bytes_read;
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
      uint32_t bytes_decoded = YAAF_LZ4DecodeBlock(&pFile->lz4State, pFile->pCache,
                                                   pFile->lz4State.maxBlockSize,
                                                   &bytes_read);
      pFile->nBytesRead += bytes_read;
      if (bytes_decoded == YAAF_LZ4_DECODE_BLOCK_FAIL)
      {
        YAAF_SetError("Filed Read: Failed to decode block");
        return 0;
      }
      else if( bytes_decoded > 0)
      {
        pFile->nBytesDecoded += bytes_decoded;
        pFile->cacheSize = bytes_decoded;
        pFile->cacheOffset = 0;
      }
      else
      {
        /* EOF */
        break;
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
      memcpy((char*)pBuffer + bytes_written, pFile->pCache + pFile->cacheOffset,
             size_to_copy);
    }
    bytes_written += size_to_copy;
    pFile->cacheOffset += size_to_copy;
  }
  return bytes_written;
}
/* --------------------------------------------------------------------------*/
int YAAF_FileSeek(YAAF_File* pFile, YAAF_signed_size_t offset, int flags)
{
  switch(flags)
  {
  case SEEK_SET:

    if(offset < 0)
    {
      return YAAF_FAIL;
    }

    /* Seek back to positon right after the LZ4 header */
    if (YAAF_STREAM_SEEK(pFile->lz4State.pInputStream, pFile->offsetAfterHeader,
                         SEEK_SET) == YAAF_FAIL)
    {
      return YAAF_FAIL;
    }

    /* reset status */
    pFile->nBytesRead = YAAF_LZ4HeaderSize();
    pFile->nBytesDecoded = 0;
    pFile->cacheSize = 0;
    pFile->cacheOffset = 0;

    /* keep reading until the offset is in the pcache */
    do
    {
      YAAF_FileRead(pFile, NULL, pFile->lz4State.maxBlockSize);
    } while(pFile->nBytesDecoded < (size_t)offset &&
            (size_t)offset >= pFile->nBytesDecoded + pFile->lz4State.maxBlockSize);

    /* Adjust offset */
    pFile->cacheOffset = ((size_t)offset - pFile->nBytesDecoded);
    return YAAF_SUCCESS;
  case SEEK_CUR:

    if (offset < 0)
    {
      /* check if the offset is still in the cach */
      YAAF_signed_size_t new_offset = offset + (YAAF_signed_size_t)pFile->cacheOffset;
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
      size_t offset_to_reach;
      /* check if the offset is still in the cache */
      if (pFile->cacheSize - pFile->cacheOffset < offset && offset <= 0xFFFFFFFF)
      {
        pFile->cacheOffset += (uint32_t) offset;
      }
      else
      {
        /* read forward untill offset in cache */

        /* invalidate cache */
        pFile->cacheOffset = pFile->cacheSize;
        offset_to_reach = pFile->nBytesDecoded + offset;
        do
        {
          YAAF_FileRead(pFile, NULL, pFile->lz4State.maxBlockSize);
        } while(pFile->nBytesDecoded < offset_to_reach &&
                offset_to_reach >= pFile->nBytesDecoded + pFile->lz4State.maxBlockSize);

        /* Adjust offset */
        pFile->cacheOffset = (offset_to_reach - pFile->nBytesDecoded);
      }
    }
    return YAAF_SUCCESS;
  case SEEK_END:
    pFile->nBytesRead = pFile->nBytesCompressed;
    pFile->nBytesDecoded = pFile->nBytesUncompressed;
    return YAAF_SUCCESS;
  default:
    return YAAF_FAIL;
  }
}

/* --------------------------------------------------------------------------*/
int YAAF_FileEOF(const YAAF_File* pFile)
{
  return pFile->nBytesRead >= pFile->nBytesCompressed &&
      pFile->cacheOffset >= pFile->cacheSize;
}
/* --------------------------------------------------------------------------*/
size_t YAAF_FileTell(const YAAF_File* pFile)
{
  return pFile->nBytesDecoded;
}
/* --------------------------------------------------------------------------*/
void YAAF_FileClose(YAAF_File* pFile)
{
  YAAF_LZ4DecodeBlockStateDestroy(&pFile->lz4State);
  if (pFile->pCache)
  {
    YAAF_free(pFile->pCache);
  }
  YAAF_free(pFile);
}
/* --------------------------------------------------------------------------*/
