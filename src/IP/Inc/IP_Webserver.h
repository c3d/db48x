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
File        : IP_WebServer.h
Purpose     : Publics for the WebServer
---------------------------END-OF-HEADER------------------------------

Attention : Do not modify this file !

Note:
  TimeDate is a 32 bit variable in the following format:
    Bit 0-4:   2-second count (0-29)
    Bit 5-10:  Minutes (0-59)
    Bit 11-15: Hours (0-23)
    Bit 16-20: Day of month (1-31)
    Bit 21-24: Month of year (1-12)
    Bit 25-31: Count of years from 1980 (0-127)
*/

#ifndef  IP_WEBS_H
#define  IP_WEBS_H

#ifdef __ICCARM__  // IAR
  #pragma diag_suppress=Pa029  // No warning for unknown pragmas in earlier verions of EWARM
  #pragma diag_suppress=Pa137  // No warning for C-Style-Casts with C++
#endif

#include "IP_FS.h"
#include "WEBS_Conf.h"

/*********************************************************************
*
*       Defaults for config values
*
**********************************************************************
*/

#ifndef   WEBS_IN_BUFFER_SIZE
  #define WEBS_IN_BUFFER_SIZE               256
#endif

#ifndef   WEBS_OUT_BUFFER_SIZE
  #define WEBS_OUT_BUFFER_SIZE              512
#endif

#ifndef   WEBS_TEMP_BUFFER_SIZE
  #define WEBS_TEMP_BUFFER_SIZE             512         // Used as file input buffer and for form parameters
#endif

#ifndef   WEBS_PARA_BUFFER_SIZE
  #define WEBS_PARA_BUFFER_SIZE             0           // Required for dynamic content parameter handling
#endif

#ifndef   WEBS_ERR_BUFFER_SIZE
  #define WEBS_ERR_BUFFER_SIZE              128         // Used in case of connection limit only
#endif

#ifndef   WEBS_AUTH_BUFFER_SIZE
  #define WEBS_AUTH_BUFFER_SIZE             32
#endif

#ifndef   WEBS_FILENAME_BUFFER_SIZE
  #define WEBS_FILENAME_BUFFER_SIZE         32
#endif

#ifndef   WEBS_UPLOAD_FILENAME_BUFFER_SIZE
  #define WEBS_UPLOAD_FILENAME_BUFFER_SIZE  64
#endif

#ifndef   WEBS_SUPPORT_UPLOAD
  #define WEBS_SUPPORT_UPLOAD               0
#endif

#ifndef   WEBS_URI_BUFFER_SIZE
  #define WEBS_URI_BUFFER_SIZE              0
#endif

#ifndef   WEBS_MAX_ROOT_PATH_LEN
  #define WEBS_MAX_ROOT_PATH_LEN            0
#endif

#ifndef   WEBS_STACK_SIZE_CHILD
  #define WEBS_STACK_SIZE_CHILD             (1536 + WEBS_IN_BUFFER_SIZE + WEBS_OUT_BUFFER_SIZE + WEBS_TEMP_BUFFER_SIZE + WEBS_PARA_BUFFER_SIZE + WEBS_AUTH_BUFFER_SIZE + WEBS_FILENAME_BUFFER_SIZE + WEBS_UPLOAD_FILENAME_BUFFER_SIZE)  // This size can not be guaranteed on all systems. Actual size depends on CPU & compiler
#endif

#ifndef   WEBS_USE_PARA                                 // Some compiler complain about unused parameters.
  #define WEBS_USE_PARA(Para)               (void)Para  // This works for most compilers.
#endif

#ifndef   WEBS_USE_AUTH_DIGEST
  #define WEBS_USE_AUTH_DIGEST              0
#endif

