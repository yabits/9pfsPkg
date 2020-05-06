/** @file
  9P library.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pfs.h"

typedef struct _P9_VERSION_PRIVATE_DATA P9_VERSION_PRIVATE_DATA;

struct _P9_VERSION_PRIVATE_DATA {
  EFI_TCP4_IO_TOKEN         TxIoToken;
  EFI_TCP4_IO_TOKEN         RxIoToken;
  BOOLEAN                   IsTxDone;
  BOOLEAN                   IsRxDone;
};

#define Tversion  100
#define Rversion  101

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
#pragma pack()

VOID
EFIAPI
TxVersionCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_VERSION_PRIVATE_DATA *Version;

  Version = (P9_VERSION_PRIVATE_DATA *)Context;

  Print (L"%a: %r\r\n", __func__, Version->TxIoToken.CompletionToken.Status);

  Version->IsTxDone = TRUE;
}

VOID
EFIAPI
RxVersionCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_VERSION_PRIVATE_DATA *Version;

  Version = (P9_VERSION_PRIVATE_DATA *)Context;

  Print (L"%a: %r\r\n", __func__, Version->RxIoToken.CompletionToken.Status);

  Version->IsRxDone = TRUE;
}

EFI_STATUS
P9Version (
  IN P9_VOLUME          *Volume,
  IN OUT UINT32         *MSize,
  IN CHAR8              *VersionString
  )
{
  EFI_STATUS                    Status;
  P9_VERSION_PRIVATE_DATA       *Version;
  EFI_TCP4_PROTOCOL             *Tcp4;
  UINTN                         VersionSize;
  UINTN                         TxVersionSize;
  UINTN                         RxVersionSize;
  struct P9TxVersion            *TxVersion;
  struct P9RxVersion            *RxVersion;

  Tcp4 = Volume->Tcp4;

  Version = AllocateZeroPool (sizeof (P9_VERSION_PRIVATE_DATA));
  if (Version == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      TxVersionCallback,
      Version,
      &Version->TxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RxVersionCallback,
      Version,
      &Version->RxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  VersionSize = AsciiStrLen (VersionString);
  TxVersionSize = sizeof (struct P9TxVersion) + sizeof (CHAR8) * VersionSize;
  TxVersion = AllocateZeroPool (TxVersionSize);
  if (TxVersion == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxVersion->Header.Size = TxVersionSize;
  TxVersion->Header.Id = Tversion;
  TxVersion->Header.Tag = (UINT16)~0;
  TxVersion->MSize = *MSize;
  TxVersion->Version.Size = VersionSize;
  CopyMem (&TxVersion->Version.String, VersionString, sizeof (CHAR8) * VersionSize);

  Version->IsTxDone = FALSE;
  Status = TransmitTcp4 (Tcp4, &Version->TxIoToken, TxVersion, TxVersionSize);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Version->IsTxDone) {
    Tcp4->Poll (Tcp4);
  }

  RxVersionSize = TxVersionSize;
  RxVersion = AllocateZeroPool (RxVersionSize);
  if (RxVersion == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Version->IsRxDone = FALSE;
  Status = ReceiveTcp4 (Tcp4, &Version->RxIoToken, RxVersion, RxVersionSize);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Version->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxVersion->Header.Id != Rversion) {
    Print (L"Rversion expected\n");
    Status = EFI_UNSUPPORTED;
    goto Exit;
  }

  if (AsciiStrnCmp (TxVersion->Version.String, RxVersion->Version.String, TxVersion->Version.Size) != 0) {
    Print (L"Unsupported 9P Protocol: %a\n", RxVersion->Version.String);
    Status = EFI_UNSUPPORTED;
    goto Exit;
  }

  *MSize = RxVersion->MSize;

Exit:
  if (Version != NULL) {
    FreePool (Version);
  }

  if (TxVersion != NULL) {
    FreePool (TxVersion);
  }

  if (RxVersion != NULL) {
    FreePool (RxVersion);
  }

  return Status;
}