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

#include "YAAF.h"
#include "YAAF_LZ4.h"
#include "YAAF_File.h"
#include "YAAF_Archive.h"
#include "YAAF_Internal.h"

static int DoEncode(const char* input, const char* output)
{
  YAAF_Stream input_stream, ouput_stream;
  if (YAAF_StreamFromFOpen(&input_stream,input, "rb") != 0)
  {
    fprintf(stderr,"Failed to open input '%s'\n", input);
    return EXIT_FAILURE;
  }

  if (YAAF_StreamFromFOpen(&ouput_stream, output, "wb") != 0)
  {
    fprintf(stderr,"Failed to open output '%s'\n", input);
    input_stream.close(input_stream.pStreamData);
    return EXIT_FAILURE;
  }

  YAAF_FileHeader fhdr;
  fhdr.magic = YAAF_FILE_HEADER_MAGIC;

  ouput_stream.write(&fhdr, sizeof(fhdr), ouput_stream.pStreamData);

  YAAF_LZ4EncodeInfo info;
  int result = YAAF_LZ4Encode(&input_stream, &ouput_stream, &info);
  if (result < 0)
  {
    fprintf(stderr,"Failed to Encode: %s\n", YAAF_GetError());
  }
  else
  {
    fprintf(stdout,"Hash :               : %X\n", info.hash);
    fprintf(stdout,"Max Block Size       : %d\n", info.maxBlockSize);
    fprintf(stdout,"Number of Blocks     : %d\n", info.nBlocks);
    fprintf(stdout,"File size            : %" PRIu64 "\n", info.fileSize);
    fprintf(stdout,"Compressed size      : %" PRIu64 "\n", info.compressedSize);
    fprintf(stdout,"Ratio                : %f%%\n", (double)(info.fileSize - info.compressedSize) / (double)info.fileSize * 100.0);
  }

  input_stream.close(input_stream.pStreamData);
  ouput_stream.close(ouput_stream.pStreamData);
  return (result < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}

static int DoDecode(const char* input, const char* output)
{
  YAAF_Stream input_stream, ouput_stream;
  int result = 0;
  if (YAAF_StreamFromFOpen(&input_stream,input, "rb") != 0)
  {
    fprintf(stderr,"Failed to open input '%s'\n", input);
    return EXIT_FAILURE;
  }

  if (YAAF_StreamFromFOpen(&ouput_stream, output, "wb") != 0)
  {
    fprintf(stderr,"Failed to open output '%s'\n", input);
    input_stream.close(input_stream.pStreamData);
    return EXIT_FAILURE;
  }


  YAAF_FileHeader fhdr;

  input_stream.read(&fhdr, sizeof(fhdr), input_stream.pStreamData);
  if (fhdr.magic != YAAF_FILE_HEADER_MAGIC)
  {
    fprintf(stderr,"Invalid file header\n");
    goto exit;
  }

  YAAF_LZ4DecodeInfo info;
  result = YAAF_LZ4DecodeByBlock(&input_stream, &ouput_stream, &info);
  if (result < 0)
  {
    fprintf(stderr,"Failed to Decode: %s\n", YAAF_GetError());
  }
  else
  {
    fprintf(stdout,"Hash :               : %X\n", info.hash);
    fprintf(stdout,"File size            : %" PRIu64 "\n", info.nBytesDecoded);
  }
exit:
  input_stream.close(input_stream.pStreamData);
  ouput_stream.close(ouput_stream.pStreamData);
  return (result < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}

/*
static int DoDecodeFile(const char* input, const char* output)
{
  int result = 0;
  YAAF_Allocator allocator;
  allocator.allocate = malloc;
  allocator.free = free;
  YAAF_Stream input_stream, ouput_stream;
  if (YAAF_StreamFromFOpen(&input_stream,input, "rb") != 0)
  {
    fprintf(stderr,"Failed to open input '%s'\n", input);
    return EXIT_FAILURE;
  }

  if (YAAF_StreamFromFOpen(&ouput_stream, output, "wb") != 0)
  {
    fprintf(stderr,"Failed to open output '%s'\n", output);
    input_stream.close(input_stream.pStreamData);
    return EXIT_FAILURE;
  }

  YAAF_File* p_file = YAAF_FileCreate(&input_stream, &allocator, 0, 9786232);

  if (p_file)
  {
    char buffer[1024];
    size_t bytes_read, bytes_written;
    while (!YAAF_FileEOF(p_file))
    {
      bytes_read = YAAF_FileRead(p_file, buffer, 1024);
      if (bytes_read)
      {
        bytes_written = ouput_stream.write(buffer, bytes_read, ouput_stream.pStreamData);
        if (bytes_written != bytes_read)
        {
          fprintf(stderr,"Failed to write bytes to output:'%s'\n", YAAF_GetError());
          result = -1;
          break;
        }
      }
    }
  }
  else
  {
    fprintf(stderr,"Failed to setup input:'%s'\n", YAAF_GetError());
  }
  YAAF_FileClose(p_file);
  input_stream.close(input_stream.pStreamData);
  ouput_stream.close(ouput_stream.pStreamData);
  return (result < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
*/
int main(int argc, char** argv)
{

  if (argc < 4)
  {
    fprintf(stderr,"Usage: %s [encode|decode] [input file] [output file]\n", argv[0]);
    return EXIT_FAILURE;
  }

  YAAF_Allocator allocator;
  allocator.malloc = malloc;
  allocator.calloc = calloc;
  allocator.free = free;

  YAAF_SetAllocator(&allocator);

  if (YAAF_StrCompareNoCase("encode", argv[1]) == 0)
  {
    return DoEncode(argv[2], argv[3]);
  }
  else if (YAAF_StrCompareNoCase("decode", argv[1]) == 0)
  {
    return DoDecode(argv[2], argv[3]);
  }
  /*
  else if (YAAF_StrCompareNoCase("decode2", argv[1]) == 0)
  {
    return DoDecodeFile(argv[2], argv[3]);
  }*/
  else
  {
    fprintf(stderr,"Invalid mode '%s', expected encode or decode\n", argv[1]);
    return EXIT_FAILURE;
  }
}
