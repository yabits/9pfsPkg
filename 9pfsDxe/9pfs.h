/** @file
  Main header file for EFI 9P file system driver.

Copyright (c) 2005 - 2013, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _9PFS_H_
#define _9PFS_H_

#include <Uefi.h>

#include <Protocol/ServiceBinding.h>
#include <Protocol/Tcp4.h>

#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

//
// The 9P file system signature
//
#define P9_VOLUME_SIGNATURE         SIGNATURE_32 ('9', 'f', 's', 'v')

#define VOLUME_FROM_VOL_INTERFACE(a) CR (a, P9_VOLUME, VolumeInterface, P9_VOLUME_SIGNATURE);

typedef struct _P9_VOLUME P9_VOLUME;

struct _P9_VOLUME {
  UINTN                           Signature;
  EFI_HANDLE                      Handle;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL VolumeInterface;
  EFI_TCP4_PROTOCOL               *Tcp4;
  UINT32                          MSize;
};

//
// Function Prototypes
//
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
  );

/**

  Deletes the file & Closes the file handle.

  @param  FHand                    - Handle to the file to delete.

  @retval EFI_SUCCESS              - Delete the file successfully.
  @retval EFI_WARN_DELETE_FAILURE  - Fail to delete the file.

**/
EFI_STATUS
EFIAPI
P9Delete (
  IN EFI_FILE_PROTOCOL  *FHand
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**

  Flushes & Closes the file handle.

  @param  FHand                 - Handle to the file to delete.

  @retval EFI_SUCCESS           - Closed the file successfully.

**/
EFI_STATUS
EFIAPI
P9Close (
  IN EFI_FILE_PROTOCOL  *FHand
  );

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
  );

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL    g9pfsDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL    g9pfsComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL   g9pfsComponentName2;
extern EFI_FILE_PROTOCOL              P9FileInterface;

#endif