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
File    : IP_TFTP.h
Purpose : TFTP API
--------- END-OF-HEADER --------------------------------------------*/


#ifndef _IP_TFTP_H_
#define _IP_TFTP_H_

#include "IP.h"
#include "IP_FS.h"

#if defined(__cplusplus)
extern "C" {     // Make sure we have C-declarations in C++ programs
#endif


#define TFTP_RRQ            0x0001
#define TFTP_WRQ            0x0002
#define TFTP_DATA           0x0003
#define TFTP_ACK            0x0004
#define TFTP_ERR            0x0005

#define TFTP_MODE_NETASCII  0
#define TFTP_MODE_OCTET     1

/*********************************************************************
*
*       Typedefs
*
**********************************************************************
*/
typedef struct {
  const IP_FS_API * pFS_API;             // Pointer to the required file system function
  char *            pBuffer;             // Data buffer (Must be at least 516 bytes)
  unsigned          IFace;               // Interface that should be used.
  int               Sock;                // Data socket
  U16               ServerPort;          // ServerPort
  U16               BufferSize;          // Size of the buffer (Must be at least 516 bytes)
  U16               BlockCnt;            // Block count
  U8                RetryCnt;
} TFTP_CONTEXT;

void IP_TFTP_ServerTask(void * pPara);

int IP_TFTP_InitContext(TFTP_CONTEXT * pContext, unsigned IFace, const IP_FS_API * pFS_API, char * pBuffer, int BufferSize, U16 ServerPort);
int IP_TFTP_SendFile(TFTP_CONTEXT * pContext, unsigned IFace, U32 IPAddr, U16 Port, const char * sFileName, int Mode);
int IP_TFTP_RecvFile(TFTP_CONTEXT * pContext, unsigned IFace, U32 IPAddr, U16 Port, const char * sFileName, int Mode);

#if defined(__cplusplus)
  }              // Make sure we have C-declarations in C++ programs
#endif

#endif           // Avoid multiple inclusion
