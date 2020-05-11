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

#include "9p.h"
#include "9pfs.h"

#define FRAGMENT_SIZE 576

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

UINT32
GetFid (
  VOID
  );

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
  IN OUT P9_VOLUME      *Volume,
  IN CHAR16             *StationAddrStr,
  IN CHAR16             *SubnetMaskStr,
  IN CHAR16             *RemoteAddrStr
  );

EFI_STATUS
ConnectP9 (
  IN P9_VOLUME          *Volume
  );

EFI_STATUS
P9Error (
  IN VOID               *Data,
  IN UINTN              DataSize
  );

EFI_STATUS
P9Version (
  IN P9_VOLUME          *Volume,
  IN OUT UINT32         *MSize
  );

EFI_STATUS
P9Attach (
  IN P9_VOLUME          *Volume,
  IN UINT32             Fid,
  IN UINT32             AFid,
  IN CHAR8              *UNameStr,
  IN CHAR8              *ANameStr,
  OUT P9_IFILE          *IFile
  );

EFI_STATUS
P9LOpen (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile
  );

EFI_STATUS
P9GetAttr (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile
  );

EFI_STATUS
P9LRead (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile,
  IN OUT UINT32         *Count,
  OUT VOID              *Data
  );

EFI_STATUS
P9Walk (
  IN P9_VOLUME          *Volume,
  IN P9_IFILE           *IFile,
  OUT P9_IFILE          *NewIFile,
  IN CHAR16             *Path
  );

EFI_STATUS
P9Clunk (
  IN P9_VOLUME          *Volume,
  IN P9_IFILE           *IFile
  );

#endif