/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2023  SEGGER Microcontroller GmbH                 *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File        : IP_FS_emFile.c
Purpose     : Implementation of emFile file system layer.
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <string.h>
#include "IP_FS.h"
#include "FS.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#define   MAX_PATH                128

#ifndef   FS_DIRECTORY_DELIMITER
  #define FS_DIRECTORY_DELIMITER  '\\'  // Directory delimiter used by the underlying filesystem.
#endif

#ifndef   IP_FS_VOLUMENAME
  #define IP_FS_VOLUMENAME        ""    // Label of the volume to use. Example: "mmc:". If set to NULL all volumes configured in emFile will be visible.
#endif

#ifndef   IP_FS_ROOT_PATH
  #define IP_FS_ROOT_PATH         ""    // Path to use as root directory for the file system. Example: "\\SUB\\FOLDER" .
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#ifndef   DEBUG
  #define DEBUG                   0
#endif

#define   UL_DIRECTORY_DELIMITER  '/'  // Directory delimiter used by the upper layer such as Web or FTP server.

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static U8 _IsInited;

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/

static const char * _sVolumeName = IP_FS_VOLUMENAME;
static const char * _sPath       = IP_FS_ROOT_PATH;

/*********************************************************************
*
*       _FormatIfRequired
*/
static void _FormatIfRequired(const char * sVolumeName) {
  FS_FormatLLIfRequired(sVolumeName);
  //
  // Check if volume needs to be high level formatted.
  //
  if (FS_IsHLFormatted(sVolumeName) == 0) {
#if DEBUG
    printf("High-level formatting: %s\n", sVolumeName);
#endif
    FS_Format(sVolumeName, NULL);
  }
}

/*********************************************************************
*
*       _VolumeNameToDirName
*
*  Function description
*    Removes ':' from the volume name to avoid problems on the file
*    system with the application.
*/
static void _VolumeNameToDirName(char * sVolumeName) {
  char * pSrc;
  char * pDest;
  char   c;

  pSrc  = sVolumeName;
  pDest = sVolumeName;
  while (1) {
    c = *pSrc++;
    if (c == 0) {
      *pDest = 0;
      break;
    }
    if (c != ':') {
      *pDest++ = c;
    }
  }
}

/*********************************************************************
*
*       _InitIfRequired
*/
static void _InitIfRequired(void) {
  if (_IsInited == 0) {
    char acVolumeName[32];
    int  NumVolumes;
    int  iVolume;

    FS_Init();
    if (_sVolumeName) {
      _FormatIfRequired(_sVolumeName);
    } else {
      memset(acVolumeName, 0, sizeof(acVolumeName));
      NumVolumes = FS_GetNumVolumes();
      for (iVolume = 0; iVolume < NumVolumes; ++iVolume) {
        FS_GetVolumeName(iVolume, acVolumeName, sizeof(acVolumeName));
        _FormatIfRequired(acVolumeName);
      }
    }
    //
    // Enable long file name support if LFN package is available.
    // LFN is an optional emFile package!
    //
//    FS_FAT_SupportLFN();
    _IsInited = 1;
  }
}

