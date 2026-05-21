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
File    : FS_ConfDefaults.h
Purpose : File system configuration defaults
-------------------------- END-OF-HEADER -----------------------------
*/

#ifndef FS_CONFDEFAULTS_H                   // Avoid recursive and multiple inclusion
#define FS_CONFDEFAULTS_H

/*********************************************************************
*
*       #include section
*
**********************************************************************
*/
#include <string.h>           // for memcpy().
#include "FS_Conf.h"
#include "SEGGER.h"

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       Optimization types
*
*  Description
*    Types of optimization goals.
*
*  Additional information
*    These values can be assigned at compile-time to the FS_OPTIMIZATION_TYPE
*    configuration define.
*/
#define FS_OPTIMIZATION_TYPE_BALANCED             0     // No special optimization.
#define FS_OPTIMIZATION_TYPE_MIN_SIZE             1     // Optimized for reduced ROM and RAM usage.
#define FS_OPTIMIZATION_TYPE_MAX_SPEED            2     // Optimized for maximum speed.

/*********************************************************************
*
*       Debug levels
*
*  Description
*    Amount of debug information.
*
*  Additional information
*    These values can be assigned at compile-time to the FS_DEBUG_LEVEL
*    configuration define.
*
*    The debug levels are hierarchical so that higher debug levels
*    also enable the features assigned to lower debug levels.
*/
#define FS_DEBUG_LEVEL_NOCHECK                    0     // No run time checks are performed.
#define FS_DEBUG_LEVEL_CHECK_PARA                 1     // Parameter checks are performed.
#define FS_DEBUG_LEVEL_CHECK_ALL                  2     // Parameter checks and consistency checks are performed.
#define FS_DEBUG_LEVEL_LOG_ERRORS                 3     // Error conditions are reported.
#define FS_DEBUG_LEVEL_LOG_WARNINGS               4     // Error and warning conditions are reported.
#define FS_DEBUG_LEVEL_LOG_ALL                    5     // Error and warning conditions as well as trace messages are reported.

/*********************************************************************
*
*       OS locking
*
*  Description
*    Types of locking for multitasking access.
*
*  Additional information
*    These values can be assigned at compile-time to the FS_OS_LOCKING
*    configuration define.
*/
#define FS_OS_LOCKING_NONE                        0     // No locking against concurrent access.
#define FS_OS_LOCKING_API                         1     // Locking is performed at API function level (coarse locking).
#define FS_OS_LOCKING_DRIVER                      2     // Locking is performed at device driver level (fine locking).

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

/*********************************************************************
*
*       Optimization
*/
#ifndef   FS_OPTIMIZATION_TYPE
  #define FS_OPTIMIZATION_TYPE                    FS_OPTIMIZATION_TYPE_BALANCED       // Optimization goal of the file system.
#endif

/*********************************************************************
*
*       Debugging
*/
#ifndef     FS_DEBUG_LEVEL
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_DEBUG_LEVEL                        FS_DEBUG_LEVEL_NOCHECK
  #else
    #define FS_DEBUG_LEVEL                        FS_DEBUG_LEVEL_CHECK_PARA
  #endif
#endif

#ifndef   FS_LOG_MASK_DEFAULT
  #define FS_LOG_MASK_DEFAULT                     FS_MTYPE_INIT     // Only the log initialization messages are enabled by default.
#endif

#ifndef   FS_DEBUG_MAX_LEN_MESSAGE
  #define FS_DEBUG_MAX_LEN_MESSAGE                100   // Maximum number of bytes that can be output in a single error, warning or log debug message.
                                                        // This is the size of the buffer that is used for the formatting of debug messages. This buffer is allocated on the stack.
#endif

#ifndef   FS_DEBUG_STATIC_MESSAGE_BUFFER
  #define FS_DEBUG_STATIC_MESSAGE_BUFFER          0     // Specifies if the buffer for the formatting of the debug messages has to be allocated on the stack or not. 0 means allocate on stack.
#endif

/*********************************************************************
*
*       File system defaults
*/
#ifndef   FS_MAX_PATH
  #define FS_MAX_PATH                             260   // Maximum number of characters in a path to a file including the 0-terminator.
#endif

#ifndef   FS_SUPPORT_FAT
  #define FS_SUPPORT_FAT                          1     // Support the FAT file system if enabled
#endif

#ifndef   FS_SUPPORT_EFS
  #define FS_SUPPORT_EFS                          0     // Support the EFS file system if enabled
#endif

#ifndef   FS_SUPPORT_MULTIPLE_FS
  #define FS_SUPPORT_MULTIPLE_FS                  ((FS_SUPPORT_EFS != 0) && (FS_SUPPORT_FAT != 0))
#endif

#ifndef     FS_SUPPORT_FREE_SECTOR
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_SUPPORT_FREE_SECTOR                0     // Informs lower layer of unused sectors. Makes sense only for drivers which use it.
  #else
    #define FS_SUPPORT_FREE_SECTOR                1
  #endif
#endif

#ifndef     FS_SUPPORT_CACHE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_SUPPORT_CACHE                      0     // Set to 1 to enable the support for sector caching.
                                                        // The cache has to be enabled at runtime by calling FS_AssignCache().
  #else
    #define FS_SUPPORT_CACHE                      1
  #endif
#endif

#ifndef   FS_SUPPORT_ENCRYPTION
  #define FS_SUPPORT_ENCRYPTION                   0     // Set to 1 to enable support for encryption at file level.
                                                        // The encryption has to be enabled at runtime by calling FS_SetEncryptionObject().
#endif

#ifndef   FS_MULTI_HANDLE_SAFE
  #define FS_MULTI_HANDLE_SAFE                    0     // Set to 1 to enable read and write access to same file from different tasks.
#endif

#ifndef     FS_MAX_LEN_FULL_FILE_NAME
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_MAX_LEN_FULL_FILE_NAME             64    // Maximum number of characters allowed in a full path to file. Used when FS_MULTI_HANDLE_SAFE is set to 1.
  #else
    #define FS_MAX_LEN_FULL_FILE_NAME             256
  #endif
#endif

#ifndef     FS_DRIVER_ALIGNMENT
  //
  // Added for compatibility reasons.
  //
  #ifdef    FS_DRIVER_ALIGMENT
    #define FS_DRIVER_ALIGNMENT                   FS_DRIVER_ALIGMENT
  #else
    #define FS_DRIVER_ALIGNMENT                   4     // The file system performs a 0-copy operation if the alignment of data buffer passed by the application
                                                        // matches this value. The value is specified in bytes.
  #endif
#endif

#ifndef   FS_DIRECTORY_DELIMITER
  #define FS_DIRECTORY_DELIMITER                  '\\'  // This is the character used for separating directory names in a path. Typ. '\\' or '/'.
#endif

#ifndef   FS_VERIFY_WRITE
  #define FS_VERIFY_WRITE                         0     // Set to 1 to verify every write sector operation (tests the driver and hardware)
#endif

#ifndef     FS_SUPPORT_BUSY_LED
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_SUPPORT_BUSY_LED                   0     // If set to 1 the file system calls the function registered via FS_SetBusyLEDCallback()
                                                        // at the beginning and at the end of an access to the storage medium.
  #else
    #define FS_SUPPORT_BUSY_LED                   1
  #endif
#endif

#ifndef   FS_SUPPORT_CHECK_MEMORY
  #define FS_SUPPORT_CHECK_MEMORY                 0     // Check whether the memory is accessible for device driver, otherwise burst operation are not used.
                                                        // To check the memory, please make sure the FS_SetMemAccessCallback() is called for the appropriate
                                                        // volume/device.
#endif

#ifndef   FS_OPTIMIZE
  #define FS_OPTIMIZE                                   // Allow optimizations such as "__arm __ramfunc" or similar. What works best depends on target.
#endif

#ifndef   FS_SUPPORT_DEINIT
  #define FS_SUPPORT_DEINIT                       0     // Allows to have a deinitialization of the file system
#endif

#ifndef   FS_SUPPORT_EXT_MEM_MANAGER
  #define FS_SUPPORT_EXT_MEM_MANAGER              0     // Set to 1 will use external alloc/free memory functions, these must be set with FS_SetMemFunc()
#endif

#ifdef      FS_USE_FILE_BUFFER
  #ifndef   FS_SUPPORT_FILE_BUFFER
    #define FS_SUPPORT_FILE_BUFFER                FS_USE_FILE_BUFFER  // Backward compatibility define for the old implementation of the file buffer
  #endif
#endif

#ifndef     FS_SUPPORT_FILE_BUFFER
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_SUPPORT_FILE_BUFFER                0     // Configures the type of file buffering at file handle level.
  #else
    #define FS_SUPPORT_FILE_BUFFER                1
  #endif
#endif

#ifndef   FS_USE_FILE_BUFFER
  #define FS_USE_FILE_BUFFER                      FS_SUPPORT_FILE_BUFFER  // Backward compatibility define for the old implementation of the file buffer
#endif

#ifndef   FS_SUPPORT_FILE_BUFFER_LIST
  #define FS_SUPPORT_FILE_BUFFER_LIST             0     // Configures if the file buffers allocated by the file system are managed in a separate list.
                                                        // This gives the application the ability to disable at runtime a file buffer of a file handle.
#endif

#ifndef   FS_SUPPORT_READ_AHEAD
  #define FS_SUPPORT_READ_AHEAD                   0     // When set to 1 the file system activates the functionality which requests the FS_READAHEAD_Driver
                                                        // to read sector data in advance. This is typically activated for search operations in the allocation table.
#endif

#ifndef   FS_HUGE
  #define FS_HUGE
#else
  #define FS_HUGE  huge
#endif

#ifndef     FS_X_PANIC
  #if       (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_PARA)
    #define FS_X_PANIC(ErrorCode)                 FS_X_Panic(ErrorCode)
  #else
    #define FS_X_PANIC(ErrorCode)
  #endif
#endif

#ifndef   FS_SUPPORT_PROFILE
  #define FS_SUPPORT_PROFILE                      0     // Profiling instrumentation (via SystemView by default). Disabled by default for performance reasons.
#endif

#ifndef   FS_SUPPORT_PROFILE_END_CALL
  #define FS_SUPPORT_PROFILE_END_CALL             0     // Profiling instrumentation of function return events. Disabled by default to avoid generating too many events.
