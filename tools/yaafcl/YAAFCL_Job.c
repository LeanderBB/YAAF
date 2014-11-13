/*
 * YAAFCL - Yet Another Archive Format Command Line
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
#include "YAAFCL_Job.h"
#include "YAAF_Compression.h"
#include "xxhash.h"

/* --- Single Thread Implementation -----------------------------------------*/

static int
YAAFCL_CompressFile(FILE *pInput,
                    FILE* pOutput,
                    YAAF_ManifestEntry* pEntry)
{
    YAAF_Compressor c;
    char tmp_input[YAAF_BLOCK_SIZE];
    char tmp_output[YAAF_BLOCK_CACHE_SIZE_WR];
    uint32_t file_size = 0, file_size_compressed = 0;
    int result = YAAF_FAIL;
    YAAF_BlockHeader end_block;
    XXH32_state_t hash_state;

    memset(&end_block, 0, sizeof(end_block));

    if(YAAF_CompressorCreate(&c, pEntry->flags & YAAF_SUPPORTED_COMPRESSIONS_MASK) == YAAF_FAIL)
    {
        YAAFCL_LogError("[Compress] Failed to create compressor\n");
        return YAAF_FAIL;
    }

    XXH32_reset(&hash_state, 0);


    while (!feof(pInput))
    {
        uint32_t bytes_read;
        YAAF_BlockHeader c_result;

        /* read input */
        bytes_read = fread(tmp_input, 1, YAAF_BLOCK_SIZE, pInput);
        if (!bytes_read)
        {
            YAAFCL_LogError("[Compress] Failed to read into buffer\n");
            goto cleanup;
        }

        /* compress block */
        if (YAAF_CompressBlock(&c, tmp_input, bytes_read, tmp_output,
                               YAAF_BLOCK_CACHE_SIZE_WR, &c_result) != YAAF_COMPRESSION_OK)
        {
            YAAFCL_LogError("[Compress] Failed to compress block\n");
            goto cleanup;
        }

        /* write block size */
        if (fwrite(&c_result, 1, sizeof(c_result), pOutput) != sizeof(YAAF_BlockHeader))
        {
            YAAFCL_LogError("[Compress] Failed to write block size\n");
            goto cleanup;
        }

        /* write compressed block */
        if (fwrite(tmp_output, 1, YAAF_BLOCK_SIZE_GET(c_result.size), pOutput)
                != YAAF_BLOCK_SIZE_GET(c_result.size))
        {
            YAAFCL_LogError("[Compress] Failed to write block\n");
            goto cleanup;
        }

        /* update hash */
        if (XXH32_update(&hash_state, tmp_input, bytes_read)
                != XXH_OK)
        {
            YAAFCL_LogError("[Compress] Failed to update hash\n");
            goto cleanup;
        }

        file_size += bytes_read;
        file_size_compressed += YAAF_BLOCK_SIZE_GET(c_result.size) + sizeof(YAAF_BlockHeader);
    }

    /* write end of block */
    if (fwrite(&end_block, 1, sizeof(end_block), pOutput) != sizeof(YAAF_BlockHeader))
    {
        YAAFCL_LogError("[Compress] Failed to write end block\n");
        goto cleanup;
    }

    result = YAAF_SUCCESS;
cleanup:

    YAAF_CompressorDestroy(&c);

    if (result == YAAF_SUCCESS)
    {
        pEntry->hashUncompressed = XXH32_digest(&hash_state);
        pEntry->sizeCompressed = file_size_compressed;
        if (pEntry->sizeUncompressed != file_size_compressed)
        {
            result = YAAF_SUCCESS;
        }
    }

    return result;
}

static int
YAAFCL_DirEntryCompareFnc(const void* p1,
                          const void* p2)
{
    const YAAFCL_DirEntry *p_entry1, *p_entry2;
    p_entry1 = *(YAAFCL_DirEntry**) p1;
    p_entry2 = *(YAAFCL_DirEntry**) p2;
    return YAAF_StrCompareNoCase(p_entry1->archivePath.str, p_entry2->archivePath.str);
}

