/*
$info$
tags: backend|interpreter
$end_info$
*/

#include "Interface/Core/Interpreter/InterpreterClass.h"
#include "Interface/Core/Interpreter/InterpreterOps.h"
#include "Interface/Core/Interpreter/InterpreterDefines.h"

#include <cstdint>

namespace AES {
  static __uint128_t InvShiftRows(uint8_t *State) {
    uint8_t Shifted[16] = {
      State[0], State[13], State[10], State[7],
      State[4], State[1], State[14], State[11],
      State[8], State[5], State[2], State[15],
      State[12], State[9], State[6], State[3],
    };
    __uint128_t Res{};
    memcpy(&Res, Shifted, 16);
    return Res;
  }

  static __uint128_t InvSubBytes(uint8_t *State) {
    // 16x16 matrix table
    static const uint8_t InvSubstitutionTable[256] = {
      0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
      0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
      0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
      0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
      0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
      0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
      0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
      0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
      0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
      0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
      0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
      0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
      0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
      0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
      0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
      0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d,
    };

    // Uses a byte substitution table with a constant set of values
    // Needs to do a table look up
    uint8_t Substituted[16];
    for (size_t i = 0; i < 16; ++i) {
      Substituted[i] = InvSubstitutionTable[State[i]];
    }

    __uint128_t Res{};
    memcpy(&Res, Substituted, 16);
    return Res;
  }

  static __uint128_t ShiftRows(uint8_t *State) {
    uint8_t Shifted[16] = {
      State[0], State[5], State[10], State[15],
      State[4], State[9], State[14], State[3],
      State[8], State[13], State[2], State[7],
      State[12], State[1], State[6], State[11],
    };
    __uint128_t Res{};
    memcpy(&Res, Shifted, 16);
    return Res;
  }

  static __uint128_t SubBytes(uint8_t *State, size_t Bytes) {
    // 16x16 matrix table
    static const uint8_t SubstitutionTable[256] = {
      0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
      0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
      0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
      0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
      0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
      0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
      0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
      0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
      0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
      0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
      0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
      0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
      0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
      0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
      0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
      0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16,
    };
    // Uses a byte substitution table with a constant set of values
    // Needs to do a table look up
    uint8_t Substituted[16];
    Bytes = std::min(Bytes, (size_t)16);
    for (size_t i = 0; i < Bytes; ++i) {
      Substituted[i] = SubstitutionTable[State[i]];
    }

    __uint128_t Res{};
    memcpy(&Res, Substituted, Bytes);
    return Res;
  }

  static uint8_t FFMul02(uint8_t in) {
    static const uint8_t FFMul02[256] = {
      0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e,
      0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e,
      0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e,
      0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e,
      0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e,
      0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 0xae, 0xb0, 0xb2, 0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe,
      0xc0, 0xc2, 0xc4, 0xc6, 0xc8, 0xca, 0xcc, 0xce, 0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xdc, 0xde,
      0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee, 0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe,
      0x1b, 0x19, 0x1f, 0x1d, 0x13, 0x11, 0x17, 0x15, 0x0b, 0x09, 0x0f, 0x0d, 0x03, 0x01, 0x07, 0x05,
      0x3b, 0x39, 0x3f, 0x3d, 0x33, 0x31, 0x37, 0x35, 0x2b, 0x29, 0x2f, 0x2d, 0x23, 0x21, 0x27, 0x25,
      0x5b, 0x59, 0x5f, 0x5d, 0x53, 0x51, 0x57, 0x55, 0x4b, 0x49, 0x4f, 0x4d, 0x43, 0x41, 0x47, 0x45,
      0x7b, 0x79, 0x7f, 0x7d, 0x73, 0x71, 0x77, 0x75, 0x6b, 0x69, 0x6f, 0x6d, 0x63, 0x61, 0x67, 0x65,
      0x9b, 0x99, 0x9f, 0x9d, 0x93, 0x91, 0x97, 0x95, 0x8b, 0x89, 0x8f, 0x8d, 0x83, 0x81, 0x87, 0x85,
      0xbb, 0xb9, 0xbf, 0xbd, 0xb3, 0xb1, 0xb7, 0xb5, 0xab, 0xa9, 0xaf, 0xad, 0xa3, 0xa1, 0xa7, 0xa5,
      0xdb, 0xd9, 0xdf, 0xdd, 0xd3, 0xd1, 0xd7, 0xd5, 0xcb, 0xc9, 0xcf, 0xcd, 0xc3, 0xc1, 0xc7, 0xc5,
      0xfb, 0xf9, 0xff, 0xfd, 0xf3, 0xf1, 0xf7, 0xf5, 0xeb, 0xe9, 0xef, 0xed, 0xe3, 0xe1, 0xe7, 0xe5,
    };
    return FFMul02[in];
  }

