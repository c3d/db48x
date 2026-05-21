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
File        : IP_FTPC.h
Purpose     : Publics for the FTP client
---------------------------END-OF-HEADER------------------------------

Attention : Do not modify this file !
*/

#ifndef  IP_FTPC_H
#define  IP_FTPC_H

#ifdef __ICCARM__  // IAR
  #pragma diag_suppress=Pa029  // No warning for unknown pragmas in earlier verions of EWARM
  #pragma diag_suppress=Pa137  // No warning for C-Style-Casts with C++
#endif

#include "FTPC_Conf.h"
#include "SEGGER.h"
#include "IP_FS.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       defines
*
**********************************************************************
*/

//
// Connection modes
//
#define FTPC_MODE_ACTIVE                  (0u << 0)
#define FTPC_MODE_PASSIVE                 (1u << 0)
//
#define FTPC_MODE_PLAIN_FTP               (0u << 1)
#define FTPC_MODE_EXPLICIT_TLS_REQUIRED   (1u << 1)
#define FTPC_MODE_IMPLICIT_TLS_REQUIRED   (2u << 1)

//
// FTP commands
//
typedef enum {
  FTPC_CMD_LIST,
  FTPC_CMD_CWD,
  FTPC_CMD_CDUP,
  FTPC_CMD_STOR,
  FTPC_CMD_APPE,
  FTPC_CMD_RETR,
  FTPC_CMD_USER,
  FTPC_CMD_PASS,
  FTPC_CMD_SYST,
  FTPC_CMD_PWD,
  FTPC_CMD_TYPE,
  FTPC_CMD_MKD ,
  FTPC_CMD_RMD,
  FTPC_CMD_DELE,
  FTPC_CMD_PROT,
  FTPC_CMD_PBSZ,
  FTPC_CMD_FEAT,
  FTPC_CMD_QUIT,
  //
  // These commands are internal and cannot be sent using IP_FTPC_ExecCmd[Ex]().
  //
  FTPC_CMD_NULL,
  FTPC_CMD_PASV,
  FTPC_CMD_AUTH,
  FTPC_CMD_PORT
} IP_FTPC_CMD;

#define FTPC_ERROR  -1

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

typedef void*                FTPC_SOCKET;
typedef struct FTPC_CONTEXT  IP_FTPC_CONTEXT;

typedef struct {
  FTPC_SOCKET  (*pfConnect)        (const char* SrvAddr, unsigned SrvPort);
  void         (*pfDisconnect)     (FTPC_SOCKET Socket);
  int          (*pfSend)           (const char* pData, int Len, FTPC_SOCKET Socket);
  int          (*pfReceive)        (char* pData, int Len, FTPC_SOCKET Socket);
  int          (*pfSetSecure)      (FTPC_SOCKET Socket, FTPC_SOCKET Clone);
  FTPC_SOCKET  (*pfListen)         (FTPC_SOCKET CtrlSocket, U8* pIPAddr, U16* pPort);
  FTPC_SOCKET  (*pfAccept)         (FTPC_SOCKET CtrlSocket, FTPC_SOCKET pSock, U16* pPort);
} IP_FTPC_API;

typedef struct {
  void (*pfReply)(IP_FTPC_CONTEXT* pContext, IP_FTPC_CMD Cmd, const char* sResponse, unsigned ResponseLength, unsigned IsLineComplete);
} IP_FTPC_APPLICATION;

typedef struct {
  const IP_FTPC_API* pIP_API;
        void*        pSock;
        U8*          pBuffer;                  // Pointer to the data buffer
        int          Size;                     // Size of buffer
        int          Cnt;                      // Number of bytes in buffer
        int          RdOff;
} IN_BUFFER_CONTEXT;

typedef struct {
  const IP_FTPC_API* pIP_API;
        void*        pSock;
        U8*          pBuffer;                  // Pointer to the data buffer
        int          Size;                     // Size of buffer
        int          Cnt;                      // Number of bytes in buffer
} OUT_BUFFER_CONTEXT;