#endif

#ifndef   FS_SUPPRESS_EOF_ERROR
  #define FS_SUPPRESS_EOF_ERROR                   0     // When set to 1 no error is reported via FS_FError() if the end-of-file indicator is set.
#endif

#ifndef   FS_SUPPORT_POSIX
  #define FS_SUPPORT_POSIX                        0     // When to 1 some of the API functions such as FS_Truncate() behave like the POSIX function counterpart.
#endif

#ifndef   FS_SUPPORT_MBCS
  #define FS_SUPPORT_MBCS                         0     // Enables/disables the support for multi-byte character sets such as Shift JIS.
#endif

#ifndef     FS_SUPPORT_VOLUME_ALIAS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_SUPPORT_VOLUME_ALIAS               0     // Enables/disables the support for setting a volume alias. If set to 0 the volume alias feature is not supported.
  #else
    #define FS_SUPPORT_VOLUME_ALIAS               1
  #endif
#endif

#ifndef   FS_MAX_LEN_VOLUME_ALIAS
  #define FS_MAX_LEN_VOLUME_ALIAS                 0     // Maximum number of characters allowed in a volume alias including the 0-terminator. If set to 0 only a pointer to the volume alias is stored.
#endif

#ifndef   FS_SUPPORT_EXT_ASCII
  #define FS_SUPPORT_EXT_ASCII                    0     // Set to 1 to enable support for extended ASCII characters in the file names.
#endif

#ifdef    FS_FAT_SUPPORT_UTF8                           // Compatibility define that enables UTF-8 encoding at compile time for the FAT file system.
  #define FS_SUPPORT_FILE_NAME_ENCODING           FS_FAT_SUPPORT_UTF8
#endif

#ifndef   FS_SUPPORT_FILE_NAME_ENCODING
  #define FS_SUPPORT_FILE_NAME_ENCODING           0     // Set to 1 to enable the support for encoded file names such as UTF-8 and Shift JIS.
#endif

#ifndef     FS_NUM_VOLUMES
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NUM_VOLUMES                        1     // Maximum number of volumes the file system can handle.
  #else
    #define FS_NUM_VOLUMES                        4
  #endif
#endif

#ifndef   FS_NUM_DIR_HANDLES
  #define FS_NUM_DIR_HANDLES                      1     // Maximum number of directory handles.
#endif

#ifdef    FS_VERIFY_BUFFER_SIZE                         // This configuration define is deprecated. Use FS_BUFFER_SIZE_VERIFY instead.
  #define FS_BUFFER_SIZE_VERIFY                   FS_VERIFY_BUFFER_SIZE
#endif
#ifndef     FS_BUFFER_SIZE_VERIFY
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_BUFFER_SIZE_VERIFY                 32    // Size of the work buffer allocated on the stack by FS_Verify()
  #else
    #define FS_BUFFER_SIZE_VERIFY                 128
  #endif
#endif

#ifndef     FS_SUPPORT_SECTOR_BUFFER_CACHE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MAX_SPEED)
    #define FS_SUPPORT_SECTOR_BUFFER_CACHE        1     // Set to 1 to enable the reuse of sector data between the API function calls. It increases the ROM and RAM usage.
  #else
    #define FS_SUPPORT_SECTOR_BUFFER_CACHE        0
  #endif
#endif

#ifdef    FS_UNUSED_DATA_FILL_PATTERN                   // This configuration define is deprecated. Use FS_FILL_PATTERN_UNUSED_DATA instead.
  #define FS_FILL_PATTERN_UNUSED_DATA             FS_UNUSED_DATA_FILL_PATTERN
#endif
#ifndef   FS_FILL_PATTERN_UNUSED_DATA
  #define FS_FILL_PATTERN_UNUSED_DATA             0xFF  // Value to be used to fill the unused bytes at the end of a logical sector.
#endif

#ifdef    FS_FILE_COPY_BUFFER_SIZE                      // This configuration define is deprecated. Use FS_BUFFER_SIZE_FILE_COPY instead.
  #define FS_BUFFER_SIZE_FILE_COPY                FS_FILE_COPY_BUFFER_SIZE
#endif
#ifndef     FS_BUFFER_SIZE_FILE_COPY
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_BUFFER_SIZE_FILE_COPY              32    // Size of the buffer used by FS_CopyFile(). This buffer is allocated on stack.
  #else
    #define FS_BUFFER_SIZE_FILE_COPY              512
  #endif
#endif

#ifndef     FS_BUFFER_SIZE_FILE_WIPE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_BUFFER_SIZE_FILE_WIPE              32    // Size of the buffer used by FS_WipeFile(). This buffer is allocated on stack.
  #else
    #define FS_BUFFER_SIZE_FILE_WIPE              512
  #endif
#endif

#ifndef     FS_BUFFER_SIZE_TRUNCATE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_BUFFER_SIZE_TRUNCATE               32    // Size of the buffer used by FS_Truncate(). This buffer is allocated on stack.
  #else
    #define FS_BUFFER_SIZE_TRUNCATE               512
  #endif
#endif

#ifndef     FS_BUFFER_SIZE_FILE_PRINT
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_BUFFER_SIZE_FILE_PRINT             32    // Size of the buffer used by FS_FPrintf(). This buffer is allocated on stack.
  #else
    #define FS_BUFFER_SIZE_FILE_PRINT             128
  #endif
#endif

#ifndef     FS_SUPPORT_SECTOR_BUFFER_BURST
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MAX_SPEED)
    #define   FS_SUPPORT_SECTOR_BUFFER_BURST      1     // Set to 1 to enable the sector buffer to read more than one sector at once.
  #else
    #define   FS_SUPPORT_SECTOR_BUFFER_BURST      0
  #endif
#endif

#ifndef   FS_SUPPORT_GPT
  #define FS_SUPPORT_GPT                          0     // Enables/disables the support for GPT partitions.
#endif

#ifndef   FS_MAX_NUM_BYTES_PART_NAME
  #define FS_MAX_NUM_BYTES_PART_NAME              (72 + 1)      // Maximum number of bytes in the name of a GPT partition (UTF-8 encoded)
#endif

/*********************************************************************
*
*       Journal configuration
*/
#ifndef   FS_SUPPORT_JOURNAL
  #define FS_SUPPORT_JOURNAL                      0     // Enables/disables the support for journaling.
#endif

#ifndef   FS_JOURNAL_FILE_NAME
  #define FS_JOURNAL_FILE_NAME                    "Journal.dat"   // This is the full path to the file to be used by the Journal module.
#endif

#ifndef   FS_MAX_LEN_JOURNAL_FILE_NAME
  #define FS_MAX_LEN_JOURNAL_FILE_NAME            0     // Maximum number of characters allowed in the name of the journal file including the 0-terminator.
                                                        // If set to 0 the name of the journal file is specified by FS_JOURNAL_FILE_NAME and it cannot be changed at runtime.
#endif

#ifndef   FS_JOURNAL_ENABLE_STATS
  #define FS_JOURNAL_ENABLE_STATS                 (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL)   // Statistics only in debug builds
#endif

#ifndef     FS_JOURNAL_SUPPORT_FREE_SECTOR
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_JOURNAL_SUPPORT_FREE_SECTOR        0     // When set to 1 the storage driver is informed about unused sectors.
  #else
    #define FS_JOURNAL_SUPPORT_FREE_SECTOR        1
  #endif
#endif

#ifndef     FS_JOURNAL_SUPPORT_FAST_SECTOR_SEARCH
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_JOURNAL_SUPPORT_FAST_SECTOR_SEARCH 0     // When set to 1 each entry in the logical-to-physical mapping table is 32-bit large
                                                        // which increases performance but at the same time it increases the RAM usage.
  #else
    #define FS_JOURNAL_SUPPORT_FAST_SECTOR_SEARCH 1
  #endif
#endif

#ifndef   FS_JOURNAL_OPTIMIZE_SPACE_USAGE
  #define FS_JOURNAL_OPTIMIZE_SPACE_USAGE         0     // When set to 1 consecutive sector free operations are grouped together to reduce
                                                        // the space usage of the journal file. At the same time the RAM usage increases.
#endif

/*********************************************************************
*
*       FAT file system layer defines
*/
#ifndef     FS_FAT_SUPPORT_FAT32
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_FAT_SUPPORT_FAT32                  0     // Set to 0 disable the support for FAT32.
  #else
    #define FS_FAT_SUPPORT_FAT32                  1
  #endif
#endif

#ifndef   FS_FAT_SUPPORT_UTF8                           // This configuration define is deprecated. Use FS_SUPPORT_FILE_NAME_ENCODING instead.
  #define FS_FAT_SUPPORT_UTF8                     0
#endif

#ifndef   FS_UNICODE_UPPERCASE_EXT
  #define FS_UNICODE_UPPERCASE_EXT                {0x0000, 0x0000}      // Allow to extend the static Unicode lower to upper case table
#endif

#ifndef   FS_MAINTAIN_FAT_COPY
  #define FS_MAINTAIN_FAT_COPY                    0     // Shall the 2nd FAT (copy) be maintained.
#endif

#ifndef     FS_FAT_USE_FSINFO_SECTOR
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_FAT_USE_FSINFO_SECTOR              0     // Use and update FSInfo sector on FAT32 media. For FAT12/FAT116 there is no FSInfo sector.
  #else
    #define FS_FAT_USE_FSINFO_SECTOR              1
  #endif
#endif

#ifndef     FS_FAT_OPTIMIZE_DELETE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_FAT_OPTIMIZE_DELETE                0     // Accelerate delete of large files.
  #else
    #define FS_FAT_OPTIMIZE_DELETE                1
  #endif
#endif

#ifndef     FS_FAT_OPTIMIZE_LINEAR_ACCESS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_FAT_OPTIMIZE_LINEAR_ACCESS         0     // Improves the performance when accessing files with clusters allocated linearly.
  #else
    #define FS_FAT_OPTIMIZE_LINEAR_ACCESS         1
  #endif
#endif

#ifndef   FS_FAT_PERMIT_RO_FILE_MOVE
  #define FS_FAT_PERMIT_RO_FILE_MOVE              0     // When set to 1 read-only files and directories can be moved or renamed.
#endif