  static uint8_t FFMul03(uint8_t in) {
    static const uint8_t FFMul03[256] = {
      0x00, 0x03, 0x06, 0x05, 0x0c, 0x0f, 0x0a, 0x09, 0x18, 0x1b, 0x1e, 0x1d, 0x14, 0x17, 0x12, 0x11,
      0x30, 0x33, 0x36, 0x35, 0x3c, 0x3f, 0x3a, 0x39, 0x28, 0x2b, 0x2e, 0x2d, 0x24, 0x27, 0x22, 0x21,
      0x60, 0x63, 0x66, 0x65, 0x6c, 0x6f, 0x6a, 0x69, 0x78, 0x7b, 0x7e, 0x7d, 0x74, 0x77, 0x72, 0x71,
      0x50, 0x53, 0x56, 0x55, 0x5c, 0x5f, 0x5a, 0x59, 0x48, 0x4b, 0x4e, 0x4d, 0x44, 0x47, 0x42, 0x41,
      0xc0, 0xc3, 0xc6, 0xc5, 0xcc, 0xcf, 0xca, 0xc9, 0xd8, 0xdb, 0xde, 0xdd, 0xd4, 0xd7, 0xd2, 0xd1,
      0xf0, 0xf3, 0xf6, 0xf5, 0xfc, 0xff, 0xfa, 0xf9, 0xe8, 0xeb, 0xee, 0xed, 0xe4, 0xe7, 0xe2, 0xe1,
      0xa0, 0xa3, 0xa6, 0xa5, 0xac, 0xaf, 0xaa, 0xa9, 0xb8, 0xbb, 0xbe, 0xbd, 0xb4, 0xb7, 0xb2, 0xb1,
      0x90, 0x93, 0x96, 0x95, 0x9c, 0x9f, 0x9a, 0x99, 0x88, 0x8b, 0x8e, 0x8d, 0x84, 0x87, 0x82, 0x81,
      0x9b, 0x98, 0x9d, 0x9e, 0x97, 0x94, 0x91, 0x92, 0x83, 0x80, 0x85, 0x86, 0x8f, 0x8c, 0x89, 0x8a,
      0xab, 0xa8, 0xad, 0xae, 0xa7, 0xa4, 0xa1, 0xa2, 0xb3, 0xb0, 0xb5, 0xb6, 0xbf, 0xbc, 0xb9, 0xba,
      0xfb, 0xf8, 0xfd, 0xfe, 0xf7, 0xf4, 0xf1, 0xf2, 0xe3, 0xe0, 0xe5, 0xe6, 0xef, 0xec, 0xe9, 0xea,
      0xcb, 0xc8, 0xcd, 0xce, 0xc7, 0xc4, 0xc1, 0xc2, 0xd3, 0xd0, 0xd5, 0xd6, 0xdf, 0xdc, 0xd9, 0xda,
      0x5b, 0x58, 0x5d, 0x5e, 0x57, 0x54, 0x51, 0x52, 0x43, 0x40, 0x45, 0x46, 0x4f, 0x4c, 0x49, 0x4a,
      0x6b, 0x68, 0x6d, 0x6e, 0x67, 0x64, 0x61, 0x62, 0x73, 0x70, 0x75, 0x76, 0x7f, 0x7c, 0x79, 0x7a,
      0x3b, 0x38, 0x3d, 0x3e, 0x37, 0x34, 0x31, 0x32, 0x23, 0x20, 0x25, 0x26, 0x2f, 0x2c, 0x29, 0x2a,
      0x0b, 0x08, 0x0d, 0x0e, 0x07, 0x04, 0x01, 0x02, 0x13, 0x10, 0x15, 0x16, 0x1f, 0x1c, 0x19, 0x1a,
    };
    return FFMul03[in];
  }

  static __uint128_t MixColumns(uint8_t *State) {
    uint8_t In0[16] = {
      State[0], State[4], State[8], State[12],
      State[1], State[5], State[9], State[13],
      State[2], State[6], State[10], State[14],
      State[3], State[7], State[11], State[15],
    };

    uint8_t Out0[4]{};
    uint8_t Out1[4]{};
    uint8_t Out2[4]{};
    uint8_t Out3[4]{};

    for (size_t i = 0; i < 4; ++i) {
      Out0[i] = FFMul02(In0[0 + i]) ^ FFMul03(In0[4 + i]) ^ In0[8 + i] ^ In0[12 + i];
      Out1[i] = In0[0 + i] ^ FFMul02(In0[4 + i]) ^ FFMul03(In0[8 + i]) ^ In0[12 + i];
      Out2[i] = In0[0 + i] ^ In0[4 + i] ^ FFMul02(In0[8 + i]) ^ FFMul03(In0[12 + i]);
      Out3[i] = FFMul03(In0[0 + i]) ^ In0[4 + i] ^ In0[8 + i] ^ FFMul02(In0[12 + i]);
    }

    uint8_t OutArray[16] = {
      Out0[0], Out1[0], Out2[0], Out3[0],
      Out0[1], Out1[1], Out2[1], Out3[1],
      Out0[2], Out1[2], Out2[2], Out3[2],
      Out0[3], Out1[3], Out2[3], Out3[3],
    };
    __uint128_t Res{};
    memcpy(&Res, OutArray, 16);
    return Res;
  }

