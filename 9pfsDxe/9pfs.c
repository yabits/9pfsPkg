/** @file
  9P File System driver routines that support EFI driver model.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pfs.h"

EFI_STATUS
EFIAPI
P9ServiceBindingCreateChild (
  IN EFI_SERVICE_BINDING_PROTOCOL   *This,
  IN OUT EFI_HANDLE                 *ChildHandle
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
P9ServiceBindingDestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL   *This,
  IN EFI_HANDLE                     ChildHandle
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
P9CreateService (
  IN EFI_HANDLE   Controller,
  OUT P9_SERVICE  **ServiceData
  )
{
  P9_SERVICE  *P9Service;

  ASSERT (ServiceData != NULL);
  *ServiceData = NULL;

  P9Service = AllocateZeroPool (sizeof (P9_SERVICE));
  if (P9Service == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  P9Service->Signature = P9_SERVICE_SIGNATURE;
  P9Service->ServiceBinding.CreateChild = P9ServiceBindingCreateChild;
  P9Service->ServiceBinding.DestroyChild = P9ServiceBindingDestroyChild;
  P9Service->ControllerHandle = Controller;
  P9Service->ChildrenNumber = 0;
  InitializeListHead (&P9Service->ChildrenList);

  *ServiceData = P9Service;

  return EFI_SUCCESS;
}

VOID
P9CleanService (
  IN P9_SERVICE *P9Service
  )
{
  if (P9Service == NULL) {
    return;
  }

  if (P9Service->Tcp4ChildHandle != NULL) {
    gBS->CloseProtocol (
      P9Service->Tcp4ChildHandle,
      &gEfiTcp4ProtocolGuid,
      P9Service->Ip4DriverBindingHandle,
      P9Service->ControllerHandle
      );

    NetLibDestroyServiceChild (
      P9Service->ControllerHandle,
      P9Service->Ip4DriverBindingHandle,
      &gEfiTcp4ServiceBindingProtocolGuid,
      P9Service->Tcp4ChildHandle
      );

    P9Service->Tcp4ChildHandle = NULL;
  }
}

VOID
P9CleanProtocol (
  IN P9_VOLUME  *Volume
  )
{
  if (Volume->Tcp4ChildHandle != NULL) {
    gBS->CloseProtocol (
      Volume->Tcp4ChildHandle,
      &gEfiTcp4ProtocolGuid,
      Volume->Service->Ip4DriverBindingHandle,
      Volume->Service->ControllerHandle
      );

    gBS->CloseProtocol (
      Volume->Tcp4ChildHandle,
      &gEfiTcp4ProtocolGuid,
      Volume->Service->Ip4DriverBindingHandle,
      Volume->Handle
      );

    NetLibDestroyServiceChild (
      Volume->Service->ControllerHandle,
      Volume->Service->Ip4DriverBindingHandle,
      &gEfiTcp4ServiceBindingProtocolGuid,
      Volume->Tcp4ChildHandle
      );
  }

  if (Volume->Service->Tcp4ChildHandle != NULL) {
    gBS->CloseProtocol (
      Volume->Service->Tcp4ChildHandle,
      &gEfiTcp4ProtocolGuid,
      Volume->Service->Ip4DriverBindingHandle,
      Volume->Handle
      );
  }
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
  P9_VOLUME                       *Volume;
  P9_SERVICE                      *P9Service;
  VOID                            *Interface;

  Status = gBS->OpenProtocol (
    ControllerHandle,
    &g9pServiceBindingProtocolGuid,
    (VOID **)&ServiceBinding,
    This->DriverBindingHandle,
    ControllerHandle,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

  if (!EFI_ERROR (Status)) {
    P9Service = P9_SERVICE_FROM_PROTOCOL (ServiceBinding);
  } else {
    Status = P9CreateService (ControllerHandle, &P9Service);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    ASSERT (P9Service != NULL);

    Status = gBS->InstallMultipleProtocolInterfaces (
      &ControllerHandle,
      &g9pServiceBindingProtocolGuid,
      &P9Service->ServiceBinding,
      NULL
      );
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
  }

  P9Service->Ip4DriverBindingHandle = This->DriverBindingHandle;

  if (P9Service->Tcp4ChildHandle == NULL) {
    //
    // Create a TCP4 child instance
    //
    Status = NetLibCreateServiceChild (
                    ControllerHandle,
                    This->DriverBindingHandle,
                    &gEfiTcp4ServiceBindingProtocolGuid,
                    &P9Service->Tcp4ChildHandle
                    );
    if (EFI_ERROR (Status)) {
      goto Exit;
    }

    Status = gBS->OpenProtocol (
                    P9Service->Tcp4ChildHandle,
                    &gEfiTcp4ProtocolGuid,
                    &Interface,
                    This->DriverBindingHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_BY_DRIVER
                    );
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
  } else {
    return EFI_ALREADY_STARTED;
  }

  Volume = AllocateZeroPool (sizeof (P9_VOLUME));
  if (Volume == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Volume->Signature                  = P9_VOLUME_SIGNATURE;
  Volume->Handle                     = ControllerHandle;
  Volume->Service                    = P9Service;
  Volume->VolumeInterface.Revision   = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
  Volume->VolumeInterface.OpenVolume = P9OpenVolume;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ControllerHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  &Volume->VolumeInterface,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

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
  EFI_HANDLE                      NicHandle;
  EFI_SERVICE_BINDING_PROTOCOL    *ServiceBinding;
  P9_SERVICE                      *P9Service;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  P9_VOLUME                       *Volume;

  NicHandle = NetLibGetNicHandle (ControllerHandle, &gEfiTcp4ProtocolGuid);
  if (NicHandle != NULL) {
    Status = gBS->OpenProtocol (
      NicHandle,
      &g9pServiceBindingProtocolGuid,
      (VOID **)&ServiceBinding,
      This->DriverBindingHandle,
      NicHandle,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL
      );
    if (!EFI_ERROR (Status)) {
      P9Service = P9_SERVICE_FROM_PROTOCOL (ServiceBinding);

      P9CleanService (P9Service);
      if (P9Service->Tcp4ChildHandle == NULL) {
        gBS->UninstallProtocolInterface (
          NicHandle,
          &g9pServiceBindingProtocolGuid,
          ServiceBinding
          );
        FreePool (P9Service);
      }
      Status = EFI_SUCCESS;
    }
  }

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
    if (Volume->Handle != NULL) {
      Status = gBS->UninstallProtocolInterface (
        Volume->Handle,
        &gEfiSimpleFileSystemProtocolGuid,
        &Volume->VolumeInterface
        );
    }
  }

  return Status;
}
