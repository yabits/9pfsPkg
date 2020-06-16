/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

EFI_STATUS
P9Statfs (
  IN P9_VOLUME          *Volume
  )
{
  EFI_STATUS                    Status;
  P9TStatfs                     *TxStatfs;
  P9RStatfs                     *RxStatfs;
  UINTN                         Size;
  EFI_FILE_SYSTEM_INFO          *FileSystemInfo;

  TxStatfs = AllocateZeroPool (sizeof (P9TStatfs));
  if (TxStatfs == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxStatfs->Header.Size  = sizeof (P9TStatfs);
  TxStatfs->Header.Id    = Tstatfs;
  TxStatfs->Header.Tag   = Volume->Tag;
  TxStatfs->Fid          = Volume->Root->Fid;

  RxStatfs = AllocateZeroPool (sizeof (P9RStatfs));
  if (RxStatfs == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = DoP9 (
    Volume,
    TxStatfs,
    sizeof (P9TStatfs),
    RxStatfs,
    sizeof (P9RStatfs)
    );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  if (RxStatfs->Header.Id != Rstatfs) {
    Status = P9Error (RxStatfs, sizeof (P9RStatfs));
    goto Exit;
  }

  Size = SIZE_OF_EFI_FILE_SYSTEM_INFO + StrSize (P9_VOLUME_LABEL);
  if (Volume->FileSystemInfo == NULL) {
    Volume->FileSystemInfo = AllocateZeroPool (Size);
    if (Volume->FileSystemInfo == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }
  }

  FileSystemInfo = Volume->FileSystemInfo;

  FileSystemInfo->Size        = Size;
  FileSystemInfo->ReadOnly    = TRUE; // XXX: Currently read-only.
  FileSystemInfo->VolumeSize  = RxStatfs->BSize * RxStatfs->Blocks;
  FileSystemInfo->FreeSpace   = RxStatfs->BSize * RxStatfs->BFree;
  FileSystemInfo->BlockSize   = RxStatfs->BSize;
  StrCpyS (FileSystemInfo->VolumeLabel, StrSize (P9_VOLUME_LABEL), P9_VOLUME_LABEL);

  Status = EFI_SUCCESS;

Exit:
  if (TxStatfs != NULL) {
    FreePool (TxStatfs);
  }

  if (RxStatfs != NULL) {
    FreePool (RxStatfs);
  }

  return Status;
}
