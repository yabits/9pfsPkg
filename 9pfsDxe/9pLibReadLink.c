/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

EFI_STATUS
P9LReadLink (
  IN P9_VOLUME          *Volume,
  IN OUT P9_IFILE       *IFile
  )
{
  EFI_STATUS                    Status;
  P9TReadLink                   *TxReadLink;
  P9RReadLink                   *RxReadLink;
  UINTN                         RxReadLinkSize;
  UINTN                         PathLength;

  TxReadLink = AllocateZeroPool (sizeof (P9TReadLink));
  if (TxReadLink == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxReadLink->Header.Size = sizeof (P9TReadLink);
  TxReadLink->Header.Id   = Treadlink;
  TxReadLink->Header.Tag  = Volume->Tag;
  TxReadLink->Fid         = IFile->Fid;

  RxReadLinkSize = sizeof (P9RReadLink) + P9_MAX_PATH;
  RxReadLink = AllocateZeroPool (RxReadLinkSize);
  if (RxReadLink == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = DoP9 (
    Volume,
    TxReadLink,
    sizeof (P9TReadLink),
    RxReadLink,
    RxReadLinkSize
    );
  if (EFI_ERROR (Status)) {
    goto Exit;
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
  if (TxReadLink != NULL) {
    FreePool (TxReadLink);
  }

  if (RxReadLink != NULL) {
    FreePool (RxReadLink);
  }

  return Status;
}
