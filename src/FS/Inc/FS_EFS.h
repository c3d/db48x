/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2024  SEGGER Microcontroller GmbH                 *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File        : FS_EFS.h
Purpose     : EFS File System Layer header
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef FS_EFS_H            // Avoid recursive and multiple inclusion
#define FS_EFS_H

#include "FS_Int.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_EFS_Read.c
*/
U32  FS_EFS_Read                (      FS_FILE    * pFile, void * pData, U32 NumBytes);

/*********************************************************************
*
*       FS_EFS_Write.c
*/
U32  FS_EFS_Write               (      FS_FILE    * pFile, const void * pData, U32 NumBytes);
int  FS_EFS_CloseFile           (      FS_FILE    * pFile);
void FS_EFS_Clean               (      FS_VOLUME  * pVolume);
int  FS_EFS_CloseFileEx         (      FS_FILE    * pFile, const FS_FILE_INFO * pInfo, unsigned Mask);

/*********************************************************************
*
*       FS_EFS_Open.c
*/
int  FS_EFS_OpenFile            (const char       * sFileName, FS_FILE * pFile, int DoDel, int DoOpen, int DoCreate);

/*********************************************************************
*
*       FS_EFS_Misc.c
*/
int  FS_EFS_CheckInfoSector     (      FS_VOLUME  * pVolume);
int  FS_EFS_CreateJournalFile   (      FS_VOLUME  * pVolume, U32 NumBytes, U32 * pFirstSector, U32 * pNumSectors);
int  FS_EFS_OpenJournalFile     (      FS_VOLUME  * pVolume);
U32  FS_EFS_GetIndexOfLastSector(      FS_VOLUME  * pVolume);
int  FS_EFS_FreeSectors         (      FS_VOLUME  * pVolume);
int  FS_EFS_GetFreeSpace        (      FS_VOLUME  * pVolume, void * pBuffer, int SizeOfBuffer, U32 FirstClusterId, U32 * pNumClustersFree, U32 * pNumClustersChecked);
int  FS_EFS_GetATInfo           (      FS_VOLUME  * pVolume, FS_AT_INFO * pATInfo);
int  FS_EFS_GetClusterInfo      (      FS_VOLUME  * pVolume, U32 ClusterId, FS_CLUSTER_INFO * pClusterInfo);
void FS_EFS_Save                (      FS_CONTEXT * pContext);
void FS_EFS_Restore             (const FS_CONTEXT * pContext);

/*********************************************************************
*
*       FS_EFS_Format.c
*/
int  FS_EFS_Format              (      FS_VOLUME * pVolume, const FS_FORMAT_INFO  * pFormatInfo);
int  FS_EFS_GetDiskInfo         (      FS_VOLUME * pVolume, FS_DISK_INFO    * pDiskInfo, int Flags);
int  FS_EFS_GetVolumeLabel      (      FS_VOLUME * pVolume, char       * sVolumeLabel, unsigned SizeOfVolumeLabel);
int  FS_EFS_SetVolumeLabel      (      FS_VOLUME * pVolume, const char * sVolumeLabel);
int  FS_EFS_GetDiskLayout       (      FS_VOLUME * pVolume, FS_DISK_LAYOUT  * pDiskLayout);

/*********************************************************************
*
*       FS_EFS_Move.c
*/
int  FS_EFS_Move                (      FS_VOLUME * pVolume, const char * sNameSrc, const char * sNameDest);

/*********************************************************************
*
*       FS_EFS_DirEntry.c
*/
int  FS_EFS_SetDirEntryInfo     (      FS_VOLUME  * pVolume, const char * sName, const void * p, unsigned Mask);
int  FS_EFS_GetDirEntryInfo     (      FS_VOLUME  * pVolume, const char * sName,       void * p, unsigned Mask);
int  FS_EFS_SetDirEntryInfoEx   (      FS_VOLUME  * pVolume, const FS_DIRENTRY_POS * pDirEntryPos, const void * p, unsigned Mask);
int  FS_EFS_GetDirEntryInfoEx   (      FS_VOLUME  * pVolume, const FS_DIRENTRY_POS * pDirEntryPos,       void * p, unsigned Mask);

/*********************************************************************
*
*       FS_EFS_Rename.c
*/
int  FS_EFS_Rename              (      FS_VOLUME  * pVolume, const char * sOldName, const char * sNewNameNQ);

/*********************************************************************
*
*       FS_EFS_Dir.c
*/
int  FS_EFS_OpenDir             (const char       * sDirName, FS_DIR_OBJ * pDirObj);
int  FS_EFS_CloseDir            (      FS_DIR_OBJ * pDirObj);
int  FS_EFS_ReadDir             (      FS_DIR_OBJ * pDirObj, FS_DIRENTRY_INFO * pDirEntryInfo);
int  FS_EFS_RemoveDir           (      FS_VOLUME  * pVolume, const char * sDirName);
int  FS_EFS_CreateDir           (      FS_VOLUME  * pVolume, const char * sDirName);
int  FS_EFS_DeleteDir           (      FS_VOLUME  * pVolume, const char * sDirName, int MaxRecursionLevel);

/*********************************************************************
*
*       FS_EFS_SetEndOfFile.c
*/
int  FS_EFS_SetEndOfFile        (      FS_FILE    * pFile);
int  FS_EFS_SetFileSize         (      FS_FILE    * pFile, FS_FILE_SIZE NumBytes);

/*********************************************************************
*
*       FS_EFS_CheckDisk.c
*/
int  FS_EFS_CheckVolume         (      FS_VOLUME  * pVolume, void * pBuffer, U32 BufferSize, int MaxRecursionLevel, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
int  FS_EFS_CheckDir            (      FS_VOLUME  * pVolume, const char * sPath, FS_CLUSTER_MAP * pClusterMap, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);
int  FS_EFS_CheckAT             (      FS_VOLUME  * pVolume, const FS_CLUSTER_MAP * pClusterMap, FS_CHECKDISK_ON_ERROR_CALLBACK * pfOnError);

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // FS_EFS_H

/*************************** End of file ****************************/
