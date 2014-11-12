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
#include "YAAF_MemFile.h"
#include "YAAF.h"
#include "YAAF_Internal.h"

#if defined(YAAF_OS_UNIX)
#if defined(YAAF_HAVE_MMAN_H)
#include <sys/mman.h>
#else
#error "Could not find mman.h"
#endif

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int
YAAF_MemFileOpen(YAAF_MemFile* pFile,
                 const char* path)
{
    (void) pFile;
    (void) path;
    int result = YAAF_FAIL;
    size_t file_size = 0;
    int handle = -1;

    if (YAAF_GetFileSize(&file_size, path) == YAAF_SUCCESS)
    {
        int file_flags  = O_RDONLY;
        int mmap_flags = PROT_READ;

        handle = open(path, file_flags);
        if (handle  == -1)
        {
            YAAF_SetError("[YAAF MemFile] Could not open requested file");
        }
        else
        {
            void *ptr = mmap(0, file_size, mmap_flags, MAP_SHARED, handle, 0);
            if (ptr == MAP_FAILED)
            {
                YAAF_SetError("[YAAF MemFile] Failed to map file");
            }
            else
            {
                pFile->ptr = ptr;
                pFile->oshdl = handle;
                pFile->size = file_size;
                result = YAAF_SUCCESS;
            }
        }
    }
    else
    {
        YAAF_SetError("[YAAF MemFile] Could not get file size for request file");
    }

    if (!result && handle == -1)
    {
        close(handle);
    }

    return result;
}

int
YAAF_MemFileClose(YAAF_MemFile* pFile)
{
    if (pFile->ptr)
    {
        munmap((void*)pFile->ptr, pFile->size);
        close(pFile->oshdl);
    }
    return YAAF_SUCCESS;
}


#else
#error "No Implementation for memory mapped file for current platform"
#endif

