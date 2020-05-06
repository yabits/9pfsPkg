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

  Print (L"%a: %r\r\n", __func__, Open->TxIoToken.CompletionToken.Status);

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

  Print (L"%a: %r\r\n", __func__, Open->RxIoToken.CompletionToken.Status);

  Open->IsRxDone = TRUE;
}

EFI_STATUS
P9LOpen (
  IN P9_VOLUME          *Volume,
  IN UINT16             Tag,
  IN UINT32             Fid,
  IN UINT32             Flags,
  OUT UINT8             *Qid,
  OUT UINT32            *IoUnit
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *Open;
  EFI_TCP4_PROTOCOL             *Tcp4;
  struct P9TLOpen               *TxOpen;
  struct P9RLOpen               *RxOpen;

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

  TxOpen = AllocateZeroPool (sizeof (struct P9TLOpen));
  if (TxOpen == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxOpen->Header.Size = sizeof (struct P9TLOpen);
  TxOpen->Header.Id   = Tlopen;
  TxOpen->Header.Tag  = Tag;
  TxOpen->Fid         = Fid;
  TxOpen->Flags       = Flags;

  Open->IsTxDone = FALSE;
  Status = TransmitTcp4 (
      Tcp4,
      &Open->TxIoToken,
      TxOpen,
      sizeof (struct P9TLOpen)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Open->IsTxDone) {
    Tcp4->Poll (Tcp4);
  }

  RxOpen = AllocateZeroPool (sizeof (struct P9RLOpen));
  if (RxOpen == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Open->IsRxDone = FALSE;
  Status = ReceiveTcp4 (
      Tcp4,
      &Open->RxIoToken,
      RxOpen,
      sizeof (struct P9RLOpen)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Open->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxOpen->Header.Id != Rlopen) {
    // TODO: Set Status
    Print (L"Rlopen expected\n");
    goto Exit;
  }

  CopyMem (Qid, &RxOpen->Qid, sizeof (UINT8) * 13);
  *IoUnit = RxOpen->IoUnit;

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