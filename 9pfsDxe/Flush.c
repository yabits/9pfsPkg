/** @file
  Routines that check references and flush OFiles

Copyright (c) 2005 - 2013, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent


**/

#include "9pfs.h"
#include "9pLib.h"

/**

  Flushes all data associated with the file handle.

  @param  FHand                 - Handle to file to flush.
  @param  Token                 - A pointer to the token associated with the transaction.

  @retval EFI_SUCCESS           - Flushed the file successfully.
  @retval EFI_WRITE_PROTECTED   - The volume is read only.
  @retval EFI_ACCESS_DENIED     - The file is read only.
  @return Others                - Flushing of the file failed.

**/
EFI_STATUS
EFIAPI
P9FlushEx (
  IN EFI_FILE_PROTOCOL  *FHand,
  IN EFI_FILE_IO_TOKEN  *Token
  )
{
  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));
  return EFI_SUCCESS;
}

/**

  Flushes all data associated with the file handle.

  @param  FHand                 - Handle to file to flush.

  @retval EFI_SUCCESS           - Flushed the file successfully.
  @retval EFI_WRITE_PROTECTED   - The volume is read only.
  @retval EFI_ACCESS_DENIED     - The file is read only.
  @return Others                - Flushing of the file failed.

**/
EFI_STATUS
EFIAPI
P9Flush (
  IN EFI_FILE_PROTOCOL  *FHand
  )
{
  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));
  return P9FlushEx (FHand, NULL);
}

/**

  Flushes & Closes the file handle.

  @param  FHand                 - Handle to the file to delete.

  @retval EFI_SUCCESS           - Closed the file successfully.

**/
EFI_STATUS
EFIAPI
P9Close (
  IN EFI_FILE_PROTOCOL  *FHand
  )
{
  EFI_STATUS        Status;
  P9_IFILE          *IFile;
  P9_VOLUME         *Volume;

  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));

  IFile = IFILE_FROM_FHAND (FHand);
  Volume = IFile->Volume;

  // TODO: Flush before clunk

  if (IFile != Volume->Root) {
    Status = P9Clunk (Volume, IFile);
    FreePool (IFile);
  }

  return Status;
}