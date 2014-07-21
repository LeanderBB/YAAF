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

#include "YAAF_Archive.h"
#include "YAAF_File.h"
#include "YAAF_Internal.h"

/* --------------------------------------------------------------------------*/
/* Aux functions */
int YAAF_ArchiveParse(YAAF_Archive* pArchive, YAAF_Stream *pStream);
uint32_t YAAF_ArchiveLocateFile(const YAAF_Archive* pArchive, const char* file);
/* --------------------------------------------------------------------------*/
YAAF_FORCE_INLINE const char* YAAF_ManifestEntryName(const struct YAAF_ManifestEntry* pEntry)
{
  const char* ptr = (const char*)pEntry;
  ptr += sizeof(struct YAAF_ManifestEntry);
  return ptr + pEntry->extraLen;
}
/* --------------------------------------------------------------------------*/
YAAF_Archive* YAAF_ArchiveOpen(YAAF_Stream* pStream)
{
  YAAF_Archive* p_archive = NULL;
  if (pStream)
  {
    p_archive = (YAAF_Archive*) YAAF_malloc(sizeof(YAAF_Archive));
    p_archive->nEntries = 0;
    p_archive->pEntries = NULL;
    p_archive->pManifestEntriesBlob = NULL;
    if (YAAF_ArchiveParse(p_archive, pStream))
    {
      YAAF_ArchiveClose(p_archive);
      p_archive = NULL;
    }
  }
  return p_archive;
}
/* --------------------------------------------------------------------------*/
void YAAF_ArchiveClose(YAAF_Archive* pArchive)
{
  if (pArchive)
  {
    if (pArchive->pEntries)
    {
      YAAF_free(pArchive->pEntries);
    }
    if (pArchive->pManifestEntriesBlob)
    {
      YAAF_free(pArchive->pManifestEntriesBlob);
    }
    YAAF_free(pArchive);
  }
}
/* --------------------------------------------------------------------------*/
const char**  YAAF_ArchiveListAll(YAAF_Archive* pArchive)
{
  const char ** p_result = (const char**)YAAF_malloc(sizeof(char*) * (pArchive->nEntries + 1));
  if (p_result)
  {
    size_t i;
    for (i = 0; i < pArchive->nEntries; ++i)
    {
      p_result[i] = YAAF_ManifestEntryName(pArchive->pEntries[i]);
    }
    p_result[i] = NULL;
  }
  else
  {
    YAAF_SetError("Failed to allocate memory for list");
  }
  return p_result;
}
/* --------------------------------------------------------------------------*/
const char** YAAF_ArchiveListDir(YAAF_Archive* pArchive, const char* dir)
{
  const char ** p_result = (const char**)YAAF_malloc(sizeof(char*) * (pArchive->nEntries + 1));
  size_t dir_len = strlen(dir);
  if (p_result)
  {
    size_t i, result_i = 0;
    if (strcmp(dir,".") != 0)
    {
      for (i = 0; i < pArchive->nEntries; ++i)
      {
        const char* entry_name = YAAF_ManifestEntryName(pArchive->pEntries[i]);
        if (strncmp(entry_name, dir, dir_len) == 0 &&
            (entry_name[dir_len] == YAAF_ARCHIVE_SEP_CHR || dir[dir_len - 1] == YAAF_ARCHIVE_SEP_CHR))
        {
          p_result[result_i] = entry_name;
          ++result_i;
        }
      }
    }
    else
    {
      for (i = 0; i < pArchive->nEntries; ++i)
      {
        const char* entry_name = YAAF_ManifestEntryName(pArchive->pEntries[i]);
        if (!YAAF_StrContainsChr(entry_name, YAAF_ARCHIVE_SEP_CHR))
        {
          p_result[result_i] = entry_name;
          ++result_i;
        }
      }
    }
    p_result[result_i] = NULL;
  }
  else
  {
    YAAF_SetError("Failed to allocate memory for list");
  }
  return p_result;
}
/* --------------------------------------------------------------------------*/
void YAAF_ArchiveFreeList(YAAF_Archive* pArchive, const char** pList)
{
  (void) pArchive;
  if (pList)
  {
    YAAF_free((void*)pList);
  }
}
/* --------------------------------------------------------------------------*/
int YAAF_ArchiveContains(const YAAF_Archive* pArchive, const char* file)
{
  YAAF_ASSERT(pArchive);
  return YAAF_ArchiveLocateFile(pArchive,file) == YAAF_ARCHIVE_FILE_NOT_FOUND ? YAAF_SUCCESS : YAAF_FAIL;
}
/* --------------------------------------------------------------------------*/
int YAAF_ArchiveParse(YAAF_Archive* pArchive, YAAF_Stream *pStream)
{
  size_t archive_size = 0;
  size_t manifest_offset = 0;
  size_t bytes_read = 0;
  struct YAAF_Manifest manifest;
  size_t cur_offset = 0;
  struct YAAF_ManifestEntry* pManifEntry = NULL;
  uint32_t i;

  /* Go the end of file */
  if (pStream->seek(0,SEEK_END,pStream->pStreamData))
  {
    YAAF_SetError("Failed to seek end of stream");
    return YAAF_FAIL;
  }
  archive_size = pStream->tell(pStream->pStreamData);

  /* check if the manifest fits in the archive's current size. If this fails,
   * the file is not a YAAF archive
   */
  if (archive_size < sizeof(struct YAAF_Manifest))
  {
    YAAF_SetError("File size is less than the size of the Manifest");
    return YAAF_FAIL;
  }

  manifest_offset = archive_size - sizeof(struct YAAF_Manifest);
  /* Seek to the manifest position ( end of file - sizeof(manifest)) */
  if (pStream->seek(manifest_offset, SEEK_SET, pStream->pStreamData))
  {
    YAAF_SetError("Failed to seek to start of the Manifest");
    return YAAF_FAIL;
  }

  /* Read manifest */
  bytes_read = pStream->read(&manifest, sizeof(struct YAAF_Manifest), pStream->pStreamData);
  if (bytes_read != sizeof(struct YAAF_Manifest))
  {
    YAAF_SetError("Failed to read Manifest");
    return YAAF_FAIL;
  }

#if defined(YAAF_REQUIRE_BSWAP)
  manifest.magic = YAAF_LITTLE_E32(manifest.magic);
  manifest.versionBuilt = YAAF_LITTLE_E16(manifest.versionBuilt);
  manifest.versionRequired = YAAF_LITTLE_E16(manifest.versionBuilt);
  manifest.nEntries = YAAF_LITTLE_E32(manifest.nEntries);
  manifest.manifestEntriesSize = YAAF_LITTLE_E32(manifest.manifestEntriesSize);
#endif

  /* Validate manifest */
  if (manifest.magic != YAAF_MANIFEST_MAGIC)
  {
    YAAF_SetError("Invalid Manifest magic");
    return YAAF_FAIL;
  }

  /* Check version */
  if (YAAF_VERSION < manifest.versionRequired)
  {
    YAAF_SetError("Current version is not compatible with the archive");
    return YAAF_FAIL;
  }

  /* Go to the manifest entry start */
  if (pStream->seek((YAAF_signed_size_t)manifest_offset - manifest.manifestEntriesSize,
                    SEEK_SET, pStream->pStreamData))
  {
    YAAF_SetError("Failed to seek to start of the first Manifest Entry");
    return YAAF_FAIL;
  }

  /* Allocate the manifest entry blob and pointer refernces */
  pArchive->pManifestEntriesBlob = YAAF_malloc(manifest.manifestEntriesSize);
  if (!pArchive->pManifestEntriesBlob)
  {
    YAAF_SetError("Failed to allocate memory for Manifest blob");
    return YAAF_FAIL;
  }

  pArchive->pEntries = (struct YAAF_ManifestEntry**)YAAF_malloc(sizeof(struct YAAF_ManifestEntry*) * manifest.nEntries);
  if (!pArchive->pEntries)
  {
    YAAF_SetError("Failed to allocate memory for the Manifest Entries");
    return YAAF_FAIL;
  }

  pArchive->nEntries = manifest.nEntries;

  /* Read the manifest entries into the blob */
  bytes_read = pStream->read(pArchive->pManifestEntriesBlob, manifest.manifestEntriesSize,
                             pStream->pStreamData);
  if (bytes_read != manifest.manifestEntriesSize)
  {
    YAAF_SetError("Failed to read the Manifest Entries");
    return YAAF_FAIL;
  }

  /* map the pointer to the blob and validate the entry */
  for (i = 0; i < manifest.nEntries; ++i)
  {
    pArchive->pEntries[i] = (struct YAAF_ManifestEntry*)((char*)pArchive->pManifestEntriesBlob + cur_offset);
    pManifEntry = pArchive->pEntries[i];
#if defined(YAAF_REQUIRE_BSWAP)
    pManifEntry->magic = YAAF_LITTLE_E32(pManifEntry->magic);
    pManifEntry->hashCompressed = YAAF_LITTLE_E32(pManifEntry->hashCompressed);
    pManifEntry->hashUncompressed = YAAF_LITTLE_E32(pManifEntry->hashUncompressed);
    pManifEntry->flags = YAAF_LITTLE_E16(pManifEntry->flags);
    pManifEntry->nameLen = YAAF_LITTLE_E16(pManifEntry->nameLen);
    pManifEntry->sizeCompressed = YAAF_LITTLE_E64(pManifEntry->sizeCompressed);
    pManifEntry->sizeUncompressed = YAAF_LITTLE_E64(pManifEntry->sizeUncompressed);
    pManifEntry->offset = YAAF_LITTLE_E64(pManifEntry->offset);
    pManifEntry->extraLen = YAAF_LITTLE_E16(pManifEntry->extraLen);
#endif
    /* validate entry magic */
    if (pManifEntry->magic != YAAF_MANIFEST_ENTRY_MAGIC)
    {
      YAAF_SetError("Invalid Manifest Entry");
      return YAAF_FAIL;
    }

    /* check compression */
    if (!(pManifEntry->flags & YAAF_SUPPORTED_COMPRESSIONS))
    {
      YAAF_SetError("Unsupported compression");
      return YAAF_FAIL;
    }

    /* calculate offset for the next entry */
    cur_offset += sizeof(struct YAAF_ManifestEntry) + pManifEntry->nameLen +
        pManifEntry->extraLen;
  }

  /* Everything succeeded */
  return YAAF_SUCCESS;
}
/* --------------------------------------------------------------------------*/
uint32_t YAAF_ArchiveLocateFile(const YAAF_Archive* pArchive, const char* file)
{
  uint32_t start = 0, curr = 0, end = pArchive->nEntries;
  const char* p_entry_name;
  while (end - start >= 1)
  {
    curr = start + ((end - start) / 2);
    p_entry_name = YAAF_ManifestEntryName(pArchive->pEntries[curr]);
    int cmp = YAAF_StrCompareNoCase(file, p_entry_name);
    if (cmp < 0)
    {
      /* search right */
      end = curr;
    }
    else if (cmp > 0)
    {
      /* search left */
      start = curr + 1;
    }
    else
    {
      /* found the file */
      return curr;
    }
  }
  return YAAF_ARCHIVE_FILE_NOT_FOUND;
}
/* --------------------------------------------------------------------------*/
YAAF_File* YAAF_ArchiveFile(YAAF_Archive* pArchive, YAAF_Stream* pStream,
                            const char* filePath)
{
  uint32_t index;
  if (!pStream)
  {
    return NULL;
  }

  /* locate file in archive */
  index = YAAF_ArchiveLocateFile(pArchive, filePath);
  if (index == YAAF_ARCHIVE_FILE_NOT_FOUND)
  {
    return NULL;
  }

  /* Open th file */
  return  YAAF_FileCreate(pStream, pArchive->pEntries[index]->offset,
                          pArchive->pEntries[index]->sizeCompressed,
                          pArchive->pEntries[index]->sizeUncompressed);
}
/* --------------------------------------------------------------------------*/
int YAAF_ArchiveFileInfo(YAAF_Archive* pArchive, const char* filePath,
                         YAAF_FileInfo* pInfo)
{
  uint32_t index;

  /* locate file in archive */
  index = YAAF_ArchiveLocateFile(pArchive, filePath);
  if (index == YAAF_ARCHIVE_FILE_NOT_FOUND)
  {
    return YAAF_FAIL;
  }

  /* copy info */
  pInfo->lastModification = pArchive->pEntries[index]->lastModDateTime;
  pInfo->sizeCompressed = pArchive->pEntries[index]->sizeCompressed;
  pInfo->sizeUncompressed = pArchive->pEntries[index]->sizeUncompressed;

  return YAAF_SUCCESS;
}
/* --------------------------------------------------------------------------*/