#ifndef   FS_FAT_UPDATE_DIRTY_FLAG
  #define FS_FAT_UPDATE_DIRTY_FLAG                0     // When set to 1 a flag is updated in the boot sector to indicate that the volume was correctly unmounted before reset.
#endif

#ifndef     FS_FAT_SUPPORT_FREE_CLUSTER_CACHE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_FAT_SUPPORT_FREE_CLUSTER_CACHE     0     // Set to 0 to disable the support for free cluster cache and to reduce ROM usage). The free cluster cache is active only in the FAST write mode.
  #else
    #define FS_FAT_SUPPORT_FREE_CLUSTER_CACHE     1
  #endif
#endif

#ifndef   FS_FAT_LFN_MAX_SHORT_NAME
  #define FS_FAT_LFN_MAX_SHORT_NAME               1000  // Limit for the index of a short file name. The maximum index value of a short file name is FS_FAT_LFN_MAX_SHORT_NAME + FS_FAT_LFN_BIT_ARRAY_SIZE - 1.
#endif

#ifndef     FS_FAT_LFN_BIT_ARRAY_SIZE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_FAT_LFN_BIT_ARRAY_SIZE             32    // Size in bits of the array used to remember the index of the short file names found. The bit array is allocated on the stack.
  #else
    #define FS_FAT_LFN_BIT_ARRAY_SIZE             256
  #endif
#endif

#ifndef     FS_FAT_LFN_LOWER_CASE_SHORT_NAMES
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_FAT_LFN_LOWER_CASE_SHORT_NAMES     0     // When set to 1 only a short name is created if all the characters in the base name or extension are small letters or punctuation characters.
  #else
    #define FS_FAT_LFN_LOWER_CASE_SHORT_NAMES     1
  #endif
#endif

#if       FS_FAT_SUPPORT_UTF8                           // Compatibility define that enables UTF-8 encoding at compile time.
  #define FS_FAT_LFN_UNICODE_CONV_DEFAULT         &FS_UNICODE_CONV_UTF8
#endif

#if FS_SUPPORT_FILE_NAME_ENCODING
  #ifndef   FS_FAT_LFN_UNICODE_CONV_DEFAULT             // Default Unicode file name encoder. Can be set to NULL in order to save ROM space if another Unicode encoder is configured at runtime.
    #define FS_FAT_LFN_UNICODE_CONV_DEFAULT       &FS_UNICODE_CONV_CP437
  #endif
#endif

#ifndef   FS_FAT_CHECK_UNUSED_DIR_ENTRIES
  #define FS_FAT_CHECK_UNUSED_DIR_ENTRIES         0     // Enables/disables the checking of directory entries located after the first directory entry marked as not in use.
#endif

/*********************************************************************
*
*       EFS file system layer defines
*/
#ifndef     FS_EFS_OPTIMIZE_DELETE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_EFS_OPTIMIZE_DELETE                0     // Accelerate delete of large files.
  #else
    #define FS_EFS_OPTIMIZE_DELETE                1
  #endif
#endif

#ifndef   FS_EFS_CASE_SENSITIVE
  #define FS_EFS_CASE_SENSITIVE                   0     // Configures if the case of letters should be considered when comparing the name of a file or directory.
                                                        // 0 means that the case of a letter is not important.
#endif

#ifndef   FS_EFS_MAX_DIR_ENTRY_SIZE
  #define FS_EFS_MAX_DIR_ENTRY_SIZE               255   // Maximum number of bytes of an EFS directory entry
#endif

#ifndef     FS_EFS_SUPPORT_STATUS_SECTOR
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_EFS_SUPPORT_STATUS_SECTOR          0     // Use and update the status sector which stores the number of free clusters.
  #else
    #define FS_EFS_SUPPORT_STATUS_SECTOR          1
  #endif
#endif

#ifndef     FS_EFS_SUPPORT_DIRENTRY_BUFFERS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_EFS_SUPPORT_DIRENTRY_BUFFERS       1     // When set to 1 the buffers for reading the directory entries are allocated
                                                        // from the memory pool of file system instead on the stack.
                                                        // Each buffer is FS_EFS_MAX_DIR_ENTRY_SIZE + 1 bytes large.
  #else
    #define FS_EFS_SUPPORT_DIRENTRY_BUFFERS       0
  #endif
#endif

#ifndef   FS_EFS_NUM_DIRENTRY_BUFFERS
  #define FS_EFS_NUM_DIRENTRY_BUFFERS             2     // Number of buffers to store directory entries.
#endif

#if FS_SUPPORT_FILE_NAME_ENCODING
  #ifndef   FS_EFS_UNICODE_CONV_DEFAULT
    #define FS_EFS_UNICODE_CONV_DEFAULT           &FS_UNICODE_CONV_UTF8
  #endif
#endif // FS_SUPPORT_FILE_NAME_ENCODING

#ifndef     FS_EFS_SUPPORT_FREE_CLUSTER_CACHE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MAX_SPEED)
    #define FS_EFS_SUPPORT_FREE_CLUSTER_CACHE     1     // Set to 0 to disable the support for free cluster cache and to reduce ROM usage. The free cluster cache is active only in the FAST write mode.
  #else
    #define FS_EFS_SUPPORT_FREE_CLUSTER_CACHE     0
  #endif
#endif

#ifndef     FS_EFS_OPTIMIZE_LINEAR_ACCESS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_EFS_OPTIMIZE_LINEAR_ACCESS         0     // Improves the performance when accessing files with clusters allocated linearly.
  #else
    #define FS_EFS_OPTIMIZE_LINEAR_ACCESS         1
  #endif
#endif

/*********************************************************************
*
*       Storage layer
*/
#ifndef   FS_STORAGE_ENABLE_STAT_COUNTERS
  #define FS_STORAGE_ENABLE_STAT_COUNTERS         (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_PARA)     // Enables / disables the statistical counters of the storage layer.
#endif

#ifndef   FS_STORAGE_SUPPORT_DEVICE_ACTIVITY
  #define FS_STORAGE_SUPPORT_DEVICE_ACTIVITY      (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_PARA)     // Enables / disables the support for device activity callback.
#endif

#ifndef   FS_STORAGE_SUPPORT_MOUNT
  #define FS_STORAGE_SUPPORT_MOUNT                ((FS_SUPPORT_FAT == 0) && (FS_SUPPORT_EFS == 0))  // Enables / disables the support for the mount operation.
#endif

/*********************************************************************
*
*       CLib
*/
#ifndef     FS_NO_CLIB
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NO_CLIB                            1     // When set to 1 the file system uses the internal standard C library functions.
  #else
    #define FS_NO_CLIB                            0
  #endif
#endif

#ifndef   FS_VSNPRINTF
  #define FS_VSNPRINTF(s, n, f, a)                SEGGER_vsnprintf(s, n, f, a)
#endif

#ifndef   FS_MEMCPY
  #define FS_MEMCPY(s1, s2, n)                    memcpy(s1, s2, n)
#endif

#ifndef   FS_MEMXOR
  #define FS_MEMXOR(pd, ps, n)                    SEGGER_memxor(pd, ps, n);
#endif

#if (FS_NO_CLIB == 0)
#ifndef   FS_MEMCMP
  #define FS_MEMCMP(s1, s2, n)                    memcmp(s1, s2, n)
#endif
#ifndef   FS_MEMSET
  #define FS_MEMSET(s, c, n)                      memset(s, c, n)
#endif
#ifndef   FS_STRCMP
  #define FS_STRCMP(s1, s2)                       strcmp(s1, s2)
#endif
#ifndef   FS_STRCPY
  #define FS_STRCPY(s1, s2)                       strcpy(s1, s2)
#endif
#ifndef   FS_STRNCPY
  #define FS_STRNCPY(s1, s2, n)                   strncpy(s1, s2, n)
#endif
#ifndef   FS_STRLEN
  #define FS_STRLEN(s)                            strlen(s)
#endif
#ifndef   FS_STRNCAT
  #define FS_STRNCAT(s1, s2, n)                   strncat(s1, s2, n)
#endif
#ifndef   FS_STRNCMP
  #define FS_STRNCMP(s1, s2, n)                   strncmp(s1, s2, n)
#endif
#ifndef   FS_STRCHR
  #define FS_STRCHR(s, c)                         strchr(s, c)
#endif
#ifndef   FS_TOUPPER
  #define FS_TOUPPER(c)                           toupper(c)
#endif
#ifndef   FS_TOLOWER
  #define FS_TOLOWER(c)                           tolower(c)
#endif
#ifndef   FS_ISUPPER
  #define FS_ISUPPER(c)                           isupper(c)
#endif
#ifndef   FS_ISLOWER
  #define FS_ISLOWER(c)                           islower(c)
#endif
#endif // FS_NO_CLIB == 0

/*********************************************************************
*
*       CRC calculation functions
*/
#ifndef   FS_CRC32_CALC
  #define FS_CRC32_CALC(pData, NumBytes, crc)         FS_CRC32_Calc(pData, NumBytes, crc)         // 32-bit CRC that uses the polynomial 0x04C11DB7 (CRC-32)
#endif

#ifndef   FS_CRC16_CCITT_CALC
  #define FS_CRC16_CCITT_CALC(pData, NumBytes, crc)   FS_CRC16_CCITT_Calc(pData, NumBytes, crc)   // 16-bit CRC that uses the polynomial 0x1021 (CRC-16-CCITT)
#endif

#ifndef   FS_CRC16_ANSI_CALC
  #define FS_CRC16_ANSI_CALC(pData, NumBytes, crc)    FS_CRC16_ANSI_Calc(pData, NumBytes, crc)    // 16-bit CRC that uses the polynomial 0x8005 (CRC-16-ANSI)
#endif

#ifndef   FS_CRC8_CCITT_CALC
  #define FS_CRC8_CCITT_CALC(pData, NumBytes, crc)    FS_CRC8_CCITT_Calc(pData, NumBytes, crc)    // 8-bit CRC that uses the polynomial 0x07 (CRC-8-CCITT)
#endif

/*********************************************************************
*
*       OS layer
*/
#ifndef   FS_OS_LOCKING
  #define FS_OS_LOCKING                           FS_OS_LOCKING_NONE     // Configures the locking of the file system against concurrent access form different tasks.
#endif

