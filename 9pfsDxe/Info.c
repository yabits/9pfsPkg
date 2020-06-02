/** @file
  Routines dealing with setting/getting file/volume info

Copyright (c) 2005 - 2015, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
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
  UINTN             Size;

  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));

  IFile = IFILE_FROM_FHAND (FHand);
  Volume = IFile->Volume;

  Size = SIZE_OF_EFI_FILE_INFO + StrSize (IFile->FileName);

  if (*BufferSize < Size) {
    *BufferSize = Size;
    DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));
    return EFI_BUFFER_TOO_SMALL;
  }

  Status = P9GetAttr (Volume, IFile);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "%a:%d: %r\n", __func__, __LINE__, Status));
    goto Exit;
  }

  CopyMem (Buffer, IFile->FileInfo, Size);
  *BufferSize = Size;

  Status = EFI_SUCCESS;

Exit:
  DEBUG ((DEBUG_INFO, "%a:%d: %r\n", __func__, __LINE__, Status));
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

  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));
  return EFI_UNSUPPORTED;
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
  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));
  return EFI_SUCCESS;
}