//
// This is a work in progress configuration that is disabled by default.
// Changes provided by this switch are planned to become defaults in
// an official release at some time in the future (will then be announced).
// Enabling enhanced form handling currently provides the following changes:
//   1) By default VFiles get their parameters via sParameters in a callback
//      instead of executing a CGI callback for each parameter/value pair.
//      In addition to sParameters the CGI callbacks will be executed as well
//      as they would for non-VFiles to use the same concept. The old concept
//      however can still be used as sParameters is still passed to VFile handlers.
//      This is not enabled by default as it might execute CGI callbacks that
//      were previously not executed automatically and might have been implemented
//      to be called from a VFile handler the user has implemented and would
//      therefore be executed twice.
//   2) Upload currently does not provide all form parameters and omits the
//      field name. Instead it adds "file=<UploadedFilename>&state=<UploadResult>"
//      to the end of the parameters in para buffer that is only handled
//      by a VFile. It is not possible to use uploading with a non-VFile.
//      This is changed by adding a callback that can be set that is then
//      executed for each uploaded file, letting the application know the
//      details about the upload like field name, status and size. The callback
//      return value decides if the uploaded file will be kept (== 0) or if the
//      Web server will delete it (!= 0).
//      The upload directory should be considered as a temporary folder
//      that the Web server will try to keep clean of unused files.
//      Therefore files uploaded should be taken care of by the upload
//      callback by moving them out of the "temporary" upload folder and
//      telling the Web server that the upload has been handled (by returning 0).
//
#ifndef   WEBS_USE_ENHANCED_FORM_HANDLING
  #define WEBS_USE_ENHANCED_FORM_HANDLING  0
#endif

//
// Use the PARA buffer for GET parameter parsing if HTTP authorization is used.
// Default is to evaluate GET parameters before authorization (if it is no VFile).
// This however is not secure if GET parameters are used as their calllbacks get
// executed before the authentication check.
//
#ifndef   WEBS_AUTH_USE_GET_PARA_BUFFER
  #if WEBS_USE_ENHANCED_FORM_HANDLING
    #define WEBS_AUTH_USE_GET_PARA_BUFFER  1  // Use the PARA buffer for GET parameter parsing if HTTP authorization is used. Default for "new" API is to always parse GET parameters into PARA buffer and evaluate them later (after authentication check).
  #else
    #define WEBS_AUTH_USE_GET_PARA_BUFFER  0  // Use the PARA buffer for GET parameter parsing if HTTP authorization is used. Default is to evaluate GET parameters before authorization (if it is no VFile).
  #endif
#endif

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define OUT_BUFFER_PRE_SPACE  8    // Number of bytes before the actual out buffer. This allows us to prepend data with chunk info.

//
// Return values, error codes are in general negative
// and might not be listed here.
//
#define WEBS_OK                      0
#define WEBS_CONNECTION_DETACHED     1  // Returned by IP_WEBS_Process*() in case a WebSocket connection has been detached from the webserver context.
#define WEBS_HEADER_FIELD_UNTOUCHED  2  // Returned by WEBS_HEADER_FIELD_HOOK callbacks in case the callback only registered the call but did not call
                                        // further API that modified the line. The line will then be regularly processed by the webserver.
#define WEBS_NO_ERROR_PAGE_SENT      3  // The callback set to handle sending custom error pages has NOT sent out an error page, send the default content.

/*********************************************************************
*
*       Enums
*
**********************************************************************
*/

enum {
  METHOD_NONE,
  METHOD_GET,
  METHOD_HEAD,
  METHOD_POST
};

enum {
  HTTP_ENCODING_RAW,
  HTTP_ENCODING_CHUNKED,
  HTTP_ENCODING_FROM_CONTEXT
};

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef struct {
  U32 NumBytesInBuf;
  U32 NumBytesOutBuf;
  U32 NumBytesFilenameBuf;
  U32 MaxRootPathLen;
  U32 NumBytesParaBuf;
  U32 NumBytesFullUriBuf;
} WEBS_BUFFER_SIZES;

typedef int   (*IP_WEBS_tSend)   (const unsigned char *pData, int len, void *pConnectInfo);
typedef int   (*IP_WEBS_tReceive)(      unsigned char *pData, int len, void *pConnectInfo);
typedef void *(*IP_WEBS_tAlloc)  (U32 NumBytesReq);
typedef void  (*IP_WEBS_tFree)   (void *p);

typedef void *WEBS_OUTPUT;

typedef struct {
  const char *sName;  // e.g. "Counter"
  void  (*pf)(WEBS_OUTPUT *pOutput, const char *sParameters, const char *sValue);
} WEBS_CGI;

typedef struct {
  const char *sName;  // e.g. "Counter.cgi"
  void  (*pf)(WEBS_OUTPUT *pOutput, const char *sParameters);
} WEBS_VFILES;

