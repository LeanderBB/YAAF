/*
 * YAAF Test Compression
 * Copyright (c) 2014 Leander Beernaert
 *
 * YAAFCL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YAAFCL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YAAFCL. If not, see <http://www.gnu.org/licenses/>.
 *
 * You can contact the author at :
 * - YAAF source repository : http://www.github.com/LeanderBB/YAAF
 */

#include "YAAF_Archive.h"
#include "YAAF_File.h"
#include "YAAF_Internal.h"
#include "YAAF_MemFile.h"
#include "YAAF_Compression.h"



static uint32_t
Test_CompressBlock(YAAF_Compressor* pCompressor,
                   const void *input,
                   unsigned int inputSize,
                   void* output)
{
    uint32_t* bytes_written = (uint32_t*)output;
    output = YAAF_PTR_OFFSET(output, sizeof(uint32_t));
    int result = YAAF_CompressBlock(pCompressor, input, inputSize,
                                    output, YAAF_BLOCK_CACHE_SIZE_WR,
                                    bytes_written);
    if (result == YAAF_COMPRESSION_OK)
    {
        return YAAF_BLOCK_SIZE_GET((*bytes_written)) + sizeof(uint32_t);
    }
    else
    {
        if (result == YAAF_COMPRESSION_OUTPUT_INSUFFICIENT)
        {
            fprintf(stderr, "Compression Failed, not enough space in output buffer");
        }
        else if (result == YAAF_COMPRESSION_FAILED)
        {
            fprintf(stderr, "Compression Failed");
        }
        else if (result == YAAF_COMPRESSION_REQUIRES_MORE_INPUT)
        {
            fprintf(stderr, "Compression Failed, not enough input");
        }
        return 0xFFFFFFFF;
    }
}

