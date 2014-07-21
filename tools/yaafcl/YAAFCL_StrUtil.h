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
#ifndef __YAAFCL_STRUTIL_H__
#define __YAAFCL_STRUTIL_H__

#include "YAAFCL.h"

/* --- String --------------------------------------------------------------*/

typedef struct
{
  size_t len;
  char* str;
} YAAFCL_Str;


void YAAFCL_StrInit(YAAFCL_Str* pStr);

void YAAFCL_StrDestroy(YAAFCL_Str* pStr);

void YAAFCL_StrResize(YAAFCL_Str* pStr, const size_t newSize,
                             const int preserveContents);

void YAAFCL_StrCopy(YAAFCL_Str* pStr, const YAAFCL_Str* pOther);

void YAAFCL_StrConcat(YAAFCL_Str* pStr, const char* other);

void YAAFCL_StrConcatStr(YAAFCL_Str* pStr, YAAFCL_Str* pOther);

void YAAFCL_StrReplace(YAAFCL_Str* pStr, const char* oldStr, const char* newStr);

void YAAFCL_StrClear(YAAFCL_Str* pStr);

void YAAFCL_StrJoinPath(YAAFCL_Str* pStr, const char* path1, const char* path2,
                        char sep);

void YAAFCL_StrMove(YAAFCL_Str* pDst, YAAFCL_Str* pSrc);

void YAAFCL_StrExtractPath(YAAFCL_Str* pPath, const YAAFCL_Str* pStr,
                           const char sep);

void YAAFCL_StrExtractName( YAAFCL_Str* pPath, const YAAFCL_Str* pStr,
                           const char sep);
/* --- String List ---------------------------------------------------------*/

typedef struct YAAFCL_SStrListNode
{
  YAAFCL_Str string;
  struct YAAFCL_SStrListNode* pNext;
} YAAFCL_StrListNode;

typedef struct
{
  YAAFCL_StrListNode* pHead;
  YAAFCL_StrListNode* pTail;
  size_t count;
} YAAFCL_StrList;

void YAAFCL_StrListInit(YAAFCL_StrList* pList);

void YAAFCL_StrListDestroy(YAAFCL_StrList* pList);

void YAAFCL_StrListPushBackCopy(YAAFCL_StrList* pList, const YAAFCL_Str* pStr);

void YAAFCL_StrListPushBackMove(YAAFCL_StrList* pList, YAAFCL_Str* pStr);

int YAAFCL_StrListPopHeadMove(YAAFCL_StrList* pList, YAAFCL_Str* pStr);

void YAAFCL_StrListPopHead(YAAFCL_StrList* pList);

const YAAFCL_Str* YAAFCL_StrListPeekHead(YAAFCL_StrList* pList);

#endif