typedef struct {
  const char *sPath;
  const char *sRealm;
  const char *sUserPass;         // Basic auth: "User:Pass"; Digest auth: "User".
  const char *sCredentialsHash;  // Basic auth: Not used; Digest auth: MD5("User:Realm:Pass").
} WEBS_ACCESS_CONTROL;

typedef struct {
  const WEBS_CGI      *paCGI;
  WEBS_ACCESS_CONTROL *paAccess;
  void               (*pfHandleParameter)(WEBS_OUTPUT * pOutput, const char * sPara, const char * sValue);
  const WEBS_VFILES   *paVFiles;
} WEBS_APPLICATION;

typedef struct {
  const IP_FS_API* pFS_API;          // Allows override of FS API to use for a file.
        U32        DateLastMod;      // Used for "Last modified" header field.
        U32        DateExp;          // Used for "Expires" header field.
        U8         IsVirtual;
        U8         AllowDynContent;  // Guard to avoid unnecessary parsing of static files.
} IP_WEBS_FILE_INFO;

typedef struct {
  const char *sExt;
  const char *sContent;
} WEBS_FILE_TYPE;

typedef struct WEBS_FILE_TYPE_HOOK {
  struct WEBS_FILE_TYPE_HOOK *pNext;
         WEBS_FILE_TYPE       FileType;
} WEBS_FILE_TYPE_HOOK;

typedef struct {
  IP_WEBS_tSend    pfSend;
  IP_WEBS_tReceive pfReceive;
} WEBS_IP_API;

typedef struct {
  IP_WEBS_tAlloc   pfAlloc;
  IP_WEBS_tFree    pfFree;
} WEBS_SYS_API;

typedef struct {
  const WEBS_IP_API      *pIP_API;
  const WEBS_SYS_API     *pSYS_API;
  const IP_FS_API        *pFS_API;
  const WEBS_APPLICATION *pApplication;
        void             *pWebsPara;
        void             *pUpload;
} WEBS_CONTEXT;

typedef void (*IP_WEBS_pfGetFileInfo)(const char *sFilename, IP_WEBS_FILE_INFO *pFileInfo);

//
// VFile extension
//
typedef struct {
  int  (*pfCheckVFile)(const char *sFileName, unsigned *pIndex);
  void (*pfSendVFile) (void *pContextIn, unsigned Index, const char *sFileName, void (*pf)(void *pContextOut, const char *pData, unsigned NumBytes));
} WEBS_VFILE_APPLICATION;

typedef struct WEBS_VFILE_HOOK {
  struct WEBS_VFILE_HOOK        *pNext;
         WEBS_VFILE_APPLICATION *pVFileApp;
         U8                      ForceEncoding;
} WEBS_VFILE_HOOK;

//
// URI extension.
//
// There are multiple use cases for URI extension that might be used
// for a more specific use case like a METHOD extension for REST
// support or an implementation for WebSockets. What all these
// implementations share is that they need a hook including a
// next pointer and the URI to search for.
// For an optimized code all specific use cases can be treated as
// WEBS_URI_HOOK internally. For this to be able the first two
// elements of all structures need to be the same and in same order.
//
typedef struct WEBS_URI_HOOK_STRUCT WEBS_URI_HOOK;
struct WEBS_URI_HOOK_STRUCT {
        WEBS_URI_HOOK* pNext;
  const char*          sURI;
};

enum {
  WEBS_URI_HOOK_TYPE_NONE = 0,
  WEBS_URI_HOOK_TYPE_METHOD,
  WEBS_URI_HOOK_TYPE_WEBSOCKET
};

//
// METHOD extension (variation of WEBS_URI_HOOK).
//
typedef int (*IP_WEBS_pfMethod)(void *pContext, WEBS_OUTPUT *pOutput, const char *sMethod, const char *sAccept, const char *sContentType, const char *sResource, U32 ContentLen);

typedef struct WEBS_METHOD_HOOK_STRUCT WEBS_METHOD_HOOK;
struct WEBS_METHOD_HOOK_STRUCT {
        WEBS_METHOD_HOOK* pNext;
  const char*             sURI;
  const char*             sMethod;
        IP_WEBS_pfMethod  pf;
};

