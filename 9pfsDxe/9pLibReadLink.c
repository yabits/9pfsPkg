/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

VOID
EFIAPI
TxReadLinkCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *ReadLink;

  ReadLink = (P9_MESSAGE_PRIVATE_DATA *)Context;
  ReadLink->IsTxDone = TRUE;
}

VOID
EFIAPI
RxReadLinkCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *ReadLink;

  ReadLink = (P9_MESSAGE_PRIVATE_DATA *)Context;
  ReadLink->IsRxDone = TRUE;
}

EFI_STATUS
P9LReadLink (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *ReadLink;
  EFI_TCP4_PROTOCOL             *Tcp4;
  P9TReadLink                   *TxReadLink;
  P9RReadLink                   *RxReadLink;
  UINTN                         RxReadLinkSize;
  UINTN                         PathLength;

  Tcp4 = Volume->Tcp4;

  ReadLink = AllocateZeroPool (sizeof (P9_MESSAGE_PRIVATE_DATA));
  if (ReadLink == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      TxReadLinkCallback,
      ReadLink,
      &ReadLink->TxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RxReadLinkCallback,
      ReadLink,
      &ReadLink->RxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  TxReadLink = AllocateZeroPool (sizeof (P9TReadLink));
  if (TxReadLink == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxReadLink->Header.Size = sizeof (P9TReadLink);
  TxReadLink->Header.Id   = Treadlink;
  TxReadLink->Header.Tag  = Volume->Tag;
  TxReadLink->Fid         = IFile->Fid;

  ReadLink->IsTxDone = FALSE;
  Status = TransmitTcp4 (
      Tcp4,
      &ReadLink->TxIoToken,
      TxReadLink,
      sizeof (P9TReadLink)
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!ReadLink->IsTxDone) {
    Tcp4->Poll (Tcp4);
  }

  RxReadLinkSize = sizeof (P9RReadLink) + P9_MAX_PATH;
  RxReadLink = AllocateZeroPool (RxReadLinkSize);
  if (RxReadLink == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  ReadLink->IsRxDone = FALSE;
  Status = ReceiveTcp4 (
      Tcp4,
      &ReadLink->RxIoToken,
      RxReadLink,
      RxReadLinkSize
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!ReadLink->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxReadLink->Header.Id != Rreadlink) {
    Status = P9Error (RxReadLink, RxReadLinkSize);
    goto Exit;
  }

  PathLength = RxReadLink->Target.Size;
  AsciiStrnToUnicodeStrS (
    RxReadLink->Target.String,
    RxReadLink->Target.Size,
    IFile->SymLinkTarget,
    P9_MAX_PATH,
    &PathLength
    );

  Status = EFI_SUCCESS;

Exit:
  if (ReadLink != NULL) {
    FreePool (ReadLink);
  }

  if (TxReadLink != NULL) {
    FreePool (TxReadLink);
  }

  if (RxReadLink != NULL) {
    FreePool (RxReadLink);
  }

  return Status;
}
