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
-------------------------- END-OF-HEADER -----------------------------

File    : IP_SMTPC.h
Purpose : Header file for the SMTP client add-on.
*/

#ifndef IP_SMTPC_H            // Avoid multiple inclusion.
#define IP_SMTPC_H

#ifdef __ICCARM__  // IAR
  #pragma diag_suppress=Pa029  // No warning for unknown pragmas in earlier versions of EWARM
  #pragma diag_suppress=Pa137  // No warning for C-Style-Casts with C++
#endif

#include "SMTPC_Conf.h"
#include "SEGGER.h"

#if defined(__cplusplus)
  extern "C" {                // Make sure we have C-declarations in C++ programs.
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   SMTPC_OUT_BUFFER_SIZE
  #define SMTPC_OUT_BUFFER_SIZE   256
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

//
// Recipient and sender field types.
//
#define SMTPC_REC_TYPE_TO    1
#define SMTPC_REC_TYPE_CC    2
#define SMTPC_REC_TYPE_BCC   3
#define SMTPC_REC_TYPE_FROM  4

//
// Security policy used in case IP_SMTPC_API
// pfUpgrade and pfDowngrade are not NULL.
//
#define SMTPC_SEC_POLICY_ALLOW_INSECURE  0  // Try to establish a secure connection using STARTTLS but allow fallback to insecure.
#define SMTPC_SEC_POLICY_SECURE_ONLY     1  // Refuse further processing if STARTTLS is not supported by the server.

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef void* IP_SMTPC_CONTEXT;


typedef struct {
  const char* sServer;    // Address of the SMTP server.
  const char* sUser;      // User name used for the authentication. Set sUser and sPass to NULL or an empty string to not use athentication.
  const char* sPass;      // Password used for the authentication. Set sUser and sPass to NULL or an empty string to not use athentication.
        U8    SecPolicy;  // Security policy to use.
} IP_SMTPC_MTA;

typedef struct {
  const char* sName;  // Name that is shown instead of the address.
  const char* sAddr;  // Address to use for this entry.
        int   Type;   // SMTPC_REC_TYPE_TO, SMTPC_REC_TYPE_CC, SMTPC_REC_TYPE_BCC, SMTPC_REC_TYPE_FROM.
} IP_SMTPC_MAIL_ADDR;

typedef struct {
  void (*pfSend)(IP_SMTPC_CONTEXT* pContext, const char* pData, unsigned NumBytes);
} IP_SMTPC_MULTIPART_API;

typedef struct IP_SMTPC_MULTIPART_ITEM_STRUCT IP_SMTPC_MULTIPART_ITEM;
struct IP_SMTPC_MULTIPART_ITEM_STRUCT {
  const char* sFilename;     // Filename suggested to client in case the multipart item is an attachment.
  const void* pUserContext;  // User context. Can be used to pass a file path or other application information to the pfSendItem callback.
  const char* sContentType;  // Value for the "Content-Type: " field of the multipart item. Examples for
                             // a text file attachment are: "text/plain" or "text/plain; name=\"Test.txt\"" .
  //
  // Callback for sending the content of a multipart item
  // without having to know its length upfront.
  //
  int (*pfSendItem)(IP_SMTPC_CONTEXT* pContext, const IP_SMTPC_MULTIPART_ITEM* pItem, const IP_SMTPC_MULTIPART_API* pAPI);
};

typedef struct {
  const char*                    sSubject;           // Subject of the message to send.
  const char*                    sBody;              // Content of the message to send.
  const char*                    sBoundary;          // Boundary to use for multipart encoding (e.g. when using attachments). Can be NULL if (NumMultipartItems == 0).
  const IP_SMTPC_MULTIPART_ITEM* paMultipartItem;    // Pointer to list of items (attachments) to multipart encode with the message. Can be NULL if (NumMultipartItems == 0).
        unsigned                 NumMultipartItems;  // Number of multipart items that can be found at paMultipartItem .
} IP_SMTPC_MESSAGE;

typedef struct {
  U32 (*pfGetTimeDate)(void);
  int (*pfCallback)   (int Stat, void* p);
  const char* sDomain;   // email domain.
  const char* sTimezone; // Time zone. The zone specifies the offset from Coordinated Universal Time (UTC). Can be NULL.
} IP_SMTPC_APPLICATION;

typedef void* SMTPC_SOCKET;

typedef struct {
  SMTPC_SOCKET (*pfConnect)   (const char* sServer);
  void         (*pfDisconnect)(SMTPC_SOCKET hSock);
  int          (*pfSend)      (const char* pData, int NumBytes, SMTPC_SOCKET hSock);
  int          (*pfReceive)   (      char* pData, int NumBytes, SMTPC_SOCKET hSock);
  int          (*pfUpgrade)   (SMTPC_SOCKET hSock, const char* sServer);
  void         (*pfDowngrade) (SMTPC_SOCKET hSock);
} IP_SMTPC_API;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

int IP_SMTPC_Send(const IP_SMTPC_API* pIP_API, const IP_SMTPC_MAIL_ADDR* paMailAddr, int NumMailAddr, const IP_SMTPC_MESSAGE* pMessage, const IP_SMTPC_MTA* pMTA, const IP_SMTPC_APPLICATION* pApplication);


#if defined(__cplusplus)
}                             // Make sure we have C-declarations in C++ programs.
#endif

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
