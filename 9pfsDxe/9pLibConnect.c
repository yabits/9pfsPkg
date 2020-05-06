/** @file
  9P library.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pfs.h"

VOID
EFIAPI
P9ConnectionCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_VOLUME                 *Volume;
  EFI_TCP4_CONNECTION_TOKEN *ConnectionToken;
  EFI_STATUS                Status;

  Volume = (P9_VOLUME *)Context;
  ConnectionToken = &Volume->ConnectionToken;
  Status = ConnectionToken->CompletionToken.Status;
  Print (L"%a: %r\r\n", __func__, Status);

  Volume->IsConnectDone = TRUE;
}

EFI_STATUS
ConnectP9 (
  IN P9_VOLUME              *Volume,
  EFI_TCP4_CONNECTION_TOKEN *ConnectionToken
)
{
  EFI_STATUS                Status;

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      P9ConnectionCallback,
      Volume,
      &ConnectionToken->CompletionToken.Event);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Volume->Tcp4->Connect (Volume->Tcp4, ConnectionToken);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}