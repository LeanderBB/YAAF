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

#include "YAAFCL.h"
#include "YAAFCL_DirUtils.h"

#include "YAAF_Hash.h"

#if defined(YAAF_OS_UNIX)
#include <sys/time.h>
#include <sys/param.h>
#include <unistd.h>
#else
#include <time.h>
#endif
#include <sys/stat.h>
#if defined(YAAF_HAVE_DIRENT_H)
#include <dirent.h>
#elif defined(YAAF_OS_WIN)
#include "win_dirent.h"
#include <direct.h>
#else
#error Could not find dirent.h
#endif
#include <time.h>

#if defined (YAAF_OS_UNIX)
#include <errno.h>
#endif

#include <limits.h>

void
YAAFCL_DirEntryInit(YAAFCL_DirEntry* pEntry)
{
    YAAFCL_StrInit(&pEntry->fullPath);
    YAAFCL_StrInit(&pEntry->archivePath);
    memset(&pEntry->manifestInfo, 0 , sizeof(pEntry->manifestInfo));
    pEntry->manifestInfo.magic = YAAF_MANIFEST_ENTRY_MAGIC;
}

void
YAAFCL_DirEntryDestroy(YAAFCL_DirEntry* pEntry)
{
    YAAFCL_StrDestroy(&pEntry->fullPath);
    YAAFCL_StrDestroy(&pEntry->archivePath);
}

void
YAAFCL_DirEntryStackInit(YAAFCL_DirEntryStack* pStack)
{
    memset(pStack, 0, sizeof(YAAFCL_DirEntryStack));
}

void
YAAFCL_DirEntryStackDestroy(YAAFCL_DirEntryStack* pStack)
{
    YAAFCL_DirEntry* p_entry;
    while((p_entry = YAAFCL_DirEntryStackPop(pStack)))
    {
        YAAFCL_DirEntryDestroy(p_entry);
        YAAF_free(p_entry);
    };
}

void
YAAFCL_DirEntryStackPush(YAAFCL_DirEntryStack* pStack,
                         YAAFCL_DirEntry* pEntry)
{
    if (pEntry)
    {
        YAAFCL_DirEntryStackNode* p_node = (YAAFCL_DirEntryStackNode*)YAAF_malloc(sizeof(YAAFCL_DirEntryStackNode));
        p_node->pEntry = pEntry;
        p_node->pNext = pStack->pNodes;
        pStack->pNodes = p_node;
        ++pStack->count;
    }
}

YAAFCL_DirEntry*
YAAFCL_DirEntryStackPop(YAAFCL_DirEntryStack* pStack)
{
    YAAFCL_DirEntry* p_entry = (pStack->pNodes) ? pStack->pNodes->pEntry : NULL;
    if (p_entry)
    {
        YAAFCL_DirEntryStackNode* p_aux = pStack->pNodes->pNext;
        YAAF_free(pStack->pNodes);
        pStack->pNodes = p_aux;
        --pStack->count;
    }
    return p_entry;
}

int
YAAFCL_RealPath(YAAFCL_Str* pStr,
                const char* path)
{
    char* full_path;

    YAAFCL_StrClear(pStr);
#if defined(YAAF_OS_UNIX)
    full_path = realpath(path, NULL);
#elif defined(YAAF_OS_WIN)
    full_path = _fullpath(NULL, path, _MAX_PATH);
#else
#error Missing implemenation of realpath
#endif
    if (!full_path)
    {
        return YAAF_FAIL;
    }
    YAAFCL_StrConcat(pStr, full_path);
    /* system allocator needs to be used here*/
    free(full_path);
    return YAAF_SUCCESS;
}

#if defined(YAAF_OS_UNIX)
#define YAAFCL_mkdir(dir) mkdir(dir, (S_IWUSR | S_IRUSR | S_IXUSR))
#elif defined (YAAF_OS_WIN)
#define YAAFCL_mkdir(dir) _mkdir(dir)
#else
#error Missing implementation of mkdir
#endif
static int YAAFCL_MakeDirRecursive(char* path, char* search_offset)
{
    char* char_loc = strchr(search_offset, YAAFCL_PATH_SEP_CHR);
    if (char_loc)
    {
        *char_loc = '\0';
        errno = 0;
        if (YAAFCL_mkdir(path) == -1)
        {
            if (errno!=EEXIST)
            {
                *char_loc= YAAFCL_PATH_SEP_CHR;
                return YAAF_FAIL;
            }
        }
        *char_loc= YAAFCL_PATH_SEP_CHR;
        return YAAFCL_MakeDirRecursive(path, char_loc + 1);
    }
    else
    {
        if (YAAFCL_mkdir(path) == -1)
        {
            if (errno!=EEXIST)
            {
                return YAAF_FAIL;
            }
        }
        return YAAF_SUCCESS;
    }
}

