/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

EFI_STATUS
P9LOpen (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile
  )
{
  EFI_STATUS                    Status;
  P9TLOpen                      *TxOpen;
  P9RLOpen                      *RxOpen;

  TxOpen = AllocateZeroPool (sizeof (P9TLOpen));
  if (TxOpen == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxOpen->Header.Size = sizeof (P9TLOpen);
  TxOpen->Header.Id   = Tlopen;
  TxOpen->Header.Tag  = Volume->Tag;
  TxOpen->Fid         = IFile->Fid;
  TxOpen->Flags       = IFile->Flags;

  RxOpen = AllocateZeroPool (sizeof (P9RLOpen));
  if (RxOpen == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = DoP9 (
    Volume,
    TxOpen,
    sizeof (P9TLOpen),
    RxOpen,
    sizeof (P9RLOpen)
    );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  if (RxOpen->Header.Id != Rlopen) {
    Status = P9Error (RxOpen, sizeof (P9RLOpen));
    goto Exit;
  }

  CopyMem (&IFile->Qid, &RxOpen->Qid, QID_SIZE);
  IFile->IoUnit = RxOpen->IoUnit;

Exit:
  if (TxOpen != NULL) {
    FreePool (TxOpen);
  }

  if (RxOpen != NULL) {
    FreePool (RxOpen);
  }

  return Status;
}