/** @file
  9P library.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

VOID
EFIAPI
TxReadDirCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *ReadDir;

  ReadDir = (P9_MESSAGE_PRIVATE_DATA *)Context;
  ReadDir->IsTxDone = TRUE;
}

VOID
EFIAPI
RxReadDirCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *ReadDir;

  ReadDir = (P9_MESSAGE_PRIVATE_DATA *)Context;
  ReadDir->IsRxDone = TRUE;
}

EFI_STATUS
P9LReadDir (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile,
  IN UINT64             Offset,
  IN OUT UINT32         *Count,
  OUT VOID              *Data
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *ReadDir;
  EFI_TCP4_PROTOCOL             *Tcp4;
  P9TReadDir                    *TxReadDir;
  P9RReadDir                    *RxReadDir;
  UINTN                         RxReadDirSize;

  Tcp4 = Volume->Tcp4;

  ReadDir = AllocateZeroPool (sizeof (P9_MESSAGE_PRIVATE_DATA));
  if (ReadDir == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      TxReadDirCallback,
      ReadDir,
      &ReadDir->TxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RxReadDirCallback,
      ReadDir,
      &ReadDir->RxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  TxReadDir = AllocateZeroPool (sizeof (P9TReadDir));
  if (TxReadDir == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxReadDir->Header.Size = sizeof (P9TReadDir);
  TxReadDir->Header.Id   = Treaddir;
  TxReadDir->Header.Tag  = Volume->Tag;
  TxReadDir->Fid         = IFile->Fid;
  TxReadDir->Offset      = Offset;
  TxReadDir->Count       = *Count;

  ReadDir->IsTxDone = FALSE;
  Status = TransmitTcp4 (
      Tcp4,
      &ReadDir->TxIoToken,
      TxReadDir,
      sizeof (P9TReadDir)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!ReadDir->IsTxDone) {
    Tcp4->Poll (Tcp4);
  }

  RxReadDirSize = sizeof (P9RReadDir) + *Count;
  RxReadDir = AllocateZeroPool (RxReadDirSize);
  if (RxReadDir == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  ReadDir->IsRxDone = FALSE;
  Status = ReceiveTcp4 (
      Tcp4,
      &ReadDir->RxIoToken,
      RxReadDir,
      RxReadDirSize
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!ReadDir->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxReadDir->Header.Id != Rreaddir) {
    Status = P9Error (RxReadDir, RxReadDirSize);
    goto Exit;
  }

  CopyMem (Data, (VOID *)RxReadDir->Data, RxReadDir->Count);
  *Count = RxReadDir->Count;

  Status = EFI_SUCCESS;

Exit:
  if (ReadDir != NULL) {
    FreePool (ReadDir);
  }

  if (TxReadDir != NULL) {
    FreePool (TxReadDir);
  }

  if (RxReadDir != NULL) {
    FreePool (RxReadDir);
  }

  return Status;
}
