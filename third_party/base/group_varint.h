// Copyright 2013 Yunrang Inc. All Rights Reserved.
// Author: yubogong@yunrang.com (Yubo Gong)

#ifndef BASE_GROUP_VARINT_H_
#define BASE_GROUP_VARINT_H_

#include "base/basictypes.h"

// GroupVarint compress/uncompress 4 int32 in group

namespace base {

#define   MAX_UINT8            0xff
#define   MAX_UINT16           0xffff
#define   MAX_UINT24           0xffffff
#define   MAX_UINT32           0xffffffff
#define   MAX_UINT40           0xffffffffff
#define   MAX_UINT48           0xffffffffffff
#define   MAX_UINT56           0xffffffffffffff
#define   MAX_UINT64           0xffffffffffffffff

inline uint32 GetGroupVarint32Length(const uint32 value) {
  uint32 length = 0;
  if (value <= MAX_UINT8) {
    length   = 1;
  } else if (value <= MAX_UINT16) {
    length = 2;
  } else if (value <= MAX_UINT24) {
    length = 3;
  } else {
    length = 4;
  }
  return length;
}

inline uint32 GetGroupVarint64Length(const uint64 value) {
  uint32 length = 0;
  if (value <= MAX_UINT8) {
    length = 1;
  } else if (value <= MAX_UINT16) {
    length = 2;
  } else if (value <= MAX_UINT24) {
    length = 3;
  } else if (value <= MAX_UINT32) {
    length = 4;
  } else if (value <= MAX_UINT40) {
    length = 5;
  } else if (value <= MAX_UINT48) {
    length = 6;
  } else if (value <= MAX_UINT56) {
    length = 7;
  } else {
    length = 8;
  }
  return length;
}

// value should point to 4 size int array
inline uint8* WriteGroupVarint32(const uint32* value, uint8* target) {
  uint8 length1 = 0;
  uint8 length2 = 0;
  uint8 length3 = 0;
  uint8 length4 = 0;
  uint8 length_total = 0;

  uint8* buffer = target + 1;

  // first number
  if (value[0] <= MAX_UINT8) {
    length1   = 1;
    buffer[0] = (uint8) value[0];
  } else if (value[0] <= MAX_UINT16) {
    length1 = 2;
    (reinterpret_cast<uint16 *>(buffer))[0] = static_cast<uint16>(value[0]);
  } else if (value[0] <= MAX_UINT24) {
    length1 = 3;
    (reinterpret_cast<uint32*>(buffer))[0] = value[0];
  } else {
    length1 = 4;
    (reinterpret_cast<uint32*>(buffer))[0] = value[0];
  }
  length_total = length1;

  // second number
  if (value[1] <= MAX_UINT8) {
    length2 = 1;
    buffer[length_total] = (uint8) value[1];
  } else if (value[1] <= MAX_UINT16) {
    length2 = 2;
    (reinterpret_cast<uint16*>(buffer + length_total))[0] =
     static_cast<uint16>(value[1]);
  } else if (value[1] <= MAX_UINT24) {
    length2 = 3;
    (reinterpret_cast<uint32*>(buffer + length_total))[0] = value[1];
  } else {
    length2 = 4;
    (reinterpret_cast<uint32*>(buffer + length_total))[0] = value[1];
  }
  length_total += length2;

  // third number
  if (value[2] <= MAX_UINT8) {
    length3 = 1;
    buffer[length_total] = (uint8) value[2];
  } else if (value[2] <= MAX_UINT16) {
    length3 = 2;
    (reinterpret_cast<uint16*>(buffer + length_total))[0] =
     static_cast<uint16>(value[2]);
  } else if (value[2] <= MAX_UINT24) {
    length3 = 3;
    (reinterpret_cast<uint32*>(buffer + length_total))[0] = value[2];
  } else {
    length3 = 4;
    (reinterpret_cast<uint32*>(buffer + length_total))[0] = value[2];
  }
  length_total += length3;

  // fourth number
  if (value[3] <= MAX_UINT8) {
    length4 = 1;
    buffer[length_total] = (uint8) value[3];
  } else if (value[3] <= MAX_UINT16) {
    length4 = 2;
    (reinterpret_cast<uint16*>(buffer + length_total))[0] =
     static_cast<uint16>(value[3]);
  } else if (value[3] <= MAX_UINT24) {
    length4 = 3;
    (reinterpret_cast<uint32*>(buffer + length_total))[0] = value[3];
  } else {
    length4 = 4;
    (reinterpret_cast<uint32*>(buffer + length_total))[0] = value[3];
  }
  length_total += length4;

  target[0] =
    ((length1 - 1) << 6) | ((length2 - 1) << 4) |
    ((length3 - 1) << 2) | (length4 - 1);

  return buffer + length_total;
}

// value should point to 2 size uint64 array
// target's length should be enough, max size is 17
inline uint8* WriteGroupVarint64(const uint64* value, uint8* target) {
  uint8 length1 = 0;
  uint8 length2 = 0;
  uint8 length_total = 0;

  uint8* buffer = target + 1;

  // first number
  if (value[0] <= MAX_UINT8) {
    length1 = 1;
    buffer[0] = (uint8) value[0];
  } else if (value[0] <= MAX_UINT16) {
    length1 = 2;
    (reinterpret_cast<uint16 *>(buffer))[0] = static_cast<uint16>(value[0]);
  } else if (value[0] <= MAX_UINT24) {
    length1 = 3;
    (reinterpret_cast<uint32*>(buffer))[0] = value[0];
  } else if (value[0] <= MAX_UINT32) {
    length1 = 4;
    (reinterpret_cast<uint32*>(buffer))[0] = value[0];
  } else if (value[0] <= MAX_UINT40) {
    length1 = 5;
    (reinterpret_cast<uint64*>(buffer))[0] = value[0];
  } else if (value[0] <= MAX_UINT48) {
    length1 = 6;
    (reinterpret_cast<uint64*>(buffer))[0] = value[0];
  } else if (value[0] <= MAX_UINT56) {
    length1 = 7;
    (reinterpret_cast<uint64*>(buffer))[0] = value[0];
  } else {
    length1 = 8;
    (reinterpret_cast<uint64*>(buffer))[0] = value[0];
  }
  length_total = length1;

  // second number
  if (value[1] <= MAX_UINT8) {
    length2   = 1;
    buffer[length_total] = (uint8) value[1];
  } else if (value[1] <= MAX_UINT16) {
    length2 = 2;
    (reinterpret_cast<uint16 *>(buffer + length_total))[0] =
      static_cast<uint16>(value[1]);
  } else if (value[1] <= MAX_UINT24) {
    length2 = 3;
    (reinterpret_cast<uint32*>(buffer + length_total))[0] = value[1];
  } else if (value[1] <= MAX_UINT32) {
    length2 = 4;
    (reinterpret_cast<uint32*>(buffer + length_total))[0] = value[1];
  } else if (value[1] <= MAX_UINT40) {
    length2 = 5;
    (reinterpret_cast<uint64*>(buffer + length_total))[0] = value[1];
  } else if (value[1] <= MAX_UINT48) {
    length2 = 6;
    (reinterpret_cast<uint64*>(buffer + length_total))[0] = value[1];
  } else if (value[1] <= MAX_UINT56) {
    length2 = 7;
    (reinterpret_cast<uint64*>(buffer + length_total))[0] = value[1];
  } else {
    length2 = 8;
    (reinterpret_cast<uint64*>(buffer + length_total))[0] = value[1];
  }
  length_total += length2;

  target[0] = (((length1 - 1) << 3) | (length2 - 1)) & (0xff >> 2);

  return buffer + length_total;
}

static const int GROUP_VARINT_IDX_ARR[256][5] = {
    /* 00 00 00 00 */ {1, 2, 3, 4, 5},
    /* 00 00 00 01 */ {1, 2, 3, 4, 6},
    /* 00 00 00 10 */ {1, 2, 3, 4, 7},
    /* 00 00 00 11 */ {1, 2, 3, 4, 8},

    /* 00 00 01 00 */ {1, 2, 3, 5, 6},
    /* 00 00 01 01 */ {1, 2, 3, 5, 7},
    /* 00 00 01 10 */ {1, 2, 3, 5, 8},
    /* 00 00 01 11 */ {1, 2, 3, 5, 9},

    /* 00 00 10 00 */ {1, 2, 3, 6, 7},
    /* 00 00 10 01 */ {1, 2, 3, 6, 8},
    /* 00 00 10 10 */ {1, 2, 3, 6, 9},
    /* 00 00 10 11 */ {1, 2, 3, 6, 10},

    /* 00 00 11 00 */ {1, 2, 3, 7, 8},
    /* 00 00 11 01 */ {1, 2, 3, 7, 9},
    /* 00 00 11 10 */ {1, 2, 3, 7, 10},
    /* 00 00 11 11 */ {1, 2, 3, 7, 11},

    /* 00 01 00 00 */ {1, 2, 4, 5, 6},
    /* 00 01 00 01 */ {1, 2, 4, 5, 7},
    /* 00 01 00 10 */ {1, 2, 4, 5, 8},
    /* 00 01 00 11 */ {1, 2, 4, 5, 9},

    /* 00 01 01 00 */ {1, 2, 4, 6, 7},
    /* 00 01 01 01 */ {1, 2, 4, 6, 8},
    /* 00 01 01 10 */ {1, 2, 4, 6, 9},
    /* 00 01 01 11 */ {1, 2, 4, 6, 10},

    /* 00 01 10 00 */ {1, 2, 4, 7, 8},
    /* 00 01 10 01 */ {1, 2, 4, 7, 9},
    /* 00 01 10 10 */ {1, 2, 4, 7, 10},
    /* 00 01 10 11 */ {1, 2, 4, 7, 11},

    /* 00 01 11 00 */ {1, 2, 4, 8, 9},
    /* 00 01 11 01 */ {1, 2, 4, 8, 10},
    /* 00 01 11 10 */ {1, 2, 4, 8, 11},
    /* 00 01 11 11 */ {1, 2, 4, 8, 12},

    /* 00 10 00 00 */ {1, 2, 5, 6, 7},
    /* 00 10 00 01 */ {1, 2, 5, 6, 8},
    /* 00 10 00 10 */ {1, 2, 5, 6, 9},
    /* 00 10 00 11 */ {1, 2, 5, 6, 10},

    /* 00 10 01 00 */ {1, 2, 5, 7, 8},
    /* 00 10 01 01 */ {1, 2, 5, 7, 9},
    /* 00 10 01 10 */ {1, 2, 5, 7, 10},
    /* 00 10 01 11 */ {1, 2, 5, 7, 11},

    /* 00 10 10 00 */ {1, 2, 5, 8, 9},
    /* 00 10 10 01 */ {1, 2, 5, 8, 10},
    /* 00 10 10 10 */ {1, 2, 5, 8, 11},
    /* 00 10 10 11 */ {1, 2, 5, 8, 12},

    /* 00 10 11 00 */ {1, 2, 5, 9, 10},
    /* 00 10 11 01 */ {1, 2, 5, 9, 11},
    /* 00 10 11 10 */ {1, 2, 5, 9, 12},
    /* 00 10 11 11 */ {1, 2, 5, 9, 13},

    /* 00 11 00 00 */ {1, 2, 6, 7, 8},
    /* 00 11 00 01 */ {1, 2, 6, 7, 9},
    /* 00 11 00 10 */ {1, 2, 6, 7, 10},
    /* 00 11 00 11 */ {1, 2, 6, 7, 11},

    /* 00 11 01 00 */ {1, 2, 6, 8, 9},
    /* 00 11 01 01 */ {1, 2, 6, 8, 10},
    /* 00 11 01 10 */ {1, 2, 6, 8, 11},
    /* 00 11 01 11 */ {1, 2, 6, 8, 12},

    /* 00 11 10 00 */ {1, 2, 6, 9, 10},
    /* 00 11 10 01 */ {1, 2, 6, 9, 11},
    /* 00 11 10 10 */ {1, 2, 6, 9, 12},
    /* 00 11 10 11 */ {1, 2, 6, 9, 13},

    /* 00 11 11 00 */ {1, 2, 6, 10, 11},
    /* 00 11 11 01 */ {1, 2, 6, 10, 12},
    /* 00 11 11 10 */ {1, 2, 6, 10, 13},
    /* 00 11 11 11 */ {1, 2, 6, 10, 14},

    /* 01 00 00 00 */ {1, 3, 4, 5, 6},
    /* 01 00 00 01 */ {1, 3, 4, 5, 7},
    /* 01 00 00 10 */ {1, 3, 4, 5, 8},
    /* 01 00 00 11 */ {1, 3, 4, 5, 9},

    /* 01 00 01 00 */ {1, 3, 4, 6, 7},
    /* 01 00 01 01 */ {1, 3, 4, 6, 8},
    /* 01 00 01 10 */ {1, 3, 4, 6, 9},
    /* 01 00 01 11 */ {1, 3, 4, 6, 10},

    /* 01 00 10 00 */ {1, 3, 4, 7, 8},
    /* 01 00 10 01 */ {1, 3, 4, 7, 9},
    /* 01 00 10 10 */ {1, 3, 4, 7, 10},
    /* 01 00 10 11 */ {1, 3, 4, 7, 11},

    /* 01 00 11 00 */ {1, 3, 4, 8, 9},
    /* 01 00 11 01 */ {1, 3, 4, 8, 10},
    /* 01 00 11 10 */ {1, 3, 4, 8, 11},
    /* 01 00 11 11 */ {1, 3, 4, 8, 12},

    /* 01 01 00 00 */ {1, 3, 5, 6, 7},
    /* 01 01 00 01 */ {1, 3, 5, 6, 8},
    /* 01 01 00 10 */ {1, 3, 5, 6, 9},
    /* 01 01 00 11 */ {1, 3, 5, 6, 10},

    /* 01 01 01 00 */ {1, 3, 5, 7, 8},
    /* 01 01 01 01 */ {1, 3, 5, 7, 9},
    /* 01 01 01 10 */ {1, 3, 5, 7, 10},
    /* 01 01 01 11 */ {1, 3, 5, 7, 11},

    /* 01 01 10 00 */ {1, 3, 5, 8, 9},
    /* 01 01 10 01 */ {1, 3, 5, 8, 10},
    /* 01 01 10 10 */ {1, 3, 5, 8, 11},
    /* 01 01 10 11 */ {1, 3, 5, 8, 12},

    /* 01 01 11 00 */ {1, 3, 5, 9, 10},
    /* 01 01 11 01 */ {1, 3, 5, 9, 11},
    /* 01 01 11 10 */ {1, 3, 5, 9, 12},
    /* 01 01 11 11 */ {1, 3, 5, 9, 13},

    /* 01 10 00 00 */ {1, 3, 6, 7, 8},
    /* 01 10 00 01 */ {1, 3, 6, 7, 9},
    /* 01 10 00 10 */ {1, 3, 6, 7, 10},
    /* 01 10 00 11 */ {1, 3, 6, 7, 11},

    /* 01 10 01 00 */ {1, 3, 6, 8, 9},
    /* 01 10 01 01 */ {1, 3, 6, 8, 10},
    /* 01 10 01 10 */ {1, 3, 6, 8, 11},
    /* 01 10 01 11 */ {1, 3, 6, 8, 12},

    /* 01 10 10 00 */ {1, 3, 6, 9, 10},
    /* 01 10 10 01 */ {1, 3, 6, 9, 11},
    /* 01 10 10 10 */ {1, 3, 6, 9, 12},
    /* 01 10 10 11 */ {1, 3, 6, 9, 13},

    /* 01 10 11 00 */ {1, 3, 6, 10, 11},
    /* 01 10 11 01 */ {1, 3, 6, 10, 12},
    /* 01 10 11 10 */ {1, 3, 6, 10, 13},
    /* 01 10 11 11 */ {1, 3, 6, 10, 14},

    /* 01 11 00 00 */ {1, 3, 7, 8, 9},
    /* 01 11 00 01 */ {1, 3, 7, 8, 10},
    /* 01 11 00 10 */ {1, 3, 7, 8, 11},
    /* 01 11 00 11 */ {1, 3, 7, 8, 12},

    /* 01 11 01 00 */ {1, 3, 7, 9, 10},
    /* 01 11 01 01 */ {1, 3, 7, 9, 11},
    /* 01 11 01 10 */ {1, 3, 7, 9, 12},
    /* 01 11 01 11 */ {1, 3, 7, 9, 13},

    /* 01 11 10 00 */ {1, 3, 7, 10, 11},
    /* 01 11 10 01 */ {1, 3, 7, 10, 12},
    /* 01 11 10 10 */ {1, 3, 7, 10, 13},
    /* 01 11 10 11 */ {1, 3, 7, 10, 14},

    /* 01 11 11 00 */ {1, 3, 7, 11, 12},
    /* 01 11 11 01 */ {1, 3, 7, 11, 13},
    /* 01 11 11 10 */ {1, 3, 7, 11, 14},
    /* 01 11 11 11 */ {1, 3, 7, 11, 15},

    /* 10 00 00 00 */ {1, 4, 5, 6, 7},
    /* 10 00 00 01 */ {1, 4, 5, 6, 8},
    /* 10 00 00 10 */ {1, 4, 5, 6, 9},
    /* 10 00 00 11 */ {1, 4, 5, 6, 10},

    /* 10 00 01 00 */ {1, 4, 5, 7, 8},
    /* 10 00 01 01 */ {1, 4, 5, 7, 9},
    /* 10 00 01 10 */ {1, 4, 5, 7, 10},
    /* 10 00 01 11 */ {1, 4, 5, 7, 11},

    /* 10 00 10 00 */ {1, 4, 5, 8, 9},
    /* 10 00 10 01 */ {1, 4, 5, 8, 10},
    /* 10 00 10 10 */ {1, 4, 5, 8, 11},
    /* 10 00 10 11 */ {1, 4, 5, 8, 12},

    /* 10 00 11 00 */ {1, 4, 5, 9, 10},
    /* 10 00 11 01 */ {1, 4, 5, 9, 11},
    /* 10 00 11 10 */ {1, 4, 5, 9, 12},
    /* 10 00 11 11 */ {1, 4, 5, 9, 13},

    /* 10 01 00 00 */ {1, 4, 6, 7, 8},
    /* 10 01 00 01 */ {1, 4, 6, 7, 9},
    /* 10 01 00 10 */ {1, 4, 6, 7, 10},
    /* 10 01 00 11 */ {1, 4, 6, 7, 11},

    /* 10 01 01 00 */ {1, 4, 6, 8, 9},
    /* 10 01 01 01 */ {1, 4, 6, 8, 10},
    /* 10 01 01 10 */ {1, 4, 6, 8, 11},
    /* 10 01 01 11 */ {1, 4, 6, 8, 12},

    /* 10 01 10 00 */ {1, 4, 6, 9, 10},
    /* 10 01 10 01 */ {1, 4, 6, 9, 11},
    /* 10 01 10 10 */ {1, 4, 6, 9, 12},
    /* 10 01 10 11 */ {1, 4, 6, 9, 13},

    /* 10 01 11 00 */ {1, 4, 6, 10, 11},
    /* 10 01 11 01 */ {1, 4, 6, 10, 12},
    /* 10 01 11 10 */ {1, 4, 6, 10, 13},
    /* 10 01 11 11 */ {1, 4, 6, 10, 14},

    /* 10 10 00 00 */ {1, 4, 7, 8, 9},
    /* 10 10 00 01 */ {1, 4, 7, 8, 10},
    /* 10 10 00 10 */ {1, 4, 7, 8, 11},
    /* 10 10 00 11 */ {1, 4, 7, 8, 12},

    /* 10 10 01 00 */ {1, 4, 7, 9, 10},
    /* 10 10 01 01 */ {1, 4, 7, 9, 11},
    /* 10 10 01 10 */ {1, 4, 7, 9, 12},
    /* 10 10 01 11 */ {1, 4, 7, 9, 13},

    /* 10 10 10 00 */ {1, 4, 7, 10, 11},
    /* 10 10 10 01 */ {1, 4, 7, 10, 12},
    /* 10 10 10 10 */ {1, 4, 7, 10, 13},
    /* 10 10 10 11 */ {1, 4, 7, 10, 14},

    /* 10 10 11 00 */ {1, 4, 7, 11, 12},
    /* 10 10 11 01 */ {1, 4, 7, 11, 13},
    /* 10 10 11 10 */ {1, 4, 7, 11, 14},
    /* 10 10 11 11 */ {1, 4, 7, 11, 15},

    /* 10 11 00 00 */ {1, 4, 8, 9, 10},
    /* 10 11 00 01 */ {1, 4, 8, 9, 11},
    /* 10 11 00 10 */ {1, 4, 8, 9, 12},
    /* 10 11 00 11 */ {1, 4, 8, 9, 13},

    /* 10 11 01 00 */ {1, 4, 8, 10, 11},
    /* 10 11 01 01 */ {1, 4, 8, 10, 12},
    /* 10 11 01 10 */ {1, 4, 8, 10, 13},
    /* 10 11 01 11 */ {1, 4, 8, 10, 14},

    /* 10 11 10 00 */ {1, 4, 8, 11, 12},
    /* 10 11 10 01 */ {1, 4, 8, 11, 13},
    /* 10 11 10 10 */ {1, 4, 8, 11, 14},
    /* 10 11 10 11 */ {1, 4, 8, 11, 15},

    /* 10 11 11 00 */ {1, 4, 8, 12, 13},
    /* 10 11 11 01 */ {1, 4, 8, 12, 14},
    /* 10 11 11 10 */ {1, 4, 8, 12, 15},
    /* 10 11 11 11 */ {1, 4, 8, 12, 16},

    /* 11 00 00 00 */ {1, 5, 6, 7, 8},
    /* 11 00 00 01 */ {1, 5, 6, 7, 9},
    /* 11 00 00 10 */ {1, 5, 6, 7, 10},
    /* 11 00 00 11 */ {1, 5, 6, 7, 11},

    /* 11 00 01 00 */ {1, 5, 6, 8, 9},
    /* 11 00 01 01 */ {1, 5, 6, 8, 10},
    /* 11 00 01 10 */ {1, 5, 6, 8, 11},
    /* 11 00 01 11 */ {1, 5, 6, 8, 12},

    /* 11 00 10 00 */ {1, 5, 6, 9, 10},
    /* 11 00 10 01 */ {1, 5, 6, 9, 11},
    /* 11 00 10 10 */ {1, 5, 6, 9, 12},
    /* 11 00 10 11 */ {1, 5, 6, 9, 13},

    /* 11 00 11 00 */ {1, 5, 6, 10, 11},
    /* 11 00 11 01 */ {1, 5, 6, 10, 12},
    /* 11 00 11 10 */ {1, 5, 6, 10, 13},
    /* 11 00 11 11 */ {1, 5, 6, 10, 14},

    /* 11 01 00 00 */ {1, 5, 7, 8, 9},
    /* 11 01 00 01 */ {1, 5, 7, 8, 10},
    /* 11 01 00 10 */ {1, 5, 7, 8, 11},
    /* 11 01 00 11 */ {1, 5, 7, 8, 12},

    /* 11 01 01 00 */ {1, 5, 7, 9, 10},
    /* 11 01 01 01 */ {1, 5, 7, 9, 11},
    /* 11 01 01 10 */ {1, 5, 7, 9, 12},
    /* 11 01 01 11 */ {1, 5, 7, 9, 13},

    /* 11 01 10 00 */ {1, 5, 7, 10, 11},
    /* 11 01 10 01 */ {1, 5, 7, 10, 12},
    /* 11 01 10 10 */ {1, 5, 7, 10, 13},
    /* 11 01 10 11 */ {1, 5, 7, 10, 14},

    /* 11 01 11 00 */ {1, 5, 7, 11, 12},
    /* 11 01 11 01 */ {1, 5, 7, 11, 13},
    /* 11 01 11 10 */ {1, 5, 7, 11, 14},
    /* 11 01 11 11 */ {1, 5, 7, 11, 15},

    /* 11 10 00 00 */ {1, 5, 8, 9, 10},
    /* 11 10 00 01 */ {1, 5, 8, 9, 11},
    /* 11 10 00 10 */ {1, 5, 8, 9, 12},
    /* 11 10 00 11 */ {1, 5, 8, 9, 13},

    /* 11 10 01 00 */ {1, 5, 8, 10, 11},
    /* 11 10 01 01 */ {1, 5, 8, 10, 12},
    /* 11 10 01 10 */ {1, 5, 8, 10, 13},
    /* 11 10 01 11 */ {1, 5, 8, 10, 14},

    /* 11 10 10 00 */ {1, 5, 8, 11, 12},
    /* 11 10 10 01 */ {1, 5, 8, 11, 13},
    /* 11 10 10 10 */ {1, 5, 8, 11, 14},
    /* 11 10 10 11 */ {1, 5, 8, 11, 15},

    /* 11 10 11 00 */ {1, 5, 8, 12, 13},
    /* 11 10 11 01 */ {1, 5, 8, 12, 14},
    /* 11 10 11 10 */ {1, 5, 8, 12, 15},
    /* 11 10 11 11 */ {1, 5, 8, 12, 16},

    /* 11 11 00 00 */ {1, 5, 9, 10, 11},
    /* 11 11 00 01 */ {1, 5, 9, 10, 12},
    /* 11 11 00 10 */ {1, 5, 9, 10, 13},
    /* 11 11 00 11 */ {1, 5, 9, 10, 14},

    /* 11 11 01 00 */ {1, 5, 9, 11, 12},
    /* 11 11 01 01 */ {1, 5, 9, 11, 13},
    /* 11 11 01 10 */ {1, 5, 9, 11, 14},
    /* 11 11 01 11 */ {1, 5, 9, 11, 15},

    /* 11 11 10 00 */ {1, 5, 9, 12, 13},
    /* 11 11 10 01 */ {1, 5, 9, 12, 14},
    /* 11 11 10 10 */ {1, 5, 9, 12, 15},
    /* 11 11 10 11 */ {1, 5, 9, 12, 16},

    /* 11 11 11 00 */ {1, 5, 9, 13, 14},
    /* 11 11 11 01 */ {1, 5, 9, 13, 15},
    /* 11 11 11 10 */ {1, 5, 9, 13, 16},
    /* 11 11 11 11 */ {1, 5, 9, 13, 17}
};

static const int GROUP_VARINT64_IDX_ARR[64][3] = {
    /* 00 00 00 00 */ {1, 2, 3},
    /* 00 00 00 01 */ {1, 2, 4},
    /* 00 00 00 10 */ {1, 2, 5},
    /* 00 00 00 11 */ {1, 2, 6},

    /* 00 00 01 00 */ {1, 2, 7},
    /* 00 00 01 01 */ {1, 2, 8},
    /* 00 00 01 10 */ {1, 2, 9},
    /* 00 00 01 11 */ {1, 2, 10},

    /* 00 00 10 00 */ {1, 3, 4},
    /* 00 00 10 01 */ {1, 3, 5},
    /* 00 00 10 10 */ {1, 3, 6},
    /* 00 00 10 11 */ {1, 3, 7},

    /* 00 00 11 00 */ {1, 3, 8},
    /* 00 00 11 01 */ {1, 3, 9},
    /* 00 00 11 10 */ {1, 3, 10},
    /* 00 00 11 11 */ {1, 3, 11},

    /* 00 01 00 00 */ {1, 4, 5},
    /* 00 01 00 01 */ {1, 4, 6},
    /* 00 01 00 10 */ {1, 4, 7},
    /* 00 01 00 11 */ {1, 4, 8},

    /* 00 01 01 00 */ {1, 4, 9},
    /* 00 01 01 01 */ {1, 4, 10},
    /* 00 01 01 10 */ {1, 4, 11},
    /* 00 01 01 11 */ {1, 4, 12},

    /* 00 01 10 00 */ {1, 5, 6},
    /* 00 01 10 01 */ {1, 5, 7},
    /* 00 01 10 10 */ {1, 5, 8},
    /* 00 01 10 11 */ {1, 5, 9},

    /* 00 01 11 00 */ {1, 5, 10},
    /* 00 01 11 01 */ {1, 5, 11},
    /* 00 01 11 10 */ {1, 5, 12},
    /* 00 01 11 11 */ {1, 5, 13},

    /* 00 10 00 00 */ {1, 6, 7},
    /* 00 10 00 01 */ {1, 6, 8},
    /* 00 10 00 10 */ {1, 6, 9},
    /* 00 10 00 11 */ {1, 6, 10},

    /* 00 10 01 00 */ {1, 6, 11},
    /* 00 10 01 01 */ {1, 6, 12},
    /* 00 10 01 10 */ {1, 6, 13},
    /* 00 10 01 11 */ {1, 6, 14},

    /* 00 10 10 00 */ {1, 7, 8},
    /* 00 10 10 01 */ {1, 7, 9},
    /* 00 10 10 10 */ {1, 7, 10},
    /* 00 10 10 11 */ {1, 7, 11},

    /* 00 10 11 00 */ {1, 7, 12},
    /* 00 10 11 01 */ {1, 7, 13},
    /* 00 10 11 10 */ {1, 7, 14},
    /* 00 10 11 11 */ {1, 7, 15},

    /* 00 11 00 00 */ {1, 8, 9},
    /* 00 11 00 01 */ {1, 8, 10},
    /* 00 11 00 10 */ {1, 8, 11},
    /* 00 11 00 11 */ {1, 8, 12},

    /* 00 11 01 00 */ {1, 8, 13},
    /* 00 11 01 01 */ {1, 8, 14},
    /* 00 11 01 10 */ {1, 8, 15},
    /* 00 11 01 11 */ {1, 8, 16},

    /* 00 11 10 00 */ {1, 9, 10},
    /* 00 11 10 01 */ {1, 9, 11},
    /* 00 11 10 10 */ {1, 9, 12},
    /* 00 11 10 11 */ {1, 9, 13},

    /* 00 11 11 00 */ {1, 9, 14},
    /* 00 11 11 01 */ {1, 9, 15},
    /* 00 11 11 10 */ {1, 9, 16},
    /* 00 11 11 11 */ {1, 9, 17},
};


#define   GROUP_VARINT_DECODE(idx)              \
    value[0] = ((GROUP_VARINT_TYPE_##idx *) start)[0].u0; \
    value[1] = ((GROUP_VARINT_TYPE_##idx *) start)[0].u1; \
    value[2] = ((GROUP_VARINT_TYPE_##idx *) start)[0].u2; \
    value[3] = ((GROUP_VARINT_TYPE_##idx *) start)[0].u3;

inline const uint8* ReadGroupVarint32(const uint8* buffer, uint32* value) {
  union UU {
    struct {
      uint8 u3:2;
      uint8 u2:2;
      uint8 u1:2;
      uint8 u0:2;
    } d;
    uint8 e;
  };
  const UU& u = *(reinterpret_cast<const UU*>(buffer));
  const int* len_arr = GROUP_VARINT_IDX_ARR[buffer[0]];

  struct U2 {
    uint32 u0:24;
    uint32 u1:8;
  };

  switch (u.d.u0) {
    case 0:
      value[0] = *(buffer + len_arr[0]);
      break;
    case 1:
      value[0] = *(reinterpret_cast<const uint16*>(buffer+len_arr[0]));
      break;
    case 2:
      value[0] = (*(reinterpret_cast<const U2*>(buffer+len_arr[0]))).u0;
      break;
    case 3:
      value[0] = *(reinterpret_cast<const uint32*>(buffer+len_arr[0]));
      break;
  }

  switch (u.d.u1) {
    case 0:
      value[1] = *(buffer + len_arr[1]);
      break;
    case 1:
      value[1] = *(reinterpret_cast<const uint16*>(buffer+len_arr[1]));
      break;
    case 2:
      value[1] = (*(reinterpret_cast<const U2*>(buffer+len_arr[1]))).u0;
      break;
    case 3:
      value[1] = *(reinterpret_cast<const uint32*>(buffer+len_arr[1]));
      break;
  }

  switch (u.d.u2) {
    case 0:
      value[2] = *(buffer + len_arr[2]);
      break;
    case 1:
      value[2] = *(reinterpret_cast<const uint16*>(buffer+len_arr[2]));
      break;
    case 2:
      value[2] = (*(reinterpret_cast<const U2*>(buffer+len_arr[2]))).u0;
      break;
    case 3:
      value[2] = *(reinterpret_cast<const uint32*>(buffer+len_arr[2]));
      break;
  }

  switch (u.d.u3) {
    case 0:
      value[3] = *(buffer + len_arr[3]);
      break;
    case 1:
      value[3] = *(reinterpret_cast<const uint16*>(buffer+len_arr[3]));
      break;
    case 2:
      value[3] = (*(reinterpret_cast<const U2*>(buffer+len_arr[3]))).u0;
      break;
    case 3:
      value[3] = *(reinterpret_cast<const uint32*>(buffer+len_arr[3]));
      break;
  }

  return buffer + len_arr[4];
}

inline const uint32 ReadFirstGroupVarint32(const uint8* buffer) {
  union UU {
    struct {
      uint8 u3:2;
      uint8 u2:2;
      uint8 u1:2;
      uint8 u0:2;
    } d;
    uint8 e;
  };
  const UU& u = *(reinterpret_cast<const UU*>(buffer));
  const int* len_arr = GROUP_VARINT_IDX_ARR[buffer[0]];

  struct U2 {
    uint32 u0:24;
    uint32 u1:8;
  };
  uint32 value = 0;
  switch (u.d.u0) {
    case 0:
      value = *(buffer + len_arr[0]);
      break;
    case 1:
      value = *(reinterpret_cast<const uint16*>(buffer+len_arr[0]));
      break;
    case 2:
      value = (*(reinterpret_cast<const U2*>(buffer+len_arr[0]))).u0;
      break;
    case 3:
      value = *(reinterpret_cast<const uint32*>(buffer+len_arr[0]));
      break;
  }
  return value;
}

inline const uint32 ReadSecondGroupVarint32(const uint8* buffer) {
  union UU {
    struct {
      uint8 u3:2;
      uint8 u2:2;
      uint8 u1:2;
      uint8 u0:2;
    } d;
    uint8 e;
  };
  const UU& u = *(reinterpret_cast<const UU*>(buffer));
  const int* len_arr = GROUP_VARINT_IDX_ARR[buffer[0]];

  struct U2 {
    uint32 u0:24;
    uint32 u1:8;
  };
  uint32 value = 0;
  switch (u.d.u1) {
    case 0:
      value = *(buffer + len_arr[1]);
      break;
    case 1:
      value = *(reinterpret_cast<const uint16*>(buffer+len_arr[1]));
      break;
    case 2:
      value = (*(reinterpret_cast<const U2*>(buffer+len_arr[1]))).u0;
      break;
    case 3:
      value = *(reinterpret_cast<const uint32*>(buffer+len_arr[1]));
      break;
  }
  return value;
}

inline const uint32 ReadThirdGroupVarint32(const uint8* buffer) {
  union UU {
    struct {
      uint8 u3:2;
      uint8 u2:2;
      uint8 u1:2;
      uint8 u0:2;
    } d;
    uint8 e;
  };
  const UU& u = *(reinterpret_cast<const UU*>(buffer));
  const int* len_arr = GROUP_VARINT_IDX_ARR[buffer[0]];

  struct U2 {
    uint32 u0:24;
    uint32 u1:8;
  };
  uint32 value = 0;
  switch (u.d.u2) {
    case 0:
      value = *(buffer + len_arr[2]);
      break;
    case 1:
      value = *(reinterpret_cast<const uint16*>(buffer+len_arr[2]));
      break;
    case 2:
      value = (*(reinterpret_cast<const U2*>(buffer+len_arr[2]))).u0;
      break;
    case 3:
      value = *(reinterpret_cast<const uint32*>(buffer+len_arr[2]));
      break;
  }
  return value;
}

inline const uint32 ReadFourthGroupVarint32(const uint8* buffer) {
  union UU {
    struct {
      uint8 u3:2;
      uint8 u2:2;
      uint8 u1:2;
      uint8 u0:2;
    } d;
    uint8 e;
  };
  const UU& u = *(reinterpret_cast<const UU*>(buffer));
  const int* len_arr = GROUP_VARINT_IDX_ARR[buffer[0]];

  struct U2 {
    uint32 u0:24;
    uint32 u1:8;
  };
  uint32 value = 0;
  switch (u.d.u3) {
    case 0:
      value = *(buffer + len_arr[3]);
      break;
    case 1:
      value = *(reinterpret_cast<const uint16*>(buffer+len_arr[3]));
      break;
    case 2:
      value = (*(reinterpret_cast<const U2*>(buffer+len_arr[3]))).u0;
      break;
    case 3:
      value = *(reinterpret_cast<const uint32*>(buffer+len_arr[3]));
      break;
  }
  return value;
}

// value should point to 2 size uint64 array
inline const uint8* ReadGroupVarint64(const uint8* buffer, uint64* value) {
  union UU {
    struct {
      uint8 u2:3;
      uint8 u1:3;
      uint8 u0:2;
    } d;
    uint8 e;
  };
  const UU& u = *(reinterpret_cast<const UU*>(buffer));
  const int* len_arr = GROUP_VARINT64_IDX_ARR[buffer[0]];

  struct U2 {
    uint32 u0:24;
    uint32 u1:8;
  };
  struct U3 {
    uint64 u0:40;
    uint64 u1:24;
  };
  struct U4 {
    uint64 u0:48;
    uint64 u1:16;
  };
  struct U5 {
    uint64 u0:56;
    uint64 u1:8;
  };

  switch (u.d.u1) {
    case 0:
      value[0] = *(buffer + len_arr[0]);
      break;
    case 1:
      value[0] = *(reinterpret_cast<const uint16*>(buffer+len_arr[0]));
      break;
    case 2:
      value[0] = (*(reinterpret_cast<const U2*>(buffer+len_arr[0]))).u0;
      break;
    case 3:
      value[0] = *(reinterpret_cast<const uint32*>(buffer+len_arr[0]));
      break;
    case 4:
      value[0] = (*(reinterpret_cast<const U3*>(buffer+len_arr[0]))).u0;
      break;
    case 5:
      value[0] = (*(reinterpret_cast<const U4*>(buffer+len_arr[0]))).u0;
      break;
    case 6:
      value[0] = (*(reinterpret_cast<const U5*>(buffer+len_arr[0]))).u0;
      break;
    case 7:
      value[0] = *(reinterpret_cast<const uint64*>(buffer+len_arr[0]));
      break;
  }

  switch (u.d.u2) {
    case 0:
      value[1] = *(buffer + len_arr[1]);
      break;
    case 1:
      value[1] = *(reinterpret_cast<const uint16*>(buffer+len_arr[1]));
      break;
    case 2:
      value[1] = (*(reinterpret_cast<const U2*>(buffer+len_arr[1]))).u0;
      break;
    case 3:
      value[1] = *(reinterpret_cast<const uint32*>(buffer+len_arr[1]));
      break;
    case 4:
      value[1] = (*(reinterpret_cast<const U3*>(buffer+len_arr[1]))).u0;
      break;
    case 5:
      value[1] = (*(reinterpret_cast<const U4*>(buffer+len_arr[1]))).u0;
      break;
    case 6:
      value[1] = (*(reinterpret_cast<const U5*>(buffer+len_arr[1]))).u0;
      break;
    case 7:
      value[1] = *(reinterpret_cast<const uint64*>(buffer+len_arr[1]));
      break;
  }

  return buffer + len_arr[2];
}

// 一次处理两个uint64整数, 但是只会写乱高位1个byte数据
inline uint8* WriteGroupVarint64_v2(const uint64* value, uint8* target) {
  union UU {
    struct {
      uint32 u1:32;
      uint32 u0:32;
    } d;
    uint64 e;
  };
  const UU& uu0 = *(reinterpret_cast<const UU*>(&value[0]));
  const UU& uu1 = *(reinterpret_cast<const UU*>(&value[1]));

  uint32 arr[4] = {uu0.d.u0, uu0.d.u1,
                   uu1.d.u0, uu1.d.u1};
  return WriteGroupVarint32(arr, target);
}

inline const uint8* ReadGroupVarint64_v2(const uint8* buffer, uint64* value) {
  uint32 int_arr[4] = {0};
  buffer = ReadGroupVarint32(buffer, int_arr);
  value[0] = ((static_cast<uint64>(int_arr[0])) << 32) + int_arr[1];
  value[1] = ((static_cast<uint64>(int_arr[2])) << 32) + int_arr[3];
  return buffer;
}

inline const uint64 ReadFirstGroupVarint64_v2(const uint8* buffer) {
  union UU {
    struct {
      uint8 u3:2;
      uint8 u2:2;
      uint8 u1:2;
      uint8 u0:2;
    } d;
    uint8 e;
  };
  const UU& u = *(reinterpret_cast<const UU*>(buffer));
  const int* len_arr = GROUP_VARINT_IDX_ARR[buffer[0]];

  struct U2 {
    uint32 u0:24;
    uint32 u1:8;
  };
  uint32 value1 = 0;
  switch (u.d.u0) {
    case 0:
      value1 = *(buffer + len_arr[0]);
      break;
    case 1:
      value1 = *(reinterpret_cast<const uint16*>(buffer+len_arr[0]));
      break;
    case 2:
      value1 = (*(reinterpret_cast<const U2*>(buffer+len_arr[0]))).u0;
      break;
    case 3:
      value1 = *(reinterpret_cast<const uint32*>(buffer+len_arr[0]));
      break;
  }
  uint32 value2 = 0;
  switch (u.d.u1) {
    case 0:
      value2 = *(buffer + len_arr[1]);
      break;
    case 1:
      value2 = *(reinterpret_cast<const uint16*>(buffer+len_arr[1]));
      break;
    case 2:
      value2 = (*(reinterpret_cast<const U2*>(buffer+len_arr[1]))).u0;
      break;
    case 3:
      value2 = *(reinterpret_cast<const uint32*>(buffer+len_arr[1]));
      break;
  }
  return ((static_cast<uint64>(value1)) << 32) + value2;
}
/*
inline const uint64 ReadFirstGroupVarint64_v2(const uint8* buffer) {
  return (static_cast<uint64>(ReadFirstGroupVarint32(buffer)) << 32)
    + ReadSecondGroupVarint32(buffer);
}
*/
inline const uint64 ReadSecondGroupVarint64_v2(const uint8* buffer) {
  return (static_cast<uint64>(ReadThirdGroupVarint32(buffer)) << 32)
    + ReadFourthGroupVarint32(buffer);
}

inline uint32 GetGroupVarint64Length_v2(const uint64 value) {
  return GetGroupVarint32Length(value >> 32) +
    GetGroupVarint32Length((value << 32) >> 32);
}

inline uint32 GetGroupVarint64Length_v2(const uint8* buffer) {
  const int* len_arr = GROUP_VARINT_IDX_ARR[buffer[0]];
  return len_arr[4];
}

}  // namespace base
#endif  // BASE_GROUP_VARINT_H_