//
// WebSocket extension (variation of WEBS_URI_HOOK).
//
typedef struct {
  int  (*pfGenerateAcceptKey) (WEBS_OUTPUT* pOutput, void* pSecWebSocketKey, int SecWebSocketKeyLen, void* pBuffer, int BufferSize);  // Input/output buffer is the same. If this is not supported by your calculation callback the input buffer needs to be saved by your callback.
  void (*pfDispatchConnection)(WEBS_OUTPUT* pOutput, void* pConnection);                                                              // Dispatch the connection (/socket) handle from the web server to a WebSocket handler. From now on the new handler is responsible for it.
} IP_WEBS_WEBSOCKET_API;

typedef struct WEBS_WEBSOCKET_HOOK_STRUCT WEBS_WEBSOCKET_HOOK;
struct WEBS_WEBSOCKET_HOOK_STRUCT {
        WEBS_WEBSOCKET_HOOK*   pNext;
  const char*                  sURI;    // Resource location to assign WebSocket functionality to.
  const char*                  sProto;  // Single WebSocket protocol assigned to this hook. For more protocols add a hook with the same sURI and another protocol.
  const IP_WEBS_WEBSOCKET_API* pAPI;    // IP_WEBS_WEBSOCKET_API to use for this hook.
};

//
// HEADER extension
//
typedef struct {
        WEBS_OUTPUT* pOutput;
  const char*        sField;   // The header field that the hook has been registered to.
} WEBS_HEADER_FIELD_INFO;

typedef int (*IP_WEBS_pfHeaderField)(WEBS_HEADER_FIELD_INFO* pInfo);

typedef struct WEBS_HEADER_FIELD_HOOK_STRUCT WEBS_HEADER_FIELD_HOOK;
struct WEBS_HEADER_FIELD_HOOK_STRUCT {
        WEBS_HEADER_FIELD_HOOK* pNext;
        IP_WEBS_pfHeaderField   pf;
  const char*                   sField;
};

//
// Request notifier
//
typedef struct {
  const char*  sUri;
  WEBS_OUTPUT* pOutput;
        U8     Method;
} WEBS_REQUEST_NOTIFY_INFO;

typedef void (*IP_WEBS_pfRequestNotify)(WEBS_REQUEST_NOTIFY_INFO* pInfo);

typedef struct WEBS_REQUEST_NOTIFY_HOOK {
  struct WEBS_REQUEST_NOTIFY_HOOK *pNext;
         IP_WEBS_pfRequestNotify   pf;
} WEBS_REQUEST_NOTIFY_HOOK;

//
// Request progress status identifier.
//
typedef enum {
  WEBS_PROGRESS_STATUS_BEGIN = 0u,
  WEBS_PROGRESS_STATUS_METHOD_URI_VER_PARSED,
  WEBS_PROGRESS_STATUS_HEADER_PARSED,
  WEBS_PROGRESS_STATUS_END
} WEBS_PROGRESS_STATUS;

//
// Main info structure about the current progression of serving the request.
// Further status sensitive details may be available based on the reported status.
//
typedef struct {
  WEBS_OUTPUT* pOutput;
  U8           Status;
} WEBS_PROGRESS_INFO;

typedef void (*IP_WEBS_pfProgress)(WEBS_PROGRESS_INFO* pInfo);

typedef struct WEBS_PROGRESS_HOOK {
  struct WEBS_PROGRESS_HOOK *pNext;
         IP_WEBS_pfProgress  pf;
} WEBS_PROGRESS_HOOK;

//
// Pre-content output hook.
//
typedef int (*IP_WEBS_pfPreContentOutput)(WEBS_OUTPUT* pOutput);  // Return value: 0: O.K., send content as usual; 1: Suppress output of further content.

typedef struct WEBS_PRE_CONTENT_OUTPUT_HOOK {
  struct WEBS_PRE_CONTENT_OUTPUT_HOOK* pNext;
         IP_WEBS_pfPreContentOutput    pf;
} WEBS_PRE_CONTENT_OUTPUT_HOOK;

#define WEBS_PRE_DYNAMIC_CONTENT_OUTPUT  (1uL << 0)

