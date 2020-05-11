/** @file
  9P library.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

VOID
EFIAPI
TxOpenCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *Open;

  Open = (P9_MESSAGE_PRIVATE_DATA *)Context;
  Open->IsTxDone = TRUE;
}

VOID
EFIAPI
RxOpenCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *Open;

  Open = (P9_MESSAGE_PRIVATE_DATA *)Context;
  Open->IsRxDone = TRUE;
}

EFI_STATUS
P9LOpen (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *Open;
  EFI_TCP4_PROTOCOL             *Tcp4;
  P9TLOpen                      *TxOpen;
  P9RLOpen                      *RxOpen;

  Tcp4 = Volume->Tcp4;

  Open = AllocateZeroPool (sizeof (P9_MESSAGE_PRIVATE_DATA));
  if (Open == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      TxOpenCallback,
      Open,
      &Open->TxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RxOpenCallback,
      Open,
      &Open->RxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  TxOpen = AllocateZeroPool (sizeof (P9TLOpen));
  if (TxOpen == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxOpen->Header.Size = sizeof (P9TLOpen);
  TxOpen->Header.Id   = Tlopen;
  TxOpen->Header.Tag  = Volume->Tag;
  TxOpen->Fid         = IFile->Fid;
  TxOpen->Flags       = IFile->Flags;

  Open->IsTxDone = FALSE;
  Status = TransmitTcp4 (
      Tcp4,
      &Open->TxIoToken,
      TxOpen,
      sizeof (P9TLOpen)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Open->IsTxDone) {
    Tcp4->Poll (Tcp4);
  }

  RxOpen = AllocateZeroPool (sizeof (P9RLOpen));
  if (RxOpen == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Open->IsRxDone = FALSE;
  Status = ReceiveTcp4 (
      Tcp4,
      &Open->RxIoToken,
      RxOpen,
      sizeof (P9RLOpen)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Open->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxOpen->Header.Id != Rlopen) {
    Status = P9Error (RxOpen, sizeof (P9RLOpen));
    goto Exit;
  }

  CopyMem (&IFile->Qid, &RxOpen->Qid, QID_SIZE);
  IFile->IoUnit = RxOpen->IoUnit;

Exit:
  if (Open != NULL) {
    FreePool (Open);
  }

  if (TxOpen != NULL) {
    FreePool (TxOpen);
  }

  if (RxOpen != NULL) {
    FreePool (RxOpen);
  }

  return Status;
}