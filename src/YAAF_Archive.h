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
#ifndef __YAAF_ARCHIVE_H__
#define __YAAF_ARCHIVE_H__

#include "YAAF.h"
#include "YAAF_MemFile.h"
/*
 * YAAF Archive layout
 * Each Manifest Entry is sorted alphabetically so that the
 * search can be done in log2(n) time. File entries are not required to
 * be sorted.
 *
 * [ YAAF_FileHeader 0      ]
 * [ YAAF File Data 0       ]
 *
 * [ YAAF_FileHeader N      ]
 * [ YAAF File Data N       ]
 *
 * [ YAAF Manifest Entry 0  ]
 * [ YAAF File Name 0       ]
 * [ YAAF File Extra 0      ]
 * ...
 * [ YAAF Manifest Entry N  ]
 * [ YAAF File Name N       ]
 * [ YAAF File Extra N      ]
 *
 * [ YAAF Manifest          ]
 */

#define YAAF_MANIFEST_MAGIC (0x9fb18cbf)
#define YAAF_MANIFEST_ENTRY_MAGIC (0x137647f6)
#define YAAF_FILE_HEADER_MAGIC (0xa0116f80)
#define YAAF_ARCHIVE_FILE_NOT_FOUND 0xFFFFFFFF

#pragma pack(push)
#pragma pack(1)
typedef struct YAAF_Manifest
{
  uint32_t magic;
  uint16_t versionBuilt;
  uint16_t versionRequired;
  uint32_t nEntries;
  uint32_t manifestEntriesSize;
} YAAF_Manifest;

typedef struct YAAF_ManifestEntry
{
  uint32_t magic;
  uint32_t hashUncompressed;
  uint16_t flags;
  uint16_t nameLen;
  uint64_t sizeCompressed;
  uint64_t sizeUncompressed;
  uint64_t offset;
  struct YAAF_DateTime lastModDateTime;
  uint16_t extraLen;
} YAAF_ManifestEntry;

typedef struct YAAF_FileHeader
{
  uint32_t magic;
} YAAF_FileHeader;
#pragma pack(pop)

struct YAAF_Archive
{
  YAAF_MemFile memFile;
  const YAAF_Manifest* pManifest;
  const YAAF_ManifestEntry* pEntries;
};

#endif