enum {
  WEBS_UPLOAD_OK = 0,             // Upload of file succeeded.
  WEBS_UPLOAD_ERR_MISC,           // Error states that have no own return code.
  WEBS_UPLOAD_ERR_PARSE,          // Parsing error.
  WEBS_UPLOAD_ERR_CREATE,         // Error creating upload file.
  WEBS_UPLOAD_ERR_WRITE,          // Error writing to upload file.
  WEBS_UPLOAD_ERR_MAX_FILE_SIZE   // Maximum file size for upload exceeded.
};

typedef void* WEBS_AUTH_DIGEST_OUTPUT;

typedef struct {
  void (*pfStoreNonce)(WEBS_OUTPUT* pOutput, WEBS_AUTH_DIGEST_OUTPUT* pDigestOutput, void (*pfStore)(WEBS_OUTPUT* pOutput, WEBS_AUTH_DIGEST_OUTPUT* pDigestOutput, const char* pNonce, unsigned NonceLen), int GenerateNew);
  //
  // Place for further extensions like pfStoreCNonce .
  //
} WEBS_AUTH_DIGEST_APP_API;

//
// Main info structure about the reason why an error page
// needs to be sent.
//
typedef struct {
  WEBS_OUTPUT* pOutput;
  const char*  sErrHeader;
  const char*  sErrBody;
  U16          ErrorCode;
} WEBS_SEND_ERROR_PAGE_INFO;

//
// Custom error page callback.
//
typedef int (*IP_WEBS_pfSendErrorPage)(WEBS_SEND_ERROR_PAGE_INFO* pInfo);  // Return value: WEBS_NO_ERROR_PAGE_SENT: let webserver handle the reply; WEBS_OK: Suppress output of further content, page has been sent.

//
// Header config.
// Used by IP_WEBS_SendHeader[Ex]() internally or
// with WEBS_USE_ENHANCED_FORM_HANDLING public.
//
typedef struct {
  const char* sFileName;   // Filename that will be delivered. Extension is used to determine MIME type.
  const char* sMIMEType;   // Override the MIME type based on the filename extension. Can be NULL
  const char* sAddFields;  // String with additional header fields. Can be NULL.
  const char* sStartLine;  // "HTTP/1.1 200 OK" type start line. Can be NULL, in this case the shown start line will be used.
        U8    ReqKeepCon;  // Typically the connection will be closed on custom headers. Try to keep the connection open if possible (not HTTP1.0 and not ProcessLast).
                           //   0: Close connection, after data transmission.
                           //   1: If possible, keep connection open after data transmission.
} WEBS_HEADER_CONFIG;

/*********************************************************************
*
*       WEBS_STATIC_ENCODED_FILETYPES
*
*  Function description
*    Configures a list of filetypes for which static compressed
*    content is delivered if available.
*/
typedef struct {
  const char* sEncoding;        // Encoding as used by the browser in its "Accept-Encoding" field, such as "gzip".
  const char* sExtension;       // Extension including period letter to add to the end of the URI requested.
                                // The resulting URI will be tried to open and sent back gzip compressed.
  U8          ReplaceLastChar;  // Replace the last character of the requested URI with the first character of the extension.
} WEBS_STATIC_ENCODED_FILETYPES;

//
// Enhanced form handling.
//
#if WEBS_USE_ENHANCED_FORM_HANDLING

/*********************************************************************
*
*       WEBS_UPLOAD_INFO
*
*  Function description
*    Provides information about afile that has just been uploaded.
*/
typedef struct WEBS_UPLOAD_INFO {
  const char* sFieldName;     // The HTTP form field name of the <input> tag.
  const char* sTempFilename;  // The temporary upload filename. This is either the original filename as submitted or
                              // the modified name from IP_WEBS_pfModifyTempFilename .
  const char* sFilename;      // The originally submitted upload filename if not comletely replaced using IP_WEBS_pfModifyTempFilename .
                              // If completely replaced, this is the same as sTempFilename .
  const char* sPath;          // Complete path to the uploaded file.
        U32   Size;           // Size of the uploaded file in bytes.
        U8    Status;         // Status of the upload:
                              //   * Upload succeeded: WEBS_UPLOAD_OK
                              //   * Error           : WEBS_UPLOAD_ERR_*
} WEBS_UPLOAD_INFO;

