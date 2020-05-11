/** @file
  9P library.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

VOID
EFIAPI
TxWalkCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *Walk;

  Walk = (P9_MESSAGE_PRIVATE_DATA *)Context;
  Walk->IsTxDone = TRUE;
}

VOID
EFIAPI
RxWalkCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA  *Walk;

  Walk = (P9_MESSAGE_PRIVATE_DATA *)Context;
  Walk->IsRxDone = TRUE;
}

// TODO: Support depth > 1 path.
EFI_STATUS
P9Walk (
  IN P9_VOLUME          *Volume,
  IN P9_IFILE           *IFile,
  OUT P9_IFILE          *NewIFile,
  IN CHAR16             *Path
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *Walk;
  EFI_TCP4_PROTOCOL             *Tcp4;
  BOOLEAN                       IsAbsolutePath;
  UINTN                         PathSize;
  P9TWalk                       *TxWalk;
  UINTN                         TxWalkSize;
  P9RWalk                       *RxWalk;
  UINTN                         RxWalkSize;

  Tcp4 = Volume->Tcp4;

  Walk = AllocateZeroPool (sizeof (P9_MESSAGE_PRIVATE_DATA));
  if (Walk == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = gBS->CreateEvent (
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      TxWalkCallback,
      Walk,
      &Walk->TxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->CreateEvent (
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RxWalkCallback,
      Walk,
      &Walk->RxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  if (Path[0] == L'\\') {
    IsAbsolutePath = TRUE;
    Path = &Path[1];
  } else {
    IsAbsolutePath = FALSE;
  }

  PathSize = StrLen (Path);
  TxWalkSize = sizeof (P9TWalk) + sizeof (P9String) * 1 + sizeof (CHAR8) * PathSize;

  TxWalk = AllocateZeroPool (TxWalkSize);
  if (TxWalk == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxWalk->Header.Size = TxWalkSize;
  TxWalk->Header.Id   = Twalk;
  TxWalk->Header.Tag  = Volume->Tag;
  TxWalk->Fid         = (IsAbsolutePath == TRUE) ? Volume->Root->Fid : IFile->Fid;
  TxWalk->NewFid      = NewIFile->Fid;
  TxWalk->NWName      = 1;
  TxWalk->WName[0].Size = PathSize;
  UnicodeStrToAsciiStrS (Path, TxWalk->WName[0].String, TxWalk->WName[0].Size + 1);

  Walk->IsTxDone = FALSE;
  Status = TransmitTcp4 (
      Tcp4,
      &Walk->TxIoToken,
      TxWalk,
      TxWalkSize
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Walk->IsTxDone) {
    Tcp4->Poll (Tcp4);
  }

  RxWalkSize = sizeof (P9RWalk) + sizeof (Qid) * 1;
  RxWalk = AllocateZeroPool (RxWalkSize);
  if (RxWalk == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Walk->IsRxDone = FALSE;
  Status = ReceiveTcp4 (
      Tcp4,
      &Walk->RxIoToken,
      RxWalk,
      RxWalkSize
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Walk->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxWalk->Header.Id != Rwalk) {
    Status = P9Error (RxWalk, RxWalkSize);
    goto Exit;
  }

  CopyMem (&NewIFile->Qid, &RxWalk->WQid[0], QID_SIZE);

Exit:
  if (Walk != NULL) {
    FreePool (Walk);
  }

  if (TxWalk != NULL) {
    FreePool (TxWalk);
  }

  if (RxWalk != NULL) {
    FreePool (RxWalk);
  }

  return Status;
}