/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

EFI_STATUS
P9LRead (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile,
  IN OUT UINT32         *Count,
  OUT VOID              *Data
  )
{
  EFI_STATUS                    Status;
  P9TRead                       *TxRead;
  P9RRead                       *RxRead;
  UINTN                         RxReadSize;

  TxRead = AllocateZeroPool (sizeof (P9TRead));
  if (TxRead == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxRead->Header.Size = sizeof (P9TRead);
  TxRead->Header.Id   = Tread;
  TxRead->Header.Tag  = Volume->Tag;
  TxRead->Fid         = IFile->Fid;
  TxRead->Offset      = IFile->Position;
  TxRead->Count       = *Count;

  RxReadSize = sizeof (P9RRead) + *Count;
  RxRead = AllocateZeroPool (RxReadSize);
  if (RxRead == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = DoP9 (
    Volume,
    TxRead,
    sizeof (P9TRead),
    RxRead,
    RxReadSize
    );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  if (RxRead->Header.Id != Rread) {
    Status = P9Error (RxRead, RxReadSize);
    goto Exit;
  }

  CopyMem (Data, (VOID *)RxRead->Data, RxRead->Count);
  *Count = RxRead->Count;
  IFile->Position += RxRead->Count;

  Status = EFI_SUCCESS;

Exit:
  if (TxRead != NULL) {
    FreePool (TxRead);
  }

  if (RxRead != NULL) {
    FreePool (RxRead);
  }

  return Status;
}