int
YAAFCL_MakeDir(const char* path)
{
    YAAFCL_Str path_copy;
    int result = YAAF_FAIL;
    YAAFCL_StrInit(&path_copy);
    YAAFCL_StrConcat(&path_copy, path);
    result = YAAFCL_MakeDirRecursive(path_copy.str, path_copy.str);
    YAAFCL_StrDestroy(&path_copy);
    return result;
}

int
YAAFCL_AddFileToEntryStack(YAAFCL_DirEntryStack* pStack,
                           const char* file_path,
                           const char* file_name,
                           const char* start_path)
{
    struct stat stat_inf;
    YAAFCL_DirEntry* p_dir_entry = NULL;


    p_dir_entry = (YAAFCL_DirEntry*)YAAF_malloc(sizeof(YAAFCL_DirEntry));

    /* create paths */
    YAAFCL_DirEntryInit(p_dir_entry);
    YAAFCL_StrJoinPath(&p_dir_entry->fullPath, file_path, file_name,
                       YAAFCL_PATH_SEP_CHR);
    if (start_path)
    {
        YAAFCL_StrCopy(&p_dir_entry->archivePath, &p_dir_entry->fullPath);
        YAAFCL_StrReplace(&p_dir_entry->archivePath, start_path, "");
    }
    else
    {
        YAAFCL_StrConcat(&p_dir_entry->archivePath, file_name);
    }

    /* Check if archive path fits in a uint16_t */
    if (p_dir_entry->archivePath.len > 0xFFFF)
    {

        YAAFCL_LogError("Archive path ('%s') is too long for entry '%s'\n",
                        p_dir_entry->archivePath.str, p_dir_entry->fullPath.str);
        goto exit_fail;
    }

    /* Get size and time info */
    if (stat(p_dir_entry->fullPath.str, &stat_inf) != 0)
    {
        YAAFCL_LogError("Could not get file information for '%s'\n",
                        p_dir_entry->fullPath.str);
        goto exit_fail;
    }

    if (stat_inf.st_size == 0)
    {
        /* do not add 0 sized files skip */
        YAAFCL_DirEntryDestroy(p_dir_entry);
        YAAF_free(p_dir_entry);
        return YAAF_SUCCESS;
    }

    if (stat_inf.st_size > YAAF_MAX_FILE_SIZE)
    {
        /* File is bigger than max addressable file size*/
        YAAFCL_LogError("File is larger than maximum addressable size: '%s'\n",
                        p_dir_entry->fullPath.str);
        goto exit_fail;
    }


    p_dir_entry->manifestInfo.sizeUncompressed = stat_inf.st_size;
#if defined(YAAF_OS_APPLE)
    if (YAAF_TimeToArchiveTime(stat_inf.st_mtimespec.tv_sec,
                               &p_dir_entry->manifestInfo.lastModDateTime) != YAAF_SUCCESS)
#elif defined(YAAF_OS_UNIX)
    if (YAAF_TimeToArchiveTime(stat_inf.st_mtim.tv_sec,
                               &p_dir_entry->manifestInfo.lastModDateTime) != YAAF_SUCCESS)
#else
    if (YAAF_TimeToArchiveTime(stat_inf.st_mtime,
                               &p_dir_entry->manifestInfo.lastModDateTime) != YAAF_SUCCESS)
#endif
    {
        YAAFCL_DirEntryDestroy(p_dir_entry);
        YAAF_free(p_dir_entry);
        return YAAF_FAIL;
    }

    /* Replace separators to match archive conventions */
    if (YAAFCL_PATH_SEP_CHR != YAAF_ARCHIVE_SEP_CHR)
    {
        YAAFCL_StrReplace(&p_dir_entry->archivePath,YAAFCL_PATH_SEP_STR,
                          YAAF_ARCHIVE_SEP_STR);
    }

    p_dir_entry->manifestInfo.extraLen = 0;
    p_dir_entry->manifestInfo.nameLen = (uint16_t)p_dir_entry->archivePath.len + 1;
    p_dir_entry->manifestInfo.nameHash = YAAF_OnceAtATimeHashNoCase(p_dir_entry->archivePath.str);

    YAAFCL_DirEntryStackPush(pStack, p_dir_entry);
    return YAAF_SUCCESS;
exit_fail:
    YAAFCL_DirEntryDestroy(p_dir_entry);
    YAAF_free(p_dir_entry);
    return YAAF_FAIL;
}

