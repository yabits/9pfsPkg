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