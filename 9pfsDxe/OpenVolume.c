/** @file
  OpenVolume() function of Simple File System Protocol.

Copyright (c) 2005 - 2013, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pfs.h"

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
  *File = AllocateZeroPool (sizeof (EFI_FILE_PROTOCOL));
  if (*File == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (*File, &P9FileInterface, sizeof (EFI_FILE_PROTOCOL));

  return EFI_SUCCESS;
}