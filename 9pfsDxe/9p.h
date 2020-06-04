/** @file
  Main header file for 9P library.

Copyright (c) 2020, Akira Moroo. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _9P_H_
#define _9P_H_

#include <Uefi.h>

#define P9_VERSION  "9P2000.L"
#define P9_MSIZE    (UINT32)(0x10000)
#define P9_NOTAG    (UINT16)(~0)
#define P9_NOFID    (UINT32)(~0)
#define P9_MAX_PATH (UINT16)(4096)

#define QID_SIZE    (UINTN)(13)

#define P9_GETATTR_MODE         0x00000001ULL
#define P9_GETATTR_NLINK        0x00000002ULL
#define P9_GETATTR_UID          0x00000004ULL
#define P9_GETATTR_GID          0x00000008ULL
#define P9_GETATTR_RDEV         0x00000010ULL
#define P9_GETATTR_ATIME        0x00000020ULL
#define P9_GETATTR_MTIME        0x00000040ULL
#define P9_GETATTR_CTIME        0x00000080ULL
#define P9_GETATTR_INO          0x00000100ULL
#define P9_GETATTR_SIZE         0x00000200ULL
#define P9_GETATTR_BLOCKS       0x00000400ULL

#define P9_GETATTR_BTIME        0x00000800ULL
#define P9_GETATTR_GEN          0x00001000ULL
#define P9_GETATTR_DATA_VERSION 0x00002000ULL

#define P9_GETATTR_BASIC        0x000007ffULL /* Mask for fields up to BLOCKS */
#define P9_GETATTR_ALL          0x00003fffULL /* Mask for All fields above */

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

typedef struct _P9DirEnt {
  Qid       Qid;
  UINT64    Offset;
  UINT8     Type;
  P9String  Name;
} P9DirEnt;

typedef struct _P9Header {
  UINT32  Size;
  UINT8   Id;
  UINT16  Tag;
} P9Header;

typedef struct _P9RLError {
  P9Header        Header;
  UINT32          ECode;
} P9RLError;

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

typedef struct _P9TStatfs {
  P9Header        Header;
  UINT32          Fid;
} P9TStatfs;

typedef struct _P9RStatfs {
  P9Header        Header;
  UINT32          Type;
  UINT32          BSize;
  UINT64          Blocks;
  UINT64          BFree;
  UINT64          BAvail;
  UINT64          Files;
  UINT64          FFree;
  UINT64          Fsid;
  UINT16          NameLen;
} P9RStatfs;

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

typedef struct _P9TRead {
  P9Header        Header;
  UINT32          Fid;
  UINT64          Offset;
  UINT32          Count;
} P9TRead;

typedef struct _P9RRead {
  P9Header        Header;
  UINT32          Count;
  UINT8           Data[0];
} P9RRead;

typedef struct _P9TReadDir {
  P9Header        Header;
  UINT32          Fid;
  UINT64          Offset;
  UINT32          Count;
} P9TReadDir;

typedef struct _P9RReadDir {
  P9Header        Header;
  UINT32          Count;
  UINT8           Data[0];
} P9RReadDir;

typedef struct _P9TReadLink {
  P9Header        Header;
  UINT32          Fid;
} P9TReadLink;

typedef struct _P9RReadLink {
  P9Header        Header;
  P9String        Target;
} P9RReadLink;

typedef struct _P9TWalk {
  P9Header        Header;
  UINT32          Fid;
  UINT32          NewFid;
  UINT16          NWName;
  P9String        WName[0];
} P9TWalk;

typedef struct _P9RWalk {
  P9Header        Header;
  UINT16          NWQid;
  Qid             WQid[0];
} P9RWalk;

typedef struct _P9TClunk {
  P9Header        Header;
  UINT32          Fid;
} P9TClunk;

typedef struct _P9RClunk {
  P9Header        Header;
} P9RClunk;
#pragma pack()

enum {
  Tlerror   = 6,
  Rlerror,
  Tstatfs   = 8,
  Rstatfs,
  Tlopen    = 12,
  Rlopen,
  Treadlink = 22,
  Rreadlink,
  Tgetattr  = 24,
  Rgetattr,
  Treaddir  = 40,
  Rreaddir,
  Tversion  = 100,
  Rversion,
  Tattach   = 104,
  Rattach,
  Twalk     = 110,
  Rwalk,
  Tread     = 116,
  Rread,
  Tclunk    = 120,
  Rclunk,
};

enum {
  QTDir     = 0x80,
  QTAppend  = 0x40,
  QTExcl    = 0x20,
  QTMount   = 0x10,
  QTAuth    = 0x08,
  QTTmp     = 0x04,
  QTSymLink = 0x02,
  QTLink    = 0x01,
  QTFile    = 0x00,
};
#endif