#ifndef   FS_OS_SUPPORT_RUNTIME_CONFIG
  #define FS_OS_SUPPORT_RUNTIME_CONFIG            0     // Enables/disables the runtime configuration of the OS layer. 0 means not configurable at runtime.
#endif

/*********************************************************************
*
*       FS_USE_PARA
*
*   Typ. used to avoid warnings for undefined parameters.
*/
#ifndef FS_USE_PARA
  #if defined(NC30) || defined(NC308) || defined(NC100)
    #define FS_USE_PARA(para)
  #else
    #define FS_USE_PARA(para) (void)(para)
  #endif
#endif

/*********************************************************************
*
*       NAND driver
*/
#ifdef    FS_NAND_MAXUNIT                               // This configuration define is deprecated. Use FS_NAND_NUM_UNITS instead.
  #define FS_NAND_NUM_UNITS                       FS_NAND_MAXUNIT
#endif
#ifndef     FS_NAND_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NAND_NUM_UNITS                     1     // Maximum number of NAND driver instances.
  #else
    #define FS_NAND_NUM_UNITS                     4
  #endif
#endif

#ifndef   FS_NAND_MAX_ERASE_CNT_DIFF
  #define FS_NAND_MAX_ERASE_CNT_DIFF              200   // Configures the threshold of the active wear leveling.
#endif

#ifndef   FS_NAND_NUM_READ_RETRIES
  #define FS_NAND_NUM_READ_RETRIES                10    // Number of retries in case of a read error (device error or uncorrectable bit error)
#endif

#ifndef   FS_NAND_NUM_WRITE_RETRIES
  #define FS_NAND_NUM_WRITE_RETRIES               10    // Number of retries performed in case of a write error
#endif

#ifndef     FS_NAND_ENABLE_STATS
  #if (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL)
    #define FS_NAND_ENABLE_STATS                  1      // Enables / disables statistical counters.
  #else
    #define FS_NAND_ENABLE_STATS                  0      // Enables / disables statistical counters.
  #endif
#endif

#ifndef   FS_NAND_ENABLE_STATS_SECTOR_STATUS
  #define FS_NAND_ENABLE_STATS_SECTOR_STATUS      1     // Enables / disables the collection of statistical information about the number of valid logical sectors.
#endif

#ifndef   FS_NAND_RECLAIM_DRIVER_BAD_BLOCKS
  #define FS_NAND_RECLAIM_DRIVER_BAD_BLOCKS       0     // Configures how the driver handles the blocks it marked as defective when the NAND flash device is low-level formatted.
#endif

#ifndef   FS_NAND_VERIFY_WRITE
  #define FS_NAND_VERIFY_WRITE                    0     // Reads back and compares the data of sector to check if the write operation was successful.
                                                        // Enabling this feature reduces the write performance.
#endif

#ifndef   FS_NAND_VERIFY_ERASE
  #define FS_NAND_VERIFY_ERASE                    0     // If set to 1 the NAND driver checks after each block erase operation if all the bytes in the block are set to 0xFF.
                                                        // Enabling this feature reduces the write performance.
#endif

#ifdef    FS_NAND_ENABLE_TRIM                           // This configuration define is deprecated. Use FS_NAND_SUPPORT_TRIM instead.
  #define FS_NAND_SUPPORT_TRIM                    FS_NAND_ENABLE_TRIM
#endif
#ifndef   FS_NAND_SUPPORT_TRIM
  #define FS_NAND_SUPPORT_TRIM                    FS_SUPPORT_FREE_SECTOR     // Enables/disables the TRIM functionality
#endif

#ifdef    FS_NAND_ENABLE_CLEAN                          // This configuration define is deprecated. Use FS_NAND_SUPPORT_CLEAN instead.
  #define FS_NAND_SUPPORT_CLEAN                   FS_NAND_ENABLE_CLEAN
#endif
#ifndef     FS_NAND_SUPPORT_CLEAN
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NAND_SUPPORT_CLEAN                 0     // Enables/disables the CLEAN functionality.
  #else
    #define FS_NAND_SUPPORT_CLEAN                 1
  #endif
#endif

#ifndef   FS_NAND_OPTIMIZE_SPARE_AREA_READ
  #define FS_NAND_OPTIMIZE_SPARE_AREA_READ        0     // If set to 1 the Universal NAND driver reads only the data it has to evaluate from the spare area of a page.
                                                        // By default, the Universal NAND driver reads the entire spare area.
#endif

#ifndef     FS_NAND_ECC_HOOK_DEFAULT
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NAND_ECC_HOOK_DEFAULT              NULL  // Default ECC calculation algorithm. Can be set to NULL for NAND flash devices with HW ECC to reduce the ROM usage.
  #else
    #define FS_NAND_ECC_HOOK_DEFAULT              &FS_NAND_ECC_SW_1BIT
  #endif
#endif

#ifndef     FS_NAND_SUPPORT_BLOCK_GROUPING
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NAND_SUPPORT_BLOCK_GROUPING        0     // If set to 1 the driver handles two or more adjacent physical NAND blocks as a single block to reduce the RAM usage.
                                                        // The number of blocks in the group has to be configured at runtime via FS_NAND_UNI_SetNumBlocksPerGroup().
                                                        // Can be set to 0 to reduce the RAM and ROM usage of the driver.
  #else
    #define FS_NAND_SUPPORT_BLOCK_GROUPING        1
  #endif
#endif

#ifndef   FS_NAND_ENABLE_ERROR_RECOVERY
  #define FS_NAND_ENABLE_ERROR_RECOVERY           0     // Enables/disable the bit error recovery via RAID driver.
#endif

#ifndef   FS_NAND_MAX_SPARE_AREA_SIZE
  #define FS_NAND_MAX_SPARE_AREA_SIZE             0     // Configures the size of the buffer which stores the data of spare area as a number of bytes.
                                                        // Usually this is the size of the file system sector divided by 32. The Micron MT29F8G08ABABA device (and possibly others)
                                                        // have spare area of 224 bytes for a 4KB page. The default computation will not work in this case and it is required to use
                                                        // this define to set the size of the spare area.
#endif

#ifndef   FS_NAND_MAX_PAGE_SIZE
  #define FS_NAND_MAX_PAGE_SIZE                   0     // Configures the size of the buffer which stores the data of a page.
#endif

#ifndef     FS_NAND_FILL_READ_BUFFER
  #if       ((FS_SUPPORT_FAT == 0) && (FS_SUPPORT_EFS == 0))
    #define FS_NAND_FILL_READ_BUFFER              1     // Fills the read buffer with a known value when the sector data is invalid.
                                                        // The fill pattern is configured via FS_NAND_READ_BUFFER_FILL_PATTERN.
                                                        // By default this feature is active only if no file system is used,
                                                        // since a file system reads only from sectors that contain valid data.

  #else
    #define FS_NAND_FILL_READ_BUFFER              0
  #endif
#endif

#ifndef   FS_NAND_READ_BUFFER_FILL_PATTERN
  #define FS_NAND_READ_BUFFER_FILL_PATTERN        0xFF  // The read sector buffer is filled with this value if the sector data is not valid.
#endif

#ifndef   FS_NAND_SUPPORT_READ_CACHE
  #define FS_NAND_SUPPORT_READ_CACHE              0     // Set to 1 to enable the support for read cache.
#endif

#ifndef   FS_NAND_SUPPORT_AUTO_DETECTION
  #define FS_NAND_SUPPORT_AUTO_DETECTION          1     // Set to 0 to disable the automatic identification of the NAND flash parameters
                                                        // in the NAND physical layers FS_NAND_PHY_x and FS_NAND_PHY_x8.
#endif

#ifndef   FS_NAND_STAT_MAX_BIT_ERRORS
  #define FS_NAND_STAT_MAX_BIT_ERRORS             1     // Number of statistical counters for bit errors maintained by the NAND driver.
#endif

#ifdef    FS_NAND_ENABLE_FAST_WRITE                     // This configuration define is deprecated. Use FS_NAND_SUPPORT_FAST_WRITE instead.
  #define FS_NAND_SUPPORT_FAST_WRITE              FS_NAND_ENABLE_FAST_WRITE
#endif
#ifndef     FS_NAND_SUPPORT_FAST_WRITE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NAND_SUPPORT_FAST_WRITE            0     // SLC1 NAND driver: If set to 1 the application will be able to perform write operations at maximum speed during which no NAND block is erased or copied.
                                                        // Universal NAND driver: If set to 1 data is written also directly to a data block, if possible. Set to 0 to reduce the RAM and ROM usage of the driver.
  #else
    #define FS_NAND_SUPPORT_FAST_WRITE            1
  #endif
#endif

#ifndef   FS_NAND_MAX_BIT_ERROR_CNT
  #define FS_NAND_MAX_BIT_ERROR_CNT               0     // Number of bit errors in a page which trigger a relocation of a block.
                                                        // It prevents the accumulation of more bit errors than ECC can correct which can lead to data loss.
                                                        // Valid values: ==0 - feature disabled, !=0 - number of bit errors that triggers the relocation of the block.
                                                        // Enabling this feature increases the data reliability but it might affect performance.
#endif

#ifndef   FS_NAND_SUPPORT_FAIL_SAFE_ERASE
  #define FS_NAND_SUPPORT_FAIL_SAFE_ERASE         0     // Enables or disables the detection of an interrupted block erase operation.
#endif

#ifdef      FS_NAND_USE_LARGE_BLOCKS                    // This configuration define is deprecated. Use FS_NAND_BLOCK_TYPE instead.
  #if       FS_NAND_USE_LARGE_BLOCKS
    #define FS_NAND_BLOCK_TYPE                    2     // Use 1 DataFlash sector for 1 NAND driver block.
  #else
    #define FS_NAND_BLOCK_TYPE                    1      // Use 8 DataFlash blocks for 1 NAND driver block.
  #endif
#endif

#ifndef   FS_NAND_BLOCK_TYPE
  #define FS_NAND_BLOCK_TYPE                      2     // For better performance we use 1 DataFlash sector as a NAND driver block.
                                                        // A sector is the largest erasable unit in a DataFlash.
                                                        // Note: the physical layer is not fail-safe when FS_NAND_BLOCK_TYPE is set to 1
                                                        //       since a group of 8 consecutive DataFlash blocks are used as one NAND driver block.
#endif