/*********************************************************************
*
*       _ConvertPath
*
*  Function description
*    Converts the URL encoded path into a valid file system path.
*    Typically the only change applied is to replace '/' with '\'.
*
*    A fully qualified file name looks like:
*    [DevName:[UnitNum:]][DirPathList]Filename
*
*  Samples
*    _sVolumeName | sFilename   | sAbsFilename
*    --------------------------------------------------
*    ""           | "/TEST"     | "TEST"
*    ""           | "/TEST/"    | "TEST"
*    ""           | "/Test.htm" | "Test.htm"
*    "ram:"       | "/Test.htm" | "ram:Test.htm"
*
*  Return value
*       0          - O.K.
*    != 0          - Error
*/
static int _ConvertPath(const char * sFilename, char * sOutFilename, U32 BufferSize) {
  char c;
  char PrependVolume;
  char PrependPath;
  const char * p;

  PrependVolume = 0;
  PrependPath   = 0;
  //
  // Do not allow '\' (comes as '/') as first character as not supported by file system. In this case skip it.
  // In case we prepend a root path this is not necessary.
  //
  if ((*_sPath == '\0') && (*sFilename == UL_DIRECTORY_DELIMITER)) {  // No root path to prepend and starts with '/' ?
    sFilename++;                                                      // ==> Skip the first '/'.
  }
  //
  // Prepare for simply converting the given path or for converting the path and prepending a volume label and/or path.
  //
  if (_sVolumeName) {
    if (*_sVolumeName == 0) {
      if (*_sPath == 0) {
        p = sFilename;
      } else {
        PrependPath = 1;
        p = _sPath;
      }
    } else {
      PrependVolume = 1;
      p = _sVolumeName;
    }
  } else {
    //
    // Convert the first directory in the path to a volume name.
    // Ingnore the root directory. This is the case if volumes
    // have been automatically converted to directory names when
    // listing the "volume root" and a volume "mmc:0:" for example
    // was converted to "mmc0" by stripping out all colons. We then
    // have to restore the colons to biuld the real FS volume again.
    //
    // The other case is when the "application" prepends the path
    // with a real volume, resulting in a filename such as "mmc:0:/index.html".
    // In this case we detect colons being used and simply copy the
    // volume as is.
    //
    p = sFilename;
    if (BufferSize && *p) {
      int FoundUnitNo;
      int FoundColon;

      FoundUnitNo = 0;
      FoundColon  = 0;

      do {
        if (--BufferSize == 0) {
          break;
        }
        c = *p;
        if (c == ':') {
          FoundColon = 1;
        }
        if ((c == 0) || (c == UL_DIRECTORY_DELIMITER)) {             // End of first directory name?
          if (FoundColon == 0) {
            *sOutFilename++ = ':';
          }
          break;
        }
        if ((FoundUnitNo == 0) && (FoundColon == 0)) {
          if ((c >= '0') && (c <= '9')) {
            *sOutFilename++ = ':';
            FoundUnitNo = 1;
            continue;
          }
        }
        *sOutFilename++ = c;
        ++p;
      } while (1);
    }
  }
  //
  // Convert (and prepend volume label if needed)
  //
  if (BufferSize) {
    do {
      if (--BufferSize == 0) {
        break;                  // Buffer full. We have to stop.
      }
      c = *p++;
      if (c == 0) {
        if        (PrependVolume) {
          PrependVolume = 0;
          PrependPath   = 1;
          p = _sPath;
          continue;             // End of volume label, continue with path
        } else if (PrependPath) {
          PrependPath = 0;
          p = sFilename;
          continue;             // End of path, continue with filename
        }
        break;                  // End of string
      }
#if (UL_DIRECTORY_DELIMITER != FS_DIRECTORY_DELIMITER)
      if (c == UL_DIRECTORY_DELIMITER) {
        c = FS_DIRECTORY_DELIMITER;
      }
#endif
      *sOutFilename++ = c;
    } while (1);
  }
  //
  // Terminate string. If the path is a directory remove the trailing slash.
  //
  if (*(sOutFilename - 1) == FS_DIRECTORY_DELIMITER) {
    sOutFilename--;
    *sOutFilename = 0;
  } else {
    *sOutFilename++ = 0;
  }
  return 0;
}

/*********************************************************************
*
*       _FS_OpenEx()
*/
static void * _FS_OpenEx(const char* sFilename, int AllowHiddenAccess) {
  char acAbsFilename[MAX_PATH];
  U8   Attrib;

  _InitIfRequired();  // Perform automatic initialisation so that explicit call to FS_Init is not required
  _ConvertPath(sFilename, acAbsFilename, sizeof(acAbsFilename));
  Attrib = FS_GetFileAttributes(acAbsFilename);
  if (Attrib != 0xFF) {
    if ((Attrib & FS_ATTR_HIDDEN) != 0) {
      if (AllowHiddenAccess == 0) {
        return NULL;  // Access to hidden file not permitted.
      }
    }
  }
  return FS_FOpen(acAbsFilename, "r");
}

/*********************************************************************
*
*       _FS_Open()
*/
static void * _FS_Open(const char *sFilename) {
  return _FS_OpenEx(sFilename, 1);
}

/*********************************************************************
*
*       _FS_Open_NoHidden()
*/
static void * _FS_Open_NoHidden(const char *sFilename) {
  return _FS_OpenEx(sFilename, 0);
}

