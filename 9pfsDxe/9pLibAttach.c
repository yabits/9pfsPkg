/** @file
  9P library.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

VOID
EFIAPI
TxAttachCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *Attach;

  Attach = (P9_MESSAGE_PRIVATE_DATA *)Context;

  Print (L"%a: %r\r\n", __func__, Attach->TxIoToken.CompletionToken.Status);

  Attach->IsTxDone = TRUE;
}

VOID
EFIAPI
RxAttachCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *Attach;

  Attach = (P9_MESSAGE_PRIVATE_DATA *)Context;

  Print (L"%a: %r\r\n", __func__, Attach->RxIoToken.CompletionToken.Status);

  Attach->IsRxDone = TRUE;
}

EFI_STATUS
P9Attach (
  IN P9_VOLUME          *Volume,
  IN UINT16             Tag,
  IN UINT32             Fid,
  IN CHAR8              *UNameStr,
  IN CHAR8              *ANameStr,
  OUT UINT8             *Qid
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *Attach;
  EFI_TCP4_PROTOCOL             *Tcp4;
  UINTN                         UNameSize;
  UINTN                         ANameSize;
  UINTN                         TxAttachSize;
  struct P9TAttach              *TxAttach;
  struct P9RAttach              *RxAttach;
  struct P9String               *UName;
  struct P9String               *AName;

  Tcp4 = Volume->Tcp4;

  Attach = AllocateZeroPool (sizeof (P9_MESSAGE_PRIVATE_DATA));
  if (Attach == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      TxAttachCallback,
      Attach,
      &Attach->TxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RxAttachCallback,
      Attach,
      &Attach->RxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  UNameSize = AsciiStrLen (UNameStr);
  ANameSize = AsciiStrLen (ANameStr);
  TxAttachSize = sizeof (struct P9TAttach) + sizeof (CHAR8) * UNameSize + sizeof (CHAR8) * ANameSize;
  TxAttach = AllocateZeroPool (TxAttachSize);
  if (TxAttach == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxAttach->Header.Size = TxAttachSize;
  TxAttach->Header.Id = Tattach;
  TxAttach->Header.Tag = Tag;
  TxAttach->Fid = Fid;
  TxAttach->AFid = (UINT32)~0; // NOFID

  UName = &TxAttach->UName;
  AName = (struct P9String *)((UINT8 *)&TxAttach->UName + sizeof (struct P9String) + sizeof (CHAR8) * UNameSize);

  UName->Size = UNameSize;
  AName->Size = ANameSize;
  AsciiStrnCpy (UName->String, UNameStr, UNameSize);
  AsciiStrnCpy (AName->String, ANameStr, ANameSize);

  Attach->IsTxDone = FALSE;
  Status = TransmitTcp4 (
      Tcp4,
      &Attach->TxIoToken,
      TxAttach,
      TxAttachSize);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Attach->IsTxDone) {
    Tcp4->Poll (Tcp4);
  }

  RxAttach = AllocateZeroPool (sizeof (struct P9RAttach));
  if (RxAttach == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Attach->IsRxDone = FALSE;
  Status = ReceiveTcp4 (
      Tcp4,
      &Attach->RxIoToken,
      RxAttach,
      sizeof (struct P9RAttach));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  while (!Attach->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxAttach->Header.Id != Rattach) {
    Print (L"Rattach expected\r\n");
    goto Exit;
  }

  CopyMem (Qid, &RxAttach->Qid, sizeof (UINT8) * 13);

Exit:
  if (Attach != NULL) {
    FreePool (Attach);
  }

  if (TxAttach != NULL) {
    FreePool (TxAttach);
  }

  if (RxAttach != NULL) {
    FreePool (RxAttach);
  }

  return Status;
}