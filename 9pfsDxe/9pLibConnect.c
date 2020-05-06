/** @file
  9P library.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

VOID
EFIAPI
P9ConnectionCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_CONNECT_PRIVATE_DATA *Connect;

  Connect = (P9_CONNECT_PRIVATE_DATA *)Context;

  Print (L"%a: %r\r\n", __func__, Connect->ConnectionToken.CompletionToken.Status);

  Connect->IsConnectDone = TRUE;
}

EFI_STATUS
ConnectP9 (
  IN P9_VOLUME              *Volume
)
{
  EFI_STATUS                Status;
  P9_CONNECT_PRIVATE_DATA   *Connect;

  Connect = AllocateZeroPool (sizeof (P9_CONNECT_PRIVATE_DATA));
  if (Connect == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      P9ConnectionCallback,
      Connect,
      &Connect->ConnectionToken.CompletionToken.Event);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Connect->IsConnectDone = FALSE;
  Status = Volume->Tcp4->Connect (Volume->Tcp4, &Connect->ConnectionToken);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  while (!Connect->IsConnectDone) {
    Volume->Tcp4->Poll (Volume->Tcp4);
  }

  if (EFI_ERROR (Connect->ConnectionToken.CompletionToken.Status)) {
    Status = Connect->ConnectionToken.CompletionToken.Status;
  }

  FreePool (Connect);

  return Status;
}