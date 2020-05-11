/** @file
  9P library.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

VOID
EFIAPI
TxVersionCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA *Version;

  Version = (P9_MESSAGE_PRIVATE_DATA *)Context;
  Version->IsTxDone = TRUE;
}

VOID
EFIAPI
RxVersionCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_MESSAGE_PRIVATE_DATA *Version;

  Version = (P9_MESSAGE_PRIVATE_DATA *)Context;
  Version->IsRxDone = TRUE;
}

EFI_STATUS
P9Version (
  IN P9_VOLUME          *Volume,
  IN OUT UINT32         *MSize
  )
{
  EFI_STATUS                    Status;
  P9_MESSAGE_PRIVATE_DATA       *Version;
  EFI_TCP4_PROTOCOL             *Tcp4;
  UINTN                         VersionSize;
  CHAR8                         *VersionString;
  UINTN                         TxVersionSize;
  UINTN                         RxVersionSize;
  P9TVersion                    *TxVersion;
  P9RVersion                    *RxVersion;

  Tcp4 = Volume->Tcp4;

  Version = AllocateZeroPool (sizeof (P9_MESSAGE_PRIVATE_DATA));
  if (Version == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      TxVersionCallback,
      Version,
      &Version->TxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RxVersionCallback,
      Version,
      &Version->RxIoToken.CompletionToken.Event
      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  VersionString = P9_VERSION;
  VersionSize = AsciiStrLen (VersionString);
  TxVersionSize = sizeof (P9TVersion) + sizeof (CHAR8) * VersionSize;
  TxVersion = AllocateZeroPool (TxVersionSize);
  if (TxVersion == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  TxVersion->Header.Size = TxVersionSize;
  TxVersion->Header.Id = Tversion;
  TxVersion->Header.Tag = P9_NOTAG;
  TxVersion->MSize = *MSize;
  TxVersion->Version.Size = VersionSize;
  CopyMem (&TxVersion->Version.String, VersionString, sizeof (CHAR8) * VersionSize);

  Version->IsTxDone = FALSE;
  Status = TransmitTcp4 (Tcp4, &Version->TxIoToken, TxVersion, TxVersionSize);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Version->IsTxDone) {
    Tcp4->Poll (Tcp4);
  }

  RxVersionSize = TxVersionSize;
  RxVersion = AllocateZeroPool (RxVersionSize);
  if (RxVersion == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  Version->IsRxDone = FALSE;
  Status = ReceiveTcp4 (Tcp4, &Version->RxIoToken, RxVersion, RxVersionSize);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (!Version->IsRxDone) {
    Tcp4->Poll (Tcp4);
  }

  if (RxVersion->Header.Id != Rversion) {
    Status = P9Error (RxVersion, sizeof (RxVersionSize));
    goto Exit;
  }

  if (AsciiStrnCmp (TxVersion->Version.String, RxVersion->Version.String, TxVersion->Version.Size) != 0) {
    Status = EFI_UNSUPPORTED;
    goto Exit;
  }

  *MSize = RxVersion->MSize;

Exit:
  if (Version != NULL) {
    FreePool (Version);
  }

  if (TxVersion != NULL) {
    FreePool (TxVersion);
  }

  if (RxVersion != NULL) {
    FreePool (RxVersion);
  }

  return Status;
}