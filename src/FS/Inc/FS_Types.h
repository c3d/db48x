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
File        : FS_Types.h
Purpose     : Common types and defines.
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef FS_TYPES_H                    // Avoid recursive and multiple inclusion
#define FS_TYPES_H

#include "SEGGER.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       Global types
*/
#define FS_WCHAR                              U16
#define FS_WCHAR_INVALID                      0xFFFFu
#define FS_WCHAR_MAX_SIZE                     2         // Maximum number of bytes in a multi-byte character.
#define FS_FILE_OFF                           I32       // TBD: Use FS_SUPPORT_LARGE_FILES as option to enable support for files larger than 4 Gbytes at the API level.
#define FS_FILE_SIZE                          U32

/*********************************************************************
*
*       File access modes
*/
#define FS_FILE_ACCESS_FLAG_A                 (1uL << 0)
#define FS_FILE_ACCESS_FLAG_B                 (1uL << 1)
#define FS_FILE_ACCESS_FLAG_C                 (1uL << 2)
#define FS_FILE_ACCESS_FLAG_R                 (1uL << 3)
#define FS_FILE_ACCESS_FLAG_W                 (1uL << 4)

/*********************************************************************
*
*       Combined file access modes which are frequently used
*/
#define FS_FILE_ACCESS_FLAGS_AW               (FS_FILE_ACCESS_FLAG_A | FS_FILE_ACCESS_FLAG_W)
#define FS_FILE_ACCESS_FLAGS_BR               (FS_FILE_ACCESS_FLAG_B | FS_FILE_ACCESS_FLAG_R)
#define FS_FILE_ACCESS_FLAGS_CW               (FS_FILE_ACCESS_FLAG_C | FS_FILE_ACCESS_FLAG_W)
#define FS_FILE_ACCESS_FLAGS_RW               (FS_FILE_ACCESS_FLAG_R | FS_FILE_ACCESS_FLAG_W)

#define FS_FILE_ACCESS_FLAGS_ACW              (FS_FILE_ACCESS_FLAG_C | FS_FILE_ACCESS_FLAGS_AW)
#define FS_FILE_ACCESS_FLAGS_ARW              (FS_FILE_ACCESS_FLAG_A | FS_FILE_ACCESS_FLAGS_RW)
#define FS_FILE_ACCESS_FLAGS_BCW              (FS_FILE_ACCESS_FLAG_B | FS_FILE_ACCESS_FLAGS_CW)
#define FS_FILE_ACCESS_FLAGS_BRW              (FS_FILE_ACCESS_FLAG_W | FS_FILE_ACCESS_FLAGS_BR)
#define FS_FILE_ACCESS_FLAGS_CRW              (FS_FILE_ACCESS_FLAG_C | FS_FILE_ACCESS_FLAG_R | FS_FILE_ACCESS_FLAG_W)

#define FS_FILE_ACCESS_FLAGS_ABCW             (FS_FILE_ACCESS_FLAG_B | FS_FILE_ACCESS_FLAGS_ACW)
#define FS_FILE_ACCESS_FLAGS_ACRW             (FS_FILE_ACCESS_FLAG_A | FS_FILE_ACCESS_FLAGS_CRW)
#define FS_FILE_ACCESS_FLAGS_BCRW             (FS_FILE_ACCESS_FLAG_B | FS_FILE_ACCESS_FLAGS_CRW)

#define FS_FILE_ACCESS_FLAGS_ABCRW            (FS_FILE_ACCESS_FLAGS_ACRW | FS_FILE_ACCESS_FLAG_B)

/*********************************************************************
*
*       Directory entry get/set macros
*/
#define FS_DIRENTRY_GET_ATTRIBUTES             0x01uL
#define FS_DIRENTRY_GET_TIMESTAMP_CREATE       0x02uL
#define FS_DIRENTRY_GET_TIMESTAMP_MODIFY       0x04uL
#define FS_DIRENTRY_GET_TIMESTAMP_ACCESS       0x08uL
#define FS_DIRENTRY_GET_SIZE                   0x10uL
#define FS_DIRENTRY_GET_FIRST_CLUSTER          0x20uL
#define FS_DIRENTRY_GET_ALL                    (FS_DIRENTRY_GET_ATTRIBUTES       | \
                                                FS_DIRENTRY_GET_TIMESTAMP_CREATE | \
                                                FS_DIRENTRY_GET_TIMESTAMP_MODIFY | \
                                                FS_DIRENTRY_GET_TIMESTAMP_ACCESS | \
                                                FS_DIRENTRY_GET_SIZE)
#define FS_DIRENTRY_SET_ATTRIBUTES             FS_DIRENTRY_GET_ATTRIBUTES
#define FS_DIRENTRY_SET_TIMESTAMP_CREATE       FS_DIRENTRY_GET_TIMESTAMP_CREATE
#define FS_DIRENTRY_SET_TIMESTAMP_MODIFY       FS_DIRENTRY_GET_TIMESTAMP_MODIFY
#define FS_DIRENTRY_SET_TIMESTAMP_ACCESS       FS_DIRENTRY_GET_TIMESTAMP_ACCESS

