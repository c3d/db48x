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
-------------------------- END-OF-HEADER -----------------------------

File    : FS_ConfigRAMDisk_23KB.c
Purpose : Configuration file for FS with RAM disk
*/

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/
#include "FS.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#ifndef   ALLOC_SIZE
  #define ALLOC_SIZE          0x1100    // Memory pool for the file system in bytes.
#endif

#ifndef   NUM_LOG_SECTORS
  #define NUM_LOG_SECTORS     46        // Number of logical sectors on the storage.
#endif

#ifndef   LOG_SECTOR_SIZE
  #define LOG_SECTOR_SIZE     512       // Size of a logical sector in bytes.
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

//
// Memory pool used for semi-dynamic allocation and memory for storage.
//
#ifdef __ICCARM__
  #pragma location="FS_RAM"
  static __no_init U32 _aMemBlock[ALLOC_SIZE / 4];
  #pragma location="FS_RAM"
  static __no_init U32 _aRAMDisk[(NUM_LOG_SECTORS * LOG_SECTOR_SIZE) / 4];
#elif defined __CC_ARM
  __attribute__ ((section ("FS_RAM"), zero_init))
  U32 static _aMemBlock[ALLOC_SIZE / 4];
  __attribute__ ((section ("FS_RAM"), zero_init))
  U32 static _aRAMDisk[(NUM_LOG_SECTORS * LOG_SECTOR_SIZE) / 4] ;
#else
  static U32 _aMemBlock[ALLOC_SIZE / 4];
  static U32 _aRAMDisk[(NUM_LOG_SECTORS * LOG_SECTOR_SIZE) / 4];
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_X_AddDevices
*
*  Function description
*    This function is called by the FS during FS_Init().
*    It is supposed to add all devices, using primarily FS_AddDevice().
*
*  Note
*    (1) Other API functions may NOT be called, since this function is called
*        during initialization. The devices are not yet ready at this point.
*/
void FS_X_AddDevices(void) {
  //
  // Give the file system memory to work with.
  //
  FS_AssignMemory(&_aMemBlock[0], sizeof(_aMemBlock));
  //
  // Add and configure the RAMDISK driver.
  //
  FS_AddDevice(&FS_RAMDISK_Driver);
  FS_RAMDISK_Configure(0, _aRAMDisk, LOG_SECTOR_SIZE, NUM_LOG_SECTORS);
#if FS_SUPPORT_FILE_BUFFER
  //
  // Enable the file buffer to increase the performance when reading/writing a small number of bytes.
  //
  FS_ConfigFileBufferDefault(LOG_SECTOR_SIZE, FS_FILE_BUFFER_WRITE);
#endif // FS_SUPPORT_FILE_BUFFER
}


/*************************** End of file ****************************/
