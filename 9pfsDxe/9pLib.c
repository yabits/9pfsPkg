/** @file
  9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

UINT32 mFid = 1;

UINT32
GetFid (
  VOID
  )
{
  return mFid++;
}

EFI_STATUS
TransmitTcp4 (
  IN EFI_TCP4_PROTOCOL  *Tcp4,
  IN EFI_TCP4_IO_TOKEN  *TransmitToken,
  IN VOID               *Data,
  IN UINTN              DataSize
  )
{
  EFI_STATUS                    Status;
  UINTN                         Index;
  UINT32                        FragmentCount;
  UINTN                         TransmitDataSize;
  EFI_TCP4_TRANSMIT_DATA        *TransmitData;
  EFI_TCP4_FRAGMENT_DATA        *FragmentTable;

  FragmentCount = DataSize / FRAGMENT_SIZE + (DataSize % FRAGMENT_SIZE ? 1 : 0);
  TransmitDataSize = sizeof (EFI_TCP4_TRANSMIT_DATA) + sizeof (EFI_TCP4_FRAGMENT_DATA) * (FragmentCount - 1);
  TransmitData = AllocateZeroPool (TransmitDataSize);
  if (TransmitData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  FragmentTable = TransmitData->FragmentTable;
  for (Index = 0; Index < FragmentCount; Index++) {
    FragmentTable[Index].FragmentLength =
      (Index < FragmentCount - 1) ? FRAGMENT_SIZE : (DataSize % FRAGMENT_SIZE);
    FragmentTable[Index].FragmentBuffer = (UINT8 *)Data + FRAGMENT_SIZE * Index;
  }

  TransmitData->Push = FALSE;
  TransmitData->Urgent = FALSE;
  TransmitData->DataLength = DataSize;
  TransmitData->FragmentCount = FragmentCount;
  TransmitToken->Packet.TxData = TransmitData;

  Status = Tcp4->Transmit (Tcp4, TransmitToken);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ReceiveTcp4 (
  IN EFI_TCP4_PROTOCOL  *Tcp4,
  IN EFI_TCP4_IO_TOKEN  *ReceiveToken,
  OUT VOID              *Data,
  OUT UINTN             DataSize
  )
{
  EFI_STATUS                    Status;
  UINTN                         Index;
  UINT32                        FragmentCount;
  UINTN                         ReceiveDataSize;
  EFI_TCP4_RECEIVE_DATA         *ReceiveData;
  EFI_TCP4_FRAGMENT_DATA        *FragmentTable;

  FragmentCount = DataSize / FRAGMENT_SIZE + (DataSize % FRAGMENT_SIZE ? 1 : 0);
  ReceiveDataSize = sizeof (EFI_TCP4_RECEIVE_DATA) + sizeof (EFI_TCP4_FRAGMENT_DATA) * (FragmentCount - 1);
  ReceiveData = AllocateZeroPool (ReceiveDataSize);
  if (ReceiveData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  FragmentTable = ReceiveData->FragmentTable;
  for (Index = 0; Index < FragmentCount; Index++) {
    FragmentTable[Index].FragmentLength =
      (Index < FragmentCount - 1) ? FRAGMENT_SIZE : (DataSize % FRAGMENT_SIZE);
    FragmentTable[Index].FragmentBuffer = (UINT8 *)Data + FRAGMENT_SIZE * Index;
  }

  ReceiveData->UrgentFlag = FALSE;
  ReceiveData->DataLength = (UINT32)DataSize;
  ReceiveData->FragmentCount = FragmentCount;
  ReceiveToken->Packet.RxData = ReceiveData;

  Status = Tcp4->Receive (Tcp4, ReceiveToken);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
TxCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_VOLUME *Volume;

  Volume = (P9_VOLUME *)Context;
  Volume->IsTxDone = TRUE;
}

VOID
EFIAPI
RxCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  P9_VOLUME *Volume;

  Volume = (P9_VOLUME *)Context;
  Volume->IsRxDone = TRUE;
}

EFI_STATUS
DoP9 (
  IN P9_VOLUME          *Volume,
  IN VOID               *TxData,
  IN UINTN              TxDataSize,
  IN OUT VOID           *RxData,
  IN OUT UINTN          RxDataSize
  )
{
  EFI_STATUS                    Status;
  EFI_TCP4_PROTOCOL             *Tcp4;
  VOID                          *RxDataPtr;
  UINTN                         RxDataSizeCount;
  UINT32                        RxDataLength;
  BOOLEAN                       HasMoreRxData;

  if (Volume == NULL || TxData == NULL || RxData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Tcp4 = Volume->Tcp4;

  if (Volume->TxIoToken.CompletionToken.Event == NULL) {
    Status = gBS->CreateEvent (
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      TxCallback,
      Volume,
      &Volume->TxIoToken.CompletionToken.Event
      );
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
  }

  if (Volume->RxIoToken.CompletionToken.Event == NULL) {
    Status = gBS->CreateEvent (
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RxCallback,
      Volume,
      &Volume->RxIoToken.CompletionToken.Event
      );
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
  }

  Volume->IsTxDone = FALSE;
  Status = TransmitTcp4 (
    Tcp4,
    &Volume->TxIoToken,
    TxData,
    TxDataSize
    );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  while (Volume->IsTxDone != TRUE) {
    Tcp4->Poll (Tcp4);
  }

  RxDataPtr = RxData;
  RxDataSizeCount = RxDataSize;
  HasMoreRxData = TRUE;
  while (HasMoreRxData == TRUE) {
    Volume->IsRxDone = FALSE;
    Status = ReceiveTcp4 (
      Tcp4,
      &Volume->RxIoToken,
      RxDataPtr,
      Volume->MSize
      );
    if (EFI_ERROR (Status)) {
      goto Exit;
    }

    while (Volume->IsRxDone != TRUE) {
      Tcp4->Poll (Tcp4);
    }
    RxDataLength = Volume->RxIoToken.Packet.RxData->DataLength;
    RxDataPtr = (UINT8 *)RxDataPtr + RxDataLength;
    if (RxDataLength < RxDataSizeCount) {
      HasMoreRxData = TRUE;
      RxDataSizeCount -= RxDataLength;
    } else {
      HasMoreRxData = FALSE;
    }
  }

  Status = EFI_SUCCESS;

Exit:
  return Status;
}

EFI_STATUS
AsciiStrToP9StringS (
  IN CONST CHAR8        *Source,
  OUT P9String          *Destination,
  IN UINTN              DestMax
  )
{
  UINTN SourceLen;
  CHAR8 *DestStr;

  if (Destination == NULL || Source == NULL || DestMax == 0) {
    return EFI_INVALID_PARAMETER;
  }

  SourceLen = AsciiStrnLenS (Source, DestMax);
  if (!(DestMax >= SourceLen)) {
    return EFI_BUFFER_TOO_SMALL;
  }

  Destination->Size = 0;
  DestStr = Destination->String;
  while (*Source != '\0') {
    *(DestStr++) = (CHAR8)*(Source++);
    Destination->Size++;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
UnicodeStrToP9StringS (
  IN CONST CHAR16       *Source,
  OUT P9String          *Destination,
  IN UINTN              DestMax
  )
{
  UINTN SourceLen;
  CHAR8 *DestStr;

  if (Destination == NULL || Source == NULL || DestMax == 0) {
    return EFI_INVALID_PARAMETER;
  }

  SourceLen = StrLen (Source);
  if (!(DestMax >= SourceLen)) {
    return EFI_BUFFER_TOO_SMALL;
  }

  Destination->Size = 0;
  DestStr = Destination->String;
  while (*Source != L'\0') {
    *(DestStr++) = (CHAR8)(CHAR16)*(Source++);
    Destination->Size++;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
P9StringToUnicodeStrS (
  IN P9String           *Source,
  OUT CHAR16            *Destination,
  IN UINTN              DestMax
  )
{
  CHAR8   *SourceStr;
  UINT16  SourceSize;

  if (Destination == NULL || Source == NULL || DestMax == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (!(DestMax > Source->Size)) {
    return EFI_BUFFER_TOO_SMALL;
  }

  SourceStr = Source->String;
  SourceSize = Source->Size;
  for (; SourceSize > 0; *(Destination++) = (CHAR16)(CHAR8)*(SourceStr++), SourceSize--) {
  }
  *Destination = L'\0';

  return EFI_SUCCESS;
}