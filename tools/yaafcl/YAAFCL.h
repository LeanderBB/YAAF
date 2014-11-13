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
#ifndef __YAAFCL_H__
#define __YAAFCL_H__

#include "YAAF.h"
#include "YAAF_Archive.h"
#include "YAAFCL_StrUtil.h"
#include "YAAF_Internal.h"

#define YAAFCL_VERSION_MAJOR 1
#define YAAFCL_VERSION_MINOR 0
#define YAAFCL_VERSION_PATCH 0

enum YAAFCL_EOptions
{
  YAAFCL_OPTION_INVALID,
  YAAFCL_OPTION_LIST_ARCHIVE,
  YAAFCL_OPTION_LIST_DIRECTORY,
  YAAFCL_OPTION_EXTRACT_ARCHIVE,
  YAAFCL_OPTION_EXTRACT_PATH,
  YAAFCL_OPTION_CREATE
};

enum YAAFCL_ESwithces
{
  YAAFCL_SWITCH_RECURSIVE_BIT = 1 << 0,
  YAAFCL_SWITCH_VERBOSE_BIT = 1 << 1,
  YAAFCL_SWITCH_QUIET_BIT = 1 << 2,
  YAAFCL_SWITCH_FOLLOW_SYMLINK = 1 << 3,
  YAAFCL_SWITCH_ALLOW_FILE_OVERWRITE = 1 << 4
};

void YAAFCL_LogError(const char* error, ...);
#endif
