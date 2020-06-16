/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

EFI_STATUS
P9Clunk (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile
  )
{
  EFI_STATUS                    Status;
  P9TClunk                      *TxClunk;
  P9RClunk                      *RxClunk;

  TxClunk = AllocateZeroPool (sizeof (P9TClunk));
  if (TxClunk == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxClunk->Header.Size  = sizeof (P9TClunk);
  TxClunk->Header.Id    = Tclunk;
  TxClunk->Header.Tag   = Volume->Tag;
  TxClunk->Fid          = IFile->Fid;

  RxClunk = AllocateZeroPool (sizeof (P9RClunk));
  if (RxClunk == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = DoP9 (
    Volume,
    TxClunk,
    sizeof (P9TClunk),
    RxClunk,
    sizeof (P9RClunk)
    );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  if (RxClunk->Header.Id != Rclunk) {
    Status = P9Error (RxClunk, sizeof (P9RClunk));
    goto Exit;
  }

  Status = EFI_SUCCESS;

Exit:
  if (TxClunk != NULL) {
    FreePool (TxClunk);
  }

  if (RxClunk != NULL) {
    FreePool (RxClunk);
  }

  return Status;
}