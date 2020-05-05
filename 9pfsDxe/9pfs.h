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

#endif