/*********************************************************************
*
*       _Close
*/
static int _Close(void * hFile) {
  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  return FS_FClose((FS_FILE*) hFile);
}

/*********************************************************************
*
*       _ReadAt
*/
static int _ReadAt(void * hFile, void *pDest, U32 Pos, U32 NumBytes) {
  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  FS_FSeek((FS_FILE*) hFile, Pos, FS_SEEK_SET);
  FS_Read((FS_FILE*) hFile, pDest, NumBytes);
  return 0;
}

/*********************************************************************
*
*       _GetLen
*/
static long _GetLen(void * hFile) {
  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  return FS_GetFileSize((FS_FILE*) hFile);
}

/*********************************************************************
*
*       _ForEachDirEntryEx()
*/
static void _ForEachDirEntryEx(void * pContext, const char * sDir, void (*pf) (void * pContext, void * pFileEntry), int ShowHidden) {
  FS_FIND_DATA fd;
  char acDirname[MAX_PATH];
  char acFilename[MAX_PATH];
  U8   Attrib;

  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  _ConvertPath(sDir, acDirname, sizeof(acDirname));
  if (acDirname[0] != 0) {
    Attrib = FS_GetFileAttributes(acDirname);
    if (Attrib != 0xFF) {
      if ((Attrib & FS_ATTR_HIDDEN) != 0) {
        if (ShowHidden == 0) {
          return;  // Access to hidden folder not permitted.
        }
      }
    }
  }
  if (_sVolumeName || (acDirname[0] != 0)) {  // No multi volume support enabled OR not root directory
    if (FS_FindFirstFile(&fd, acDirname, acFilename, sizeof(acFilename)) == 0) {
      do {
        if ((((fd.Attributes & FS_ATTR_HIDDEN) != 0) && (ShowHidden != 0)) ||  // Hidden but allowed to show.
             ((fd.Attributes & FS_ATTR_HIDDEN) == 0)) {                        // Not hidden at all.
          pf(pContext, &fd);
        }
      } while (FS_FindNextFile (&fd));
    }
    FS_FindClose(&fd);
  } else {
    char acVolumeName[32];
    int  NumVolumes;
    int  iVolume;

    memset(&fd, 0, sizeof(fd));
    fd.sFileName  = acVolumeName;
    fd.Attributes = FS_ATTR_DIRECTORY;
    NumVolumes = FS_GetNumVolumes();
    for (iVolume = 0; iVolume < NumVolumes; ++iVolume) {
      FS_GetVolumeName(iVolume, acVolumeName, sizeof(acVolumeName));
      _VolumeNameToDirName(acVolumeName);
      pf(pContext, &fd);
    }
  }
}

/*********************************************************************
*
*       _ForEachDirEntry()
*/
static void _ForEachDirEntry(void * pContext, const char * sDir, void (*pf) (void * pContext, void * pFileEntry)) {
  _ForEachDirEntryEx(pContext, sDir, pf, 1);
}

/*********************************************************************
*
*       _ForEachDirEntry_NoHidden()
*/
static void _ForEachDirEntry_NoHidden(void * pContext, const char * sDir, void (*pf) (void * pContext, void * pFileEntry)) {
  _ForEachDirEntryEx(pContext, sDir, pf, 0);
}

/*********************************************************************
*
*       _GetDirEntryFilename
*/
static void _GetDirEntryFilename(void * pFileEntry, char * sFilename, U32 SizeofBuffer) {
  FS_FIND_DATA * pFD;

  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  pFD = (FS_FIND_DATA *)pFileEntry;
  strncpy(sFilename, pFD->sFileName, SizeofBuffer);
  * (sFilename + SizeofBuffer - 1) = 0;
}

/*********************************************************************
*
*       _GetDirEntryFileSize
*/
static U32 _GetDirEntryFileSize(void * pFileEntry, U32 * pFileSizeHigh) {
  FS_FIND_DATA * pFD;

  (void)pFileSizeHigh;

  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  pFD = (FS_FIND_DATA *)pFileEntry;
  return pFD->FileSize;
}

