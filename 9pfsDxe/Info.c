/** @file
  Routines dealing with setting/getting file/volume info

Copyright (c) 2005 - 2015, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent



**/

#include "9pfs.h"
#include "9pLib.h"

EFI_STATUS
P9GetFileInfo (
  IN     EFI_FILE_PROTOCOL   *FHand,
  IN OUT UINTN               *BufferSize,
     OUT VOID                *Buffer
  )
{
  EFI_STATUS        Status;
  P9_IFILE          *IFile;
  P9_VOLUME         *Volume;

  if (*BufferSize < sizeof (EFI_FILE_INFO)) {
    *BufferSize = sizeof (EFI_FILE_INFO) + sizeof (CHAR16) * 1;
    return EFI_BUFFER_TOO_SMALL;
  }

  IFile = IFILE_FROM_FHAND (FHand);
  Volume = IFile->Volume;

  Status = P9GetAttr (Volume, IFile);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Buffer = IFile->FileInfo;
  *BufferSize = sizeof (EFI_FILE_INFO) + sizeof (CHAR16) * 1;

  Status = EFI_SUCCESS;

Exit:
  return Status;
}

/**

  Get the some types info of the file into Buffer.

  @param  FHand                 - The handle of file.
  @param  Type                  - The type of the info.
  @param  BufferSize            - Size of Buffer.
  @param  Buffer                - Buffer containing volume info.

  @retval EFI_SUCCESS           - Get the info successfully.
  @retval EFI_DEVICE_ERROR      - Can not find the OFile for the file.

**/
EFI_STATUS
EFIAPI
P9GetInfo (
  IN     EFI_FILE_PROTOCOL   *FHand,
  IN     EFI_GUID            *Type,
  IN OUT UINTN               *BufferSize,
     OUT VOID                *Buffer
  )
{
  if (CompareGuid (Type, &gEfiFileInfoGuid)) {
    return P9GetFileInfo (FHand, BufferSize, Buffer);
  }

  return EFI_SUCCESS;
}

/**

  Set the some types info of the file into Buffer.

  @param  FHand                 - The handle of file.
  @param  Type                  - The type of the info.
  @param  BufferSize            - Size of Buffer
  @param  Buffer                - Buffer containing volume info.

  @retval EFI_SUCCESS           - Set the info successfully.
  @retval EFI_DEVICE_ERROR      - Can not find the OFile for the file.

**/
EFI_STATUS
EFIAPI
P9SetInfo (
  IN EFI_FILE_PROTOCOL  *FHand,
  IN EFI_GUID           *Type,
  IN UINTN              BufferSize,
  IN VOID               *Buffer
  )
{
  return EFI_SUCCESS;
}