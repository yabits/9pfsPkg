/** @file
  Main header file for 9P library.

Copyright (c) 2005 - 2013, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _9PLIB_H_
#define _9PLIB_H_

#include <Uefi.h>

#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>
#include <Protocol/ServiceBinding.h>
#include <Protocol/Tcp4.h>

#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "9pfs.h"

#define FRAGMENT_SIZE 576

#define Tversion    100
#define Rversion    101
#define Tattach     104
#define Rattach     105
#define Tlopen      12
#define Rlopen      13
#define Tgetattr    24
#define Rgetattr    25

#pragma pack(1)
struct P9Header {
  UINT32  Size;
  UINT8   Id;
  UINT16  Tag;
};

struct P9String {
  UINT16  Size;
  CHAR8   String[0];
};

struct P9TxVersion {
  struct P9Header Header;
  UINT32          MSize;
  struct P9String Version;
};

struct P9RxVersion {
  struct P9Header Header;
  UINT32          MSize;
  struct P9String Version;
};

struct P9TAttach {
  struct P9Header Header;
  UINT32          Fid;
  UINT32          AFid;
  struct P9String UName;
  struct P9String AName;
};

struct P9RAttach {
  struct P9Header Header;
  UINT8           Qid[13];
};

struct P9TLOpen {
  struct P9Header Header;
  UINT32          Fid;
  UINT32          Flags;
};

struct P9RLOpen {
  struct P9Header Header;
  UINT8           Qid[13];
  UINT32          IoUnit;
};

struct P9TGetAttr {
  struct P9Header Header;
  UINT32          Fid;
  UINT64          RequestMask;
};

struct P9RGetAttr {
  struct P9Header Header;
  UINT64          Valid;
  UINT8           Qid[13];
  UINT32          Mode;
  UINT32          Uid;
  UINT32          Gid;
  UINT64          NLink;
  UINT64          RDev;
  UINT64          Size;
  UINT64          BlkSize;
  UINT64          Blocks;
  UINT64          ATimeSec;
  UINT64          ATimeNSec;
  UINT64          MTimeSec;
  UINT64          MTimeNSec;
  UINT64          CTimeSec;
  UINT64          CTimeNSec;
  UINT64          BTimeSec;
  UINT64          BTimeNSec;
  UINT64          Gen;
  UINT64          DataVersion;
};
#pragma pack()

typedef struct _P9_CONNECT_PRIVATE_DATA P9_CONNECT_PRIVATE_DATA;
typedef struct _P9_MESSAGE_PRIVATE_DATA P9_MESSAGE_PRIVATE_DATA;

struct _P9_CONNECT_PRIVATE_DATA {
  EFI_TCP4_CONNECTION_TOKEN ConnectionToken;
  BOOLEAN                   IsConnectDone;
};

struct _P9_MESSAGE_PRIVATE_DATA {
  EFI_TCP4_IO_TOKEN         TxIoToken;
  EFI_TCP4_IO_TOKEN         RxIoToken;
  BOOLEAN                   IsTxDone;
  BOOLEAN                   IsRxDone;
};

EFI_STATUS
TransmitTcp4 (
  IN EFI_TCP4_PROTOCOL  *Tcp4,
  IN EFI_TCP4_IO_TOKEN  *TransmitToken,
  IN VOID               *Data,
  IN UINTN              DataSize
  );

EFI_STATUS
ReceiveTcp4 (
  IN EFI_TCP4_PROTOCOL  *Tcp4,
  IN EFI_TCP4_IO_TOKEN  *ReceiveToken,
  OUT VOID              *Data,
  OUT UINTN             DataSize
  );

EFI_STATUS
ConfigureP9 (
  IN OUT P9_VOLUME          *Volume,
  IN CHAR16                 *StationAddrStr,
  IN CHAR16                 *SubnetMaskStr,
  IN CHAR16                 *RemoteAddrStr
  );

EFI_STATUS
ConnectP9 (
  IN P9_VOLUME                  *Volume
  );

EFI_STATUS
P9Version (
  IN P9_VOLUME          *Volume,
  IN OUT UINT32         *MSize,
  IN CHAR8              *VersionString
  );

EFI_STATUS
P9Attach (
  IN P9_VOLUME          *Volume,
  IN UINT16             Tag,
  IN UINT32             Fid,
  IN CHAR8              *UNameStr,
  IN CHAR8              *ANameStr,
  OUT UINT8             *Qid
  );

EFI_STATUS
P9LOpen (
  IN P9_VOLUME          *Volume,
  IN UINT16             Tag,
  IN UINT32             Fid,
  IN UINT32             Flags,
  OUT UINT8             *Qid,
  OUT UINT32            *IoUnit
  );

EFI_STATUS
P9GetAttr (
  IN P9_VOLUME          *Volume,
  IN UINT16             Tag,
  IN UINT32             Fid,
  IN UINT64             RequestMask,
  OUT struct P9RGetAttr *RxGetAttr
  );

#endif