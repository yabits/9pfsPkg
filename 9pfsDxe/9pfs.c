/** @file
  9P File System driver routines that support EFI driver model.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pfs.h"

/**

  Register Driver Binding protocol for this driver.

  @param  ImageHandle           - Handle for the image of this driver.
  @param  SystemTable           - Pointer to the EFI System Table.

  @retval EFI_SUCCESS           - Driver loaded.
  @return other                 - Driver not loaded.

**/
EFI_STATUS
EFIAPI
P9EntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

/**

  Unload function for this image. Uninstall DriverBinding protocol.

  @param ImageHandle           - Handle for the image of this driver.

  @retval EFI_SUCCESS           - Driver unloaded successfully.
  @return other                 - Driver can not unloaded.

**/
EFI_STATUS
EFIAPI
P9Unload (
  IN EFI_HANDLE         ImageHandle
  );

/**

  Register Driver Binding protocol for this driver.

  @param  ImageHandle           - Handle for the image of this driver.
  @param  SystemTable           - Pointer to the EFI System Table.

  @retval EFI_SUCCESS           - Driver loaded.
  @return other                 - Driver not loaded.

**/
EFI_STATUS
EFIAPI
P9EntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  return EFI_SUCCESS;
}

/**

  Unload function for this image. Uninstall DriverBinding protocol.

  @param ImageHandle           - Handle for the image of this driver.

  @retval EFI_SUCCESS           - Driver unloaded successfully.
  @return other                 - Driver can not unloaded.

**/
EFI_STATUS
EFIAPI
P9Unload (
  IN EFI_HANDLE         ImageHandle
  )
{
  return EFI_SUCCESS;
}
