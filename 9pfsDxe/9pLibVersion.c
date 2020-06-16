/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

EFI_STATUS
P9Version (
  IN P9_VOLUME          *Volume,
  IN OUT UINT32         *MSize
  )
{
  EFI_STATUS                    Status;
  UINTN                         VersionSize;
  CHAR8                         *VersionString;
  UINTN                         TxVersionSize;
  UINTN                         RxVersionSize;
  P9TVersion                    *TxVersion;
  P9RVersion                    *RxVersion;

  VersionString = P9_VERSION;
  VersionSize = AsciiStrLen (VersionString);
  TxVersionSize = sizeof (P9TVersion) + sizeof (CHAR8) * VersionSize;
  TxVersion = AllocateZeroPool (TxVersionSize);
  if (TxVersion == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxVersion->Header.Size = TxVersionSize;
  TxVersion->Header.Id = Tversion;
  TxVersion->Header.Tag = P9_NOTAG;
  TxVersion->MSize = *MSize;
  AsciiStrToP9StringS (VersionString, &TxVersion->Version, VersionSize);

  RxVersionSize = TxVersionSize;
  RxVersion = AllocateZeroPool (RxVersionSize);
  if (RxVersion == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = DoP9 (
    Volume,
    TxVersion,
    TxVersionSize,
    RxVersion,
    RxVersionSize
    );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  if (RxVersion->Header.Id != Rversion) {
    Status = P9Error (RxVersion, sizeof (RxVersionSize));
    goto Exit;
  }

  if (AsciiStrnCmp (TxVersion->Version.String, RxVersion->Version.String, TxVersion->Version.Size) != 0) {
    Status = EFI_UNSUPPORTED;
    goto Exit;
  }

  *MSize = RxVersion->MSize;

Exit:
  if (TxVersion != NULL) {
    FreePool (TxVersion);
  }

  if (RxVersion != NULL) {
    FreePool (RxVersion);
  }

  return Status;
}