static const char* s_output_file = "compressed_data.tmp";
static int
Test_CompressFile(const char* path)
{
    YAAF_Compressor c;
    YAAF_File *p_yfile = NULL;
    int result = YAAF_FAIL;
    FILE* p_file = NULL;
    FILE* p_output = NULL;
    size_t file_size = 0;
    uint32_t compressed_size = 0;
    uint32_t i = 0;
    YAAF_MemFile mem_file;
    YAAF_ManifestEntry entry_hdr;

    memset(&mem_file, 0, sizeof(mem_file));

    /* get file size */

    if (YAAF_GetFileSize(&file_size, path) == YAAF_FAIL)
    {
        fprintf(stderr, "Failed to get file size for '%s'\n", path);
        goto cleanup;
    }

    if (file_size > 0xFFFFFF00)
    {
        fprintf(stderr, "File size is larger than max supported size '%s'\n", path);
        goto cleanup;
    }

    memset(&c, 0, sizeof(c));

    if (YAAF_CompressorCreate(&c, YAAF_COMPRESSION_LZ4_BIT) != YAAF_SUCCESS)
    {
        fprintf(stderr, "Failed to create compressor\n");
        goto cleanup;
    }

    p_file = fopen(path, "rb");
    if (!p_file)
    {
        fprintf(stderr, "Failed to open '%s'\n", path);
        goto cleanup;
    }

    p_output = fopen(s_output_file, "w");
    if (!p_output)
    {
        fprintf(stderr, "Failed to open tmp output file \n");
        goto cleanup;
    }

    /* compress file */

    /* write file header */

    i = YAAF_FILE_HEADER_MAGIC;
    if (fwrite(&i, 1, sizeof(uint32_t), p_output) != sizeof(uint32_t))
    {
        fprintf(stderr, "Failed to write file header to tmp file\n");
        goto cleanup;
    }
    /* compress file */
    for(i = 0; i < file_size; i+= YAAF_BLOCK_SIZE)
    {
        static char tmp_in[YAAF_BLOCK_SIZE];
        static char tmp_out[YAAF_BLOCK_CACHE_SIZE_WR];
        uint32_t input_size = ((file_size - i) < YAAF_BLOCK_SIZE) ? (file_size - i) : YAAF_BLOCK_SIZE;
         uint32_t bytes_written = 0;
        /* read into tmp in */

        if (fread(tmp_in, 1, input_size, p_file) != input_size)
        {
            fprintf(stderr, "Failed read input file into buffer\n");
            goto cleanup;
        }

        bytes_written = Test_CompressBlock(&c, tmp_in, input_size,
                                           tmp_out);
        if (bytes_written == 0xFFFFFFFF)
        {
            goto cleanup;
        }

        /* write output  */
        if (fwrite(tmp_out, 1, bytes_written, p_output) != bytes_written)
        {
            fprintf(stderr, "Failed to write compressed data to output file\n");
            goto cleanup;
        }
        compressed_size += bytes_written;
    }
    i = 0;
    if (fwrite(&i, 1, sizeof(uint32_t), p_output) != sizeof(uint32_t))
    {
        fprintf(stderr, "Failed to write end block to output file \n");
        goto cleanup;
    }

    fflush(p_output);
    fclose(p_output);
    p_output = NULL;

    /* open output file */

    if (YAAF_MemFileOpen(&mem_file, s_output_file) != YAAF_SUCCESS)
    {
        fprintf(stderr, "Failed to open temporary output file \n");
        goto cleanup;
    }

    /* read back compressed file and compare with original */

    memset(&entry_hdr, 0, sizeof(entry_hdr));
    entry_hdr.sizeCompressed = compressed_size;
    entry_hdr.sizeUncompressed = file_size;
    entry_hdr.offset = 0;
    entry_hdr.flags = YAAF_COMPRESSION_LZ4_BIT;
    p_yfile = YAAF_FileCreate(mem_file.ptr, &entry_hdr);

    if (!p_yfile)
    {
        fprintf(stderr," Failed to create yaaf file: %s\n", YAAF_GetError());
        goto cleanup;
    }


    /* seek back to begining on input file */
    if (fseek(p_file, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "Failed to seek back to the begining of the file \n");
        goto cleanup;
    }


    for(i = 0; i < file_size; i+= YAAF_BLOCK_SIZE)
    {
        static char tmp_in[YAAF_BLOCK_SIZE];
        static char tmp_buffer[YAAF_BLOCK_SIZE];
        uint32_t input_size = ((file_size - i) < YAAF_BLOCK_SIZE) ? (file_size - i) : YAAF_BLOCK_SIZE;
        uint32_t bytes_read = 0;


        /* read into tmp in */
        if (fread(tmp_in, 1, input_size, p_file) != input_size)
        {
            fprintf(stderr, "Failed read input file into buffer\n");
            goto cleanup;
        }

         /* decompress into tmp buffer*/
        bytes_read = YAAF_FileRead(p_yfile, tmp_buffer, input_size);
        if (bytes_read != input_size)
        {
            fprintf(stderr," Failed to read yaaf file: %s\n", YAAF_GetError());
            goto cleanup;
        }

        /* compare memory */
        if (memcmp(tmp_buffer, tmp_in, input_size) != 0)
        {
            fprintf(stderr," Data does not match for offset %u\n", i);
            goto cleanup;
        }

    }

    for(i = 0; i < 10; ++i)
    {
        static char tmp_in[YAAF_BLOCK_SIZE];
        static char tmp_buffer[YAAF_BLOCK_SIZE];
        uint32_t random_seek = rand() % file_size;
        uint32_t input_size = ((file_size - random_seek) < YAAF_BLOCK_SIZE) ? (file_size - random_seek) : YAAF_BLOCK_SIZE;
        uint32_t bytes_read = 0;


        /* seek back to begining on input file */
        if (fseek(p_file, random_seek, SEEK_SET) != 0)
        {
            fprintf(stderr, "Failed to seek back to %u on input\n", random_seek);
            goto cleanup;
        }

        /* seek back to begining on input file */
        if (YAAF_FileSeek(p_yfile, random_seek, SEEK_SET) != YAAF_SUCCESS)
        {
            fprintf(stderr, "Failed to seek back to %u on output\n", random_seek);
            goto cleanup;
        }

        /* read into tmp in */
        if (fread(tmp_in, 1, input_size, p_file) != input_size)
        {
            fprintf(stderr, "Failed read input file into buffer\n");
            goto cleanup;
        }

         /* decompress into tmp buffer*/
        bytes_read = YAAF_FileRead(p_yfile, tmp_buffer, input_size);
        if (bytes_read != input_size)
        {
            fprintf(stderr," Failed to read yaaf file: %s\n", YAAF_GetError());
            goto cleanup;
        }

        /* compare memory */
        if (memcmp(tmp_buffer, tmp_in, input_size) != 0)
        {
            fprintf(stderr," Data does not match for offset (random seek(%d) at %u)\n", i, random_seek);
            goto cleanup;
        }

    }




    printf("File Size: %lu kb Compression Size: %d kb\n", file_size/ 1024, compressed_size/1024);
    result = YAAF_SUCCESS;
cleanup:

    if (p_output)
    {
        fclose(p_output);
    }

    if (p_file)
    {
        fclose(p_file);
    }

    if (p_yfile)
    {
        YAAF_FileDestroy(p_yfile);
    }

    if (mem_file.ptr)
    {
        YAAF_MemFileClose(&mem_file);
    }
    YAAF_CompressorDestroy(&c);

    return result;
}



int main(const int argc,
         const char** argv)
{
    (void) argv;
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s [input file]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (YAAF_Init(NULL) == YAAF_FAIL)
    {
        fprintf(stderr, "Failed to init yaaf!\n");
        return EXIT_FAILURE;
    }

    int res = Test_CompressFile(argv[1]);

    YAAF_Shutdown();

    return (res == YAAF_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
