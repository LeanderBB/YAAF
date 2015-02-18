/*
 * YAAF - Yet Another Archive Format
 * Copyright (C) 2014-2015, Leander Beernaert
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
#include "YAAF_TLS.h"
#include "YAAF.h"
#include "YAAF_Internal.h"


#if defined(YAAF_HAVE_PTHREAD_H)
#include <pthread.h>
#define YAAF_TLS_PTRHEAD 1
#elif defined(YAAF_OS_WIN) && defined(YAAF_HAVE_WINDOWS_H)
#define YAAF_TLS_WINDOWS 1
#include <windows.h>
#else
#error "No implementation of TLS for current platform"
#endif

#if defined(YAAF_TLS_PTRHEAD)

int
YAAF_TLSCreate(YAAF_TLSKey_t* key)
{
    pthread_key_t * p_tkey = (pthread_key_t*) key;
    return (pthread_key_create(p_tkey, NULL) == 0) ? YAAF_SUCCESS : YAAF_FAIL;
}

int
YAAF_TLSSet(YAAF_TLSKey_t key,
            const void *ptr)
{
    pthread_key_t tkey = (pthread_key_t) key;
    return (pthread_setspecific(tkey, ptr) == 0 ) ? YAAF_SUCCESS : YAAF_FAIL;
}

void*
YAAF_TLSGet(YAAF_TLSKey_t key)
{
    pthread_key_t tkey = (pthread_key_t) key;
    return pthread_getspecific(tkey);
}

void
YAAF_TLSDestroy(YAAF_TLSKey_t key)
{
    pthread_key_t tkey = (pthread_key_t) key;
    pthread_key_delete(tkey);
}

#elif defined(YAAF_TLS_WINDOWS)
int
YAAF_TLSCreate(YAAF_TLSKey_t* key)
{
    DWORD wkey = TlsAlloc();
    if (wkey != TLS_OUT_OF_INDEXES)
    {
        *key = wkey;
        return YAAF_SUCCESS;
    }
    else
    {
        return YAAF_FAIL;
    }
}

int
YAAF_TLSSet(YAAF_TLSKey_t key,
            const void *ptr)
{
    return (!TlsSetValue(key, (void*)ptr)) ? YAAF_FAIL : YAAF_SUCCESS;
}

void*
YAAF_TLSGet(YAAF_TLSKey_t key)
{
    return TlsGetValue(key);
}

void
YAAF_TLSDestroy(YAAF_TLSKey_t key)
{
    TlsFree(key);
}
#endif