  static uint8_t FFMul09(uint8_t in) {
    static const uint8_t FFMul09[256] = {
      0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f, 0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
      0x90, 0x99, 0x82, 0x8b, 0xb4, 0xbd, 0xa6, 0xaf, 0xd8, 0xd1, 0xca, 0xc3, 0xfc, 0xf5, 0xee, 0xe7,
      0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04, 0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
      0xab, 0xa2, 0xb9, 0xb0, 0x8f, 0x86, 0x9d, 0x94, 0xe3, 0xea, 0xf1, 0xf8, 0xc7, 0xce, 0xd5, 0xdc,
      0x76, 0x7f, 0x64, 0x6d, 0x52, 0x5b, 0x40, 0x49, 0x3e, 0x37, 0x2c, 0x25, 0x1a, 0x13, 0x08, 0x01,
      0xe6, 0xef, 0xf4, 0xfd, 0xc2, 0xcb, 0xd0, 0xd9, 0xae, 0xa7, 0xbc, 0xb5, 0x8a, 0x83, 0x98, 0x91,
      0x4d, 0x44, 0x5f, 0x56, 0x69, 0x60, 0x7b, 0x72, 0x05, 0x0c, 0x17, 0x1e, 0x21, 0x28, 0x33, 0x3a,
      0xdd, 0xd4, 0xcf, 0xc6, 0xf9, 0xf0, 0xeb, 0xe2, 0x95, 0x9c, 0x87, 0x8e, 0xb1, 0xb8, 0xa3, 0xaa,
      0xec, 0xe5, 0xfe, 0xf7, 0xc8, 0xc1, 0xda, 0xd3, 0xa4, 0xad, 0xb6, 0xbf, 0x80, 0x89, 0x92, 0x9b,
      0x7c, 0x75, 0x6e, 0x67, 0x58, 0x51, 0x4a, 0x43, 0x34, 0x3d, 0x26, 0x2f, 0x10, 0x19, 0x02, 0x0b,
      0xd7, 0xde, 0xc5, 0xcc, 0xf3, 0xfa, 0xe1, 0xe8, 0x9f, 0x96, 0x8d, 0x84, 0xbb, 0xb2, 0xa9, 0xa0,
      0x47, 0x4e, 0x55, 0x5c, 0x63, 0x6a, 0x71, 0x78, 0x0f, 0x06, 0x1d, 0x14, 0x2b, 0x22, 0x39, 0x30,
      0x9a, 0x93, 0x88, 0x81, 0xbe, 0xb7, 0xac, 0xa5, 0xd2, 0xdb, 0xc0, 0xc9, 0xf6, 0xff, 0xe4, 0xed,
      0x0a, 0x03, 0x18, 0x11, 0x2e, 0x27, 0x3c, 0x35, 0x42, 0x4b, 0x50, 0x59, 0x66, 0x6f, 0x74, 0x7d,
      0xa1, 0xa8, 0xb3, 0xba, 0x85, 0x8c, 0x97, 0x9e, 0xe9, 0xe0, 0xfb, 0xf2, 0xcd, 0xc4, 0xdf, 0xd6,
      0x31, 0x38, 0x23, 0x2a, 0x15, 0x1c, 0x07, 0x0e, 0x79, 0x70, 0x6b, 0x62, 0x5d, 0x54, 0x4f, 0x46,
    };
    return FFMul09[in];
  }

