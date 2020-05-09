/** @file
  Main header file for 9P library.

Copyright (c) 2005 - 2013, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _9P_H_
#define _9P_H_

#include <Uefi.h>

#define P9_VERSION  "9P2000.L"
#define P9_NOTAG    (UINT16)(~0)
#define P9_NOFID    (UINT32)(~0)

#define QID_SIZE    (UINTN)(13)

#pragma pack(1)
typedef struct _Qid {
  UINT8   Type;
  UINT32  Version;
  UINT64  Path;
} Qid;

typedef struct _P9String {
  UINT16  Size;
  CHAR8   String[0];
} P9String;

typedef struct _P9Header {
  UINT32  Size;
  UINT8   Id;
  UINT16  Tag;
} P9Header;

typedef struct _P9TVersion {
  P9Header        Header;
  UINT32          MSize;
  P9String        Version;
} P9TVersion;

typedef struct _P9RVersion {
  P9Header        Header;
  UINT32          MSize;
  P9String        Version;
} P9RVersion;

typedef struct _P9TAttach {
  P9Header        Header;
  UINT32          Fid;
  UINT32          AFid;
  P9String        UName;
  P9String        AName;
} P9TAttach;

typedef struct _P9RAttach {
  P9Header        Header;
  Qid             Qid;
} P9RAttach;

typedef struct _P9TLOpen {
  P9Header        Header;
  UINT32          Fid;
  UINT32          Flags;
} P9TLOpen;

typedef struct _P9RLOpen {
  P9Header        Header;
  Qid             Qid;
  UINT32          IoUnit;
} P9RLOpen;

typedef struct _P9TGetAttr {
  P9Header        Header;
  UINT32          Fid;
  UINT64          RequestMask;
} P9TGetAttr;

typedef struct _P9RGetAttr {
  P9Header        Header;
  UINT64          Valid;
  Qid             Qid;
  UINT32          Mode;
  UINT32          Uid;
  UINT32          Gid;
  UINT64          NLink;
  UINT64          RDev;
  UINT64          Size;
  UINT64          BlkSize;
  UINT64          Blocks;
  UINT64          ATimeSec;
  UINT64          ATimeNSec;
  UINT64          MTimeSec;
  UINT64          MTimeNSec;
  UINT64          CTimeSec;
  UINT64          CTimeNSec;
  UINT64          BTimeSec;
  UINT64          BTimeNSec;
  UINT64          Gen;
  UINT64          DataVersion;
} P9RGetAttr;
#pragma pack()

enum {
  Tlopen    = 12,
  Rlopen,
  Tgetattr  = 24,
  Rgetattr,
  Tversion  = 100,
  Rversion,
  Tattach   = 104,
  Rattach,
  Tread     = 116,
  Rread
};
#endif