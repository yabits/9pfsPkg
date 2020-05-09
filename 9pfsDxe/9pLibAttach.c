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
  IN UINT32             Fid,
  IN UINT32             AFid,
  IN CHAR8              *UNameStr,
  IN CHAR8              *ANameStr,
  OUT P9_IFILE          *IFile
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *Attach;
  EFI_TCP4_PROTOCOL             *Tcp4;
  UINTN                         UNameSize;
  UINTN                         ANameSize;
  UINTN                         TxAttachSize;
  P9TAttach                     *TxAttach;
  P9RAttach                     *RxAttach;
  P9String                      *UName;
  P9String                      *AName;

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
  TxAttachSize = sizeof (P9TAttach) + sizeof (CHAR8) * UNameSize + sizeof (CHAR8) * ANameSize;
  TxAttach = AllocateZeroPool (TxAttachSize);
  if (TxAttach == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxAttach->Header.Size = TxAttachSize;
  TxAttach->Header.Id = Tattach;
  TxAttach->Header.Tag = Volume->Tag;
  TxAttach->Fid = Fid;
  TxAttach->AFid = AFid;

  UName = &TxAttach->UName;
  AName = (P9String *)((UINT8 *)&TxAttach->UName + sizeof (P9String) + sizeof (CHAR8) * UNameSize);

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

  RxAttach = AllocateZeroPool (sizeof (P9RAttach));
  if (RxAttach == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Attach->IsRxDone = FALSE;
  Status = ReceiveTcp4 (
      Tcp4,
      &Attach->RxIoToken,
      RxAttach,
      sizeof (P9RAttach));
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Attach->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxAttach->Header.Id != Rattach) {
    // TODO: Set Status
    Print (L"Rattach expected\r\n");
    goto Exit;
  }

  CopyMem (&IFile->Qid, &RxAttach->Qid, QID_SIZE);

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