/*********************************************************************
*
*       Sector data flags
*
*  Description
*    Type of data stored in a logical sector.
*
*  Additional information
*    These flags are typically used with the sector cache API functions
*    to specify the type of sector data for which a specified parameter
*    applies. The bit positions of the flags correspond to the values
*    defined via \ref{Sector data type}
*/
#define FS_SECTOR_TYPE_MASK_DATA                (1u << FS_SECTOR_TYPE_DATA)     // Sector that stores file data.
#define FS_SECTOR_TYPE_MASK_DIR                 (1u << FS_SECTOR_TYPE_DIR)      // Sector that stores directory entries.
#define FS_SECTOR_TYPE_MASK_MAN                 (1u << FS_SECTOR_TYPE_MAN)      // Sector that stores entries of the allocation table.
#define FS_SECTOR_TYPE_MASK_ALL                 (FS_SECTOR_TYPE_MASK_DATA | \
                                                 FS_SECTOR_TYPE_MASK_DIR  | \
                                                 FS_SECTOR_TYPE_MASK_MAN)       // All sector types.
#define FS_SECTOR_TYPE_COUNT                    3u                              // Number of sector types (for internal use only)

/*********************************************************************
*
*       Partitioning
*/
#define FS_MAX_NUM_PARTITIONS_MBR               4           // Maximum number of MBR partitions.
#define FS_MAX_NUM_PARTITIONS_GPT               128         // Maximum number of GPT partitions.
#define FS_NUM_BYTES_GUID                       16          // Number of bytes in a GUID.

/*********************************************************************
*
*       Public types
*
**********************************************************************
*/

/*********************************************************************
*
*       FS_FILE_BUFFER
*
*  Description
*    Buffer for the file data.
*/
typedef struct FS_FILE_BUFFER FS_FILE_BUFFER;
struct FS_FILE_BUFFER {                 //lint -esym(9058, FS_FILE_BUFFER) tag unused outside of typedefs N:999. Rationale: the type definition is used as forward declaration.
  U8             * pData;               // Buffer to store the file data.
#if (FS_SUPPORT_FILE_BUFFER != 0) && (FS_SUPPORT_FILE_BUFFER_LIST != 0)
  FS_FILE_BUFFER * pNext;               // Next file buffer in the list.
#endif // (FS_SUPPORT_FILE_BUFFER != 0) && (FS_SUPPORT_FILE_BUFFER_LIST != 0)
  FS_FILE_SIZE     FilePos;             // Position in the file of the first byte stored in buffer.
  U32              NumBytesInBuffer;    // Number of bytes stored in the buffer.
  U32              BufferSize;          // Buffer size in bytes.
  U8               IsDirty;             // Set to 1 if the application changed the data in the buffer.
  U8               Flags;               // Flags which indicate the operating mode (read/write).
#if (FS_SUPPORT_FILE_BUFFER != 0) && (FS_SUPPORT_FILE_BUFFER_LIST != 0)
  U8               InUse;               // Indicates if the file buffer is used or not (0 - free, 1 - in use).
#endif // (FS_SUPPORT_FILE_BUFFER != 0) && (FS_SUPPORT_FILE_BUFFER_LIST != 0)
};

/*********************************************************************
*
*       FS_WRBUF_SECTOR_INFO
*
*  FS internal structure. One instance per sector.
*/
typedef struct {
  U32 SectorIndex;
  U32 IsValid;
} FS_WRBUF_SECTOR_INFO;

/*********************************************************************
*
*       FS_CACHE_ALL_BLOCK_INFO
*
*  FS internal structure. One instance per block. Every cache block can cache a single sector.
*  It starts with FS_CACHE_ALL_BLOCK_INFO, followed by the cached data.
*/
typedef struct {
  U32 SectorIndex;
} FS_CACHE_ALL_BLOCK_INFO;

/*********************************************************************
*
*       FS_CACHE_ALL_DATA
*
*  FS internal structure. Once instance per cache. Size needs to be a multiple of 4.
*/
typedef struct {
  U32 NumSectors;
  U32 SectorSize;
  U32 NumBytesCache;
} FS_CACHE_ALL_DATA;

/*********************************************************************
*
*       FS_CACHE_MAN_BLOCK_INFO
*
*  FS internal structure. One instance per block. Every cache block can cache a single sector.
*  It starts with FS_CACHE_MAN_BLOCK_INFO, followed by the cached data.
*/
typedef struct {
  U32 SectorIndex;
} FS_CACHE_MAN_BLOCK_INFO;

/*********************************************************************
*
*       FS_CACHE_MAN_DATA
*
*  FS internal structure. Once instance per cache. Size needs to be a multiple of 4.
*/
typedef struct {
  U32 NumSectors;
  U32 SectorSize;
  U32 NumBytesCache;
} FS_CACHE_MAN_DATA;