  static uint8_t FFMul0B(uint8_t in) {
    static const uint8_t FFMul0B[256] = {
      0x00, 0x0b, 0x16, 0x1d, 0x2c, 0x27, 0x3a, 0x31, 0x58, 0x53, 0x4e, 0x45, 0x74, 0x7f, 0x62, 0x69,
      0xb0, 0xbb, 0xa6, 0xad, 0x9c, 0x97, 0x8a, 0x81, 0xe8, 0xe3, 0xfe, 0xf5, 0xc4, 0xcf, 0xd2, 0xd9,
      0x7b, 0x70, 0x6d, 0x66, 0x57, 0x5c, 0x41, 0x4a, 0x23, 0x28, 0x35, 0x3e, 0x0f, 0x04, 0x19, 0x12,
      0xcb, 0xc0, 0xdd, 0xd6, 0xe7, 0xec, 0xf1, 0xfa, 0x93, 0x98, 0x85, 0x8e, 0xbf, 0xb4, 0xa9, 0xa2,
      0xf6, 0xfd, 0xe0, 0xeb, 0xda, 0xd1, 0xcc, 0xc7, 0xae, 0xa5, 0xb8, 0xb3, 0x82, 0x89, 0x94, 0x9f,
      0x46, 0x4d, 0x50, 0x5b, 0x6a, 0x61, 0x7c, 0x77, 0x1e, 0x15, 0x08, 0x03, 0x32, 0x39, 0x24, 0x2f,
      0x8d, 0x86, 0x9b, 0x90, 0xa1, 0xaa, 0xb7, 0xbc, 0xd5, 0xde, 0xc3, 0xc8, 0xf9, 0xf2, 0xef, 0xe4,
      0x3d, 0x36, 0x2b, 0x20, 0x11, 0x1a, 0x07, 0x0c, 0x65, 0x6e, 0x73, 0x78, 0x49, 0x42, 0x5f, 0x54,
      0xf7, 0xfc, 0xe1, 0xea, 0xdb, 0xd0, 0xcd, 0xc6, 0xaf, 0xa4, 0xb9, 0xb2, 0x83, 0x88, 0x95, 0x9e,
      0x47, 0x4c, 0x51, 0x5a, 0x6b, 0x60, 0x7d, 0x76, 0x1f, 0x14, 0x09, 0x02, 0x33, 0x38, 0x25, 0x2e,
      0x8c, 0x87, 0x9a, 0x91, 0xa0, 0xab, 0xb6, 0xbd, 0xd4, 0xdf, 0xc2, 0xc9, 0xf8, 0xf3, 0xee, 0xe5,
      0x3c, 0x37, 0x2a, 0x21, 0x10, 0x1b, 0x06, 0x0d, 0x64, 0x6f, 0x72, 0x79, 0x48, 0x43, 0x5e, 0x55,
      0x01, 0x0a, 0x17, 0x1c, 0x2d, 0x26, 0x3b, 0x30, 0x59, 0x52, 0x4f, 0x44, 0x75, 0x7e, 0x63, 0x68,
      0xb1, 0xba, 0xa7, 0xac, 0x9d, 0x96, 0x8b, 0x80, 0xe9, 0xe2, 0xff, 0xf4, 0xc5, 0xce, 0xd3, 0xd8,
      0x7a, 0x71, 0x6c, 0x67, 0x56, 0x5d, 0x40, 0x4b, 0x22, 0x29, 0x34, 0x3f, 0x0e, 0x05, 0x18, 0x13,
      0xca, 0xc1, 0xdc, 0xd7, 0xe6, 0xed, 0xf0, 0xfb, 0x92, 0x99, 0x84, 0x8f, 0xbe, 0xb5, 0xa8, 0xa3,
    };
    return FFMul0B[in];
  }

