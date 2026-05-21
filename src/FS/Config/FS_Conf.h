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

File    : FS_Conf.h
Purpose : File system configuration
*/

#ifndef FS_CONF_H           // Avoid multiple inclusion.
#define FS_CONF_H

#ifdef DEBUG
  #if (DEBUG)
    #define FS_DEBUG_LEVEL               5
//    #define FS_SUPPORT_PROFILE           1
//    #define FS_SUPPORT_PROFILE_END_CALL  1
  #endif
#endif

    #define FS_DEBUG_LEVEL               5

#define FS_OS_LOCKING          (1)
//
// Necessary for SEGGER Eval Software
//
//#define FS_SUPPORT_JOURNAL     (1)
//#define FS_SUPPORT_ENCRYPTION  (1)

//
// Required for FS NOR on STM32L475_DiscoveryKit_IoT_Node 
//
//#define FS_NOR_TIMEOUT_PAGE_WRITE   (100)
#define FS_NOR_TIMEOUT_PAGE_WRITE   (800)


// modif re
// new
#define FS_VERIFY_WRITE                         1
#define  FS_NOR_VERIFY_WRITE     1
//#define FS_NOR_SUPPORT_CRC                      1
//#define FS_NOR_MAX_NUM_SECTOR_SIZES           13



//#define FS_NOR_MAX_SECTOR_BLOCKS              1
//#define FS_NOR_DELAY_SECTOR_ERASE               (50)
#define FS_NOR_TIMEOUT_SECTOR_ERASE             5000    // Timeout for the sector erase operation (ms)
//#define FS_NOR_LINE_SIZE 4
//#define FS_NOR_CAN_REWRITE                    0
//  #define FS_NOR_NUM_WRITE_RETRIES                75
//  #define FS_NOR_DELAY_SECTOR_ERASE               10
//   #define FS_NOR_BYTES_PER_PAGE                   512     // Maximum number of bytes that can be stored to a NOR flash page
// #define FS_NOR_AMD_STATUS_CHECK_TYPE            2  
// #define FS_NOR_OPTIMIZE_SECTOR_SIZE_LIST        0


//
// 20221021: updated
//
#define FS_MMC_SUPPORT_UHS                 1
#define FS_NOR_SUPPORT_CRC                 1
#define FS_SUPPORT_SECTOR_BUFFER_CACHE     1
#define FS_SUPPORT_CHECK_MEMORY            1
#define FS_SUPPORT_DEINIT                  1
#define FS_SUPPORT_EXT_ASCII               1
#define FS_SUPPORT_FILE_NAME_ENCODING      1
#define FS_SUPPORT_MBCS                    1


#endif                      // Avoid multiple inclusion.

/*************************** End of file ****************************/