#ifndef   FS_NAND_DEVICE_OPERATION_TIMEOUT
  #define FS_NAND_DEVICE_OPERATION_TIMEOUT        500   // Timeout for any device operation in milliseconds
                                                        // The erase operation of a NAND block takes the longest time to complete (about 2ms).
#endif

#ifndef   FS_NAND_DEVICE_OPERATION_POLL_DELAY
  #define FS_NAND_DEVICE_OPERATION_POLL_DELAY     0     // Delay between two status requests in milliseconds
#endif

#ifndef   FS_NAND_RESET_TIME
  #define FS_NAND_RESET_TIME                      2     // Number of milliseconds to wait for NAND flash device to reset.
#endif

#ifndef   FS_NAND_SUPPORT_COMPATIBILITY_MODE
  #define FS_NAND_SUPPORT_COMPATIBILITY_MODE      0     // emFile versions older than 4.06b wrongly stored the management data to regions
                                                        // of the spare area that were not protected by the HW ECC of Micron MT29F1G01ABAFD NAND flash device.
                                                        // The correction of this behavior introduces a data compatibility that can be avoided
                                                        // by setting this define either to 1 or to 2. When set to 1 the physical layer reads
                                                        // the management data from old location if at the new location no management data is present.
                                                        // When set to 2 the physical layer updates the management data to old and new location
                                                        // in addition to trying to read it from both of these locations.
#endif

#ifdef    FS_NAND_DEVICE_LIST_DEFAULT
  #define FS_NAND_SPI_DEVICE_LIST_DEFAULT         FS_NAND_DEVICE_LIST_DEFAULT
#endif
#ifndef     FS_NAND_SPI_DEVICE_LIST_DEFAULT
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NAND_SPI_DEVICE_LIST_DEFAULT       NULL
  #else
    #define FS_NAND_SPI_DEVICE_LIST_DEFAULT       &FS_NAND_SPI_DeviceListAll
  #endif
#endif

#ifndef     FS_NAND_SUPPORT_EXT_ONFI_PARA
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NAND_SUPPORT_EXT_ONFI_PARA         0     // Enables/disables support for extended ONFI parameters.
  #else
    #define FS_NAND_SUPPORT_EXT_ONFI_PARA         1
  #endif
#endif

#ifndef     FS_NAND_ONFI_DEVICE_LIST_DEFAULT
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NAND_ONFI_DEVICE_LIST_DEFAULT      NULL
  #else
    #define FS_NAND_ONFI_DEVICE_LIST_DEFAULT      &FS_NAND_ONFI_DeviceListDefault
  #endif
#endif

#ifndef     FS_NAND_2048X8_DEVICE_LIST_DEFAULT
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NAND_2048X8_DEVICE_LIST_DEFAULT    NULL
  #else
    #define FS_NAND_2048X8_DEVICE_LIST_DEFAULT    &FS_NAND_2048X8_DeviceListDefault
  #endif
#endif

/*********************************************************************
*
*       NOR driver
*/
#ifdef    FS_NOR_MAXUNIT                                  // This configuration define is deprecated. Use FS_NOR_NUM_UNITS instead.
  #define FS_NOR_NUM_UNITS                        FS_NOR_MAXUNIT
#endif
#ifndef     FS_NOR_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NOR_NUM_UNITS                      1       // Maximum number of driver instances.
  #else
    #define FS_NOR_NUM_UNITS                      4
  #endif
#endif

#ifdef    FS_NOR_ENABLE_CLEAN                             // This configuration define is deprecated. Use FS_NOR_SUPPORT_CLEAN instead.
  #define FS_NOR_SUPPORT_CLEAN                    FS_NOR_ENABLE_CLEAN
#endif
#ifndef     FS_NOR_SUPPORT_CLEAN
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NOR_SUPPORT_CLEAN                  0       // Enables / disables the support for application-driven garbage collection.
  #else
    #define FS_NOR_SUPPORT_CLEAN                  1
  #endif
#endif

#ifndef   FS_NOR_MAX_ERASE_CNT_DIFF
  #define FS_NOR_MAX_ERASE_CNT_DIFF               5000    // Maximum erase count difference. Usually 5% of the guaranteed erase cycles is a good value.
                                                          // Low values provide better leveling, but also more overhead for copying.
#endif

#ifndef   FS_NOR_SUPPORT_CRC
  #define FS_NOR_SUPPORT_CRC                      0       // If set to 1 the integrity of the management and sector data is protected by CRC.
#endif

#ifndef   FS_NOR_SUPPORT_ECC
  #define FS_NOR_SUPPORT_ECC                      0       // If set to 1 the integrity of the management and sector data is protected by ECC.
#endif

#ifndef   FS_NOR_MAX_NUM_BYTES_ECC_MAN
  #define FS_NOR_MAX_NUM_BYTES_ECC_MAN            2       // Maximum number of bytes in the parity check of the management data for 1 bit ECC.
#endif

#ifndef   FS_NOR_MAX_NUM_BYTES_ECC_DATA
  #define FS_NOR_MAX_NUM_BYTES_ECC_DATA           3       // Maximum number of bytes in the parity check of an ECC data block for 1 bit ECC (3 bytes over 256 bytes of data).
#endif

#ifndef   FS_NOR_MAX_NUM_BLOCKS_ECC_DATA
  #define FS_NOR_MAX_NUM_BLOCKS_ECC_DATA          2       // Maximum number of ECC blocks in a data sector for 1 bit ECC (2 blocks of 256 bytes for a logical sector of 512 bytes).
#endif

#ifndef     FS_NOR_PHY_SECTOR_RESERVE                     // Number of bytes to be reserved in a physical sector header.
  #if FS_NOR_SUPPORT_ECC
    #define FS_NOR_PHY_SECTOR_RESERVE             16      // Reserve place for 3 ECCs of 2 bytes and one status byte rounded up to a 4 byte boundary.
  #else
    #define FS_NOR_PHY_SECTOR_RESERVE             8
  #endif // FS_NOR_SUPPORT_ECC
#endif

#ifndef     FS_NOR_LOG_SECTOR_RESERVE                     // Number of bytes to be reserved in a logical sector header.
  #if FS_NOR_SUPPORT_ECC
    #define FS_NOR_LOG_SECTOR_RESERVE             16      // Reserve place for 2 ECCs of 2 bytes, one status byte and one 6 byte data ECC rounded up to a 4 byte boundary.
  #else
    #define FS_NOR_LOG_SECTOR_RESERVE             4
  #endif
#endif

#ifndef   FS_NOR_SUPPORT_VARIABLE_LINE_SIZE
  #define FS_NOR_SUPPORT_VARIABLE_LINE_SIZE       0       // If set to 1 the application can configure at runtime the line size of the NOR flash device
                                                          // and if the NOR flash device is able to rewrite the same data or not.
#endif

#ifndef   FS_NOR_LINE_SIZE
  #define FS_NOR_LINE_SIZE                        1       // Minimum number of bytes to be written at once. This is 1 for most SPI NOR flash devices and 4 for most external parallel NOR flash devices.
                                                          // The internal flash of some MCUs requires 16 bytes to be written at once. This information can be found in the data sheet of the storage device.
#endif

#ifndef     FS_NOR_CAN_REWRITE
  #if (FS_NOR_LINE_SIZE > 4)
    #define FS_NOR_CAN_REWRITE                    0       // Most flash chips can rewrite the same data, as long as 0-bits remain
  #else
    #define FS_NOR_CAN_REWRITE                    1
  #endif
#endif

#ifndef     FS_NOR_SUPPORT_FAIL_SAFE_ERASE
  #if       ((FS_NOR_CAN_REWRITE != 0) || (FS_NOR_SUPPORT_VARIABLE_LINE_SIZE != 0)) && (FS_NOR_PHY_SECTOR_RESERVE >= 4)
    #define FS_NOR_SUPPORT_FAIL_SAFE_ERASE        1       // The fail-safe erase operation works only if there is a sufficient number of reserved bytes in the physical sector header.
                                                          // In addition, the initial implementation of the Block Map NOR driver supported this feature only for NOR flash
                                                          // devices that support modifying a data byte more than once without an erase operation in between. This limitation
                                                          // in no longer true for the latest emFile version.
  #else
    #define FS_NOR_SUPPORT_FAIL_SAFE_ERASE        0
  #endif
#endif

#ifndef     FS_NOR_PHY_SECTOR_RESERVE_EX                  // Number of bytes to be reserved in the area of the physical sector header that stores information about the data status.
  #if FS_NOR_SUPPORT_ECC
    #define FS_NOR_PHY_SECTOR_RESERVE_EX          8       // Reserve place for 1 ECC of 2 bytes.
  #else
    #if (FS_NOR_LINE_SIZE < 4)
      #define FS_NOR_PHY_SECTOR_RESERVE_EX        0
    #else
      #if (FS_NOR_SUPPORT_CRC != 0) || (FS_NOR_SUPPORT_FAIL_SAFE_ERASE != 0)
        #define FS_NOR_PHY_SECTOR_RESERVE_EX      4
      #else
        #define FS_NOR_PHY_SECTOR_RESERVE_EX      0
      #endif
    #endif
  #endif
#endif

#ifndef     FS_NOR_LOG_SECTOR_RESERVE_EX                  // Number of bytes to be reserved in the area of the logical sector header that stores information about the data status.
  #if FS_NOR_SUPPORT_ECC
    #define FS_NOR_LOG_SECTOR_RESERVE_EX          4       // Reserve place for 1 ECC of 2 bytes.
  #else
    #if (FS_NOR_LINE_SIZE < 4)
      #define FS_NOR_LOG_SECTOR_RESERVE_EX        0
    #else
      #define FS_NOR_LOG_SECTOR_RESERVE_EX        (FS_NOR_LINE_SIZE - 4)
    #endif
  #endif
#endif

#ifndef     FS_NOR_SKIP_BLANK_SECTORS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NOR_SKIP_BLANK_SECTORS             0       // If set to 1 the low-level format operation does not erase the physical sectors that are already blank.
                                                          // This reduces the time it takes to perform a low-level format of a blank NOR flash.
  #else
    #define FS_NOR_SKIP_BLANK_SECTORS             1
  #endif
#endif