/*********************************************************************
*
*       _GetDirEntryFileTime
*/
static U32 _GetDirEntryFileTime(void * pFileEntry) {
  FS_FIND_DATA * pFD;
  U32            r;

  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  pFD = (FS_FIND_DATA *)pFileEntry;
  //
  // If the file system does not set the time of the last modification, return the creation time.
  //
  r = pFD->LastWriteTime;
  if (r == 0) {
    r = pFD->CreationTime;
  }
  //
  // Every timestamp is better than no timestamp.
  // In case we are unable to retrieve a valid timestamp, for example
  // volumes have no timestamp at all according to the standard like
  // FAT, we simply use the base timestamp of January 1st, 1980 00:00 .
  //
  if (r == 0) {
    r = (U32)(1 + (1 << 5) + (0 << 9)) << 16;  // Day 1, Month 1, 1980 year offset. Lower half-word is used for the time, 00:00 .
  }
  return r;
}

/*********************************************************************
*
*       _GetDirEntryAttributes
*
*  Return value
*    bit 0   - 0: File, 1: Directory
*/
static int  _GetDirEntryAttributes(void * pFileEntry) {
  FS_FIND_DATA * pFD;

  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  pFD = (FS_FIND_DATA *)pFileEntry;
  return (pFD->Attributes & FS_ATTR_DIRECTORY) ? 1 : 0;
}

/*********************************************************************
*
*       _CreateEx()
*/
static void * _CreateEx(const char * sFilename, int AllowHiddenAccess) {
  char acAbsFilename[MAX_PATH];
  U8   Attrib;

  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  _ConvertPath(sFilename, acAbsFilename, sizeof(acAbsFilename));
  Attrib = FS_GetFileAttributes(acAbsFilename);
  if (Attrib != 0xFF) {
    if ((Attrib & FS_ATTR_HIDDEN) != 0) {
      if (AllowHiddenAccess == 0) {
        return NULL;  // Hidden file exists, overwrite not permitted.
      }
    }
  }
  return FS_FOpen(acAbsFilename, "wb");
}

/*********************************************************************
*
*       _Create()
*/
static void * _Create(const char * sFilename) {
  return _CreateEx(sFilename, 1);
}

/*********************************************************************
*
*       _Create_NoHidden()
*/
static void * _Create_NoHidden(const char * sFilename) {
  return _CreateEx(sFilename, 0);
}

/*********************************************************************
*
*       _DeleteFileEx()
*/
static void * _DeleteFileEx(const char *sFilename, int AllowHiddenAccess) {
  char acAbsFilename[MAX_PATH];
  U8   Attrib;

  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  _ConvertPath(sFilename, acAbsFilename, sizeof(acAbsFilename));
  Attrib = FS_GetFileAttributes(acAbsFilename);
  if (Attrib != 0xFF) {
    if ((Attrib & FS_ATTR_HIDDEN) != 0) {
      if (AllowHiddenAccess == 0) {
        return (void*)-1;  // Access on hidden file not permitted.
      }
    }
  }
  return (void*)FS_Remove(acAbsFilename);
}

/*********************************************************************
*
*       _DeleteFile()
*/
static void * _DeleteFile(const char *sFilename) {
  return _DeleteFileEx(sFilename, 1);
}

/*********************************************************************
*
*       _DeleteFile_NoHidden()
*/
static void * _DeleteFile_NoHidden(const char *sFilename) {
  return _DeleteFileEx(sFilename, 0);
}

/*********************************************************************
*
*       _RenameFileEx()
*/
static int _RenameFileEx(const char *sOldFilename, const char *sNewFilename, int AllowHiddenAccess) {
  char acAbsOldFilename[MAX_PATH];
  U8   Attrib;

  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  _ConvertPath(sOldFilename, acAbsOldFilename, sizeof(acAbsOldFilename));
  Attrib = FS_GetFileAttributes(acAbsOldFilename);
  if (Attrib != 0xFF) {
    if ((Attrib & FS_ATTR_HIDDEN) != 0) {
      if (AllowHiddenAccess == 0) {
        return -1;  // Access on hidden file not permitted.
      }
    }
  }
  return FS_Rename(acAbsOldFilename, sNewFilename);
}

