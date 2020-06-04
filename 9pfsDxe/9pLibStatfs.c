/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

VOID
EFIAPI
TxStatfsCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *Statfs;

  Statfs = (P9_MESSAGE_PRIVATE_DATA *)Context;
  Statfs->IsTxDone = TRUE;
  gBS->CloseEvent (Statfs->TxIoToken.CompletionToken.Event);
}

VOID
EFIAPI
RxStatfsCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *Statfs;

  Statfs = (P9_MESSAGE_PRIVATE_DATA *)Context;
  Statfs->IsRxDone = TRUE;
  gBS->CloseEvent (Statfs->RxIoToken.CompletionToken.Event);
}

EFI_STATUS
P9Statfs (
  IN P9_VOLUME          *Volume
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *Statfs;
  EFI_TCP4_PROTOCOL             *Tcp4;
  P9TStatfs                     *TxStatfs;
  P9RStatfs                     *RxStatfs;
  UINTN                         Size;
  EFI_FILE_SYSTEM_INFO          *FileSystemInfo;

  Tcp4 = Volume->Tcp4;

  Statfs = AllocateZeroPool (sizeof (P9_MESSAGE_PRIVATE_DATA));
  if (Statfs == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      TxStatfsCallback,
      Statfs,
      &Statfs->TxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RxStatfsCallback,
      Statfs,
      &Statfs->RxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  TxStatfs = AllocateZeroPool (sizeof (P9TStatfs));
  if (TxStatfs == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxStatfs->Header.Size  = sizeof (P9TStatfs);
  TxStatfs->Header.Id    = Tstatfs;
  TxStatfs->Header.Tag   = Volume->Tag;
  TxStatfs->Fid          = Volume->Root->Fid;

  Statfs->IsTxDone = FALSE;
  Status = TransmitTcp4 (
      Tcp4,
      &Statfs->TxIoToken,
      TxStatfs,
      sizeof (P9TStatfs)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Statfs->IsTxDone) {
    Tcp4->Poll (Tcp4);
  }

  RxStatfs = AllocateZeroPool (sizeof (P9RStatfs));
  if (RxStatfs == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Statfs->IsRxDone = FALSE;
  Status = ReceiveTcp4 (
      Tcp4,
      &Statfs->RxIoToken,
      RxStatfs,
      sizeof (P9RStatfs)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Statfs->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxStatfs->Header.Id != Rstatfs) {
    Status = P9Error (RxStatfs, sizeof (P9RStatfs));
    goto Exit;
  }

  Size = SIZE_OF_EFI_FILE_SYSTEM_INFO + StrSize (P9_VOLUME_LABEL);
  if (Volume->FileSystemInfo == NULL) {
    Volume->FileSystemInfo = AllocateZeroPool (Size);
    if (Volume->FileSystemInfo == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }
  }

  FileSystemInfo = Volume->FileSystemInfo;

  FileSystemInfo->Size        = Size;
  FileSystemInfo->ReadOnly    = TRUE; // XXX: Currently read-only.
  FileSystemInfo->VolumeSize  = RxStatfs->BSize * RxStatfs->Blocks;
  FileSystemInfo->FreeSpace   = RxStatfs->BSize * RxStatfs->BFree;
  FileSystemInfo->BlockSize   = RxStatfs->BSize;
  StrCpyS (FileSystemInfo->VolumeLabel, StrSize (P9_VOLUME_LABEL), P9_VOLUME_LABEL);

  Status = EFI_SUCCESS;

Exit:
  if (Statfs != NULL) {
    FreePool (Statfs);
  }

  if (TxStatfs != NULL) {
    FreePool (TxStatfs);
  }

  if (RxStatfs != NULL) {
    FreePool (RxStatfs);
  }

  return Status;
}
