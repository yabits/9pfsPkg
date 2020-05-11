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
  P9_IFILE                  *IFile;

  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));

  Volume  = VOLUME_FROM_VOL_INTERFACE (This);
  IFile   = NULL;

  Status = ConfigureP9 (Volume, L"10.0.2.2:564", L"255.255.255.0", L"10.0.2.100:564");
  if (Status == EFI_ALREADY_STARTED) {
    DEBUG ((DEBUG_INFO, "9P volume is already configured.\n"));
    *File = &Volume->Root->Handle;
    Status = EFI_SUCCESS;
    goto Exit;
  } else if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to configure 9P volume: %r\n", Status));
    goto Exit;
  }

  Status = ConnectP9 (Volume);
  if (Status == EFI_ALREADY_STARTED) {
    DEBUG ((DEBUG_INFO, "9P volume is already connected.\n"));
    *File = &Volume->Root->Handle;
    Status = EFI_SUCCESS;
    goto Exit;
  } else if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to connect 9P volume: %r\n", Status));
    goto Exit;
  }

  Volume->Tag   = 1;
  Volume->MSize = P9_MSIZE;
  Status = P9Version (Volume, &Volume->MSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a:%d\n", __func__, __LINE__));
    goto Exit;
  }

  IFile = AllocateZeroPool (sizeof (P9_IFILE));
  if (IFile == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a:%d\n", __func__, __LINE__));
    goto Exit;
  }

  IFile->Signature  = P9_IFILE_SIGNATURE;
  IFile->Volume     = Volume;
  IFile->Fid        = GetFid ();
  IFile->FileName   = L"";
  CopyMem (&IFile->Handle, &P9FileInterface, sizeof (EFI_FILE_PROTOCOL));

  Status = P9Attach (Volume, IFile->Fid, P9_NOFID, "root", "/tmp/9", IFile);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a:%d\n", __func__, __LINE__));
    goto Exit;
  }

  *File = &IFile->Handle;
  Volume->Root = IFile;

  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));

  return EFI_SUCCESS;

Exit:
  if (IFile != NULL) {
    FreePool (IFile);
  }

  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));

  return Status;
}