#ifndef     FS_NOR_VERIFY_WRITE
  #if       (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL)
    #define FS_NOR_VERIFY_WRITE                   1       // Reads back and compares the data of sector to check if the write operation was successful.
                                                          // Enabling this feature reduces the write performance.
  #else
    #define FS_NOR_VERIFY_WRITE                   0
  #endif
#endif

#ifndef     FS_NOR_VERIFY_ERASE
  #if       (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL)
    #define FS_NOR_VERIFY_ERASE                   1       // If set to 1 the driver checks after each sector erase operation if all the bytes in the block are set to 0xFF.
                                                          // Enabling this feature reduces the write performance.
  #else
    #define FS_NOR_VERIFY_ERASE                   0
  #endif
#endif

#ifndef   FS_NOR_MAX_ERASE_CNT
  #define FS_NOR_MAX_ERASE_CNT                    500000  // Maximum value for an erase count. Used to detect invalid erase counts when mounting the NOR flash.
                                                          // Typ. a NOR phy. sector can be erase at least 100,000 times.
#endif

#ifndef   FS_NOR_READ_BUFFER_FILL_PATTERN
  #define FS_NOR_READ_BUFFER_FILL_PATTERN         0xFF    // The read sector buffer is filled with this value if the sector data is not valid.
#endif

#ifndef     FS_NOR_NUM_FREE_SECTORCACHE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NOR_NUM_FREE_SECTORCACHE           32      // Number of logical sectors in the cache.
  #else
    #define FS_NOR_NUM_FREE_SECTORCACHE           100
  #endif
#endif

#ifndef   FS_NOR_ENABLE_STATS
  #define FS_NOR_ENABLE_STATS                     (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL)   // The statistics are enabled by default only in debug builds.
#endif

#ifndef   FS_NOR_NUM_READ_RETRIES
  #define FS_NOR_NUM_READ_RETRIES                 10      // Number of retries in case of a read error (device or CRC error)
#endif

#ifndef   FS_NOR_NUM_WRITE_RETRIES
  #define FS_NOR_NUM_WRITE_RETRIES                10      // Number of retries in case of a write error (verify or CRC error)
#endif

#ifndef   FS_NOR_NUM_ERASE_RETRIES
  #define FS_NOR_NUM_ERASE_RETRIES                10      // Number of retries in case of an erase error (device error)
#endif

#ifndef   FS_NOR_SUPPORT_VARIABLE_BYTE_ORDER
  #define FS_NOR_SUPPORT_VARIABLE_BYTE_ORDER      0       // If set to 1 the byte order of multi-byte the management data can be configured at runtime.
                                                          // The byte order of the host CPU is used if this define is set to 0.
#endif

#ifndef     FS_NOR_CRC_HOOK_DEFAULT
  #if       FS_NOR_SUPPORT_CRC
    #define FS_NOR_CRC_HOOK_DEFAULT               &FS_NOR_CRC_SW    // Default routines for the CRC calculation.
  #endif
#endif

#ifndef   FS_NOR_STRICT_FORMAT_CHECK
  #define FS_NOR_STRICT_FORMAT_CHECK              0       // If set to 1 an error is reported when the number of logical blocks stored during the low-level format operation is not equal to the actual number of logical blocks.
#endif

#ifndef   FS_NOR_OPTIMIZE_HEADER_WRITE
  #define FS_NOR_OPTIMIZE_HEADER_WRITE            0       // If set to 1 only the flash lines of the phy. and log. header are updated to storage that were actually modified.
#endif

#ifndef   FS_NOR_DATA_BUFFER_SIZE
  #define FS_NOR_DATA_BUFFER_SIZE                 32      // Size of the buffer used for accessing the data of the NOR flash device for internal operations. This buffer is allocated on the stack.
#endif

#ifndef   FS_NOR_SUPPORT_LEGACY_MODE
  #define FS_NOR_SUPPORT_LEGACY_MODE              0       // If set to 1 the data stored by an emFile version greater than 4.04c is fully backward compatible.
                                                          // The fail-safety of a data stored to a serial NOR flash is no longer guaranteed if this option is set to 1.
                                                          // For parallel NOR flash devices it can be safely set to 1 without affecting the fail-safety of write operations.
#endif

#ifndef   FS_NOR_SUPPORT_COMPATIBILITY_MODE
  #define FS_NOR_SUPPORT_COMPATIBILITY_MODE       0       // If set to 1 data written by an emFile version older than 4.04d to NOR flash sectors formatted by a version
                                                          // newer than or equal to 4.04d is recognized as valid. Setting both FS_NOR_SUPPORT_LEGACY_MODE and
                                                          // FS_NOR_SUPPORT_COMPATIBILITY_MODE to 1 is not supported.
#endif

#ifndef   FS_NOR_OPTIMIZE_DIRTY_CHECK
  #define FS_NOR_OPTIMIZE_DIRTY_CHECK             0       // If set to 1 the logical sectors that are marked as empty are checked for blank (all bytes 0xFF) only if required.
                                                          // When enabled this option requires 1 bit of RAM for each physical sector used as storage.
#endif

#ifndef   FS_NOR_SUPPORT_TRIM
  #define FS_NOR_SUPPORT_TRIM                     FS_SUPPORT_FREE_SECTOR      // Enables/disables the TRIM functionality of the NOR driver.
#endif

#ifndef   FS_NOR_ECC_HOOK_MAN_DEFAULT
  #define FS_NOR_ECC_HOOK_MAN_DEFAULT             &FS_NOR_ECC_SW_1bit_Man     // Default routines for the ECC calculation of management data.
#endif

#ifndef   FS_NOR_ECC_HOOK_DATA_DEFAULT
  #define FS_NOR_ECC_HOOK_DATA_DEFAULT            &FS_NOR_ECC_SW_1bit_Data    // Default routines for the ECC calculation of payload data.
#endif

#ifndef   FS_NOR_STAT_MAX_BIT_ERRORS
  #define FS_NOR_STAT_MAX_BIT_ERRORS              1     // Number of statistical counters for bit errors maintained by the NOR driver.
#endif

#ifndef     FS_NOR_SUPPORT_FORMAT
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NOR_SUPPORT_FORMAT                 0     // Enables or disables the ability to perform a low-level format. Disabling this option reduces the ROM usage.
  #else
    #define FS_NOR_SUPPORT_FORMAT                 1
  #endif
#endif

#ifndef     FS_NOR_OPTIMIZE_DATA_WRITE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NOR_OPTIMIZE_DATA_WRITE            0       // If set to 1 the data is written directly to a data block if possible instead of a work block.
  #else
    #define FS_NOR_OPTIMIZE_DATA_WRITE            1
  #endif
#endif

#ifndef   FS_NOR_OPTIMIZE_EMPTY_CHECK
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MAX_SPEED)
    #define FS_NOR_OPTIMIZE_EMPTY_CHECK           1       // Enables or disables the optimization related to the checking if a physical sector can be written to or not.
  #else
    #define FS_NOR_OPTIMIZE_EMPTY_CHECK           0
  #endif
#endif

#ifndef   FS_NOR_DI
  #define FS_NOR_DI()                                     // Macro to disable the interrupts globally
#endif

#ifndef   FS_NOR_EI
  #define FS_NOR_EI()                                     // Macro to enable the interrupts globally
#endif

#ifndef   FS_NOR_DELAY
  #define FS_NOR_DELAY()                                  // Macro to block the execution for a fixed time interval.
#endif

#ifndef   FS_NOR_FAR
  #define FS_NOR_FAR                                      // Memory type specifier. Used for 8/16-bit CPUs only.
#endif

#ifndef   FS_NOR_DMB
  #define FS_NOR_DMB()                                    // Macro for memory access synchronization.
#endif

#ifndef     FS_NOR_MAX_SECTOR_BLOCKS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NOR_MAX_SECTOR_BLOCKS              1       // Worst (known) case serial NOR device has 6 sector blocks.
                                                          // Typical serial NOR devices have only 1 (uniform sectors)
  #else
    #define FS_NOR_MAX_SECTOR_BLOCKS              6
  #endif
#endif

#ifndef   FS_NOR_MAX_NUM_DEVICES
  #define FS_NOR_MAX_NUM_DEVICES                  0       // Specifies the maximum number of device parameters that the user is allowed to define.
#endif

#ifndef   FS_NOR_TIMEOUT_SECTOR_ERASE
  #define FS_NOR_TIMEOUT_SECTOR_ERASE             3000    // Timeout for the sector erase operation (ms)
#endif

#ifndef   FS_NOR_TIMEOUT_PAGE_WRITE
  #define FS_NOR_TIMEOUT_PAGE_WRITE               5       // Timeout for the page program operation (ms)
#endif

#ifndef   FS_NOR_DELAY_SECTOR_ERASE
  #define FS_NOR_DELAY_SECTOR_ERASE               0       // Delay between two status requests sent when waiting for the completion of the erase operation (ms)
#endif

#ifndef   FS_NOR_BYTES_PER_PAGE
  #define FS_NOR_BYTES_PER_PAGE                   256     // Maximum number of bytes that can be stored to a NOR flash page
#endif

#ifndef     FS_NOR_DEVICE_LIST_DEFAULT
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NOR_DEVICE_LIST_DEFAULT            NULL    // Default list of recognized serial NOR flash devices.
  #else
    #define FS_NOR_DEVICE_LIST_DEFAULT            &FS_NOR_SPI_DeviceListDefault
  #endif
#endif

#ifndef   FS_NOR_MAX_NUM_BYTES_DUMMY
  #define FS_NOR_MAX_NUM_BYTES_DUMMY              5       // Maximum number of dummy bytes of a read operation that the physical layer can handle.
#endif

#ifndef   FS_NOR_AMD_WRITE_BUFFER_SIZE
  #define FS_NOR_AMD_WRITE_BUFFER_SIZE            32      // Size of the internal write buffer of AMD compatible NOR flash devices (in bytes). Typ. 32 bytes large.
#endif

#ifndef   FS_NOR_INTEL_WRITE_BUFFER_SIZE
  #define FS_NOR_INTEL_WRITE_BUFFER_SIZE          32      // Size of the internal write buffer of Intel compatible NOR flash devices (in bytes). Typ. 32 bytes large.
#endif