/*********************************************************************
*
*       IP_WEBS_pfOnUpload()
*
*  Function description
*    Callback executed for each uploaded file.
*
*  Parameters
*    pOutput: Connection context.
*    pInfo  : Pointer to a WEBS_UPLOAD_INFO structure with further
*             information about the uploaded file.
*
*  Return value
*    == 0: Keep the uploaded file.
*    != 0: Delete the uploaded file.
*/
typedef int (*IP_WEBS_pfOnUpload)(WEBS_OUTPUT* pOutput, WEBS_UPLOAD_INFO* pInfo);

/*********************************************************************
*
*       IP_WEBS_pfModifyTempFilename()
*
*  Function description
*    Optional callback executed before each uploaded file that allows
*    modifying the upload filename.
*
*  Parameters
*    pOutput   : Connection context.
*    p         : Pointer to after the upload root path, e.g. "/upload/"
*                (on the '\0').
*    BufferSize: Characters available to be used to set an upload  filename.
*
*  Return value
*    Number of bytes used for the filename including '\0' if used to
*    set an absolute filename instead of prefixing the filename.
*
*  Additional information
*    The original filename can be fetched using IP_WEBS_GetUploadFilename() .
*    Please make sure that characters added plus the original filename
*    do not exceed the available BufferSize .
*
*    Two modifications can be applied to the original filename:
*      1) Storing non-terminated characters into the buffer to prefix the
*         original filename. Example:
*           - "/upload/" is used as upload root path and is stored in the path buffer.
*           - The callback stores the characters "Temp_" without '\0' into the buffer.
*           - The original filename will be added to the buffer resulting in a path
*             like "/upload/Temp_Filename.txt" .
*         The characters added should be random to generate a unique
*         temp filename per Web server child thread. A simple implementation
*         can use the address of a variable that is on the current task stack
*         and prefix it using the callback.
*      2) Storing a terminated string into the buffer to completely change the
*         temporary filename. Example:
*           - "/upload/" is used as upload root path and is stored in the path buffer.
*           - The callback stores the characters "Temp.bin" including '\0' into the buffer.
*           - The temporary path now results in "/upload/Temp.bin" .
*/
typedef int (*IP_WEBS_pfModifyTempFilename)(WEBS_OUTPUT* pOutput, char* p, U32 BufferSize);

typedef struct WEBS_UPLOAD_API {
  IP_WEBS_pfOnUpload           pfOnUpload;
  IP_WEBS_pfModifyTempFilename pfModifyTempFilename;  // Can be NULL.
} WEBS_UPLOAD_API;


#endif

/*********************************************************************
*
*       Extern variables and helper functions for samples.
*
**********************************************************************
*/

extern const WEBS_APPLICATION       WebsSample_Application;
extern const WEBS_VFILE_APPLICATION WebsSample_UPnP_VFileAPI;

void WebsSample_SendPageFooter(WEBS_OUTPUT* pOutput);
void WebsSample_SendPageHeader(WEBS_OUTPUT* pOutput, const char* sName);

//
// IP_WEBS_X_SampleConfig() is a helper function used in web server
// samples. It is used as central point to configure sample
// functionality like adding a REST resource or switching between
// Basic and Digest authentication.
//
void IP_WEBS_X_SampleConfig(void);

/*********************************************************************
*
*       Functions
*
**********************************************************************
*/

extern const IP_FS_API IP_FS_ReadOnly;

//
// General Web Server API functions
//
      void   IP_WEBS_Init                        (WEBS_CONTEXT *pContext, const WEBS_IP_API *pIP_API, const WEBS_SYS_API *pSYS_API, const IP_FS_API *pFS_API, const WEBS_APPLICATION *pApplication);
      int    IP_WEBS_AddUpload                   (void);
      int    IP_WEBS_ProcessEx                   (WEBS_CONTEXT *pContext, void *pConnectInfo, const char *sRootPath);
      int    IP_WEBS_ProcessLastEx               (WEBS_CONTEXT *pContext, void *pConnectInfo, const char *sRootPath);
      void   IP_WEBS_ConfigBufSizes              (WEBS_BUFFER_SIZES *pBufferSizes);
      U32    IP_WEBS_CountRequiredMem            (WEBS_CONTEXT* pContext);
      int    IP_WEBS_Flush                       (WEBS_OUTPUT *pOutput);

      int    IP_WEBS_Process                     (IP_WEBS_tSend pfSend, IP_WEBS_tReceive pfReceive, void *pConnectInfo, const IP_FS_API *pFS_API, const WEBS_APPLICATION *pApplication);
      int    IP_WEBS_ProcessLast                 (IP_WEBS_tSend pfSend, IP_WEBS_tReceive pfReceive, void *pConnectInfo, const IP_FS_API *pFS_API, const WEBS_APPLICATION *pApplication);
      void   IP_WEBS_OnConnectionLimit           (IP_WEBS_tSend pfSend, IP_WEBS_tReceive pfReceive, void *pConnectInfo);

      void * IP_WEBS_GetConnectInfo              (WEBS_OUTPUT *pOutput);
