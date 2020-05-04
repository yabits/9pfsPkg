/** @file
  Main header file for EFI 9P file system driver.

Copyright (c) 2005 - 2013, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _9PFS_H_
#define _9PFS_H_

#include <Uefi.h>

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
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL    g9pfsDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL    g9pfsComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL   g9pfsComponentName2;

#endif