  static uint8_t FFMul0D(uint8_t in) {
    static const uint8_t FFMul0D[256] = {
      0x00, 0x0d, 0x1a, 0x17, 0x34, 0x39, 0x2e, 0x23, 0x68, 0x65, 0x72, 0x7f, 0x5c, 0x51, 0x46, 0x4b,
      0xd0, 0xdd, 0xca, 0xc7, 0xe4, 0xe9, 0xfe, 0xf3, 0xb8, 0xb5, 0xa2, 0xaf, 0x8c, 0x81, 0x96, 0x9b,
      0xbb, 0xb6, 0xa1, 0xac, 0x8f, 0x82, 0x95, 0x98, 0xd3, 0xde, 0xc9, 0xc4, 0xe7, 0xea, 0xfd, 0xf0,
      0x6b, 0x66, 0x71, 0x7c, 0x5f, 0x52, 0x45, 0x48, 0x03, 0x0e, 0x19, 0x14, 0x37, 0x3a, 0x2d, 0x20,
      0x6d, 0x60, 0x77, 0x7a, 0x59, 0x54, 0x43, 0x4e, 0x05, 0x08, 0x1f, 0x12, 0x31, 0x3c, 0x2b, 0x26,
      0xbd, 0xb0, 0xa7, 0xaa, 0x89, 0x84, 0x93, 0x9e, 0xd5, 0xd8, 0xcf, 0xc2, 0xe1, 0xec, 0xfb, 0xf6,
      0xd6, 0xdb, 0xcc, 0xc1, 0xe2, 0xef, 0xf8, 0xf5, 0xbe, 0xb3, 0xa4, 0xa9, 0x8a, 0x87, 0x90, 0x9d,
      0x06, 0x0b, 0x1c, 0x11, 0x32, 0x3f, 0x28, 0x25, 0x6e, 0x63, 0x74, 0x79, 0x5a, 0x57, 0x40, 0x4d,
      0xda, 0xd7, 0xc0, 0xcd, 0xee, 0xe3, 0xf4, 0xf9, 0xb2, 0xbf, 0xa8, 0xa5, 0x86, 0x8b, 0x9c, 0x91,
      0x0a, 0x07, 0x10, 0x1d, 0x3e, 0x33, 0x24, 0x29, 0x62, 0x6f, 0x78, 0x75, 0x56, 0x5b, 0x4c, 0x41,
      0x61, 0x6c, 0x7b, 0x76, 0x55, 0x58, 0x4f, 0x42, 0x09, 0x04, 0x13, 0x1e, 0x3d, 0x30, 0x27, 0x2a,
      0xb1, 0xbc, 0xab, 0xa6, 0x85, 0x88, 0x9f, 0x92, 0xd9, 0xd4, 0xc3, 0xce, 0xed, 0xe0, 0xf7, 0xfa,
      0xb7, 0xba, 0xad, 0xa0, 0x83, 0x8e, 0x99, 0x94, 0xdf, 0xd2, 0xc5, 0xc8, 0xeb, 0xe6, 0xf1, 0xfc,
      0x67, 0x6a, 0x7d, 0x70, 0x53, 0x5e, 0x49, 0x44, 0x0f, 0x02, 0x15, 0x18, 0x3b, 0x36, 0x21, 0x2c,
      0x0c, 0x01, 0x16, 0x1b, 0x38, 0x35, 0x22, 0x2f, 0x64, 0x69, 0x7e, 0x73, 0x50, 0x5d, 0x4a, 0x47,
      0xdc, 0xd1, 0xc6, 0xcb, 0xe8, 0xe5, 0xf2, 0xff, 0xb4, 0xb9, 0xae, 0xa3, 0x80, 0x8d, 0x9a, 0x97,
    };

    return FFMul0D[in];
  }

  static uint8_t FFMul0E(uint8_t in) {
    static const uint8_t FFMul0E[256] = {
      0x00, 0x0e, 0x1c, 0x12, 0x38, 0x36, 0x24, 0x2a, 0x70, 0x7e, 0x6c, 0x62, 0x48, 0x46, 0x54, 0x5a,
      0xe0, 0xee, 0xfc, 0xf2, 0xd8, 0xd6, 0xc4, 0xca, 0x90, 0x9e, 0x8c, 0x82, 0xa8, 0xa6, 0xb4, 0xba,
      0xdb, 0xd5, 0xc7, 0xc9, 0xe3, 0xed, 0xff, 0xf1, 0xab, 0xa5, 0xb7, 0xb9, 0x93, 0x9d, 0x8f, 0x81,
      0x3b, 0x35, 0x27, 0x29, 0x03, 0x0d, 0x1f, 0x11, 0x4b, 0x45, 0x57, 0x59, 0x73, 0x7d, 0x6f, 0x61,
      0xad, 0xa3, 0xb1, 0xbf, 0x95, 0x9b, 0x89, 0x87, 0xdd, 0xd3, 0xc1, 0xcf, 0xe5, 0xeb, 0xf9, 0xf7,
      0x4d, 0x43, 0x51, 0x5f, 0x75, 0x7b, 0x69, 0x67, 0x3d, 0x33, 0x21, 0x2f, 0x05, 0x0b, 0x19, 0x17,
      0x76, 0x78, 0x6a, 0x64, 0x4e, 0x40, 0x52, 0x5c, 0x06, 0x08, 0x1a, 0x14, 0x3e, 0x30, 0x22, 0x2c,
      0x96, 0x98, 0x8a, 0x84, 0xae, 0xa0, 0xb2, 0xbc, 0xe6, 0xe8, 0xfa, 0xf4, 0xde, 0xd0, 0xc2, 0xcc,
      0x41, 0x4f, 0x5d, 0x53, 0x79, 0x77, 0x65, 0x6b, 0x31, 0x3f, 0x2d, 0x23, 0x09, 0x07, 0x15, 0x1b,
      0xa1, 0xaf, 0xbd, 0xb3, 0x99, 0x97, 0x85, 0x8b, 0xd1, 0xdf, 0xcd, 0xc3, 0xe9, 0xe7, 0xf5, 0xfb,
      0x9a, 0x94, 0x86, 0x88, 0xa2, 0xac, 0xbe, 0xb0, 0xea, 0xe4, 0xf6, 0xf8, 0xd2, 0xdc, 0xce, 0xc0,
      0x7a, 0x74, 0x66, 0x68, 0x42, 0x4c, 0x5e, 0x50, 0x0a, 0x04, 0x16, 0x18, 0x32, 0x3c, 0x2e, 0x20,
      0xec, 0xe2, 0xf0, 0xfe, 0xd4, 0xda, 0xc8, 0xc6, 0x9c, 0x92, 0x80, 0x8e, 0xa4, 0xaa, 0xb8, 0xb6,
      0x0c, 0x02, 0x10, 0x1e, 0x34, 0x3a, 0x28, 0x26, 0x7c, 0x72, 0x60, 0x6e, 0x44, 0x4a, 0x58, 0x56,
      0x37, 0x39, 0x2b, 0x25, 0x0f, 0x01, 0x13, 0x1d, 0x47, 0x49, 0x5b, 0x55, 0x7f, 0x71, 0x63, 0x6d,
      0xd7, 0xd9, 0xcb, 0xc5, 0xef, 0xe1, 0xf3, 0xfd, 0xa7, 0xa9, 0xbb, 0xb5, 0x9f, 0x91, 0x83, 0x8d,
    };

    return FFMul0E[in];
  }

