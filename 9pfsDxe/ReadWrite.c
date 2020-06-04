/** @file
  Functions that perform file read/write.

Copyright (c) 2005 - 2017, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent


**/

#include "9pfs.h"
#include "9pLib.h"

/* open-only flags */
#define	O_RDONLY	0x0000		/* open for reading only */
#define	O_WRONLY	0x0001		/* open for writing only */
#define	O_RDWR		0x0002		/* open for reading and writing */
#define	O_ACCMODE	0x0003		/* mask for above modes */

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
  P9_IFILE          *IFile;

  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));
  IFile = IFILE_FROM_FHAND (FHand);
  // TODO: Return EFI_UNSUPPORTED if it is not a file.
  *Position = IFile->Position;

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
  P9_IFILE          *IFile;

  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));
  IFile = IFILE_FROM_FHAND (FHand);
  // TODO: Return EFI_UNSUPPORTED if it is not a file.
  IFile->Position = Position;

  return EFI_SUCCESS;
}

/**

  Read the file.

  @param  FHand                 - The handle of the file.
  @param  BufferSize            - Size of Buffer.
  @param  Buffer                - Buffer containing read data.


  @retval EFI_SUCCESS           - Get the file info successfully.
  @retval EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  @retval EFI_VOLUME_CORRUPTED  - The file type of open file is error.
  @return other                 - An error occurred when operation the disk.

**/
EFI_STATUS
P9FileRead (
  IN     EFI_FILE_PROTOCOL  *FHand,
  IN OUT UINTN              *BufferSize,
     OUT VOID               *Buffer
  )
{
  EFI_STATUS        Status;
  P9_IFILE          *IFile;
  P9_VOLUME         *Volume;
  UINT32            MaxRxSize;
  UINT64            Position;
  UINTN             Index;
  UINTN             NReads;
  UINT32            Count;

  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));

  IFile = IFILE_FROM_FHAND (FHand);
  Volume = IFile->Volume;

  MaxRxSize = P9_MSIZE - 24;
  Position  = IFile->Position;
  NReads    = (*BufferSize + (MaxRxSize - 1)) / MaxRxSize;
  for (Index = 0; Index < NReads; Index++) {
    Count = (Index < NReads - 1) ? MaxRxSize : (*BufferSize % MaxRxSize);
    Status = P9LRead (Volume, IFile, (UINT32 *)&Count, Buffer);
    if (EFI_ERROR (Status)) {
      IFile->Position = Position;
      DEBUG ((DEBUG_ERROR, "%a:%d: %r\n", __func__, __LINE__, Status));
      goto Exit;
    }
    Buffer = (UINT8 *)Buffer + Count;
  }

  Status = EFI_SUCCESS;

Exit:
  DEBUG ((DEBUG_INFO, "%a:%d: %r\n", __func__, __LINE__, Status));
  return Status;
}

