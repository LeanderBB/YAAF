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
#ifndef __YAAF_DIRUTILS_H__
#define __YAAF_DIRUTILS_H__


#include "YAAFCL.h"

#if defined(YAAF_OS_UNIX)
#define YAAFCL_PATH_SEP_STR "/"
#define YAAFCL_PATH_SEP_CHR '/'
#else
#define YAAFCL_PATH_SEP_STR "\\"
#define YAAFCL_PATH_SEP_CHR '\\'
#endif

/* --- Dir Entry ------------------------------------------------------------*/
typedef struct
{
  YAAFCL_Str fullPath;
  YAAFCL_Str archivePath;
  YAAF_ManifestEntry manifestInfo;
} YAAFCL_DirEntry;

void YAAFCL_DirEntryInit(YAAFCL_DirEntry* pEntry);

void YAAFCL_DirEntryDestroy(YAAFCL_DirEntry* pEntry);

/* --- DirEntryStack --------------------------------------------------------*/

typedef struct YAAFCL_SDirEntryStackNode
{
  YAAFCL_DirEntry* pEntry;
  struct YAAFCL_SDirEntryStackNode* pNext;
} YAAFCL_DirEntryStackNode;

typedef struct
{
  size_t count;
  YAAFCL_DirEntryStackNode* pNodes;
}YAAFCL_DirEntryStack;

void YAAFCL_DirEntryStackInit(YAAFCL_DirEntryStack* pStack);

void YAAFCL_DirEntryStackDestroy(YAAFCL_DirEntryStack* pStack);

void YAAFCL_DirEntryStackPush(YAAFCL_DirEntryStack* pStack, YAAFCL_DirEntry* pEntry);

YAAFCL_DirEntry* YAAFCL_DirEntryStackPop(YAAFCL_DirEntryStack* pStack);

/* --- Dir Utils -----------------------------------------------------------*/

int YAAFCL_ScanDirectory(YAAFCL_DirEntryStack* pStack,
                         const char* directory, const int flags);

int YAAFCL_AddFileToEntryStack(YAAFCL_DirEntryStack* pStack,
                                      const char* file_path,
                                      const char* file_name,
                                      const char* start_path);

int YAAFCL_RealPath(YAAFCL_Str* pStr, const char* path);

int YAAFCL_MakeDir(const char* path);

int YAAFCL_IsFile(const char * path);

int YAAFCL_IsDir(const char* path);

int YAAFCL_IsSymlink(const char* path);

int YAAFCL_Exists(const char* path);

void YAAFCL_ArchivePathToNativePath(YAAFCL_Str* pResult, const char* archivePath,
                                    const char* extractPath);

int YAAFCL_ValidateAndCreateExtractPath(const char* extractDir, const char* extractPath,
                               const int allowOverwrite);

#endif