const char * IP_WEBS_GetProtectedPath            (WEBS_OUTPUT* pOutput);
const char * IP_WEBS_GetURI                      (WEBS_OUTPUT *pOutput, char GetFullURI);
      int    IP_WEBS_Redirect                    (WEBS_OUTPUT *pOutput, const char *sFileName, const char *sMimeType);
      void   IP_WEBS_Reset                       (void);
      void * IP_WEBS_RetrieveUserContext         (WEBS_OUTPUT *pOutput);
      int    IP_WEBS_Send204NoContent            (WEBS_OUTPUT* pOutput);
#if (WEBS_USE_ENHANCED_FORM_HANDLING == 0)
      int    IP_WEBS_SendHeader                  (WEBS_OUTPUT *pOutput, const char *sFileName, const char *sMimeType);
      int    IP_WEBS_SendHeaderEx                (WEBS_OUTPUT *pOutput, const char *sFileName, const char *sMIMEType, U8 ReqKeepCon);
#endif
      int    IP_WEBS_SendMem                     (WEBS_OUTPUT *pOutput, const char *s, unsigned NumBytes);
      int    IP_WEBS_SendFormattedString         (WEBS_OUTPUT *pOutput, const char *sFormat, ...);
      int    IP_WEBS_SendString                  (WEBS_OUTPUT *pOutput, const char *s);
      int    IP_WEBS_SendStringEnc               (WEBS_OUTPUT *pOutput, const char *s);
      int    IP_WEBS_SendUnsigned                (WEBS_OUTPUT *pOutput, unsigned v, unsigned Base, int NumDigits);
      void   IP_WEBS_StoreUserContext            (WEBS_OUTPUT *pOutput, void *pContext);
      void   IP_WEBS_UseRawEncoding              (WEBS_OUTPUT* pOutput);
      void   IP_WEBS_MarkSendingCustomHeader     (WEBS_OUTPUT* pOutput, void* p);

      void   IP_WEBS_ConfigFindGZipFiles         (const char* sExtension, int ReplaceLastChar);
      int    IP_WEBS_ConfigRootPath              (const char* sRootPath);
      void   IP_WEBS_ConfigStaticEncodedFiletypes(const WEBS_STATIC_ENCODED_FILETYPES* paList);
      int    IP_WEBS_ConfigUploadRootPath        (const char* sUploadRootPath);
      void   IP_WEBS_ConfigSendVFileHeader       (U8 OnOff);
      void   IP_WEBS_ConfigSendVFileHookHeader   (U8 OnOff);
      int    IP_WEBS_GetDecodedStrLen            (const char *sBuffer, int Len);
      int    IP_WEBS_GetNumParas                 (const char *sParameters);
      int    IP_WEBS_GetParaValue                (const char *sBuffer, int ParaNum,       char   *sPara, int   ParaLen,       char   *sValue, int   ValueLen);
      int    IP_WEBS_GetParaValuePtr             (const char *sBuffer, int ParaNum, const char **ppPara, int *pParaLen, const char **ppValue, int *pValueLen);
      void   IP_WEBS_DecodeAndCopyStr            (char *pDest, int DestLen, const char *pSrc, int SrcLen);
      int    IP_WEBS_DecodeString                (const char *s);
      void   IP_WEBS_SendLocationHeader          (WEBS_OUTPUT* pOutput, const char* sURI, const char* sCodeDesc);
      void   IP_WEBS_SetErrorPageCallback        (IP_WEBS_pfSendErrorPage pf, void* pConfig);
      void   IP_WEBS_SetFileInfoCallback         (IP_WEBS_pfGetFileInfo pf);
      void   IP_WEBS_SetHeaderCacheControl       (const char* sCacheControl);
      void   IP_WEBS_SetUploadFileSystemAPI      (const IP_FS_API* pFS_API);
      void   IP_WEBS_SetUploadMaxFileSize        (U32 NumBytes);
      char   IP_WEBS_CompareFilenameExt          (const char *sFilename, const char *sExt);

      void   IP_WEBS_AddFileTypeHook             (WEBS_FILE_TYPE_HOOK *pHook, const char *sExt, const char *sContent);
      void   IP_WEBS_AddProgressHook             (WEBS_PROGRESS_HOOK* pHook, IP_WEBS_pfProgress pf);
      void   IP_WEBS_AddRequestNotifyHook        (WEBS_REQUEST_NOTIFY_HOOK* pHook, IP_WEBS_pfRequestNotify pf);
      void   IP_WEBS_AddPreContentOutputHook     (WEBS_PRE_CONTENT_OUTPUT_HOOK* pHook, IP_WEBS_pfPreContentOutput pf, U32 Mask);

