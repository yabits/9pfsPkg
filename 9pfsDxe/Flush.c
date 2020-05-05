/** @file
  Routines that check references and flush OFiles

Copyright (c) 2005 - 2013, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent


**/

#include "9pfs.h"

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
  return EFI_SUCCESS;
}