/*********************************************************************
*
*       _RenameFile()
*/
static int _RenameFile(const char *sOldFilename, const char *sNewFilename) {
  return _RenameFileEx(sOldFilename, sNewFilename, 1);
}

/*********************************************************************
*
*       _RenameFile_NoHidden()
*/
static int _RenameFile_NoHidden(const char *sOldFilename, const char *sNewFilename) {
  return _RenameFileEx(sOldFilename, sNewFilename, 0);
}

/*********************************************************************
*
*       _WriteAt
*
* Function description
*   Writes data to a specific offset in a file.
*
* Return value
*   >=  0: NumBytes written.
*      -1: Generic write error.
*      -2: Storage medium full.
*/
static int _WriteAt(void *hFile, void *pBuffer, U32 Pos, U32 NumBytes) {
  int r;
  U32 NumBytesFree;

  if (NumBytes > 0) {
    _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
    FS_FSeek((FS_FILE*) hFile, Pos, FS_SEEK_SET);
    r = FS_Write((FS_FILE*) hFile, pBuffer, NumBytes);
    if (r <= 0) {
      NumBytesFree = FS_GetVolumeFreeSpace(_sVolumeName);
      if (NumBytesFree < NumBytes) {
        r = -2;  // Storage medium full.
      } else {
        r = -1;  // Generic write error.
      }
    }
  } else {
    r = 0;
  }
  return r;
}

/*********************************************************************
*
*       _MKDirEx
*/
static int _MKDirEx(const char * sDirname, int AllowHiddenAccess) {
  char acAbsDirname[MAX_PATH];
  U8   Attrib;

  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  _ConvertPath(sDirname, acAbsDirname, sizeof(acAbsDirname));
  Attrib = FS_GetFileAttributes(acAbsDirname);
  if (Attrib != 0xFF) {
    if ((Attrib & FS_ATTR_HIDDEN) != 0) {
      if (AllowHiddenAccess == 0) {
        return -1;  // Access on hidden file not permitted.
      }
    }
  }
  return FS_MkDir(acAbsDirname);
}

/*********************************************************************
*
*       _MKDir()
*/
static int _MKDir(const char * sDirname) {
  return _MKDirEx(sDirname, 1);
}

/*********************************************************************
*
*       _MKDir_NoHidden()
*/
static int _MKDir_NoHidden(const char * sDirname) {
  return _MKDirEx(sDirname, 0);
}

/*********************************************************************
*
*       _RMDirEx()
*/
static int _RMDirEx(const char * sDirname, int AllowHiddenAccess) {
  char acAbsDirname[MAX_PATH];
  U8   Attrib;

  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  _ConvertPath(sDirname, acAbsDirname, sizeof(acAbsDirname));
  Attrib = FS_GetFileAttributes(acAbsDirname);
  if (Attrib != 0xFF) {
    if ((Attrib & FS_ATTR_HIDDEN) != 0) {
      if (AllowHiddenAccess == 0) {
        return -1;  // Access on hidden file not permitted.
      }
    }
  }
  return FS_RmDir(acAbsDirname);
}

/*********************************************************************
*
*       _RMDir()
*/
static int _RMDir(const char * sDirname) {
  return _RMDirEx(sDirname, 1);
}

/*********************************************************************
*
*       _RMDir_NoHidden()
*/
static int _RMDir_NoHidden(const char * sDirname) {
  return _RMDirEx(sDirname, 0);
}