/**

  Read the directory.

  @param  FHand                 - The handle of the file.
  @param  BufferSize            - Size of Buffer.
  @param  Buffer                - Buffer containing read data.


  @retval EFI_SUCCESS           - Get the file info successfully.
  @retval EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  @retval EFI_VOLUME_CORRUPTED  - The file type of open file is error.
  @return other                 - An error occurred when operation the disk.

**/
EFI_STATUS
P9DirRead (
  IN     EFI_FILE_PROTOCOL  *FHand,
  IN OUT UINTN              *BufferSize,
     OUT VOID               *Buffer
  )
{
  EFI_STATUS        Status;
  P9_IFILE          *IFile;
  P9_VOLUME         *Volume;
  UINT32            Count;
  UINT32            NameLength;
  UINT32            DirEntSize;
  P9DirEnt          *DirEnt;
  UINTN             Size;
  UINTN             NameSize;
  P9_IFILE          *NewIFile;
  CHAR16            *Path;

  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));

  IFile = IFILE_FROM_FHAND (FHand);
  Volume = IFile->Volume;

  NameLength = 255;
  DirEntSize = sizeof (P9DirEnt) + sizeof (CHAR8) * NameLength;
  DirEnt = AllocateZeroPool (DirEntSize);
  if (DirEnt == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a:%d: %r\n", __func__, __LINE__, Status));
    goto Exit;
  }
  if (IFile->IsOpened != TRUE) {
    Status = P9LOpen (Volume, IFile);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a:%d: %r\n", __func__, __LINE__, Status));
      goto Exit;
    }
    IFile->IsOpened = TRUE;
  }
  Count = DirEntSize;
  Status = P9LReadDir (Volume, IFile, IFile->Position, &Count, DirEnt);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a:%d: %r\n", __func__, __LINE__, Status));
    goto Exit;
  }
  // Reached EOF
  if (Count == 0) {
    *BufferSize = 0;
    Status = EFI_SUCCESS;
    goto Exit;
  }

  NameSize = sizeof (CHAR16) * (DirEnt->Name.Size + 1);
  Size = SIZE_OF_EFI_FILE_INFO + NameSize;
  if (*BufferSize < Size) {
    *BufferSize = Size;
    Status = EFI_BUFFER_TOO_SMALL;
    DEBUG ((DEBUG_ERROR, "%a:%d: %r\n", __func__, __LINE__, Status));
    goto Exit;
  }
  Path = AllocateZeroPool (NameSize);
  if (Path == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    DEBUG ((DEBUG_ERROR, "%a:%d: %r\n", __func__, __LINE__, Status));
    goto Exit;
  }
  AsciiStrToUnicodeStrS (DirEnt->Name.String, Path, NameSize);
  NewIFile = AllocateZeroPool (sizeof (P9_IFILE));
  if (NewIFile == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  NewIFile->Signature  = P9_IFILE_SIGNATURE;
  NewIFile->Volume     = Volume;
  NewIFile->Flags      = O_RDONLY; // Currently supports read only.
  NewIFile->IsOpened   = FALSE;
  NewIFile->FileName   = Path;
  CopyMem (&NewIFile->Handle, &P9FileInterface, sizeof (EFI_FILE_PROTOCOL));

  Status = P9Walk (Volume, IFile, NewIFile, Path);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a:%d: %r\n", __func__, __LINE__, Status));
    goto Exit;
  }
  Status = P9GetAttr (Volume, NewIFile);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a:%d: %r\n", __func__, __LINE__, Status));
    goto Exit;
  }
  CopyMem (Buffer, NewIFile->FileInfo, Size);
  *BufferSize = Size;
  IFile->Position = DirEnt->Offset;

  Status = EFI_SUCCESS;

Exit:
  if (NewIFile != NULL) {
    FreePool (NewIFile);
  }
  if (DirEnt != NULL) {
    FreePool (DirEnt);
  }
  if (Path != NULL) {
    FreePool (Path);
  }

  DEBUG ((DEBUG_INFO, "%a:%d: %r\n", __func__, __LINE__, Status));
  return Status;
}

/**

  Read the symbolic link.

  @param  FHand                 - The handle of the file.
  @param  BufferSize            - Size of Buffer.
  @param  Buffer                - Buffer containing read data.


  @retval EFI_SUCCESS           - Get the file info successfully.
  @retval EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  @retval EFI_VOLUME_CORRUPTED  - The file type of open file is error.
  @return other                 - An error occurred when operation the disk.

**/
EFI_STATUS
P9SymLinkRead (
  IN     EFI_FILE_PROTOCOL  *FHand,
  IN OUT UINTN              *BufferSize,
     OUT VOID               *Buffer
  )
{
  EFI_STATUS        Status;
  P9_IFILE          *IFile;
  P9_VOLUME         *Volume;
  UINTN             PathSize;

  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));

  IFile = IFILE_FROM_FHAND (FHand);
  Volume = IFile->Volume;

  Status = P9LReadLink (Volume, IFile);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a:%d: %r\n", __func__, __LINE__, Status));
    goto Exit;
  }
  DEBUG ((DEBUG_INFO, "%a:%d: TargetPath: %s\n", __func__, __LINE__, IFile->SymLinkTarget));

  PathSize = StrSize (IFile->SymLinkTarget);
  if (*BufferSize < PathSize) {
    *BufferSize = PathSize;
    Status = EFI_BUFFER_TOO_SMALL;
    goto Exit;
  }
  StrCpyS (Buffer, *BufferSize / sizeof (CHAR16), IFile->SymLinkTarget);

  Status = EFI_SUCCESS;

Exit:
  DEBUG ((DEBUG_INFO, "%a:%d: %r\n", __func__, __LINE__, Status));
  return Status;
}

/**

  Read the file.

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
  P9_IFILE          *IFile;

  IFile = IFILE_FROM_FHAND (FHand);

  if (IFile->Qid.Type & QTDir) {
    return P9DirRead (FHand, BufferSize, Buffer);
  } else if (IFile->Qid.Type & QTSymLink) {
    return P9SymLinkRead (FHand, BufferSize, Buffer);
  } else {
    return P9FileRead (FHand, BufferSize, Buffer);
  }
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
  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));
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
  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));
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
  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));
  return EFI_SUCCESS;
}