  static __uint128_t InvMixColumns(uint8_t *State) {
    uint8_t In0[16] = {
      State[0], State[4], State[8], State[12],
      State[1], State[5], State[9], State[13],
      State[2], State[6], State[10], State[14],
      State[3], State[7], State[11], State[15],
    };

    uint8_t Out0[4]{};
    uint8_t Out1[4]{};
    uint8_t Out2[4]{};
    uint8_t Out3[4]{};

    for (size_t i = 0; i < 4; ++i) {
      Out0[i] = FFMul0E(In0[0 + i]) ^ FFMul0B(In0[4 + i]) ^ FFMul0D(In0[8 + i]) ^ FFMul09(In0[12 + i]);
      Out1[i] = FFMul09(In0[0 + i]) ^ FFMul0E(In0[4 + i]) ^ FFMul0B(In0[8 + i]) ^ FFMul0D(In0[12 + i]);
      Out2[i] = FFMul0D(In0[0 + i]) ^ FFMul09(In0[4 + i]) ^ FFMul0E(In0[8 + i]) ^ FFMul0B(In0[12 + i]);
      Out3[i] = FFMul0B(In0[0 + i]) ^ FFMul0D(In0[4 + i]) ^ FFMul09(In0[8 + i]) ^ FFMul0E(In0[12 + i]);
    }

    uint8_t OutArray[16] = {
      Out0[0], Out1[0], Out2[0], Out3[0],
      Out0[1], Out1[1], Out2[1], Out3[1],
      Out0[2], Out1[2], Out2[2], Out3[2],
      Out0[3], Out1[3], Out2[3], Out3[3],
    };
    __uint128_t Res{};
    memcpy(&Res, OutArray, 16);
    return Res;
  }
}

namespace CRC32 {
  // CRC32 per byte lookup table.
  constexpr std::array<uint32_t, 256> CRC32CTable = []() consteval {
    std::array<uint32_t, 256> Table{};

    // Clang 11.x doesn't support bitreverse as a consteval
    // constexpr uint32_t Polynomial = 0x1EDC6F41;
    constexpr uint32_t PolynomialRev = 0x82F63B78; //__builtin_bitreverse32(Polynomial);

    for (size_t Char = 0; Char < std::size(Table); ++Char) {
      uint32_t CurrentChar = Char;
      for (size_t i = 0; i < 8; ++i) {
        if (CurrentChar & 1) {
          CurrentChar = (CurrentChar >> 1) ^ PolynomialRev;
        }
        else {
          CurrentChar >>= 1;
        }
      }
      Table[Char] = CurrentChar;
    }

    return Table;
  }();

  uint32_t crc32cb(uint32_t Accumulator, uint8_t data) {
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ data] ^ Accumulator >> 8;
    return Accumulator;
  }

  uint32_t crc32ch(uint32_t Accumulator, uint16_t data) {
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >> 0) & 0xFF)] ^ Accumulator >> 8;
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >> 8) & 0xFF)] ^ Accumulator >> 8;
    return Accumulator;
  }

  uint32_t crc32cw(uint32_t Accumulator, uint32_t data) {
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >>  0) & 0xFF)] ^ Accumulator >> 8;
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >>  8) & 0xFF)] ^ Accumulator >> 8;
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >> 16) & 0xFF)] ^ Accumulator >> 8;
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >> 24) & 0xFF)] ^ Accumulator >> 8;
    return Accumulator;
  }

  uint32_t crc32cx(uint32_t Accumulator, uint64_t data) {
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >>  0) & 0xFF)] ^ Accumulator >> 8;
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >>  8) & 0xFF)] ^ Accumulator >> 8;
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >> 16) & 0xFF)] ^ Accumulator >> 8;
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >> 24) & 0xFF)] ^ Accumulator >> 8;
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >> 32) & 0xFF)] ^ Accumulator >> 8;
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >> 40) & 0xFF)] ^ Accumulator >> 8;
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >> 48) & 0xFF)] ^ Accumulator >> 8;
    Accumulator = CRC32CTable[(uint8_t)Accumulator ^ ((data >> 56) & 0xFF)] ^ Accumulator >> 8;
    return Accumulator;
  }
}

