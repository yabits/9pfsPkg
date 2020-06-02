/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

VOID
EFIAPI
TxClunkCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *Clunk;

  Clunk = (P9_MESSAGE_PRIVATE_DATA *)Context;
  Clunk->IsTxDone = TRUE;
}

VOID
EFIAPI
RxClunkCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *Clunk;

  Clunk = (P9_MESSAGE_PRIVATE_DATA *)Context;
  Clunk->IsRxDone = TRUE;
}

EFI_STATUS
P9Clunk (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *Clunk;
  EFI_TCP4_PROTOCOL             *Tcp4;
  P9TClunk                      *TxClunk;
  P9RClunk                      *RxClunk;

  Tcp4 = Volume->Tcp4;

  Clunk = AllocateZeroPool (sizeof (P9_MESSAGE_PRIVATE_DATA));
  if (Clunk == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      TxClunkCallback,
      Clunk,
      &Clunk->TxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RxClunkCallback,
      Clunk,
      &Clunk->RxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  TxClunk = AllocateZeroPool (sizeof (P9TClunk));
  if (TxClunk == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxClunk->Header.Size  = sizeof (P9TClunk);
  TxClunk->Header.Id    = Tclunk;
  TxClunk->Header.Tag   = Volume->Tag;
  TxClunk->Fid          = IFile->Fid;

  Clunk->IsTxDone = FALSE;
  Status = TransmitTcp4 (
      Tcp4,
      &Clunk->TxIoToken,
      TxClunk,
      sizeof (P9TClunk)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Clunk->IsTxDone) {
    Tcp4->Poll (Tcp4);
  }

  RxClunk = AllocateZeroPool (sizeof (P9RClunk));
  if (RxClunk == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Clunk->IsRxDone = FALSE;
  Status = ReceiveTcp4 (
      Tcp4,
      &Clunk->RxIoToken,
      RxClunk,
      sizeof (P9RClunk)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Clunk->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxClunk->Header.Id != Rclunk) {
    Status = P9Error (RxClunk, sizeof (P9RClunk));
    goto Exit;
  }

  Status = EFI_SUCCESS;

Exit:
  if (Clunk != NULL) {
    FreePool (Clunk);
  }

  if (TxClunk != NULL) {
    FreePool (TxClunk);
  }

  if (RxClunk != NULL) {
    FreePool (RxClunk);
  }

  return Status;
}