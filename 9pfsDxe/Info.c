/** @file
  Routines dealing with setting/getting file/volume info

Copyright (c) 2005 - 2015, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent



**/

#include "9pfs.h"
#include "9pLib.h"

#define	S_ISDIR(m)	((m & 0170000) == 0040000)	/* directory */
#define	S_ISCHR(m)	((m & 0170000) == 0020000)	/* char special */
#define	S_ISBLK(m)	((m & 0170000) == 0060000)	/* block special */
#define	S_ISREG(m)	((m & 0170000) == 0100000)	/* regular file */
#define	S_ISFIFO(m)	((m & 0170000) == 0010000)	/* fifo */
#define	S_ISLNK(m)	((m & 0170000) == 0120000)	/* symbolic link */
#define	S_ISSOCK(m)	((m & 0170000) == 0140000)	/* socket */

/**
  Converts Epoch seconds (elapsed since 1970 JANUARY 01, 00:00:00 UTC) to EFI_TIME
 **/
STATIC
VOID
EpochToEfiTime (
  IN  UINTN     EpochSeconds,
  OUT EFI_TIME  *Time
  )
{
  UINTN         a;
  UINTN         b;
  UINTN         c;
  UINTN         d;
  UINTN         g;
  UINTN         j;
  UINTN         m;
  UINTN         y;
  UINTN         da;
  UINTN         db;
  UINTN         dc;
  UINTN         dg;
  UINTN         hh;
  UINTN         mm;
  UINTN         ss;
  UINTN         J;

  J  = (EpochSeconds / 86400) + 2440588;
  j  = J + 32044;
  g  = j / 146097;
  dg = j % 146097;
  c  = (((dg / 36524) + 1) * 3) / 4;
  dc = dg - (c * 36524);
  b  = dc / 1461;
  db = dc % 1461;
  a  = (((db / 365) + 1) * 3) / 4;
  da = db - (a * 365);
  y  = (g * 400) + (c * 100) + (b * 4) + a;
  m  = (((da * 5) + 308) / 153) - 2;
  d  = da - (((m + 4) * 153) / 5) + 122;

  Time->Year  = y - 4800 + ((m + 2) / 12);
  Time->Month = ((m + 2) % 12) + 1;
  Time->Day   = d + 1;

  ss = EpochSeconds % 60;
  a  = (EpochSeconds - ss) / 60;
  mm = a % 60;
  b = (a - mm) / 60;
  hh = b % 24;

  Time->Hour        = hh;
  Time->Minute      = mm;
  Time->Second      = ss;
  Time->Nanosecond  = 0;

}

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
  struct P9RGetAttr *Attr;
  EFI_FILE_INFO     *FileInfo;

  if (*BufferSize < sizeof (EFI_FILE_INFO)) {
    *BufferSize = sizeof (EFI_FILE_INFO);
    return EFI_BUFFER_TOO_SMALL;
  }

  IFile = IFILE_FROM_FHAND (FHand);
  Volume = IFile->Volume;

  Attr = AllocateZeroPool (sizeof (struct P9RGetAttr));
  if (Attr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = P9GetAttr (Volume, 1, 1, 0x00003fffULL, Attr);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  FileInfo = Buffer;
  SetMem (FileInfo, sizeof (EFI_FILE_INFO), 0);

  EpochToEfiTime (Attr->CTimeSec, &FileInfo->CreateTime);
  EpochToEfiTime (Attr->MTimeSec, &FileInfo->ModificationTime);
  EpochToEfiTime (Attr->ATimeSec, &FileInfo->LastAccessTime);
  FileInfo->CreateTime.Nanosecond       = (UINT32)Attr->CTimeNSec;
  FileInfo->ModificationTime.Nanosecond = (UINT32)Attr->MTimeNSec;
  FileInfo->LastAccessTime.Nanosecond   = (UINT32)Attr->ATimeNSec;
  FileInfo->Size                        = sizeof (EFI_FILE_INFO);
  FileInfo->FileSize                    = Attr->Size;
  FileInfo->PhysicalSize                = Attr->BlkSize * Attr->Blocks;
  FileInfo->Attribute                   = S_ISDIR (Attr->Mode) ? EFI_FILE_DIRECTORY : 0;
  FileInfo->FileName[0]                 = L'\0';

  Status = EFI_SUCCESS;

Exit:
  if (Attr != NULL) {
    FreePool (Attr);
  }

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