namespace FEXCore::CPU {
#define DEF_OP(x) void InterpreterOps::Op_##x(IR::IROp_Header *IROp, IROpData *Data, IR::NodeID Node)

DEF_OP(AESImc) {
  auto Op = IROp->C<IR::IROp_VAESImc>();
  __uint128_t Src1 = *GetSrc<__uint128_t*>(Data->SSAData, Op->Header.Args[0]);

  // Pseudo-code
  // Dst = InvMixColumns(STATE)
  __uint128_t Tmp{};
  Tmp = AES::InvMixColumns(reinterpret_cast<uint8_t*>(&Src1));
  memcpy(GDP, &Tmp, sizeof(Tmp));
}

DEF_OP(AESEnc) {
  auto Op = IROp->C<IR::IROp_VAESEnc>();
  __uint128_t Src1 = *GetSrc<__uint128_t*>(Data->SSAData, Op->Header.Args[0]);
  __uint128_t Src2 = *GetSrc<__uint128_t*>(Data->SSAData, Op->Header.Args[1]);

  // Pseudo-code
  // STATE = Src1
  // RoundKey = Src2
  // STATE = ShiftRows(STATE)
  // STATE = SubBytes(STATE)
  // STATE = MixColumns(STATE)
  // Dst = STATE XOR RoundKey
  __uint128_t Tmp{};
  Tmp = AES::ShiftRows(reinterpret_cast<uint8_t*>(&Src1));
  Tmp = AES::SubBytes(reinterpret_cast<uint8_t*>(&Tmp), 16);
  Tmp = AES::MixColumns(reinterpret_cast<uint8_t*>(&Tmp));
  Tmp = Tmp ^ Src2;
  memcpy(GDP, &Tmp, sizeof(Tmp));
}

DEF_OP(AESEncLast) {
  auto Op = IROp->C<IR::IROp_VAESEncLast>();
  __uint128_t Src1 = *GetSrc<__uint128_t*>(Data->SSAData, Op->Header.Args[0]);
  __uint128_t Src2 = *GetSrc<__uint128_t*>(Data->SSAData, Op->Header.Args[1]);

  // Pseudo-code
  // STATE = Src1
  // RoundKey = Src2
  // STATE = ShiftRows(STATE)
  // STATE = SubBytes(STATE)
  // Dst = STATE XOR RoundKey
  __uint128_t Tmp{};
  Tmp = AES::ShiftRows(reinterpret_cast<uint8_t*>(&Src1));
  Tmp = AES::SubBytes(reinterpret_cast<uint8_t*>(&Tmp), 16);
  Tmp = Tmp ^ Src2;
  memcpy(GDP, &Tmp, sizeof(Tmp));
}

DEF_OP(AESDec) {
  auto Op = IROp->C<IR::IROp_VAESDec>();
  __uint128_t Src1 = *GetSrc<__uint128_t*>(Data->SSAData, Op->Header.Args[0]);
  __uint128_t Src2 = *GetSrc<__uint128_t*>(Data->SSAData, Op->Header.Args[1]);

  // Pseudo-code
  // STATE = Src1
  // RoundKey = Src2
  // STATE = InvShiftRows(STATE)
  // STATE = InvSubBytes(STATE)
  // STATE = InvMixColumns(STATE)
  // Dst = STATE XOR RoundKey
  __uint128_t Tmp{};
  Tmp = AES::InvShiftRows(reinterpret_cast<uint8_t*>(&Src1));
  Tmp = AES::InvSubBytes(reinterpret_cast<uint8_t*>(&Tmp));
  Tmp = AES::InvMixColumns(reinterpret_cast<uint8_t*>(&Tmp));
  Tmp = Tmp ^ Src2;
  memcpy(GDP, &Tmp, sizeof(Tmp));
}

DEF_OP(AESDecLast) {
  auto Op = IROp->C<IR::IROp_VAESDecLast>();
  __uint128_t Src1 = *GetSrc<__uint128_t*>(Data->SSAData, Op->Header.Args[0]);
  __uint128_t Src2 = *GetSrc<__uint128_t*>(Data->SSAData, Op->Header.Args[1]);

  // Pseudo-code
  // STATE = Src1
  // RoundKey = Src2
  // STATE = InvShiftRows(STATE)
  // STATE = InvSubBytes(STATE)
  // Dst = STATE XOR RoundKey
  __uint128_t Tmp{};
  Tmp = AES::InvShiftRows(reinterpret_cast<uint8_t*>(&Src1));
  Tmp = AES::InvSubBytes(reinterpret_cast<uint8_t*>(&Tmp));
  Tmp = Tmp ^ Src2;
  memcpy(GDP, &Tmp, sizeof(Tmp));
}

DEF_OP(AESKeyGenAssist) {
  auto Op = IROp->C<IR::IROp_VAESKeyGenAssist>();
  uint8_t *Src1 = GetSrc<uint8_t*>(Data->SSAData, Op->Header.Args[0]);

  // Pseudo-code
  // X3 = Src1[127:96]
  // X2 = Src1[95:64]
  // X1 = Src1[63:32]
  // X0 = Src1[31:30]
  // RCON = (Zext)rcon
  // Dest[31:0] = SubWord(X1)
  // Dest[63:32] = RotWord(SubWord(X1)) XOR RCON
  // Dest[95:64] = SubWord(X3)
  // Dest[127:96] = RotWord(SubWord(X3)) XOR RCON
  __uint128_t Tmp{};
  uint32_t X1{};
  uint32_t X3{};
  memcpy(&X1, &Src1[4], 4);
  memcpy(&X3, &Src1[12], 4);
  uint32_t SubWord_X1 = AES::SubBytes(reinterpret_cast<uint8_t*>(&X1), 4);
  uint32_t SubWord_X3 = AES::SubBytes(reinterpret_cast<uint8_t*>(&X3), 4);

  auto Ror = [] (auto In, auto R) {
    auto RotateMask = sizeof(In) * 8 - 1;
    R &= RotateMask;
    return (In >> R) | (In << (sizeof(In) * 8 - R));
  };

  uint32_t Rot_X1 = Ror(SubWord_X1, 8);
  uint32_t Rot_X3 = Ror(SubWord_X3, 8);

  Tmp = Rot_X3 ^ Op->RCON;
  Tmp <<= 32;
  Tmp |= SubWord_X3;
  Tmp <<= 32;
  Tmp |= Rot_X1 ^ Op->RCON;
  Tmp <<= 32;
  Tmp |= SubWord_X1;
  memcpy(GDP, &Tmp, sizeof(Tmp));
}

DEF_OP(CRC32) {
  auto Op = IROp->C<IR::IROp_CRC32>();
  uint32_t Src1 = *GetSrc<uint32_t*>(Data->SSAData, Op->Src1);
  uint8_t *Src2 = GetSrc<uint8_t*>(Data->SSAData, Op->Src2);
  uint32_t Tmp{};

  switch (Op->SrcSize) {
    case 1:
      Tmp = CRC32::crc32cb(Src1, *(uint8_t*)Src2);
      break;
    case 2:
      Tmp = CRC32::crc32ch(Src1, *(uint16_t*)Src2);
      break;
    case 4:
      Tmp = CRC32::crc32cw(Src1, *(uint32_t*)Src2);
      break;
    case 8:
      Tmp = CRC32::crc32cx(Src1, *(uint64_t*)Src2);
      break;
    default:
      LOGMAN_MSG_A_FMT("Unknown CRC32C size: {}", Op->SrcSize);
      break;

  }
  memcpy(GDP, &Tmp, sizeof(Tmp));
}

DEF_OP(PCLMUL) {
  auto Op = IROp->C<IR::IROp_PCLMUL>();

  const auto Selector = Op->Selector;
  auto* Dst  = GetDest<uint64_t*>(Data->SSAData, Node);
  auto* Src1 = GetSrc<uint64_t*>(Data->SSAData, Op->Src1);
  auto* Src2 = GetSrc<uint64_t*>(Data->SSAData, Op->Src2);

  const uint64_t TMP1 = (Selector & 0x01) == 0 ? Src1[0] : Src1[1];
  const uint64_t TMP2 = (Selector & 0x10) == 0 ? Src2[0] : Src2[1];
  
  const auto make_lo = [](uint64_t lhs, uint64_t rhs) {
    uint64_t result = 0;

    for (size_t i = 0; i < 64; i++) {
      if ((lhs & (1ULL << i)) != 0) {
        result ^= rhs << i;
      }
    }

    return result;
  };
  const auto make_hi = [](uint64_t lhs, uint64_t rhs) {
    uint64_t result = 0;

    for (size_t i = 1; i < 64; i++) {
      if ((lhs & (1ULL << i)) != 0) {
        result ^= rhs >> (64 - i);
      }
    }

    return result;
  };

  Dst[0] = make_lo(TMP1, TMP2);
  Dst[1] = make_hi(TMP1, TMP2);
}

#undef DEF_OP

} // namespace FEXCore::CPU