#ifndef   FS_NOR_AMD_STATUS_CHECK_TYPE
  #define FS_NOR_AMD_STATUS_CHECK_TYPE            0       // Configures how the status of a program and erase operation should be checked.
                                                          //   0 - poll toggle bit, no error checking (supported by legacy devices). Default for backward compatibility.
                                                          //   1 - poll toggle bit and check for errors (supported by modern devices, Micron for example)
                                                          //   2 - check status register (supported by Cypress HyperFlash)
#endif

#ifndef   FS_NOR_WRITE_TIMEOUT
  #define FS_NOR_WRITE_TIMEOUT                    10000   // Number of software cycles to wait for the completion of a write operation.
#endif

#ifndef   FS_NOR_ERASE_TIMEOUT
  #define FS_NOR_ERASE_TIMEOUT                    0       // Number of software cycles to wait for the completion of an sector erase operation.
#endif

#ifndef   FS_NOR_REVERSE_ENDIANESS
  #define FS_NOR_REVERSE_ENDIANESS                0       // Specifies if the byte ordering of the CFI information has to be reversed.
#endif

#ifndef   FS_NOR_OPTIMIZE_SECTOR_SIZE_LIST
  #define FS_NOR_OPTIMIZE_SECTOR_SIZE_LIST        1       // Enables or disables the optimization of the Sector Map NOR driver related to how the sizes
                                                          // of the physical sectors used as storage are maintained. When set to 1 the driver supports
                                                          // only physical sector sizes that are a power of 2 value.
#endif

#ifndef     FS_NOR_MIN_SECTOR_SIZE_SHIFT
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NOR_MIN_SECTOR_SIZE_SHIFT          16      // Minimum physical sector size supported by the Sector Map NOR driver as a power of 2 exponent (16 means 64 KB sectors).
                                                          // This configuration define is used only when FS_NOR_OPTIMIZE_SECTOR_SIZE_LIST is set to 1.
  #else
    #define FS_NOR_MIN_SECTOR_SIZE_SHIFT          8
  #endif
#endif

#ifndef     FS_NOR_MAX_NUM_SECTOR_SIZES
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_NOR_MAX_NUM_SECTOR_SIZES           1       // Maximum number of different physical sector sizes the Sector Map NOR driver can manage.
  #else
    #define FS_NOR_MAX_NUM_SECTOR_SIZES           11
  #endif
#endif

/*********************************************************************
*
*       MMC/SD driver
*/
#ifdef    FS_MMC_MAXUNIT                                  // Compatibility define. Use FS_MMC_NUM_UNITS instead.
  #define FS_MMC_NUM_UNITS                        FS_MMC_MAXUNIT
#endif
#ifndef     FS_MMC_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_MMC_NUM_UNITS                      1       // Maximum number of driver instances.
  #else
    #define FS_MMC_NUM_UNITS                      2
  #endif
#endif

#ifndef   FS_MMC_SUPPORT_POWER_SAVE
  #define FS_MMC_SUPPORT_POWER_SAVE               0       // Set to 1 if the MMC device has to be put to sleep between data transfers.
#endif

#ifndef   FS_MMC_ENABLE_STATS
  #define FS_MMC_ENABLE_STATS                     (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL)  // Enables / disables the statistical counters.
#endif

#ifndef   FS_MMC_NUM_RETRIES
  #define FS_MMC_NUM_RETRIES                      5       // Number of times to repeat a read or write operation in case of an error
#endif

#ifndef   FS_MMC_SELECT_CARD_TIMEOUT
  #define FS_MMC_SELECT_CARD_TIMEOUT              512     // Number of times to repeat a card select operation in case of a failure
#endif

#ifndef   FS_MMC_WAIT_READY_TIMEOUT
  #define FS_MMC_WAIT_READY_TIMEOUT               0xFFFFFFUL    // Number of cycles to wait for the card to become ready
#endif

#ifndef   FS_MMC_SUPPORT_MMC
  #define FS_MMC_SUPPORT_MMC                      1       // Enables/disables the support for MMC cards and eMMC devices.
#endif

#ifndef   FS_MMC_SUPPORT_SD
  #define FS_MMC_SUPPORT_SD                       1       // Enables/disables the support for SD cards
#endif

#ifndef   FS_MMC_SUPPORT_TRIM
  #define FS_MMC_SUPPORT_TRIM                     0       // Enables/disables the support for the TRIM operation.
#endif


#ifndef   FS_MMC_READ_SINGLE_LAST_SECTOR
  #define FS_MMC_READ_SINGLE_LAST_SECTOR          0       // When set to 1 the last sector on the storage is always read using a CMD_READ_SINGLE_BLOCK command.
                                                          // This option has to be activated when the SD card is not able to handle correctly CMD_READ_MULTIPLE_BLOCKS
                                                          // commands that read the last sector on the storage device.
#endif

#ifdef    FS_MMC_CM_TEST_BUS_WIDTH                        // Compatibility define. Use FS_MMC_TEST_BUS_WIDTH instead.
  #define FS_MMC_TEST_BUS_WIDTH                   FS_MMC_CM_TEST_BUS_WIDTH
#endif
#ifndef   FS_MMC_TEST_BUS_WIDTH
  #define FS_MMC_TEST_BUS_WIDTH                   0       // Enables/disables the test of the data bus width for MMC cards
#endif

#ifdef    FS_MMC_CM_READ_DATA_TIMEOUT                     // Compatibility define. Use FS_MMC_READ_DATA_TIMEOUT instead.
  #define FS_MMC_READ_DATA_TIMEOUT                FS_MMC_CM_READ_DATA_TIMEOUT
#endif
#ifndef   FS_MMC_READ_DATA_TIMEOUT
  #define FS_MMC_READ_DATA_TIMEOUT                0       // Number of milliseconds to wait for data from card
#endif

#ifndef   FS_MMC_SUPPORT_HIGH_SPEED
  #define FS_MMC_SUPPORT_HIGH_SPEED               0       // Enables or disables the support for high-speed mode for SD cards in SPI mode
#endif

#ifndef   FS_MMC_SUPPORT_LOCKING
  #define FS_MMC_SUPPORT_LOCKING                  0       // Enables/disables the SPI bus locking.
#endif

#ifndef   FS_MMC_REPORT_HW_ERRORS
  #define FS_MMC_REPORT_HW_ERRORS                 0       // Enables/disables return values of the HW layer read and write functions.
#endif

#ifndef   FS_MMC_SUPPORT_UHS
  #define FS_MMC_SUPPORT_UHS                      0       // Enables/disables the support for the ultra-high-speed data transfer modes.
#endif

#ifndef   FS_MMC_DISABLE_DAT3_PULLUP
  #define FS_MMC_DISABLE_DAT3_PULLUP              0       // Disables the internal pull-up connected to the DAT3 line of an SD card.
#endif

#ifndef   FS_MMC_SUPPORT_ERROR_RECOVERY
  #define FS_MMC_SUPPORT_ERROR_RECOVERY           0       // Enables or disables the device reinitialization after a failed read or write operation.
#endif

#ifndef   FS_MMC_NOTIFY_EVENT
  #define FS_MMC_NOTIFY_EVENT                     0       // Enables or disables the support for driver notifications via a registerred callback.
#endif

/*********************************************************************
*
*       CF/IDE driver
*/
#ifdef    FS_IDE_MAXUNIT                                  // Compatibility define. Use FS_IDE_NUM_UNITS instead.
  #define FS_IDE_NUM_UNITS                        FS_IDE_MAXUNIT
#endif
#ifndef     FS_IDE_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_IDE_NUM_UNITS                      1       // Maximum number of driver instances.
  #else
    #define FS_IDE_NUM_UNITS                      4
  #endif
#endif

#ifndef   FS_IDE_DEVICE_BUSY_TIMEOUT
  #define FS_IDE_DEVICE_BUSY_TIMEOUT              8000000 // Maximum time to wait for a device to execute an operation in software loops.
#endif

#ifndef   FS_IDE_DEVICE_SELECT_TIMEOUT
  #define FS_IDE_DEVICE_SELECT_TIMEOUT            200     // Maximum time to wait for a device select operation in software loops.
#endif

/*********************************************************************
*
*       RAM Disk driver
*/
#ifdef    FS_RAMDISK_MAXUNIT                              // Compatibility define. Use FS_RAMDISK_NUM_UNITS instead.
  #define FS_RAMDISK_NUM_UNITS                    FS_RAMDISK_MAXUNIT
#endif
#ifndef     FS_RAMDISK_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_RAMDISK_NUM_UNITS                  1       // Maximum number of driver instances.
  #else
    #define FS_RAMDISK_NUM_UNITS                  4
  #endif
#endif

#ifndef     FS_RAMDISK_VERIFY_WRITE
  #if       (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL)
    #define FS_RAMDISK_VERIFY_WRITE               1       // If set to 1 the driver checks if a write operation was successful
                                                          // by comparing the actual written data with the data to be written.
                                                          // Enabling this feature reduces the write performance.
  #else
    #define FS_RAMDISK_VERIFY_WRITE               0
  #endif
#endif

#ifndef   FS_RAMDISK_DATA_BUFFER_SIZE
  #define FS_RAMDISK_DATA_BUFFER_SIZE             32      // Size of the buffer used for accessing the data of the RAM device
                                                          // for internal operations. This buffer is allocated on the stack.
#endif

#ifndef   FS_RAMDISK_NUM_READ_RETRIES
  #define FS_RAMDISK_NUM_READ_RETRIES             10      // Number of retries in case of a read error.
#endif

#ifndef   FS_RAMDISK_DEVICE_OPERATION_TIMEOUT
  #define FS_RAMDISK_DEVICE_OPERATION_TIMEOUT     500     // Timeout for any device operation in milliseconds
#endif

#ifndef   FS_RAMDISK_DEVICE_LIST_DEFAULT
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_RAMDISK_DEVICE_LIST_DEFAULT        NULL    // Default list of recognized serial RAM devices.
  #else
    #define FS_RAMDISK_DEVICE_LIST_DEFAULT        &FS_RAMDISK_SPI_DeviceListDefault
  #endif
#endif

/*********************************************************************
*
*       Windows Drive driver
*/
#ifdef    FS_WD_MAXUNITS                                  // Compatibility define. Use FS_WINDRIVE_NUM_UNITS instead.
  #define FS_WINDRIVE_NUM_UNITS                   FS_WD_MAXUNITS
#endif