int YAAFCL_JobCompress(FILE* pOutput,
                       YAAFCL_DirEntryStack* pFiles)
{
    YAAFCL_DirEntry** p_manifest_entries = NULL;
    YAAFCL_DirEntryStackNode* p_cur_node = pFiles->pNodes;
    YAAF_FileHeader file_hdr;
    size_t bytes_written, index;
    uint32_t total_manifest_entries_size = 0;
    YAAF_Manifest manifest;
    int result = YAAF_FAIL;
    size_t total_size = sizeof(YAAF_Manifest);
    XXH32_state_t hash_state;

    YAAF_ASSERT(pOutput);
    YAAF_ASSERT(pFiles);
    YAAF_ASSERT(pFiles->count <= (uint32_t)0xFFFFFFFF);



    /* perform size check */

    while(p_cur_node)
    {

        total_size += sizeof(YAAF_ManifestEntry);
        total_size += p_cur_node->pEntry->manifestInfo.extraLen;
        total_size += p_cur_node->pEntry->manifestInfo.nameLen;
        total_size += p_cur_node->pEntry->manifestInfo.sizeCompressed;

        if (total_size > YAAF_MAX_ARCHIVE_SIZE)
        {
            YAAFCL_LogError("[CompressArchive] Archive size exceed addressable limits\n");
            return result;
        }
        p_cur_node = p_cur_node->pNext;
    }


    file_hdr.magic = YAAF_LITTLE_E32(YAAF_FILE_HEADER_MAGIC);
    manifest.magic = YAAF_LITTLE_E32(YAAF_MANIFEST_MAGIC);
    manifest.versionBuilt = YAAF_LITTLE_E16(YAAF_VERSION);
    /* TODO: Add different specs */
    manifest.versionRequired = YAAF_LITTLE_E16(YAAF_VERSION);
    manifest.nEntries = YAAF_LITTLE_E32(pFiles->count);

    if (!pFiles->count)
    {
        YAAFCL_LogError("[CompressArchive] No files to archive. Note: Files with size 0 are not added to the archive.\n");
        return result;
    }

    /*allocate pointer array*/
    p_manifest_entries = (YAAFCL_DirEntry**)YAAF_malloc(sizeof(YAAFCL_DirEntry*) * pFiles->count);


    /* for each file*/
    index = 0;
    p_cur_node = pFiles->pNodes;
    while(p_cur_node)
    {
        const YAAFCL_DirEntry* p_entry = p_cur_node->pEntry;
        FILE* p_input = NULL;

        /* update manifest ptr */
        p_manifest_entries[index] = p_cur_node->pEntry;
        p_manifest_entries[index]->manifestInfo.offset = ftell(pOutput);
        p_manifest_entries[index]->manifestInfo.flags |= YAAF_DEFAULT_COMPRESSION_BIT;
        /* write file header */
        bytes_written = fwrite(&file_hdr,1, sizeof(file_hdr), pOutput);
        if (bytes_written != sizeof(file_hdr))
        {
            YAAFCL_LogError("[CompressArchive] Failed to write File header to output for entry \"%s\"\n.",
                            p_manifest_entries[index]->archivePath.str);
            goto fail;
        }

        /* open input file */
        p_input = fopen(p_entry->fullPath.str,"rb");
        if (!p_input)
        {
            YAAFCL_LogError("[CompressArchive] Failed to open input file \"%s\"\n",p_entry->fullPath.str);
            goto fail;
        }
        /* compress file into archive */
        if (YAAFCL_CompressFile(p_input, pOutput, &p_manifest_entries[index]->manifestInfo)
                != YAAF_SUCCESS)
        {
            YAAFCL_LogError("[CompressArchive] Failed to compress file \"%s\"\n",p_entry->fullPath.str);
            fclose(p_input);
            goto fail;
        }

        fclose(p_input);
        ++index;
        p_cur_node = p_cur_node->pNext;
    }

    /* sort manifest entries */
    qsort(p_manifest_entries,pFiles->count, sizeof(YAAFCL_DirEntry*), YAAFCL_DirEntryCompareFnc);

    XXH32_reset(&hash_state, 0);
    /* for each manifest entry */
    for(index = 0; index < pFiles->count; ++index)
    {
        /* update hash */

        if(XXH32_update(&hash_state, &p_manifest_entries[index]->manifestInfo, sizeof(YAAF_ManifestEntry)) != XXH_OK)
        {
            YAAFCL_LogError("[CompressArchive] Failed calculate hash for entry \"%s\"\n.",
                            p_manifest_entries[index]->archivePath.str);
            goto fail;
        }

        if(XXH32_update(&hash_state, p_manifest_entries[index]->archivePath.str, p_manifest_entries[index]->archivePath.len + 1) != XXH_OK)
        {
            YAAFCL_LogError("[CompressArchive] Failed calculate hash for entry name \"%s\"\n.",
                            p_manifest_entries[index]->archivePath.str);
            goto fail;
        }

        p_manifest_entries[index]->manifestInfo.magic = YAAF_LITTLE_E32(p_manifest_entries[index]->manifestInfo.magic);
        p_manifest_entries[index]->manifestInfo.hashUncompressed = YAAF_LITTLE_E32(p_manifest_entries[index]->manifestInfo.hashUncompressed);
        p_manifest_entries[index]->manifestInfo.flags = YAAF_LITTLE_E16(p_manifest_entries[index]->manifestInfo.flags);
        p_manifest_entries[index]->manifestInfo.nameLen = YAAF_LITTLE_E16(p_manifest_entries[index]->manifestInfo.nameLen);
        p_manifest_entries[index]->manifestInfo.sizeCompressed = YAAF_LITTLE_E64(p_manifest_entries[index]->manifestInfo.sizeCompressed);
        p_manifest_entries[index]->manifestInfo.sizeUncompressed = YAAF_LITTLE_E64(p_manifest_entries[index]->manifestInfo.sizeUncompressed);
        p_manifest_entries[index]->manifestInfo.offset = YAAF_LITTLE_E64(p_manifest_entries[index]->manifestInfo.offset);
        p_manifest_entries[index]->manifestInfo.extraLen = YAAF_LITTLE_E16(p_manifest_entries[index]->manifestInfo.extraLen);

        /*write manifest entry */
        bytes_written = fwrite(&p_manifest_entries[index]->manifestInfo, 1,
                               sizeof(p_manifest_entries[index]->manifestInfo), pOutput);

        if (bytes_written != sizeof(p_manifest_entries[index]->manifestInfo))
        {
            YAAFCL_LogError("[CompressArchive] Failed to write manifest entry for entry \"%s\"\n.",
                            p_manifest_entries[index]->archivePath.str);
            goto fail;
        }

        /*write manifest entry extra - Nothing at this point*/

        /*write manifest entry name */
        bytes_written = fwrite(p_manifest_entries[index]->archivePath.str,1,
                               p_manifest_entries[index]->archivePath.len + 1, pOutput);
        if (bytes_written != p_manifest_entries[index]->archivePath.len + 1)
        {
            YAAFCL_LogError("[CompressArchive] Failed to write manifest entry name for entry \"%s\"\n.",
                            p_manifest_entries[index]->archivePath.str);;
            goto fail;
        }

        total_manifest_entries_size += bytes_written +
                sizeof(p_manifest_entries[index]->manifestInfo);
    }

    /* write manifest */
    manifest.manifestEntriesSize = YAAF_LITTLE_E32(total_manifest_entries_size);
    manifest.entriesHash = XXH32_digest(&hash_state);
    bytes_written = fwrite(&manifest, 1, sizeof(manifest), pOutput);
    if (bytes_written != sizeof(manifest))
    {
        YAAFCL_LogError("[CompressArchive] Failed to write manifest\n");
        goto fail;
    }
    result = YAAF_SUCCESS;
fail:
    YAAF_free(p_manifest_entries);
    return result;
}

