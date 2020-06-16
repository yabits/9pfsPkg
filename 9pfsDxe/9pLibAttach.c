/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

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
  UINTN                         UNameSize;
  UINTN                         ANameSize;
  UINTN                         TxAttachSize;
  UINTN                         RxAttachSize;
  P9TAttach                     *TxAttach;
  P9RAttach                     *RxAttach;
  P9String                      *UName;
  P9String                      *AName;

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

  AsciiStrToP9StringS (UNameStr, UName, UNameSize);
  AsciiStrToP9StringS (ANameStr, AName, ANameSize);

  RxAttachSize = sizeof (P9RAttach);
  RxAttach = AllocateZeroPool (RxAttachSize);
  if (RxAttach == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = DoP9 (
    Volume,
    TxAttach,
    TxAttachSize,
    RxAttach,
    RxAttachSize
    );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  if (RxAttach->Header.Id != Rattach) {
    Status = P9Error (RxAttach, sizeof (P9RAttach));
    goto Exit;
  }

  CopyMem (&IFile->Qid, &RxAttach->Qid, QID_SIZE);

Exit:
  if (TxAttach != NULL) {
    FreePool (TxAttach);
  }

  if (RxAttach != NULL) {
    FreePool (RxAttach);
  }

  return Status;
}