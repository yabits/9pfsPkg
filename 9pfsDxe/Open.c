/** @file
  Routines dealing with file open.

Copyright (c) 2005 - 2018, Intel Corporation. All rights reserved.<BR>
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
  EFI_STATUS        Status;
  P9_IFILE          *IFile;
  P9_IFILE          *NewIFile;
  P9_VOLUME         *Volume;

  DEBUG ((DEBUG_INFO, "%a:%d FileName: %s\n", __func__, __LINE__, FileName));

  IFile = IFILE_FROM_FHAND (FHand);
  Volume = IFile->Volume;

  NewIFile = AllocateZeroPool (sizeof (P9_IFILE));
  if (NewIFile == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  NewIFile->Signature  = P9_IFILE_SIGNATURE;
  NewIFile->Volume     = Volume;
  NewIFile->Fid        = GetFid ();
  NewIFile->Flags      = O_RDONLY; // Currently supports read only.
  CopyMem (&NewIFile->Handle, &P9FileInterface, sizeof (EFI_FILE_PROTOCOL));
  if (FileName[0] == L'\\') {
    NewIFile->FileName   = AllocateZeroPool (StrLen (FileName));
    StrCpyS (NewIFile->FileName, StrLen (FileName), FileName + 1);
  } else {
    NewIFile->FileName   = AllocateZeroPool (StrLen (FileName) + 1);
    StrCpyS (NewIFile->FileName, StrLen (FileName) + 1, FileName);
  }

  Status = P9Walk (Volume, IFile, NewIFile, FileName);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = P9LOpen (Volume, NewIFile);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  *NewHandle = &NewIFile->Handle;

  return EFI_SUCCESS;

Exit:
  if (NewIFile != NULL) {
    FreePool (NewIFile);
  }

  return Status;
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
  DEBUG ((DEBUG_INFO, "%a:%d FileName: %s\n", __func__, __LINE__, FileName));
  return P9OpenEx (FHand, NewHandle, FileName, OpenMode, Attributes, NULL);
}