static int YAAFCL_DecompressFile(YAAF_File* pFile,
                                 const char* outPath)
{
    char buffer[1024];
    int result = YAAF_FAIL;
    size_t bytes_read, bytes_written;
    FILE* p_fout = NULL;

    p_fout = fopen(outPath, "wb");
    if (!p_fout)
    {
        YAAFCL_LogError("[DecompressFile] Failed to open \"%s\" \n", outPath);
        goto fail;
    }

    while (!YAAF_FileEOF(pFile))
    {
        bytes_read = YAAF_FileRead(pFile, buffer, 1024);
        if (bytes_read)
        {
            bytes_written = fwrite(buffer, 1, bytes_read, p_fout);
            if (bytes_written != bytes_read)
            {
                YAAFCL_LogError("[DecompressFile] Failed to write contents to \"%s\" \n", outPath);
                goto fail;
            }
        }
    }

    result = YAAF_SUCCESS;
fail:
    if (p_fout)
    {
        fclose(p_fout);
    }
    return result;
}

int
YAAFCL_JobDecompressArchive(const char* archive,
                            const char* outDir,
                            const int flags)
{
    YAAF_Archive* p_archive = NULL;
    int result = YAAF_FAIL;
    YAAF_File* p_file = NULL;
    const char** file_list = NULL;
    const char** aux_ptr = NULL;
    YAAFCL_Str extract_path;
    YAAFCL_Str extract_dir;

    YAAFCL_StrInit(&extract_path);
    YAAFCL_StrInit(&extract_dir);

    /* Open Archive */
    p_archive = YAAF_ArchiveOpen(archive);
    if (!p_archive)
    {
        YAAFCL_LogError("[DecompressArchive] Failed to parse archive \"%s\" - %s\n", archive, YAAF_GetError());
        goto fail;
    }

    /* List all files */

    file_list = YAAF_ArchiveListAll(p_archive);
    if (!file_list)
    {
        YAAFCL_LogError("[DecompressArchive] Failed to list all files in archive \"%s\": %s\n",archive,
                        YAAF_GetError());
        goto fail;
    }
    /* for each file */
    aux_ptr = file_list;
    while(*aux_ptr)
    {

        /* Get extract path */
        YAAFCL_ArchivePathToNativePath(&extract_path, *aux_ptr, outDir);

        YAAFCL_StrExtractPath(&extract_dir,&extract_path,YAAFCL_PATH_SEP_CHR);

        /*  Create directory if it does not exist */
        if (!YAAFCL_ValidateAndCreateExtractPath(extract_dir.str,extract_path.str,
                                                 flags & YAAFCL_SWITCH_ALLOW_FILE_OVERWRITE))
        {
            YAAFCL_LogError("[DecompressArchive] Failed to validate extract path \"%s\", the path may already exists, not be a directory or cannot be created\n",
                            extract_path.str);
            goto fail;
        }

        /* open archive file */
        p_file = YAAF_ArchiveFile(p_archive, *aux_ptr);

        if (!p_file)
        {
            YAAFCL_LogError("[DecompressArchive] Failed to open \"%s\" in archive \"%s\": %s\n", *aux_ptr,
                            archive, YAAF_GetError());
            goto fail;
        }

        /*  Exctract the file */
        if (YAAFCL_DecompressFile(p_file, extract_path.str) != YAAF_SUCCESS)
        {
            YAAFCL_LogError("[DecompressArchive] Failed to decompress \"%s\" in archive \"%s\": %s\n", *aux_ptr,
                            archive, YAAF_GetError());
            goto fail;
        }

        YAAF_FileDestroy(p_file);
        p_file = NULL;

        /* advance */
        ++ aux_ptr;
    }
    result = YAAF_SUCCESS;
fail:
    YAAFCL_StrDestroy(&extract_path);
    YAAFCL_StrDestroy(&extract_dir);
    if (file_list)
    {
        YAAF_ArchiveFreeList(file_list);
    }
    if(p_archive)
    {
        YAAF_ArchiveClose(p_archive);
    }
    if (p_file)
    {
        YAAF_FileDestroy(p_file);
    }

    return result;

}

