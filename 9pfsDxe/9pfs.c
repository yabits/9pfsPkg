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

  Test to see if this driver can add a file system to ControllerHandle.
  ControllerHandle must support both Disk IO and Block IO protocols.

  @param  This                  - Protocol instance pointer.
  @param  ControllerHandle      - Handle of device to test.
  @param  RemainingDevicePath   - Not used.

  @retval EFI_SUCCESS           - This driver supports this device.
  @retval EFI_ALREADY_STARTED   - This driver is already running on this device.
  @return other                 - This driver does not support this device.

**/
EFI_STATUS
EFIAPI
P9DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

/**

  Start this driver on ControllerHandle by opening a Block IO and Disk IO
  protocol, reading Device Path. Add a Simple File System protocol to
  ControllerHandle if the media contains a valid file system.

  @param  This                  - Protocol instance pointer.
  @param  ControllerHandle      - Handle of device to bind driver to.
  @param  RemainingDevicePath   - Not used.

  @retval EFI_SUCCESS           - This driver is added to DeviceHandle.
  @retval EFI_ALREADY_STARTED   - This driver is already running on DeviceHandle.
  @retval EFI_OUT_OF_RESOURCES  - Can not allocate the memory.
  @return other                 - This driver does not support this device.

**/
EFI_STATUS
EFIAPI
P9DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

/**

  Stop this driver on ControllerHandle.

  @param  This                  - Protocol instance pointer.
  @param  ControllerHandle      - Handle of device to stop driver on.
  @param  NumberOfChildren      - Not used.
  @param  ChildHandleBuffer     - Not used.

  @retval EFI_SUCCESS           - This driver is removed DeviceHandle.
  @return other                 - This driver was not removed from this device.

**/
EFI_STATUS
EFIAPI
P9DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );

//
// DriverBinding protocol instance
//
EFI_DRIVER_BINDING_PROTOCOL g9pfsDriverBinding = {
  P9DriverBindingSupported,
  P9DriverBindingStart,
  P9DriverBindingStop,
  0xa,
  NULL,
  NULL
};

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
  EFI_STATUS                 Status;

  //
  // Initialize the EFI Driver Library
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
              ImageHandle,
              SystemTable,
              &g9pfsDriverBinding,
              ImageHandle,
              &g9pfsComponentName,
              &g9pfsComponentName2
              );
  ASSERT_EFI_ERROR (Status);

  return Status;
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

/**

  Test to see if this driver can add a file system to ControllerHandle.
  ControllerHandle must support both Disk IO and Block IO protocols.

  @param  This                  - Protocol instance pointer.
  @param  ControllerHandle      - Handle of device to test.
  @param  RemainingDevicePath   - Not used.

  @retval EFI_SUCCESS           - This driver supports this device.
  @retval EFI_ALREADY_STARTED   - This driver is already running on this device.
  @return other                 - This driver does not support this device.

**/
EFI_STATUS
EFIAPI
P9DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  return EFI_SUCCESS;
}

/**

  Start this driver on ControllerHandle by opening a Block IO and Disk IO
  protocol, reading Device Path. Add a Simple File System protocol to
  ControllerHandle if the media contains a valid file system.

  @param  This                  - Protocol instance pointer.
  @param  ControllerHandle      - Handle of device to bind driver to.
  @param  RemainingDevicePath   - Not used.

  @retval EFI_SUCCESS           - This driver is added to DeviceHandle.
  @retval EFI_ALREADY_STARTED   - This driver is already running on DeviceHandle.
  @retval EFI_OUT_OF_RESOURCES  - Can not allocate the memory.
  @return other                 - This driver does not support this device.

**/
EFI_STATUS
EFIAPI
P9DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  return EFI_SUCCESS;
}

 /**

  Stop this driver on ControllerHandle.

  @param  This                  - Protocol instance pointer.
  @param  ControllerHandle      - Handle of device to stop driver on.
  @param  NumberOfChildren      - Not used.
  @param  ChildHandleBuffer     - Not used.

  @retval EFI_SUCCESS           - This driver is removed DeviceHandle.
  @return other                 - This driver was not removed from this device.

**/
EFI_STATUS
EFIAPI
P9DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  )
{
  return EFI_SUCCESS;
}