/** @file
  Routines dealing with file open.

Copyright (c) 2005 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pfs.h"

/**

  Implements OpenEx() of Simple File System Protocol.

  @param  FHand                 - File handle of the file serves as a starting reference point.
  @param  NewHandle             - Handle of the file that is newly opened.
  @param  FileName              - File name relative to FHand.
  @param  OpenMode              - Open mode.
  @param  Attributes            - Attributes to set if the file is created.
  @param  Token                 - A pointer to the token associated with the transaction.:

  @retval EFI_INVALID_PARAMETER - The FileName is NULL or the file string is empty.
                          The OpenMode is not supported.
                          The Attributes is not the valid attributes.
  @retval EFI_OUT_OF_RESOURCES  - Can not allocate the memory for file string.
  @retval EFI_SUCCESS           - Open the file successfully.
  @return Others                - The status of open file.

**/
EFI_STATUS
EFIAPI
P9OpenEx (
  IN  EFI_FILE_PROTOCOL       *FHand,
  OUT EFI_FILE_PROTOCOL       **NewHandle,
  IN  CHAR16                  *FileName,
  IN  UINT64                  OpenMode,
  IN  UINT64                  Attributes,
  IN OUT EFI_FILE_IO_TOKEN    *Token
  )
{
  return EFI_SUCCESS;
}

/**

  Implements Open() of Simple File System Protocol.


  @param   FHand                 - File handle of the file serves as a starting reference point.
  @param   NewHandle             - Handle of the file that is newly opened.
  @param   FileName              - File name relative to FHand.
  @param   OpenMode              - Open mode.
  @param   Attributes            - Attributes to set if the file is created.

  @retval EFI_INVALID_PARAMETER - The FileName is NULL or the file string is empty.
                          The OpenMode is not supported.
                          The Attributes is not the valid attributes.
  @retval EFI_OUT_OF_RESOURCES  - Can not allocate the memory for file string.
  @retval EFI_SUCCESS           - Open the file successfully.
  @return Others                - The status of open file.

**/
EFI_STATUS
EFIAPI
P9Open (
  IN  EFI_FILE_PROTOCOL   *FHand,
  OUT EFI_FILE_PROTOCOL   **NewHandle,
  IN  CHAR16              *FileName,
  IN  UINT64              OpenMode,
  IN  UINT64              Attributes
  )
{
  return P9OpenEx (FHand, NewHandle, FileName, OpenMode, Attributes, NULL);
}