/*********************************************************************
*
*       _IsFolderEx()
*
* Parameters
*   sPath            : Path to be checked if it is a file or a folder
*                      e.g. "/sub/" or "/sub".
*   AllowHiddenAccess: Allow access to files that have the FS_ATTR_HIDDEN
*                      attribute set.
*
* Return value
*   Path does NOT exist: -1.
*   Path is a file     :  0.
*   Path is a folder   :  1.
*/
static int _IsFolderEx(const char *sPath, int AllowHiddenAccess) {
  int  r;
  char acAbsPath[MAX_PATH];

  _InitIfRequired();  // Perform automatic initialisation so that explicit call to FS_Init is not required.
  if (*sPath == UL_DIRECTORY_DELIMITER) {
    if (*(sPath + 1) == 0) {
      return 1;       // Root path is always a folder.
    }
  }
  _ConvertPath(sPath, acAbsPath, sizeof(acAbsPath));
  if (_sVolumeName == NULL) {
    char * p;

    //
    // Treat the volume name like a folder.
    //
    p = acAbsPath;
    while (1) {
      if (*p == 0) {
        return 1;     // End of path reached and no directory delimiter found. This is a volume name.
      }
      if (*p == FS_DIRECTORY_DELIMITER) {
        if (*(p + 1) == 0) {
          return 1;   // The first directory delimiter is the last character in the path. This is a volume name.
        }
        break;        // The path contains more than only the volume name. Determine below whether it is a file or directory.
      }
      ++p;
    }
  }
  r = FS_GetFileAttributes(acAbsPath);
  if (r == 0xFF) {
    return -1;  // Error, file/directory does not exist ?
  } else {
    if ((r & FS_ATTR_HIDDEN) != 0) {
      if (AllowHiddenAccess == 0) {
        return -1;  // Access on hidden file/folder not permitted.
      }
    }
  }
  if ((r & FS_ATTR_DIRECTORY) != 0) {
    return 1;       // Path is a folder.
  } else {
    return 0;       // Path is a file.
  }
}

/*********************************************************************
*
*       _IsFolder()
*
* Parameters
*   sPath            : Path to be checked if it is a file or a folder
*                      e.g. "/sub/" or "/sub".
*
* Return value
*   Path does NOT exist: -1.
*   Path is a file     :  0.
*   Path is a folder   :  1.
*/
static int _IsFolder(const char *sPath) {
  return _IsFolderEx(sPath, 1);
}

/*********************************************************************
*
*       _IsFolder_NoHidden()
*
* Parameters
*   sPath            : Path to be checked if it is a file or a folder
*                      e.g. "/sub/" or "/sub".
*
* Return value
*   Path does NOT exist: -1.
*   Path is a file     :  0.
*   Path is a folder   :  1.
*/
static int _IsFolder_NoHidden(const char *sPath) {
  return _IsFolderEx(sPath, 0);
}

/*********************************************************************
*
*       _MoveEx()
*
* Parameters
*   sOldFilename     : Old file or folder path.
*   sNewFilename     : New file or folder path.
*   AllowHiddenAccess: Allow access to files that have the FS_ATTR_HIDDEN
*                      attribute set.
*
* Return value
*   O.K. :   0.
*   Error: < 0.
*/
static int _MoveEx(const char *sOldFilename, const char *sNewFilename, int AllowHiddenAccess) {
  char acAbsOldFilename[MAX_PATH];
  char acAbsNewFilename[MAX_PATH];
  U8   Attrib;

  _InitIfRequired();         // Perform automatic initialisation so that explicit call to FS_Init is not required
  _ConvertPath(sOldFilename, acAbsOldFilename, sizeof(acAbsOldFilename));
  Attrib = FS_GetFileAttributes(acAbsOldFilename);
  if (Attrib != 0xFF) {
    if ((Attrib & FS_ATTR_HIDDEN) != 0) {
      if (AllowHiddenAccess == 0) {
        return -1;  // Access on hidden file not permitted.
      }
    }
  }
  _ConvertPath(sNewFilename, acAbsNewFilename, sizeof(acAbsNewFilename));
  Attrib = FS_GetFileAttributes(acAbsNewFilename);
  if (Attrib != 0xFF) {
    if ((Attrib & FS_ATTR_HIDDEN) != 0) {
      if (AllowHiddenAccess == 0) {
        return -1;  // Access on hidden file not permitted.
      }
    }
  }
  return FS_Move(acAbsOldFilename, acAbsNewFilename);
}

/*********************************************************************
*
*       _Move()
*
* Parameters
*   sOldFilename     : Old file or folder path.
*   sNewFilename     : New file or folder path.
*
* Return value
*   O.K. :   0.
*   Error: < 0.
*/
static int _Move(const char *sOldFilename, const char *sNewFilename) {
  return _MoveEx(sOldFilename, sNewFilename, 1);
}

