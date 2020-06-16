/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

CHAR16 *
P9GetNextNameComponent (
  IN CHAR16   *Path,
  OUT CHAR16  *Name
  )
{
  while (*Path != 0 && *Path != PATH_NAME_SEPARATOR) {
    *Name++ = *Path++;
  }
  *Name = L'\0';

  while (*Path == PATH_NAME_SEPARATOR) {
    Path++;
  }

  return Path;
}

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
  gBS->CloseEvent (Walk->TxIoToken.CompletionToken.Event);
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
  gBS->CloseEvent (Walk->RxIoToken.CompletionToken.Event);
}

EFI_STATUS
DoP9Walk (
  IN EFI_TCP4_PROTOCOL  *Tcp4,
  IN UINT16             Tag,
  IN UINT32             Fid,
  IN UINT32             NewFid,
  IN CHAR16             *Path,
  OUT Qid               *NewQid
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *Walk;
  UINTN                         PathSize;
  P9TWalk                       *TxWalk;
  UINTN                         TxWalkSize;
  P9RWalk                       *RxWalk;
  UINTN                         RxWalkSize;

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

  if (Path == NULL) {
    PathSize = 0;
  } else {
    PathSize = StrLen (Path);
  }
  TxWalkSize = sizeof (P9TWalk) + sizeof (P9String) * 1 + sizeof (CHAR8) * PathSize;

  TxWalk = AllocateZeroPool (TxWalkSize);
  if (TxWalk == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxWalk->Header.Size   = TxWalkSize;
  TxWalk->Header.Id     = Twalk;
  TxWalk->Header.Tag    = Tag;
  TxWalk->Fid           = Fid;
  TxWalk->NewFid        = NewFid;
  TxWalk->NWName        = 1;
  UnicodeStrToP9StringS (Path, &TxWalk->WName[0], PathSize);

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

  CopyMem (NewQid, &RxWalk->WQid[0], QID_SIZE);

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

EFI_STATUS
P9Walk (
  IN P9_VOLUME          *Volume,
  IN P9_IFILE           *IFile,
  OUT P9_IFILE          *NewIFile,
  IN CHAR16             *Path
  )
{
  EFI_STATUS  Status;
  CHAR16      ComponentName[P9_MAX_PATH];
  UINTN       PathLen;
  CHAR16      *Next;
  UINT32      Fid;
  UINT32      NewFid;
  Qid         NewQid;

  PathLen = StrLen (Path);
  if (PathLen == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (Path[0] == PATH_NAME_SEPARATOR) {
    // Absolute path.
    Fid = Volume->Root->Fid;
    Path++;
    PathLen--;
  } else {
    // Relative path.
    Fid = IFile->Fid;
  }

  // Parent of the root directory does not exist.
  if (Fid == Volume->Root->Fid && StrnCmp (Path, L"..", 2) == 0) {
    Status = EFI_NOT_FOUND;
    goto Exit;
  }

  // Start at current location.
  Next = Path;
  for (;;) {
    // Get the next component name.
    Path = Next;
    Next = P9GetNextNameComponent (Path, ComponentName);
    // If end of the file name, we're done
    if (ComponentName[0] == L'\0') {
      break;
    }
    // If "dot", then current.
    if (StrCmp (ComponentName, L".") == 0) {
      continue;
    }

    NewFid = GetFid ();
    SetMem (&NewQid, QID_SIZE, 0);
    Status = DoP9Walk (Volume->Tcp4, Volume->Tag, Fid, NewFid, ComponentName, &NewQid);
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
    Fid = NewFid;
  }

  NewIFile->Fid = NewFid;
  CopyMem (&NewIFile->Qid, &NewQid, QID_SIZE);

Exit:
  return Status;
}
