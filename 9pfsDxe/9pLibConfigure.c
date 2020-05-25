/** @file
  9P library.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

EFI_STATUS
StrToIpv4Addr (
  IN CHAR16             *String,
  OUT EFI_IPv4_ADDRESS  *Addr,
  OUT UINT16            *Port
  )
{
  UINTN       AddrIndex;
  UINT64      Number;
  CHAR16      *Ptr;

  if (String == NULL || Addr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Parse IPv4 address
  Ptr = String;
  for (AddrIndex = 0; AddrIndex < 4; AddrIndex++) {
    Number = 0;
    while (L'0' <= *Ptr && *Ptr <= L'9') {
      Number = Number * 10 + (*Ptr - L'0');
      Ptr++;
    }
    if (!(0 <= Number && Number <= 255)) {
      return EFI_INVALID_PARAMETER;
    }
    Addr->Addr[AddrIndex] = (UINT8)Number;
    if (AddrIndex < 3 && *Ptr == L'.') {
      Ptr++;
    }
  }

  // Parse port number
  if (Port != NULL && *Ptr == L':') {
    Ptr++;
    Number = 0;
    while (L'0' <= *Ptr && *Ptr <= L'9') {
      Number = Number * 10 + (*Ptr - L'0');
      Ptr++;
    }
    *Port = (UINT16)Number;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureP9 (
  IN OUT P9_VOLUME          *Volume,
  IN CHAR16                 *StationAddrStr,
  IN CHAR16                 *SubnetMaskStr,
  IN CHAR16                 *RemoteAddrStr
  )
{
  EFI_STATUS                    Status;
  EFI_IPv4_ADDRESS              StationAddr;
  UINT16                        StationPort;
  EFI_IPv4_ADDRESS              SubnetMask;
  EFI_IPv4_ADDRESS              RemoteAddr;
  UINT16                        RemotePort;
  EFI_TCP4_CONFIG_DATA          *Tcp4Config;
  EFI_TCP4_OPTION               *ControlOption;

  if (Volume == NULL || StationAddrStr == NULL || SubnetMaskStr == NULL || RemoteAddrStr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Volume->IsConfigured == TRUE) {
    return EFI_ALREADY_STARTED;
  }

  Tcp4Config = NULL;
  ControlOption = NULL;

  Status = StrToIpv4Addr(StationAddrStr, &StationAddr, &StationPort);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  Status = StrToIpv4Addr(SubnetMaskStr, &SubnetMask, NULL);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  Status = StrToIpv4Addr(RemoteAddrStr, &RemoteAddr, &RemotePort);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Tcp4Config = AllocateZeroPool (sizeof (EFI_TCP4_CONFIG_DATA));
  if (Tcp4Config == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  ControlOption = AllocateZeroPool (sizeof (EFI_TCP4_OPTION));
  if (ControlOption == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  ControlOption->SendBufferSize = P9_MSIZE;
  ControlOption->ReceiveBufferSize = P9_MSIZE;
  Tcp4Config->TypeOfService = 0;
  Tcp4Config->TimeToLive = 255;
  Tcp4Config->AccessPoint.UseDefaultAddress = FALSE;
  Tcp4Config->AccessPoint.StationPort = StationPort;
  Tcp4Config->AccessPoint.RemotePort = RemotePort;
  Tcp4Config->AccessPoint.ActiveFlag = TRUE;
  Tcp4Config->ControlOption = ControlOption;

  CopyMem (&Tcp4Config->AccessPoint.StationAddress, &StationAddr, sizeof (EFI_IPv4_ADDRESS));
  CopyMem (&Tcp4Config->AccessPoint.SubnetMask, &SubnetMask, sizeof (EFI_IPv4_ADDRESS));
  CopyMem (&Tcp4Config->AccessPoint.RemoteAddress, &RemoteAddr, sizeof (EFI_IPv4_ADDRESS));

  Status = Volume->Tcp4->Configure (Volume->Tcp4, Tcp4Config);
  if (EFI_ERROR (Status)) {
    goto Done;
  } else {
    Volume->IsConfigured = TRUE;
  }

Done:
  if (EFI_ERROR (Status) && Tcp4Config != NULL) {
    FreePool (Tcp4Config);
  }

  if (EFI_ERROR (Status) && ControlOption != NULL) {
    FreePool (ControlOption);
  }

  return Status;
}