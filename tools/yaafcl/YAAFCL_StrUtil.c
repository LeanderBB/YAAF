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
#include "YAAFCL_StrUtil.h"


void
YAAFCL_StrResize(YAAFCL_Str* pStr,
                 const size_t newSize,
                 const int preserveContents)
{
    if (pStr->len && newSize <= pStr->len)
    {
        if (!preserveContents)
        {
            YAAFCL_StrClear(pStr);
        }
        return;
    }

    YAAF_ASSERT(newSize > 0);
    char* new_data = (char*)YAAF_malloc(newSize);
    pStr->len = newSize - 1;
    if (preserveContents && pStr->str)
    {
        new_data[pStr->len] = '\0';
        memcpy(new_data, pStr->str, pStr->len);
    }
    if (!preserveContents)
    {
        new_data[0] = '\0';
    }

    YAAF_free(pStr->str);
    pStr->str = new_data;
}

void
YAAFCL_StrInit(YAAFCL_Str* pStr)
{
    memset(pStr, 0, sizeof(YAAFCL_Str));
}

void
YAAFCL_StrDestroy(YAAFCL_Str* pStr)
{
    if (pStr->str)
    {
        YAAF_free(pStr->str);
    }
    memset(pStr, 0, sizeof(YAAFCL_Str));
}

void
YAAFCL_StrCopy(YAAFCL_Str* pStr,
               const YAAFCL_Str* pOther)
{
    YAAFCL_StrResize(pStr, pOther->len + 1, 0);
    memcpy(pStr->str, pOther->str, pOther->len + 1);
    pStr->len = pOther->len;
}

void
YAAFCL_StrConcat(YAAFCL_Str* pStr,
                 const char* other)
{
    size_t other_size = strlen(other);
    if (other_size > 0)
    {
        size_t cur_len = pStr->len;
        YAAFCL_StrResize(pStr, pStr->len + other_size + 1, 1);
        memcpy(pStr->str + cur_len, other, other_size + 1);
        pStr->len = other_size + cur_len;
    }
}

void
YAAFCL_StrConcatStr(YAAFCL_Str* pStr,
                    YAAFCL_Str* pOther)
{
    size_t cur_len = pStr->len;
    YAAFCL_StrResize(pStr, pStr->len + pOther->len + 1, 1);
    memcpy(pStr->str + cur_len, pOther->str, pOther->len + 1);
    pStr->len = cur_len + pOther->len;
}

void
YAAFCL_StrReplace(YAAFCL_Str* pStr,
                  const char* oldStr,
                  const char* newStr)
{
    if (pStr && pStr->len && oldStr && newStr)
    {
        size_t token_len, rep_len, i, j, count;

        token_len = strlen(oldStr);
        rep_len = strlen(newStr);

        if (token_len == 0 )
            return;

        count = 0;
        /* count the number of replacements */
        for (i = 0; i < pStr->len; ++i)
        {
            if (strncmp(pStr->str + i, oldStr, token_len) == 0) {
                count++;
                i += token_len - 1;
            }
        }

        if (count)
        {
            YAAFCL_Str old;
            YAAFCL_StrInit(&old);
            YAAFCL_StrCopy(&old, pStr);
            if (rep_len > token_len)
            {
                YAAFCL_StrResize(pStr, pStr->len + ((rep_len - token_len) * (count + 2)), 0);
            }
            /*
      else
      {
        YAAFCL_StrClear(pStr);
      }
      */
            /* replace data */

            for (i = 0, j = 0; i < old.len; ++i)
            {
                if (strncmp(old.str + i, oldStr, token_len) == 0)
                {
                    if(rep_len)
                    {
                        memcpy(pStr->str + j, newStr, rep_len);
                    }
                    i += token_len - 1;
                    j += rep_len;
                } else {
                    pStr->str[j] = old.str[i];
                    j++;
                }
            }

            pStr->len = j;
            pStr->str[j] = '\0';
            YAAFCL_StrDestroy(&old);
        }
    }
}

void
YAAFCL_StrClear(YAAFCL_Str* pStr)
{
    pStr->len = 0;
    if(pStr->str)
    {
        YAAF_free(pStr->str);
        pStr->str = NULL;
    }
}

void
YAAFCL_StrJoinPath(YAAFCL_Str* pStr,
                   const char* path1,
                   const char* path2,
                   char sep)
{
    size_t path1_len = strlen(path1);
    size_t path2_len = strlen(path2);
    YAAFCL_StrResize(pStr, path1_len + path2_len + 2, 0);
    memcpy(pStr->str, path1, path1_len);
    if (path1[path1_len - 1] != sep)
    {
        pStr->str[path1_len] = sep;
    }
    else
    {
        --path1_len;
    }
    memcpy(pStr->str + (path1_len + 1), path2, path2_len + 1);
    pStr->len = path1_len + path2_len + 1;
}

