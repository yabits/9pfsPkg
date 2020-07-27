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

EFI_STATUS
DoP9Walk (
  IN P9_VOLUME          *Volume,
  IN UINT32             Fid,
  IN UINT32             NewFid,
  IN CHAR16             *Path,
  OUT Qid               *NewQid
  )
{
  EFI_STATUS                    Status;
  UINTN                         PathSize;
  UINT16                        NWName;
  P9TWalk                       *TxWalk;
  UINTN                         TxWalkSize;
  P9RWalk                       *RxWalk;
  UINTN                         RxWalkSize;

  PathSize = (Path == NULL) ? 0 : StrLen (Path);
  NWName = (PathSize == 0) ? 0 : 1;

  TxWalkSize = sizeof (P9TWalk) + sizeof (P9String) * NWName + sizeof (CHAR8) * PathSize;

  TxWalk = AllocateZeroPool (TxWalkSize);
  if (TxWalk == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxWalk->Header.Size   = TxWalkSize;
  TxWalk->Header.Id     = Twalk;
  TxWalk->Header.Tag    = Volume->Tag;
  TxWalk->Fid           = Fid;
  TxWalk->NewFid        = NewFid;
  TxWalk->NWName        = NWName;
  if (NWName != 0) {
    UnicodeStrToP9StringS (Path, &TxWalk->WName[0], PathSize);
  }

  RxWalkSize = sizeof (P9RWalk) + sizeof (Qid) * NWName;
  RxWalk = AllocateZeroPool (RxWalkSize);
  if (RxWalk == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = DoP9 (
    Volume,
    TxWalk,
    TxWalkSize,
    RxWalk,
    RxWalkSize
    );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  if (RxWalk->Header.Id != Rwalk) {
    Status = P9Error (RxWalk, RxWalkSize);
    goto Exit;
  }

  if (NWName != 0) {
    CopyMem (NewQid, &RxWalk->WQid[0], QID_SIZE);
  }

Exit:
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

  // Root directory.
  if (StrCmp (Path, L"\\") == 0) {
    NewFid = GetFid ();
    ZeroMem (&NewQid, QID_SIZE);
    Status = DoP9Walk (Volume, Volume->Root->Fid, NewFid, NULL, &NewQid);
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
    NewIFile->Fid = NewFid;
    CopyMem (&NewIFile->Qid, &NewQid, QID_SIZE);
    return EFI_SUCCESS;
  }

  // Current directory.
  if (StrCmp (Path, L".") == 0) {
    NewFid = GetFid ();
    ZeroMem (&NewQid, QID_SIZE);
    Status = DoP9Walk (Volume, IFile->Fid, NewFid, NULL, &NewQid);
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
    NewIFile->Fid = NewFid;
    CopyMem (&NewIFile->Qid, &NewQid, QID_SIZE);
    return EFI_SUCCESS;
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
    Status = DoP9Walk (Volume, Fid, NewFid, ComponentName, &NewQid);
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
    Fid = NewFid;
  }

  NewIFile->Fid = NewFid;
  CopyMem (&NewIFile->Qid, &NewQid, QID_SIZE);
  Status = EFI_SUCCESS;

Exit:
  return Status;
}