/*********************************************************************
*
*       FS_CACHE_RW_BLOCK_INFO
*
*  FS internal structure. One instance per block. Every cache block can cache a single sector.
*  It starts with FS_CACHE_RW_BLOCK_INFO, followed by the cached data.
*/
typedef struct {
  U32 SectorIndex;
  U32 IsDirty;
} FS_CACHE_RW_BLOCK_INFO;

/*********************************************************************
*
*       FS_CACHE_RW_DATA
*
*  FS internal structure. Once instance per cache. Size needs to be a multiple of 4.
*/
typedef struct {
  U32 NumSectors;
  U32 SectorSize;
  U8  aCacheMode[FS_SECTOR_TYPE_COUNT];
#if ((FS_SECTOR_TYPE_COUNT % 4u) != 0u)
  U8  aPadding[4u - (FS_SECTOR_TYPE_COUNT % 4u)];   // Make sure we pad this to a multiple of 4 bytes
#endif
  U32 NumBytesCache;
} FS_CACHE_RW_DATA;

/*********************************************************************
*
*       FS_CACHE_RW_QUOTA_BLOCK_INFO
*
*  FS internal structure. One instance per block. Every cache block can cache a single sector.
*  It starts with FS_CACHE_RW_QUOTA_BLOCK_INFO, followed by the cached data.
*/
typedef struct {
  U32 SectorIndex;
  U32 IsDirty;
} FS_CACHE_RW_QUOTA_BLOCK_INFO;

/*********************************************************************
*
*       FS_CACHE_RW_QUOTA_DATA
*
*  FS internal structure. Once instance per cache. Size needs to be a multiple of 4.
*/
typedef struct {
  U32 NumSectors;
  U32 SectorSize;
  U8  aCacheMode[FS_SECTOR_TYPE_COUNT];
#if ((FS_SECTOR_TYPE_COUNT % 4u) != 0u)
  U8  aPadding[4u - (FS_SECTOR_TYPE_COUNT % 4u)];   // Make sure we pad this to a multiple of 4 bytes
#endif
  U32 aNumSectorsReq[FS_SECTOR_TYPE_COUNT];
  U32 aNumSectorsAct[FS_SECTOR_TYPE_COUNT];
  U32 aHashStart[FS_SECTOR_TYPE_COUNT];
  U32 NumBytesCache;
} FS_CACHE_RW_QUOTA_DATA;

/*********************************************************************
*
*       FS_CACHE_MULTI_WAY_BLOCK_INFO
*
*  FS internal structure. One instance per block. Every cache block can cache a single sector.
*  It starts with FS_CACHE_MULTI_WAY_BLOCK_INFO, followed by the cached data.
*/
typedef struct {
  U32 SectorIndex;    // Index of the sector stored in this block
  U16 AccessCnt;      // Access count used for the replacement policy
  U16 IsDirty;        // 1 if the sector has been modified and needs to be written to medium
} FS_CACHE_MULTI_WAY_BLOCK_INFO;

/*********************************************************************
*
*       FS_CACHE_MULTI_WAY_DATA
*
*  FS internal structure. Once instance per cache. Size needs to be a multiple of 4.
*  This structure is stored a the beginning of cache memory.
*  It is followed by an array of cache blocks.
*/
typedef struct {
  U32 NumSets;            // Total number of sets that can be stored in the cache
  U32 ldAssocLevel;       // Number of sectors in a set. Power of 2 value as bit shift.
  U32 SectorSize;         // Size of a sector in bytes
  U8  aCacheMode[FS_SECTOR_TYPE_COUNT];           // Cache strategy for each sector type (management, directory and data)
#if ((FS_SECTOR_TYPE_COUNT % 4u) != 0u)
  U8  aPadding[4u - (FS_SECTOR_TYPE_COUNT % 4u)]; // Make sure the aCacheMode[] is padded to a multiple of 4 bytes
#endif
  U32 NumBytesCache;      // Total size of the cache in bytes.
} FS_CACHE_MULTI_WAY_DATA;

typedef struct FS_SB                            FS_SB;
typedef struct FS_CACHE_API                     FS_CACHE_API;
typedef struct FS_DEVICE                        FS_DEVICE;
typedef struct FS_DIR                           FS_DIR;
typedef struct FS_FILE                          FS_FILE;
typedef struct FS_FILE_OBJ                      FS_FILE_OBJ;
typedef struct FS_PARTITION                     FS_PARTITION;
typedef struct FS_VOLUME                        FS_VOLUME;
typedef struct FS_DIRENTRY_INFO                 FS_DIRENTRY_INFO;
typedef struct FS_FILE_API                      FS_FILE_API;
typedef struct FS_CONTEXT                       FS_CONTEXT;

#if defined(__cplusplus)
}                /* Make sure we have C-declarations in C++ programs */
#endif

#endif // FS_TYPES_H

/*************************** End of file ****************************/
