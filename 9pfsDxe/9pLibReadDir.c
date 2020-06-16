/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

EFI_STATUS
P9LReadDir (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile,
  IN UINT64             Offset,
  IN OUT UINT32         *Count,
  OUT VOID              *Data
  )
{
  EFI_STATUS                    Status;
  P9TReadDir                    *TxReadDir;
  P9RReadDir                    *RxReadDir;
  UINTN                         RxReadDirSize;

  TxReadDir = AllocateZeroPool (sizeof (P9TReadDir));
  if (TxReadDir == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxReadDir->Header.Size = sizeof (P9TReadDir);
  TxReadDir->Header.Id   = Treaddir;
  TxReadDir->Header.Tag  = Volume->Tag;
  TxReadDir->Fid         = IFile->Fid;
  TxReadDir->Offset      = Offset;
  TxReadDir->Count       = *Count;

  RxReadDirSize = sizeof (P9RReadDir) + *Count;
  RxReadDir = AllocateZeroPool (RxReadDirSize);
  if (RxReadDir == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = DoP9 (
    Volume,
    TxReadDir,
    sizeof (P9TReadDir),
    RxReadDir,
    RxReadDirSize
    );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  if (RxReadDir->Header.Id != Rreaddir) {
    Status = P9Error (RxReadDir, RxReadDirSize);
    goto Exit;
  }

  CopyMem (Data, (VOID *)RxReadDir->Data, RxReadDir->Count);
  *Count = RxReadDir->Count;

  Status = EFI_SUCCESS;

Exit:
  if (TxReadDir != NULL) {
    FreePool (TxReadDir);
  }

  if (RxReadDir != NULL) {
    FreePool (RxReadDir);
  }

  return Status;
}