#ifndef     FS_WINDRIVE_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_WINDRIVE_NUM_UNITS                 1       // Maximum number of driver instances.
  #else
    #define FS_WINDRIVE_NUM_UNITS                 4
  #endif
#endif

#ifdef    WD_SECTOR_SIZE                                  // Compatibility define. Use FS_WINDRIVE_SECTOR_SIZE instead.
  #define FS_WINDRIVE_SECTOR_SIZE                 WD_SECTOR_SIZE
#endif
#ifndef   FS_WINDRIVE_SECTOR_SIZE
  #define FS_WINDRIVE_SECTOR_SIZE                 512     // Number of bytes in a logical sector.
#endif

/*********************************************************************
*
*       Encryption
*/
#ifdef    FS_CRYPT_MAXUNIT                                // This configuration define is deprecated. Use FS_CRYPT_NUM_UNITS instead.
  #define FS_CRYPT_NUM_UNITS                      FS_CRYPT_MAXUNIT
#endif
#ifndef     FS_CRYPT_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_CRYPT_NUM_UNITS                    1       // Maximum number of driver instances.
  #else
    #define FS_CRYPT_NUM_UNITS                    4
  #endif
#endif

#ifndef     FS_CRYPT_AES_OPTIMIZE_MIX_SUBST
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_CRYPT_AES_OPTIMIZE_MIX_SUBST       0       // 0: No opt, 1: Use a 32-bit table to perform "MixColumns" and "SubBytes" at the same time.
  #else
    #define FS_CRYPT_AES_OPTIMIZE_MIX_SUBST       1
  #endif
#endif

#ifndef     FS_CRYPT_AES_OPTIMIZE_MIX_COLUMNS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MAX_SPEED)
    #define FS_CRYPT_AES_OPTIMIZE_MIX_COLUMNS     2       // 0: No opt, 2: highest.
  #else
    #define FS_CRYPT_AES_OPTIMIZE_MIX_COLUMNS     0
  #endif
#endif

#ifndef   FS_CRYPT_MAX_AUTH_TAG_SIZE
  #define FS_CRYPT_MAX_AUTH_TAG_SIZE              16      // Maximum size of an authentication tag supported by all CRYPT logical driver instances. Has to be a multiple of 4 bytes.
#endif

#ifndef   FS_CRYPT_READ_BUFFER_FILL_PATTERN
  #define FS_CRYPT_READ_BUFFER_FILL_PATTERN       0xFF    // The read sector buffer is filled with this value if the sector data is not valid.
#endif

#ifndef     FS_CRYPT_ENABLE_STATS                         // Enables/disables the statistical counters of the encryption drivers.
  #if (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL)
    #define FS_CRYPT_ENABLE_STATS                 1
  #else
    #define FS_CRYPT_ENABLE_STATS                 0
  #endif
#endif

#ifndef   FS_CRYPT_SUPPORT_TRIM
  #define FS_CRYPT_SUPPORT_TRIM                   FS_SUPPORT_FREE_SECTOR      // Enables/disables the TRIM functionality of the encryption driver.
#endif

/*********************************************************************
*
*       Compression
*/
#ifndef   FS_SUPPORT_COMPRESSION
  #define FS_SUPPORT_COMPRESSION                  0     // Enables/disables the support for compressed file contents.
#endif

#ifndef     FS_COMPRESS_BUFFER_SIZE
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_COMPRESS_BUFFER_SIZE               32    // Size of the work buffer used for file system operations (in bytes).
  #else
    #define FS_COMPRESS_BUFFER_SIZE               128
  #endif
#endif

/*********************************************************************
*
*       LOGVOL driver
*/
#ifdef    FS_LOGVOL_MAXUNIT                               // This configuration define is deprecated. Use FS_LOGVOL_NUM_UNITS instead.
  #define FS_LOGVOL_NUM_UNITS                     FS_LOGVOL_MAXUNIT
#endif
#ifndef     FS_LOGVOL_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_LOGVOL_NUM_UNITS                   1       // Maximum number of driver instances.
  #else
    #define FS_LOGVOL_NUM_UNITS                   4
  #endif
#endif

#ifndef   FS_LOGVOL_SUPPORT_DRIVER_MODE
  #define FS_LOGVOL_SUPPORT_DRIVER_MODE           0       // If set to 1 LOGVOL behaves like a normal logical driver that is instantiated via FS_AddDevice().
                                                          // FS_LOGVOL_Create() is not available in this mode and it returns an error when called.
#endif

#ifndef   FS_LOGVOL_SUPPORT_TRIM
  #define FS_LOGVOL_SUPPORT_TRIM                  FS_SUPPORT_FREE_SECTOR      // Enables/disables the TRIM functionality of the LOGVOL driver.
#endif

#ifndef     FS_LOGVOL_SUPPORT_CLEAN
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_LOGVOL_SUPPORT_CLEAN               0       // Enables/disables the CLEAN functionality of the LOGVOL driver.
  #else
    #define FS_LOGVOL_SUPPORT_CLEAN               1
  #endif
#endif

/*********************************************************************
*
*       READAHEAD driver
*/
#ifdef    FS_READAHEAD_MAXUNIT                            // This configuration define is deprecated. Use FS_READAHEAD_NUM_UNITS instead.
  #define FS_READAHEAD_NUM_UNITS                  FS_READAHEAD_MAXUNIT
#endif

#ifndef     FS_READAHEAD_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_READAHEAD_NUM_UNITS                1       // Maximum number of driver instances.
  #else
    #define FS_READAHEAD_NUM_UNITS                4
  #endif
#endif

#ifndef     FS_READAHEAD_ENABLE_STATS                    // Enables/disables the statistical counters.
  #if (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL)
    #define FS_READAHEAD_ENABLE_STATS             1
  #else
    #define FS_READAHEAD_ENABLE_STATS             0
  #endif
#endif

/*********************************************************************
*
*       Disk partition driver
*/
#ifdef    FS_DISKPART_MAXUNIT                             // This configuration define is deprecated. Use FS_DISKPART_NUM_UNITS instead.
  #define FS_DISKPART_NUM_UNITS                   FS_DISKPART_MAXUNIT
#endif
#ifndef     FS_DISKPART_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_DISKPART_NUM_UNITS                 1       // Maximum number of driver instances.
  #else
    #define FS_DISKPART_NUM_UNITS                 4
  #endif
#endif

#ifndef   FS_DISKPART_SUPPORT_ERROR_RECOVERY
  #define FS_DISKPART_SUPPORT_ERROR_RECOVERY      0       // Enables/disables the support for error recovery via RAID.
#endif

/*********************************************************************
*
*       RAID driver
*/
#ifdef    FS_RAID1_MAXUNIT                                // This configuration define is deprecated. Use FS_RAID_NUM_UNITS instead.
  #define FS_RAID_NUM_UNITS                       FS_RAID1_MAXUNIT
#endif
#ifndef     FS_RAID_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_RAID_NUM_UNITS                     1       // Maximum number of driver instances.
  #else
    #define FS_RAID_NUM_UNITS                     4
  #endif
#endif

#ifndef   FS_RAID_NUM_READ_RETRIES
  #define FS_RAID_NUM_READ_RETRIES                10      // Number of times to repeat a sector read operation in case of an error.
#endif

#ifndef   FS_RAID_NUM_WRITE_RETRIES
  #define FS_RAID_NUM_WRITE_RETRIES               10      // Number of times to repeat a sector write operation in case of an error.
#endif

/*********************************************************************
*
*       SECSIZE driver
*/
#ifdef    FS_SECSIZE_MAXUNIT                              // This configuration define is deprecated. Use FS_SECSIZE_NUM_UNITS instead.
  #define FS_SECSIZE_NUM_UNITS                    FS_SECSIZE_MAXUNIT
#endif
#ifndef     FS_SECSIZE_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_SECSIZE_NUM_UNITS                  1       // Maximum number of instances that can be handled.
  #else
    #define FS_SECSIZE_NUM_UNITS                  4
  #endif
#endif

#ifndef   FS_SECSIZE_ENABLE_ERROR_RECOVERY
  #define FS_SECSIZE_ENABLE_ERROR_RECOVERY        0       // Enables/disables the read error recovery via RAID driver.
#endif

/*********************************************************************
*
*       WRBUF driver
*/
#ifdef    FS_WRBUF_MAXUNIT                                // This configuration define is deprecated. Use FS_WRBUF_NUM_UNITS instead.
  #define FS_WRBUF_NUM_UNITS                      FS_WRBUF_MAXUNIT
#endif
#ifndef     FS_WRBUF_NUM_UNITS
  #if (FS_OPTIMIZATION_TYPE == FS_OPTIMIZATION_TYPE_MIN_SIZE)
    #define FS_WRBUF_NUM_UNITS                    1       // Maximum number of instances that can be handled.
  #else
    #define FS_WRBUF_NUM_UNITS                    4
  #endif
#endif

#ifndef   FS_WRBUF_ENABLE_STATS
  #define FS_WRBUF_ENABLE_STATS                   (FS_DEBUG_LEVEL >= FS_DEBUG_LEVEL_CHECK_ALL)   // Statistics only in debug builds
#endif

/*********************************************************************
*
*       FILEDISK driver
*/
#ifndef   FS_FILEDISK_NUM_UNITS
  #define FS_FILEDISK_NUM_UNITS                   1       // Maximum number of driver instances.
#endif

#ifndef   FS_FILEDISK_SUPPORT_TRIM
  #define FS_FILEDISK_SUPPORT_TRIM                0       // Enables/disables the TRIM functionality of the FILEDISK driver.
#endif

#ifndef   FS_FILEDISK_WORK_BUFFER_SIZE
  #define FS_FILEDISK_WORK_BUFFER_SIZE            512     // Size of the buffer used by internal operations for accessing the storage
                                                          // This buffer is allocated on the stack.
#endif

/*********************************************************************
*
*       Testing (for internal use only)
*/
#ifndef   FS_SUPPORT_TEST
  #define FS_SUPPORT_TEST                         0     // Testing support is disabled by default.
#endif

#ifndef   FS_SUPPORT_SIMULATION
  #define FS_SUPPORT_SIMULATION                   0     // Set to 1 if the file system runs in the PC simulation. By default the file system is tested on the HW.
#endif

#endif      // FS_CONFDEFAULTS_H

/*************************** End of file ****************************/
