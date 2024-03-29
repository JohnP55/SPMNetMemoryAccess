#pragma once

#include <common.h>

CPP_WRAPPER(wii::ipc)

#define IPC_HEAP			 -1

#define IPC_OPEN_NONE		  0
#define IPC_OPEN_READ		  1
#define IPC_OPEN_WRITE		  2
#define IPC_OPEN_RW			  (IPC_OPEN_READ|IPC_OPEN_WRITE)

#define IPC_MAXPATH_LEN		 64

#define IPC_OK				  0
#define IPC_EINVAL			 -4
#define IPC_ENOHEAP			 -5
#define IPC_ENOENT			 -6
#define IPC_EQUEUEFULL		 -8
#define IPC_ENOMEM			-22

/*
    IOS_Open mode param
*/
enum IosOpenMode
{
/* 0x0 */ IOS_OPEN_NONE,
/* 0x1 */ IOS_OPEN_READ,
/* 0x2 */ IOS_OPEN_WRITE
};

typedef struct
{
/* 0x0 */ void * data;
/* 0x4 */ u32 len;
} Ioctlv;
SIZE_ASSERT(Ioctlv, 0x8)

UNKNOWN_FUNCTION(IPCInit);
UNKNOWN_FUNCTION(IPCReadReg);
UNKNOWN_FUNCTION(IPCWriteReg);
UNKNOWN_FUNCTION(IPCGetBufferHi);
UNKNOWN_FUNCTION(IPCGetBufferLo);
UNKNOWN_FUNCTION(IPCSetBufferLo);
UNKNOWN_FUNCTION(strnlen);
UNKNOWN_FUNCTION(IpcReplyHandler);
UNKNOWN_FUNCTION(IPCInterruptHandler);
UNKNOWN_FUNCTION(IPCCltInit);
UNKNOWN_FUNCTION(__ios_Ipc2);
UNKNOWN_FUNCTION(IOS_OpenAsync);
s32 IOS_Open(const char * path, s32 mode);
UNKNOWN_FUNCTION(IOS_CloseAsync);
s32 IOS_Close(s32 fd);
UNKNOWN_FUNCTION(IOS_ReadAsync);
s32 IOS_Read(s32 fd, void * dest, u32 length);
UNKNOWN_FUNCTION(IOS_WriteAsync);
UNKNOWN_FUNCTION(IOS_Write);
UNKNOWN_FUNCTION(IOS_SeekAsync);
UNKNOWN_FUNCTION(IOS_IoctlAsync);
s32 IOS_Ioctl(s32 fd, s32 command, void * buf, u32 bufSize, void * ioBuf, u32 ioBufSize);
UNKNOWN_FUNCTION(__ios_Ioctlv);
UNKNOWN_FUNCTION(IOS_IoctlvAsync);
s32 IOS_Ioctlv(s32 fd, s32 command, s32 inCount, s32 outCount, Ioctlv * vecs);
UNKNOWN_FUNCTION(IOS_IoctlvReboot);
UNKNOWN_FUNCTION(iosCreateHeap);
UNKNOWN_FUNCTION(__iosAlloc);
UNKNOWN_FUNCTION(iosAllocAligned);
UNKNOWN_FUNCTION(iosFree);
UNKNOWN_FUNCTION(IPCiProfInit);
UNKNOWN_FUNCTION(IPCiProfQueueReq);
UNKNOWN_FUNCTION(IPCiProfAck);
UNKNOWN_FUNCTION(IPCiProfReply);

CPP_WRAPPER_END()
