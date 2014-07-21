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
#include "YAAF_Internal.h"
/* --------------------------------------------------------------------------*/
static YAAF_Allocator YAAF_gpAllocator;
/* --------------------------------------------------------------------------*/
static const char* YAAF_gErrorStr = NULL;
/* --------------------------------------------------------------------------*/
const char* YAAF_GetError()
{
  return YAAF_gErrorStr;
}
/* --------------------------------------------------------------------------*/
void YAAF_SetError(const char* error)
{
  YAAF_gErrorStr = error;
}
/* --------------------------------------------------------------------------*/
void YAAF_SetAllocator(const YAAF_Allocator* pAlloc)
{
  YAAF_gpAllocator.malloc = pAlloc->malloc;
  YAAF_gpAllocator.calloc = pAlloc->calloc;
  YAAF_gpAllocator.free = pAlloc->free;
}
/* --------------------------------------------------------------------------*/
void* YAAF_malloc(size_t size)
{
  return YAAF_gpAllocator.malloc(size);
}
/* --------------------------------------------------------------------------*/
void YAAF_free(void* ptr)
{
  YAAF_gpAllocator.free(ptr);
}
/* --------------------------------------------------------------------------*/
void* YAAF_calloc(size_t nmb, size_t size)
{
  return YAAF_gpAllocator.calloc(nmb, size);
}
/* --------------------------------------------------------------------------*/
int YAAF_StrCompareNoCase(const char* str1, const char* str2)
{
#if defined(YAAF_HAVE_STRCASECMP)
  return strcasecmp(str1, str2);
#elif defined(YAAF_HAVE_STRICMP)
  return stricmp(str1, str2);
#else
#error No implementation of string case insentive compare
#endif
}
/* --------------------------------------------------------------------------*/
int YAAF_StrContainsChr(const char* str, const char chr)
{
   while(*str != '\0')
   {
     if (*str == chr)
     {
       return 1;
     }
     ++str;
   }
   return 0;
}
/* --------------------------------------------------------------------------*/