typedef struct {
  const char*        sServer;
  const char*        sUser;                    // User name used for the authentication
  const char*        sPass;                    // Password used for the authentication
        unsigned     PortCmd;                  // Port of the command connection
        unsigned     PortData;                 // Port of the data connection
        unsigned     Mode;                     // Data transfer process type, active: 0, passive: 1
} FTPC_CONN_DATA;

struct FTPC_CONTEXT {
  const IP_FTPC_API*         pIP_API;
  const IP_FS_API*           pFS_API;          // File system info
  const IP_FTPC_APPLICATION* pApplication;
        IN_BUFFER_CONTEXT    CtrlIn;
        IN_BUFFER_CONTEXT    DataIn;
        OUT_BUFFER_CONTEXT   CtrlOut;
        OUT_BUFFER_CONTEXT   DataOut;
        FTPC_CONN_DATA       ConnData;
};

/*********************************************************************
*
*       IP_FTPC_CMD_CONFIG
*
*  Function description
*    Configuration structure used with IP_FTPC_ExecCmdEx() for
*    extended functionality.
*/
typedef struct {
  const char*    sPara;        // Same as sPara with old IP_FTPC_ExecCmd() for backwards compatibility.
                               // Can be NULL if not required for command or sLocalPath and/or sRemotePath are used.
  const char*    sLocalPath;   // Local path (terminated string) to use with command. Can be NULL if not supported by command (or makes no sense).
                               // Overrides sPara . Can be used with the following commands:
                               // * FTPC_CMD_STOR
                               // * FTPC_CMD_APPE
                               // * FTPC_CMD_RETR
  const char*    sRemotePath;  // Remote path (terminated string) to use with command. Can be NULL if not supported by command (or makes no sense).
                               // Overrides sPara . Can be used with the following commands:
                               // * FTPC_CMD_LIST
                               // * FTPC_CMD_CWD
                               // * FTPC_CMD_STOR
                               // * FTPC_CMD_APPE
                               // * FTPC_CMD_RETR
                               // * FTPC_CMD_MKD
                               // * FTPC_CMD_RMD
                               // * FTPC_CMD_DELE
        U8*      pData;        // Indicates that input data shall be taken from the buffer at pData instead of a filesystem.
                               // Can be NULL if using a filesystem and reading is intended to be done from a filename
                               // given by sPara . Can be used with the following commands:
                               // * FTPC_CMD_STOR
                               // * FTPC_CMD_APPE
        unsigned NumBytes;     // Number of bytes to input starting from pData .
} IP_FTPC_CMD_CONFIG;

/*********************************************************************
*
*       Functions
*
**********************************************************************
*/

int IP_FTPC_Init      (IP_FTPC_CONTEXT* pContext, const IP_FTPC_API* pIP_API, const IP_FS_API* pFS_API, U8* pCtrlBuffer, unsigned NumBytesCtrl, U8* pDataInBuffer, unsigned NumBytesDataIn, U8* pDataOutBuffer, unsigned NumBytesDataOut);
int IP_FTPC_InitEx    (IP_FTPC_CONTEXT* pContext, const IP_FTPC_API* pIP_API, const IP_FS_API* pFS_API, U8* pCtrlBuffer, unsigned NumBytesCtrl, U8* pDataInBuffer, unsigned NumBytesDataIn, U8* pDataOutBuffer, unsigned NumBytesDataOut, const IP_FTPC_APPLICATION* pApplication);
int IP_FTPC_Connect   (IP_FTPC_CONTEXT* pContext, const char* sServer, const char* sUser, const char* sPass, unsigned PortCmd, unsigned Mode);
int IP_FTPC_Disconnect(IP_FTPC_CONTEXT* pContext);
int IP_FTPC_ExecCmd   (IP_FTPC_CONTEXT* pContext, IP_FTPC_CMD Cmd, const char* sPara);
int IP_FTPC_ExecCmdEx (IP_FTPC_CONTEXT* pContext, IP_FTPC_CMD Cmd, IP_FTPC_CMD_CONFIG* pConfig);

#if defined(__cplusplus)
  }
#endif


#endif   /* Avoid multiple inclusion */

/*************************** End of file ****************************/




