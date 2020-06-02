/** @file
  Global data in the 9P Filesystem driver.

Copyright (c) 2005 - 2013, Intel Corporation. All rights reserved.<BR>
Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "9pfs.h"

//
// Globals
//
//
// P9FsLock - Global lock for synchronizing all requests.
//
EFI_LOCK P9FsLock   = EFI_INITIALIZE_LOCK_VARIABLE (TPL_CALLBACK);

EFI_LOCK P9TaskLock = EFI_INITIALIZE_LOCK_VARIABLE (TPL_NOTIFY);

//
// Filesystem interface functions
//
EFI_FILE_PROTOCOL               P9FileInterface = {
  EFI_FILE_PROTOCOL_REVISION,
  P9Open,
  P9Close,
  P9Delete,
  P9Read,
  P9Write,
  P9GetPosition,
  P9SetPosition,
  P9GetInfo,
  P9SetInfo,
  P9Flush,
  P9OpenEx,
  P9ReadEx,
  P9WriteEx,
  P9FlushEx
};