/** @file
  9P library.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

VOID
EFIAPI
TxGetAttrCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *GetAttr;

  GetAttr = (P9_MESSAGE_PRIVATE_DATA *)Context;

  Print (L"%a: %r\r\n", __func__, GetAttr->TxIoToken.CompletionToken.Status);

  GetAttr->IsTxDone = TRUE;
}

VOID
EFIAPI
RxGetAttrCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *GetAttr;

  GetAttr = (P9_MESSAGE_PRIVATE_DATA *)Context;

  Print (L"%a: %r\r\n", __func__, GetAttr->RxIoToken.CompletionToken.Status);

  GetAttr->IsRxDone = TRUE;
}

EFI_STATUS
P9GetAttr (
  IN P9_VOLUME          *Volume,
  IN UINT16             Tag,
  IN UINT32             Fid,
  IN UINT64             RequestMask,
  OUT struct P9RGetAttr *RxGetAttr
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *GetAttr;
  EFI_TCP4_PROTOCOL             *Tcp4;
  struct P9TGetAttr             *TxGetAttr;

  Tcp4 = Volume->Tcp4;

  GetAttr = AllocateZeroPool (sizeof (P9_MESSAGE_PRIVATE_DATA));
  if (GetAttr == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      TxGetAttrCallback,
      GetAttr,
      &GetAttr->TxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RxGetAttrCallback,
      GetAttr,
      &GetAttr->RxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  TxGetAttr = AllocateZeroPool (sizeof (struct P9TGetAttr));
  if (TxGetAttr == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxGetAttr->Header.Size  = sizeof (struct P9TGetAttr);
  TxGetAttr->Header.Id    = Tgetattr;
  TxGetAttr->Header.Tag   = Tag;
  TxGetAttr->Fid          = Fid;
  TxGetAttr->RequestMask  = RequestMask;

  GetAttr->IsTxDone = FALSE;
  Status = TransmitTcp4 (
      Tcp4,
      &GetAttr->TxIoToken,
      TxGetAttr,
      sizeof (struct P9TGetAttr)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!GetAttr->IsTxDone) {
    Tcp4->Poll (Tcp4);
  }

  GetAttr->IsRxDone = FALSE;
  Status = ReceiveTcp4 (
      Tcp4,
      &GetAttr->RxIoToken,
      RxGetAttr,
      sizeof (struct P9RGetAttr)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!GetAttr->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxGetAttr->Header.Id != Rgetattr) {
    // TODO: Set Status
    Print (L"Rgetattr expected\n");
    goto Exit;
  }

  Status = EFI_SUCCESS;

Exit:
  if (GetAttr != NULL) {
    FreePool (GetAttr);
  }

  if (TxGetAttr != NULL) {
    FreePool (TxGetAttr);
  }
  return Status;
}