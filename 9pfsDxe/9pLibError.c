/** @file
  9P library.

Copyright (c) 2005 - 2014, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pLib.h"

#define EPERM            1      /* Operation not permitted */
#define ENOENT           2      /* No such file or directory */
#define ESRCH            3      /* No such process */
#define EINTR            4      /* Interrupted system call */
#define EIO              5      /* I/O error */

struct {
  UINT32      ECode;
  EFI_STATUS  Status;
} ECodeTable[] = {
  { 0,      EFI_SUCCESS },
  { EPERM,  EFI_ACCESS_DENIED },
  { ENOENT, EFI_NOT_FOUND },
  { EIO,    EFI_DEVICE_ERROR },
};

/**

  Returns EFI_STATUS according to Rlerror ecode.

  @param  Data                     - Recieved Data.
  @param  DataSize                 - Size of Recieved Data.

  @retval EFI_SUCCESS              - No ecode.

**/
EFI_STATUS
P9Error (
  IN VOID               *Data,
  IN UINTN              DataSize
  )
{
  P9RLError   *RxError;
  UINTN       Index;

  if (DataSize < sizeof (P9RLError)) {
    return EFI_BUFFER_TOO_SMALL;
  }

  RxError = (P9RLError *)Data;

  if (RxError->Header.Id != Rlerror) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < ARRAY_SIZE (ECodeTable); Index++) {
    if (RxError->ECode == ECodeTable[Index].ECode) {
      return ECodeTable[Index].Status;
    }
  }

  return EFI_DEVICE_ERROR;
}