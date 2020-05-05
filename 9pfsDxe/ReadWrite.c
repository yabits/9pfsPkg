/** @file
  Functions that perform file read/write.

Copyright (c) 2005 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent


**/

#include "9pfs.h"

/**

  Get the file's position of the file.


  @param  FHand                 - The handle of file.
  @param  Position              - The file's position of the file.

  @retval EFI_SUCCESS           - Get the info successfully.
  @retval EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  @retval EFI_UNSUPPORTED       - The open file is not a file.

**/
EFI_STATUS
EFIAPI
P9GetPosition (
  IN  EFI_FILE_PROTOCOL *FHand,
  OUT UINT64            *Position
  )
{
  return EFI_SUCCESS;
}

/**

  Set the file's position of the file.

  @param  FHand                 - The handle of file.
  @param  Position              - The file's position of the file.

  @retval EFI_SUCCESS           - Set the info successfully.
  @retval EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  @retval EFI_UNSUPPORTED       - Set a directory with a not-zero position.

**/
EFI_STATUS
EFIAPI
P9SetPosition (
  IN EFI_FILE_PROTOCOL  *FHand,
  IN UINT64             Position
  )
{
  return EFI_SUCCESS;
}

/**

  Get the file info.

  @param  FHand                 - The handle of the file.
  @param  BufferSize            - Size of Buffer.
  @param  Buffer                - Buffer containing read data.


  @retval EFI_SUCCESS           - Get the file info successfully.
  @retval EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  @retval EFI_VOLUME_CORRUPTED  - The file type of open file is error.
  @return other                 - An error occurred when operation the disk.

**/
EFI_STATUS
EFIAPI
P9Read (
  IN     EFI_FILE_PROTOCOL  *FHand,
  IN OUT UINTN              *BufferSize,
     OUT VOID               *Buffer
  )
{
  return EFI_SUCCESS;
}

/**

  Get the file info.

  @param  FHand                 - The handle of the file.
  @param  Token                 - A pointer to the token associated with the transaction.

  @retval EFI_SUCCESS           - Get the file info successfully.
  @retval EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  @retval EFI_VOLUME_CORRUPTED  - The file type of open file is error.
  @return other                 - An error occurred when operation the disk.

**/
EFI_STATUS
EFIAPI
P9ReadEx (
  IN     EFI_FILE_PROTOCOL  *FHand,
  IN OUT EFI_FILE_IO_TOKEN  *Token
  )
{
  return EFI_SUCCESS;
}

/**

  Write the content of buffer into files.

  @param  FHand                 - The handle of the file.
  @param  BufferSize            - Size of Buffer.
  @param  Buffer                - Buffer containing write data.

  @retval EFI_SUCCESS           - Set the file info successfully.
  @retval EFI_WRITE_PROTECTED   - The disk is write protect.
  @retval EFI_ACCESS_DENIED     - The file is read-only.
  @retval EFI_DEVICE_ERROR      - The OFile is not valid.
  @retval EFI_UNSUPPORTED       - The open file is not a file.
                        - The writing file size is larger than 4GB.
  @return other                 - An error occurred when operation the disk.

**/
EFI_STATUS
EFIAPI
P9Write (
  IN     EFI_FILE_PROTOCOL  *FHand,
  IN OUT UINTN              *BufferSize,
  IN     VOID               *Buffer
  )
{
  return EFI_SUCCESS;
}

/**

  Get the file info.

  @param  FHand                 - The handle of the file.
  @param  Token                 - A pointer to the token associated with the transaction.

  @retval EFI_SUCCESS           - Get the file info successfully.
  @retval EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  @retval EFI_VOLUME_CORRUPTED  - The file type of open file is error.
  @return other                 - An error occurred when operation the disk.

**/
EFI_STATUS
EFIAPI
P9WriteEx (
  IN     EFI_FILE_PROTOCOL  *FHand,
  IN OUT EFI_FILE_IO_TOKEN  *Token
  )
{
  return EFI_SUCCESS;
}