void YAAFCL_StrMove(YAAFCL_Str* pDst, YAAFCL_Str* pSrc)
{
    YAAFCL_StrDestroy(pDst);
    pDst->len = pSrc->len;
    pDst->str = pSrc->str;
    pSrc->len = 0;
    pSrc->str = NULL;
}

void
YAAFCL_StrExtractPath(YAAFCL_Str* pPath,
                      const YAAFCL_Str* pStr,
                      const char sep)
{
    YAAFCL_StrClear(pPath);
    if (pStr->len)
    {
        for (size_t i = pStr->len - 1; i > 0; --i)
        {
            if (pStr->str[i] == sep)
            {
                YAAFCL_StrResize(pPath, i + 1, 0);
                memcpy(pPath->str, pStr->str, i);
                pPath->str[i] = '\0';
                break;
            }
        }
    }
}

void
YAAFCL_StrExtractName(YAAFCL_Str* pPath,
                      const YAAFCL_Str* pStr,
                      const char sep)
{
    YAAFCL_StrClear(pPath);
    if (pStr->len)
    {
        size_t i;
        for ( i = pStr->len - 1; i > 0 && pStr->str[i] != sep; --i)
        {
        }
        if (pStr->str[i] == sep)
        {
            size_t len = pStr->len - i;
            YAAFCL_StrResize(pPath, len + 1, 0);
            memcpy(pPath->str, pStr->str + i + 1, len);
            pPath->str[len] = '\0';
        }
    }
}


void
YAAFCL_StrListInit(YAAFCL_StrList* pList)
{
    memset(pList, 0, sizeof(YAAFCL_StrList));
}

void
YAAFCL_StrListDestroy(YAAFCL_StrList* pList)
{
    while(pList->pHead)
    {
        YAAFCL_StrListPopHead(pList);
    }
}

static void
YAAFCL_StrListPushBack(YAAFCL_StrList* pList,
                       YAAFCL_StrListNode* pNode)
{
    ++pList->count;
    if(!pList->pHead)
    {
        YAAF_ASSERT(pList->count == 1);
        pList->pHead = pNode;
        pList->pTail = pNode;
    }
    else
    {
        pList->pTail->pNext = pNode;
        pList->pTail = pNode;
    }
}

void
YAAFCL_StrListPushBackCopy(YAAFCL_StrList* pList,
                           const YAAFCL_Str* pStr)
{
    YAAFCL_StrListNode* p_node = (YAAFCL_StrListNode*)YAAF_malloc(sizeof(YAAFCL_StrListNode));
    p_node->pNext = NULL;
    YAAFCL_StrInit(&p_node->string);
    YAAFCL_StrCopy(&p_node->string, pStr);
    YAAFCL_StrListPushBack(pList, p_node);
}

void
YAAFCL_StrListPushBackMove(YAAFCL_StrList* pList,
                           YAAFCL_Str* pStr)
{
    YAAFCL_StrListNode* p_node = (YAAFCL_StrListNode*)YAAF_malloc(sizeof(YAAFCL_StrListNode));
    p_node->pNext = NULL;
    YAAFCL_StrInit(&p_node->string);
    YAAFCL_StrMove(&p_node->string, pStr);
    YAAFCL_StrListPushBack(pList, p_node);
}

int
YAAFCL_StrListPopHeadMove(YAAFCL_StrList* pList,
                          YAAFCL_Str* pStr)
{
    if (pList->pHead)
    {
        YAAFCL_StrMove(pStr, &pList->pHead->string);
        YAAFCL_StrListPopHead(pList);
        return YAAF_SUCCESS;
    }
    return YAAF_FAIL;
}

void
YAAFCL_StrListPopHead(YAAFCL_StrList* pList)
{
    if (pList->pHead)
    {
        YAAFCL_StrDestroy(&pList->pHead->string);
        YAAFCL_StrListNode * p_aux = pList->pHead->pNext;
        YAAF_free(pList->pHead);
        pList->pHead = p_aux;
        if (!p_aux)
        {
            pList->pTail = NULL;
        }
        --pList->count;
    }
}

const YAAFCL_Str*
YAAFCL_StrListPeekHead(YAAFCL_StrList* pList)
{
    return(pList->pHead) ? &pList->pHead->string : NULL;
}