//
// Web Server VFile extension API functions
//
      void   IP_WEBS_AddVFileHook                (WEBS_VFILE_HOOK *pHook, WEBS_VFILE_APPLICATION *pVFileApp, U8 ForceEncoding);

//
// Web Server METHOD extension API functions
//
      void   IP_WEBS_METHOD_AddHook              (WEBS_METHOD_HOOK *pHook, IP_WEBS_pfMethod pf, const char *sURI);
      void   IP_WEBS_METHOD_AddHook_SingleMethod (WEBS_METHOD_HOOK *pHook, IP_WEBS_pfMethod pf, const char *sURI, const char *sMethod);
      int    IP_WEBS_METHOD_CopyData             (void *pContext, void *pBuffer, unsigned BufferSize);

//
// Web Server AUTH DIGEST API functions
//
      void   IP_WEBS_UseAuthDigest               (const WEBS_AUTH_DIGEST_APP_API* pAPI);
      void   IP_WEBS_AUTH_DIGEST_CalcHA1         (const char* pInput, unsigned InputLen, char* pBuffer, unsigned BufferSize);
      void   IP_WEBS_AUTH_DIGEST_GetURI          (WEBS_OUTPUT* pOutput, WEBS_AUTH_DIGEST_OUTPUT* pDigestOutput, char* pBuffer, unsigned* pNumBytes);

//
// Web Server WebSocket API functions
//
      void   IP_WEBS_WEBSOCKET_AddHook           (WEBS_WEBSOCKET_HOOK* pHook, const IP_WEBS_WEBSOCKET_API* pAPI, const char* sURI, const char* sProto);

//
// Web Server HEADER extension API functions
//
      void   IP_WEBS_HEADER_AddFieldHook         (WEBS_OUTPUT* pOutput, WEBS_HEADER_FIELD_HOOK* pHook, IP_WEBS_pfHeaderField pf, const char* sField);
      int    IP_WEBS_HEADER_CopyData             (WEBS_OUTPUT* pOutput, void* pBuffer, unsigned BufferSize, unsigned* pNumBytesLeft);
      int    IP_WEBS_HEADER_GetFindToken         (WEBS_OUTPUT* pOutput, const char* sToken, int TokenLen, char* pBuffer, int BufferSize);
      void   IP_WEBS_HEADER_SetCustomFields      (WEBS_OUTPUT* pOutput, const char* sAddFields);

//
// Enhanced form handling API functions.
//
#if (WEBS_USE_ENHANCED_FORM_HANDLING != 0)
      void     IP_WEBS_SetUploadAPI              (const WEBS_UPLOAD_API* pAPI);
      unsigned IP_WEBS_GetUploadFilename         (WEBS_OUTPUT* pOutput, char* pBuffer, U32 BufferSize);
      void     IP_WEBS_ChangeUploadMaxFileSize   (WEBS_OUTPUT* pOutput, U32 MaxFileSize);
      int      IP_WEBS_SendHeader                (WEBS_OUTPUT* pOutput, const WEBS_HEADER_CONFIG* pConfig);
#endif


#if defined(__cplusplus)
  }
#endif


#endif   /* Avoid multiple inclusion */

/*************************** End of file ****************************/
