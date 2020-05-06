/** @file
  OpenVolume() function of Simple File System Protocol.

Copyright (c) 2005 - 2013, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pfs.h"
#include "9pLib.h"

/**

  Implements Simple File System Protocol interface function OpenVolume().

  @param  This                  - Calling context.
  @param  File                  - the Root Directory of the volume.

  @retval EFI_OUT_OF_RESOURCES  - Can not allocate the memory.
  @retval EFI_VOLUME_CORRUPTED  - The P9 type is error.
  @retval EFI_SUCCESS           - Open the volume successfully.

**/
EFI_STATUS
EFIAPI
P9OpenVolume (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *This,
  OUT EFI_FILE_PROTOCOL                **File
  )
{
  EFI_STATUS                Status;
  P9_VOLUME                 *Volume;
  UINT8                     RootQid[13];

  Volume = VOLUME_FROM_VOL_INTERFACE (This);

  Status = ConfigureP9 (Volume, L"10.0.2.2:564", L"255.255.255.0", L"10.0.2.100:564");
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = ConnectP9 (Volume);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Volume->MSize = 8192;
  Status = P9Version (Volume, &Volume->MSize, "9P2000.L");
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = P9Attach (Volume, 1, "root", "/tmp/9", RootQid);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  *File = AllocateZeroPool (sizeof (EFI_FILE_PROTOCOL));
  if (*File == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  CopyMem (*File, &P9FileInterface, sizeof (EFI_FILE_PROTOCOL));

  Status = EFI_SUCCESS;

Exit:
  return Status;
}