/** @file
  9P File System driver routines that support EFI driver model.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pfs.h"

VOID
EFIAPI
CloseCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  DEBUG ((DEBUG_INFO, "%a:%d\n", __func__, __LINE__));
}

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
  EFI_STATUS  Status;
  EFI_HANDLE  *DeviceHandleBuffer;
  UINTN       DeviceHandleCount;
  UINTN       Index;
  VOID        *ComponentName;
  VOID        *ComponentName2;

  Status = gBS->LocateHandleBuffer (
    AllHandles,
    NULL,
    NULL,
    &DeviceHandleCount,
    &DeviceHandleBuffer
    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < DeviceHandleCount; Index++) {
    Status = EfiTestManagedDevice (DeviceHandleBuffer[Index], ImageHandle, &gEfiTcp4ProtocolGuid);
    if (!EFI_ERROR (Status)) {
      Status = gBS->DisconnectController (
        DeviceHandleBuffer[Index],
        ImageHandle,
        NULL
        );
      if (EFI_ERROR (Status)) {
        break;
      }
    }
  }

  if (Index == DeviceHandleCount) {
    //
    // Driver is stopped successfully.
    //
    Status = gBS->HandleProtocol (ImageHandle, &gEfiComponentNameProtocolGuid, &ComponentName);
    if (EFI_ERROR (Status)) {
      ComponentName = NULL;
    }

    Status = gBS->HandleProtocol (ImageHandle, &gEfiComponentName2ProtocolGuid, &ComponentName2);
    if (EFI_ERROR (Status)) {
      ComponentName2 = NULL;
    }

    if (ComponentName == NULL) {
      if (ComponentName2 == NULL) {
        Status = gBS->UninstallMultipleProtocolInterfaces (
          ImageHandle,
          &gEfiDriverBindingProtocolGuid,
          &g9pfsDriverBinding,
          NULL
          );
      } else {
        Status = gBS->UninstallMultipleProtocolInterfaces (
          ImageHandle,
          &gEfiDriverConfiguration2ProtocolGuid,
          &g9pfsDriverBinding,
          &gEfiComponentName2ProtocolGuid,
          ComponentName2,
          NULL
          );
      }
    } else {
      if (ComponentName2 == NULL) {
        Status = gBS->UninstallMultipleProtocolInterfaces (
          ImageHandle,
          &gEfiDriverBindingProtocolGuid,
          &g9pfsDriverBinding,
          &gEfiComponentNameProtocolGuid, ComponentName,
          NULL
          );
      } else {
        Status = gBS->UninstallMultipleProtocolInterfaces (
          ImageHandle,
          &gEfiDriverBindingProtocolGuid,
          &g9pfsDriverBinding,
          &gEfiComponentNameProtocolGuid,
          ComponentName,
          &gEfiComponentName2ProtocolGuid,
          ComponentName2,
          NULL
          );
      }
    }
  }

  if (DeviceHandleBuffer != NULL) {
    FreePool (DeviceHandleBuffer);
  }

  return Status;
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
  EFI_STATUS                    Status;

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiTcp4ServiceBindingProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  return Status;
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
  EFI_STATUS                      Status;
  EFI_SERVICE_BINDING_PROTOCOL    *ServiceBinding;
  EFI_HANDLE                      ChildHandle;
  P9_VOLUME                       *Volume;

  Volume = AllocateZeroPool (sizeof (P9_VOLUME));
  if (Volume == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Volume->Signature                  = P9_VOLUME_SIGNATURE;
  Volume->Handle                     = ControllerHandle;
  Volume->VolumeInterface.Revision   = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
  Volume->VolumeInterface.OpenVolume = P9OpenVolume;

  //
  // Open ServiceBinding
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiTcp4ServiceBindingProtocolGuid,
                  (VOID **) &ServiceBinding,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Creaate ChildHandle
  //
  Status = ServiceBinding->CreateChild (
                  ServiceBinding,
                  &ChildHandle
                  );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Open Tcp4
  //
  Status = gBS->OpenProtocol (
                  ChildHandle,
                  &gEfiTcp4ProtocolGuid,
                  (VOID **) &Volume->Tcp4,
                  This->DriverBindingHandle,
                  Volume->Handle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Volume->Handle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  &Volume->VolumeInterface,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  return EFI_SUCCESS;

Exit:
  if (Volume != NULL) {
    FreePool (Volume);
  }

  return Status;
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
  EFI_STATUS                      Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  P9_VOLUME                       *Volume;
  EFI_TCP4_PROTOCOL               *Tcp4;
  EFI_TCP4_CLOSE_TOKEN            *CloseToken;

  Tcp4 = NULL;
  //
  // Get our context back
  //
  Status = gBS->OpenProtocol (
    ControllerHandle,
    &gEfiSimpleFileSystemProtocolGuid,
    (VOID **)&FileSystem,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
  );

  if (!EFI_ERROR (Status)) {
    Volume = VOLUME_FROM_VOL_INTERFACE (FileSystem);
    Tcp4 = Volume->Tcp4;
    if (Volume->Handle != NULL) {
      Status = gBS->UninstallMultipleProtocolInterfaces (
        Volume->Handle,
        &gEfiSimpleFileSystemProtocolGuid,
        &Volume->VolumeInterface,
        NULL
        );
    }
  }

  if (!EFI_ERROR (Status)) {
    if (Tcp4 != NULL) {
      CloseToken = AllocateZeroPool (sizeof (EFI_TCP4_CLOSE_TOKEN));
      CloseToken->AbortOnClose = TRUE;
      Status = gBS->CreateEvent (
        EVT_NOTIFY_SIGNAL,
        TPL_CALLBACK,
        CloseCallback,
        CloseToken,
        &CloseToken->CompletionToken.Event
        );
      ASSERT_EFI_ERROR (Status);
      Status = Tcp4->Close (Tcp4, CloseToken);
      ASSERT_EFI_ERROR (Status);
      Status = gBS->CloseProtocol (
        ControllerHandle,
        &gEfiTcp4ProtocolGuid,
        This->DriverBindingHandle,
        ControllerHandle
        );
      ASSERT_EFI_ERROR (Status);
    }
  }

  return EFI_SUCCESS;
}