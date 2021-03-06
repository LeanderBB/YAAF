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
#ifndef __YAAF_HASH_H__
#define __YAAF_HASH_H__

#include "YAAF_Setup.h"
#if defined(YAAF_USE_HASH_XXHASH)
#include "YAAF_Hash_xxhash.h"
#else
#error No hashing algorithm defined
#endif

void YAAF_HashStateReset(YAAF_HashState_t* pState,
                         const uint32_t seed);

int YAAF_HashStateUpdate(YAAF_HashState_t* pState,
                         const void* input,
                         const uint32_t size);

uint32_t YAAF_HashStateDigest(YAAF_HashState_t* pState);

uint32_t YAAF_Hash(const void* input,
                   const uint32_t size,
                   const uint32_t seed);

uint32_t YAAF_OnceAtATimeHashNoCase(const char* str);


#endif