/*********************************************************************
*
*       _Move_NoHidden()
*
* Parameters
*   sOldFilename     : Old file or folder path.
*   sNewFilename     : New file or folder path.
*
* Return value
*   O.K. :   0.
*   Error: < 0.
*/
static int _Move_NoHidden(const char *sOldFilename, const char *sNewFilename) {
  return _MoveEx(sOldFilename, sNewFilename, 0);
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/

//
// emFile API that shows and allows access to hidden files.
//
const IP_FS_API IP_FS_FS = {
  //
  // Read only file operations. These have to be present on ANY file system, even the simplest one.
  //
  _FS_Open,                // pfOpenFile
  _Close,                  // pfCloseFile
  _ReadAt,                 // pfReadAt
  _GetLen,                 // pfGetLen
  //
  // Directory query operations.
  //
  _ForEachDirEntry,        // pfForEachDirEntry
  _GetDirEntryFilename,    // pfGetDirEntryFileName
  _GetDirEntryFileSize,    // pfGetDirEntryFileSize
  _GetDirEntryFileTime,    // pfGetDirEntryFileTime
  _GetDirEntryAttributes,  // pfGetDirEntryAttributes
  //
  // Write file operations.
  //
  _Create,                 // pfCreate
  _DeleteFile,             // pfDeleteFile
  _RenameFile,             // pfRenameFile
  _WriteAt,                // pfWriteAt
  //
  // Additional directory operations
  //
  _MKDir,                  // pfMKDir
  _RMDir,                  // pfRMDir
  //
  // Additional operations
  //
  _IsFolder,               // pfIsFolder
  _Move                    // pfMove
};

//
// emFile API that does not show hidden files but allows access to them.
// Can be used to hide files but allows access if you know they are there.
// Somewhat similar like a .htaccess file.
//
const IP_FS_API IP_FS_FS_AllowHiddenAccess = {
  //
  // Read only file operations. These have to be present on ANY file system, even the simplest one.
  //
  _FS_Open,                   // pfOpenFile
  _Close,                     // pfCloseFile
  _ReadAt,                    // pfReadAt
  _GetLen,                    // pfGetLen
  //
  // Directory query operations.
  //
  _ForEachDirEntry_NoHidden,  // pfForEachDirEntry
  _GetDirEntryFilename,       // pfGetDirEntryFileName
  _GetDirEntryFileSize,       // pfGetDirEntryFileSize
  _GetDirEntryFileTime,       // pfGetDirEntryFileTime
  _GetDirEntryAttributes,     // pfGetDirEntryAttributes
  //
  // Write file operations.
  //
  _Create,                    // pfCreate
  _DeleteFile,                // pfDeleteFile
  _RenameFile,                // pfRenameFile
  _WriteAt,                   // pfWriteAt
  //
  // Additional directory operations
  //
  _MKDir,                     // pfMKDir
  _RMDir,                     // pfRMDir
  //
  // Additional operations
  //
  _IsFolder,                  // pfIsFolder
  _Move                       // pfMove
};

//
// emFile API that does not show hidden files and does not allow access to them.
//
const IP_FS_API IP_FS_FS_DenyHiddenAccess = {
  //
  // Read only file operations. These have to be present on ANY file system, even the simplest one.
  //
  _FS_Open_NoHidden,          // pfOpenFile
  _Close,                     // pfCloseFile
  _ReadAt,                    // pfReadAt
  _GetLen,                    // pfGetLen
  //
  // Directory query operations.
  //
  _ForEachDirEntry_NoHidden,  // pfForEachDirEntry
  _GetDirEntryFilename,       // pfGetDirEntryFileName
  _GetDirEntryFileSize,       // pfGetDirEntryFileSize
  _GetDirEntryFileTime,       // pfGetDirEntryFileTime
  _GetDirEntryAttributes,     // pfGetDirEntryAttributes
  //
  // Write file operations.
  //
  _Create_NoHidden,           // pfCreate
  _DeleteFile_NoHidden,       // pfDeleteFile
  _RenameFile_NoHidden,       // pfRenameFile
  _WriteAt,                   // pfWriteAt
  //
  // Additional directory operations
  //
  _MKDir_NoHidden,            // pfMKDir
  _RMDir_NoHidden,            // pfRMDir
  //
  // Additional operations
  //
  _IsFolder_NoHidden,         // pfIsFolder
  _Move_NoHidden              // pfMove
};

/*************************** End of file ****************************/
