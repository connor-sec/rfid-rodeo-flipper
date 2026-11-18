// SPDX-License-Identifier: AGPL-3.0-or-later
//
// HID H10301 Wiegand parity. Same math as RFID Rodeo v1.4 (AGPL-3.0):
//   https://github.com/connor-sec/RFID_Rodeo

#include "rfid_rodeo_h10301.h"

static uint32_t rr_popcount(uint32_t x) {
    uint32_t c = 0;
    while(x) {
        c += x & 1u;
        x >>= 1;
    }
    return c;
}

uint32_t rfid_rodeo_h10301_frame(uint8_t fc, uint16_t cn) {
    uint32_t even = (rr_popcount(fc) + rr_popcount((uint32_t)((cn >> 12) & 0xF))) & 1u;
    uint32_t odd = (rr_popcount((uint32_t)(cn & 0xFFF)) & 1u) ^ 1u;
    return ((uint32_t)even << 25) | ((uint32_t)fc << 17) | ((uint32_t)cn << 1) | odd;
}
