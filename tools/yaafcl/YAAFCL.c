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
#include "YAAFCL_Job.h"
#include <stdarg.h>

static int g_AllowErrorLog = 1;

/* --------------------------------------------------------------------------*/
void YAAFCL_LogError(const char* error,...)
{
  if (g_AllowErrorLog)
  {
    va_list va;
    va_start(va, error);
    vfprintf(stderr,error, va);
    va_end(va);
  }
}



/* --------------------------------------------------------------------------*/
static int YAAFCL_CreateArchiveFromPaths(const int argc,
                                         char** argv,
                                         const int flags)
{

  YAAFCL_DirEntryStack dir_stack;
  int result = YAAF_FAIL;
  YAAFCL_DirEntryStackInit(&dir_stack);
  FILE* p_output = NULL;
  int i;
  /* check args */
  if (argc < 2)
  {
    YAAFCL_LogError("[Create Archive] Usage: yaafcl -c [archive] [path 1] ... [path n]\n");
    goto exit;
  }

  /* scan path for entries */

  for (i = 1; i < argc; ++i)
  {

    if (YAAFCL_IsFile(argv[i]))
    {
      YAAFCL_Str full_path, path_component, name_component;
      int result;

      YAAFCL_StrInit(&full_path);
      YAAFCL_StrInit(&path_component);
      YAAFCL_StrInit(&name_component);

      if ((result = YAAFCL_RealPath(&full_path, argv[i]) )== YAAF_FAIL)
      {
        YAAFCL_LogError("[Create Archive] Failed to get real path for file \"%s\"\n", argv[i]);
        goto cleanup;
      }

      YAAFCL_StrExtractPath(&path_component, &full_path, YAAFCL_PATH_SEP_CHR);
      YAAFCL_StrExtractName(&name_component, &full_path, YAAFCL_PATH_SEP_CHR);

      result = YAAFCL_AddFileToEntryStack(&dir_stack, path_component.str,
                                          name_component.str, NULL);

      cleanup:
      YAAFCL_StrDestroy(&full_path);
      YAAFCL_StrDestroy(&path_component);
      YAAFCL_StrDestroy(&name_component);

      if(result == YAAF_FAIL)
      {
        goto exit;
      }
    }
    else if(YAAFCL_IsDir(argv[i]))
    {

      if (YAAFCL_ScanDirectory(&dir_stack, argv[i], flags) != YAAF_SUCCESS)
      {
        YAAFCL_LogError("[Create Archive] Failed to scan directory \"%s\"\n", argv[i]);
        goto exit;
      }
    }
    else if(YAAFCL_IsSymlink(argv[i]) && (flags & YAAFCL_SWITCH_FOLLOW_SYMLINK))
    {
      YAAFCL_LogError("[Create Archive] Reading symlinks not supported yet");
      goto exit;
    }
    else
    {
      YAAFCL_LogError("[Create Archive] Unknown file type \"%s\"\n", argv[i]);
      goto exit;
    }
  }

  /* debug log*/
  if (flags & (YAAFCL_SWITCH_VERBOSE_BIT))
  {
    printf("[Create Archive] Found %d entries to be added.\n",
           (uint32_t)dir_stack.count);

    uint32_t i = 0;
    /* print all entries */
    YAAFCL_DirEntryStackNode* p_node = dir_stack.pNodes;
    while(p_node)
    {
      printf("[Create Archive] Entry %d:\n", i);
      printf("\t Full Path: %s\n", p_node->pEntry->fullPath.str);
      printf("\t Arch Path: %s\n", p_node->pEntry->archivePath.str);
      p_node = p_node->pNext;
      ++ i;
    }
  }

  /* create ouput archive */
  p_output = fopen(argv[0], "wb");
  if (!p_output)
  {
    YAAFCL_LogError("[Create Archive] Failed to open archive \"%s\"\n", argv[0]);
    goto exit;
  }


  /* compress and write files */
  result = YAAFCL_JobCompress(p_output, &dir_stack);
  fclose(p_output);

  result = YAAF_SUCCESS;

exit:

  YAAFCL_DirEntryStackDestroy(&dir_stack);
  return result;
}
/* --------------------------------------------------------------------------*/
static int YAAFCL_ListArchive(const int argc, char** argv, const int flags)
{
  int i = 0;
  int result = YAAF_SUCCESS;
  for ( ; i < argc && result == YAAF_SUCCESS; ++ i)
  {
    YAAF_Archive* p_archive = NULL;

    p_archive = YAAF_ArchiveOpen(argv[i]);
    if (!p_archive)
    {
      if (!(flags & YAAFCL_SWITCH_QUIET_BIT))
      {
        YAAFCL_LogError("[List Archive] Failed to parse archive \"%s\"\n", argv[i]);
      }
      goto exit;
    }

    const char** list = YAAF_ArchiveListAll(p_archive);
    if (!list)
    {
      if (!(flags & YAAFCL_SWITCH_QUIET_BIT))
      {
        YAAFCL_LogError("[List Archive] Failed to list archive \"%s\" - %s\n", argv[i], YAAF_GetError());
      }
      goto exit;
    }
    const char** aux = list;
    while(*aux)
    {
      printf("%s :: %s\n", argv[i], *aux);
      ++aux;
    }
    YAAF_ArchiveFreeList(p_archive, list);
    result = YAAF_SUCCESS;
exit:
    if(p_archive)
    {
      YAAF_ArchiveClose(p_archive);
    }
  }
  return result;
}
/* --------------------------------------------------------------------------*/
static int YAAFCL_ListArchiveDir(const int argc, char** argv, const int flags)
{
  int i;
  int result = YAAF_FAIL;
  if ( argc < 2)
  {
    YAAFCL_LogError("[List ArchiveDir] List Archive Directory usage: yaafcl -l [archive] [dir 1] ... [dir n]\n");
    return YAAF_FAIL;
  }

  YAAF_Archive* p_archive = NULL;

  p_archive = YAAF_ArchiveOpen(argv[0]);
  if (!p_archive)
  {
    if (!(flags & YAAFCL_SWITCH_QUIET_BIT))
    {
      YAAFCL_LogError("[List ArchiveDir] Failed to parse archive \"%s\"\n", argv[0]);
    }
    goto exit;
  }
  for ( i = 1; i < argc; ++ i)
  {
    const char** list = YAAF_ArchiveListDir(p_archive, argv[i]);
    if (!list)
    {
      if (!(flags & YAAFCL_SWITCH_QUIET_BIT))
      {
        YAAFCL_LogError("[List ArchiveDir] Failed to list archive \"%s\" - %s\n", argv[0], YAAF_GetError());
      }
      goto exit;
    }
    const char** aux = list;
    while(*aux)
    {
      printf("%s\n",*aux);
      ++aux;
    }
    YAAF_ArchiveFreeList(p_archive, list);
  }
  result = YAAF_SUCCESS;
exit:
  if(p_archive)
  {
    YAAF_ArchiveClose(p_archive);
  }

  return result;
}
/* --------------------------------------------------------------------------*/
static int YAAFCL_ExtractArchive(const int argc, char** argv, const int flags)
{
  if ( argc < 2)
  {
    YAAFCL_LogError("[Extract Archive] Usage: yaafcl -E [archive] [output directory]\n");
    return YAAF_FAIL;
  }

  return YAAFCL_JobDecompressArchive(argv[0], argv[1], flags);
}
/* --------------------------------------------------------------------------*/
static int YAAFCL_ExtractFile(const int argc, char** argv, const int flags)
{  
  YAAF_Archive* p_archive = NULL;
  int result = YAAF_FAIL;
  YAAF_File* p_file = NULL;
  FILE* p_fout = NULL;
  int i = 0;

  if (argc < 3)
  {
    YAAFCL_LogError("[Extract File] Usage: yaafcl -E [archive] [output directory] [file 1] .. [file n]\n");
    return YAAF_FAIL;
  }

  p_archive = YAAF_ArchiveOpen(argv[0]);
  if (!p_archive)
  {
    YAAFCL_LogError("[Extract File] Failed to parse archive \"%s\" - %s\n", argv[0], YAAF_GetError());
    goto exit;
  }

  result = YAAF_SUCCESS;
  for (i = 2; i < argc && result == YAAF_SUCCESS; ++i)
  {
    p_file = YAAF_ArchiveFile(p_archive, argv[i]);
    if (p_file)
    {
      YAAFCL_Str out_path;
      YAAFCL_Str name_component;
      YAAFCL_Str entry_name;
      YAAFCL_StrInit(&name_component);
      YAAFCL_StrInit(&out_path);
      YAAFCL_StrInit(&entry_name);
      YAAFCL_StrConcat(&entry_name, argv[i]);
      YAAFCL_StrExtractName(&name_component, &entry_name, YAAF_ARCHIVE_SEP_CHR);
      YAAFCL_StrJoinPath(&out_path, argv[1], name_component.str, YAAFCL_PATH_SEP_CHR);
      YAAFCL_StrDestroy(&name_component);
      YAAFCL_StrDestroy(&entry_name);
      
      if (YAAFCL_Exists(out_path.str) && !(flags & YAAFCL_SWITCH_ALLOW_FILE_OVERWRITE))
      {
        YAAFCL_LogError("[Extract File] Failed to extract \"%s\" to \"%s\", file already exists. Add overwrite switch to override\n", argv[i], out_path.str);
        YAAFCL_StrDestroy(&out_path);
        result = YAAF_FAIL;
        goto exit;
      }
      
      p_fout = fopen(out_path.str, "wb");

      if (!p_fout)
      {
        YAAFCL_LogError("[Extract File] Failed to extract \"%s\" to \"%s\"\n", argv[i], out_path.str);
        YAAFCL_StrDestroy(&out_path);
        result = YAAF_FAIL;
        goto exit;
      }
      YAAFCL_StrDestroy(&out_path);

      char buffer[1024];
      size_t bytes_read, bytes_written;
      while (!YAAF_FileEOF(p_file))
      {
        bytes_read = YAAF_FileRead(p_file, buffer, 1024);
        if (bytes_read)
        {
          bytes_written = fwrite(buffer, 1, bytes_read, p_fout);
          if (bytes_written != bytes_read)
          {
            fprintf(stderr,"[Extract File] Failed to write bytes to output:'%s'\n", YAAF_GetError());
            result = YAAF_FAIL;
            goto exit;
          }
        }
      }
      YAAF_FileDestroy(p_file);
      fclose(p_fout);
      p_fout = NULL;
      p_file = NULL;
    }
    else
    {
      YAAFCL_LogError("[Extract File] Failed to open entry \"%s\" - %s\n", argv[i], YAAF_GetError());
      result = YAAF_FAIL;
      goto exit;
    }
  }
  
exit:
  if(p_archive)
  {
    YAAF_ArchiveClose(p_archive);
  }
  if (p_file)
  {
    YAAF_FileDestroy(p_file);
  }
  if (p_fout)
  {
    fclose(p_fout);
  }

  return result;
}
/* --------------------------------------------------------------------------*/
static void YAAFCL_PrintVersion()
{
  printf("Copyright (c) 2014 - Leander Beernaert\n");
  printf("yaafcl %d.%d.%d (with yaaf %d.%d.%d) \n",
         YAAFCL_VERSION_MAJOR, YAAFCL_VERSION_MINOR, YAAFCL_VERSION_PATCH,
         YAAF_VERSION_MAJOR, YAAF_VERSION_MINOR, YAAF_VERSION_PATCH);
  printf("\n");
}
/* --------------------------------------------------------------------------*/
static void YAAFCL_PrintHelp()
{
  YAAFCL_PrintVersion();

  printf("Usage: [options] [switches] [archive] [arguments]\n");
  printf("\noptions:\n");
  printf("  -l : List directories specified in [arguments] in the archive\n");
  printf("  -L : List all contents of the [archive]\n");
  printf("  -c : Create archive with the files specified in [arguments]\n");
  printf("  -E : Extract [archive] into location specified in [arguments]\n");
  printf("  -e : Extract a file or directory from [archive]\n");
  printf("  -h : Print this text\n");
  printf("  -v : Print version\n");
  printf("\nswitches:\n");
  printf("  -r : Recursive operation\n");
  printf("  -q : Quiet mode, do not log any output\n");
  printf("  -w : Overwrite existing files when creating an archive or extracting\n");
  printf("  -V : Verbose\n");

  printf("\n");
}
/* --------------------------------------------------------------------------*/
static int YAAFCL_ParseArgs(const int argc, char** argv)
{
  int i = 1, flags = 0, remaining_argc = 0;
  unsigned int option = YAAFCL_OPTION_INVALID;
  (void) argc;

  if (argc < 2)
  {
    YAAFCL_PrintHelp();
    return YAAF_FAIL;
  }

  if (strcmp(argv[i],"-h") == 0)
  {
    YAAFCL_PrintHelp();
    return YAAF_SUCCESS;
  }
  else if (strcmp(argv[i],"-v") == 0)
  {
    YAAFCL_PrintVersion();
    return YAAF_SUCCESS;
  }
  else if (strcmp(argv[i],"-L") == 0)
  {
    option = YAAFCL_OPTION_LIST_ARCHIVE;
  }
  else if (strcmp(argv[i],"-l") == 0)
  {
    option = YAAFCL_OPTION_LIST_DIRECTORY;
  }
  else if (strcmp(argv[i],"-c") == 0)
  {
    option = YAAFCL_OPTION_CREATE;
  }
  else if(strcmp(argv[i], "-E") == 0)
  {
    option = YAAFCL_OPTION_EXTRACT_ARCHIVE;
  }
  else if(strcmp(argv[i], "-e") == 0)
  {
    option = YAAFCL_OPTION_EXTRACT_PATH;
  }
  else
  {
    fprintf(stderr,"%s - Unknown option '%s'\n", argv[0], argv[i]);
    return YAAF_FAIL;
  }

  /* go over switches */
  ++i;
  while( i < argc && argv[i][0] == '-')
  {
    if (strcmp(argv[i],"-r") == 0)
    {
      flags |= YAAFCL_SWITCH_RECURSIVE_BIT;
    }
    else if (strcmp(argv[i],"-q") == 0)
    {
      flags &= ~YAAFCL_SWITCH_VERBOSE_BIT;
      flags |= YAAFCL_SWITCH_QUIET_BIT;
    }
    else if (strcmp(argv[i],"-V") == 0)
    {
      flags |= YAAFCL_SWITCH_VERBOSE_BIT;
      flags &= ~YAAFCL_SWITCH_QUIET_BIT;
    }
    else if(strcmp(argv[i], "-w") == 0)
    {
      flags |= YAAFCL_SWITCH_ALLOW_FILE_OVERWRITE;
    }
    /*
    else if (strcmp(argv[i],"-s") == 0)
    {
      flags |= YAAFCL_SWITCH_STOP_ON_ERROR_BIT;
    }*/
    else
    {
      fprintf(stderr,"%s - Unknown switch '%s'", argv[0], argv[i]);
      return YAAF_FAIL;
    }
    ++i;
  }


  
  if (flags & YAAFCL_SWITCH_QUIET_BIT)
  {
    g_AllowErrorLog = 0;
  }
  
  remaining_argc = argc - i;

  if (i <= 0)
  {
    fprintf(stderr,"%s - No archive specified", argv[0]);
    return YAAF_FAIL;
  }

  /* execute commands */
  switch(option)
  {
  case YAAFCL_OPTION_LIST_ARCHIVE:
    return YAAFCL_ListArchive(remaining_argc, argv + i, flags);
  case YAAFCL_OPTION_LIST_DIRECTORY:
    return YAAFCL_ListArchiveDir(remaining_argc, argv + i, flags);
  case YAAFCL_OPTION_CREATE:
    return YAAFCL_CreateArchiveFromPaths(remaining_argc, argv + i, flags);
  case YAAFCL_OPTION_EXTRACT_ARCHIVE:
    return YAAFCL_ExtractArchive(remaining_argc, argv + i, flags);
  case YAAFCL_OPTION_EXTRACT_PATH:
    return YAAFCL_ExtractFile(remaining_argc, argv +i, flags);        
  default:
    fprintf(stderr,"%s - Option doesn't exist or is not implemented", argv[0]);
    return YAAF_FAIL;
  }

  return YAAF_FAIL;
}
/* --------------------------------------------------------------------------*/
int main(int argc, char** argv)
{
  if (YAAF_FAIL == YAAF_Init(NULL))
  {
      fprintf(stderr,"%s - Failed to init YAAF\n", argv[0]);
      return EXIT_FAILURE;
  }
  int res = YAAFCL_ParseArgs(argc, argv);

  YAAF_Shutdown();
  return (res == YAAF_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