int
YAAFCL_ScanDirectory(YAAFCL_DirEntryStack* pStack,
                     const char* directory,
                     const int flags)
{

    YAAFCL_StrList dir_queue;
    YAAFCL_Str real_path;
    YAAFCL_Str first_dir;
    int result = YAAF_FAIL;
    DIR* p_cur_dir;
    struct dirent *p_cur_dirent;

    YAAFCL_StrListInit(&dir_queue);
    YAAFCL_StrInit(&real_path);
    YAAFCL_StrInit(&first_dir);

    if (YAAFCL_RealPath(&first_dir, directory) != YAAF_SUCCESS)
    {
        YAAFCL_LogError("[ScanDirectory] Failed to get real path for \"%s\"\n", directory);
        goto YAAFCL_ScanDirFail;
    }


    YAAFCL_StrExtractPath(&real_path, &first_dir, YAAFCL_PATH_SEP_CHR);
    if (real_path.str[real_path.len] != YAAFCL_PATH_SEP_CHR)
    {
        YAAFCL_StrConcat(&real_path, YAAFCL_PATH_SEP_STR);
    }
    YAAFCL_StrListPushBackMove(&dir_queue, &first_dir);

    while (YAAFCL_StrListPeekHead(&dir_queue))
    {
        const YAAFCL_Str* p_cur_path = YAAFCL_StrListPeekHead(&dir_queue);

        /* open dir */
        p_cur_dir = opendir(p_cur_path->str);
        if (!p_cur_dir)
        {
            YAAFCL_LogError("[Scan Directory] Failed to read dir '%s'\n", p_cur_path->str);
            goto YAAFCL_ScanDirFail;
        }

        /* go through directory */
        while ((p_cur_dirent = readdir(p_cur_dir)))
        {
            /* skip . and .. */
            if (strcmp(".",p_cur_dirent->d_name) == 0 ||
                    strcmp("..", p_cur_dirent->d_name) == 0)
            {
                continue;
            }

            /* if it is a file */
            if ( DT_REG ==  p_cur_dirent->d_type)
            {
                if (YAAFCL_AddFileToEntryStack(pStack, p_cur_path->str, p_cur_dirent->d_name,
                                               real_path.str) != YAAF_SUCCESS)
                {
                    goto YAAFCL_ScanDirFail;
                }
            }
            else if (DT_DIR == p_cur_dirent->d_type && (flags & YAAFCL_SWITCH_RECURSIVE_BIT))
            {
                YAAFCL_Str full_path;
                YAAFCL_StrInit(&full_path);
                YAAFCL_StrJoinPath(&full_path, p_cur_path->str, p_cur_dirent->d_name,
                                   YAAFCL_PATH_SEP_CHR);
                YAAFCL_StrListPushBackMove(&dir_queue, &full_path);
            }
        }
        closedir(p_cur_dir);
        YAAFCL_StrListPopHead(&dir_queue);
    }
    result = YAAF_SUCCESS;

YAAFCL_ScanDirFail:
    YAAFCL_StrDestroy(&real_path);
    YAAFCL_StrListDestroy(&dir_queue);
    return result;
}

int
YAAFCL_IsFile(const char * path)
{
    struct stat stat_inf;
    if (stat(path, &stat_inf) == 0)
    {
        return S_ISREG(stat_inf.st_mode);
    }
    return 0;
}

int
YAAFCL_IsDir(const char* path)
{
    struct stat stat_inf;
    if (stat(path, &stat_inf) == 0)
    {
        return S_ISDIR(stat_inf.st_mode);
    }
    return 0;
}

int
YAAFCL_IsSymlink(const char* path)
{
    struct stat stat_inf;
    if (stat(path, &stat_inf) == 0)
    {
        return S_ISLNK(stat_inf.st_mode);
    }
    return 0;
}

int
YAAFCL_Exists(const char* path)
{
    struct stat stat_inf;
    return stat(path, &stat_inf) == 0;
}

void
YAAFCL_ArchivePathToNativePath(YAAFCL_Str* pResult, const char* archivePath,
                               const char* extractPath)
{
    YAAFCL_StrClear(pResult);
    YAAFCL_StrJoinPath(pResult, extractPath, archivePath, YAAFCL_PATH_SEP_CHR);
    if (YAAFCL_PATH_SEP_CHR != YAAF_ARCHIVE_SEP_CHR)
    {
        YAAFCL_StrReplace(pResult, YAAF_ARCHIVE_SEP_STR, YAAFCL_PATH_SEP_STR);
    }
}

int
YAAFCL_ValidateAndCreateExtractPath(const char* extractDir,
                                    const char* extractPath,
                                    const int allowOverwrite)
{
    /* check if file exists */
    if (YAAFCL_Exists(extractPath))
    {
        /* check if the item is a file */
        if(YAAFCL_IsFile(extractPath) && allowOverwrite)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        /* check if directory exists */
        if (YAAFCL_Exists(extractDir))
        {
            if (YAAFCL_IsDir(extractDir))
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            /* create directory otherwise */
            return YAAFCL_MakeDir(extractDir) == YAAF_SUCCESS;
        }
    }
}

