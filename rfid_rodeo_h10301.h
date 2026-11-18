// SPDX-License-Identifier: AGPL-3.0-or-later
//
// HID H10301 26-bit Wiegand frame. No Flipper SDK types, so a host test can
// build this file. Derived from RFID Rodeo by connor-sec (v1.4, AGPL-3.0):
//   https://github.com/connor-sec/RFID_Rodeo

#pragma once

#include <stdint.h>

// 26-bit frame packed MSB-first into the low 26 bits (print as 7 hex nibbles).
//   bit 25     : even parity over data bits 1-12  (FC[8] + top 4 bits of CN)
//   bits 24-17 : facility code (8 bits)
//   bits 16-1  : card number   (16 bits)
//   bit 0      : odd parity over data bits 13-24  (bottom 12 bits of CN)
uint32_t rfid_rodeo_h10301_frame(uint8_t fc, uint16_t cn);
