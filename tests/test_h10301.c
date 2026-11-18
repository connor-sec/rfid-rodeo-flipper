// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Host-side check of rfid_rodeo_h10301_frame. Not part of the FAP.
//
// Oracles:
//   README example: FC 167 / CN 34052 -> frame 14F0A08
//   RFID Rodeo v1.4 tests/vectors.json, Proxmark HIDPack (preamble false),
//   proxmark commit 0bf02faa49649e51e07896e42943beaf0a38e334:
//     H10301 FC 170 CN 43690 -> 01010101010101010101010101
//     H10301 FC 85  CN 21845 -> 00101010101010101010101011

#include "../rfid_rodeo_h10301.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t frame_from_bits(const char* bits) {
    uint32_t frame = 0;
    for(const char* p = bits; *p; p++) {
        frame = (frame << 1) | (uint32_t)(*p - '0');
    }
    return frame;
}

static int expect_frame(const char* name, uint8_t fc, uint16_t cn, uint32_t want) {
    uint32_t got = rfid_rodeo_h10301_frame(fc, cn);
    if(got != want) {
        fprintf(
            stderr,
            "FAIL %s: FC %u CN %u got %07X want %07X\n",
            name,
            (unsigned)fc,
            (unsigned)cn,
            (unsigned)got,
            (unsigned)want);
        return 1;
    }
    printf("ok %s %07X\n", name, (unsigned)got);
    return 0;
}

static int expect_parity(const char* name, uint8_t fc, uint16_t cn) {
    uint32_t frame = rfid_rodeo_h10301_frame(fc, cn);
    // Transmission bit 0 is uint32 bit 25. First 13 bits even, last 13 odd.
    uint32_t head = (frame >> 13) & 0x1FFFu;
    uint32_t tail = frame & 0x1FFFu;
    uint32_t head_bits = 0;
    uint32_t tail_bits = 0;
    for(uint32_t bit = head; bit; bit >>= 1) head_bits += bit & 1u;
    for(uint32_t bit = tail; bit; bit >>= 1) tail_bits += bit & 1u;
    if((head_bits & 1u) != 0u || (tail_bits & 1u) != 1u) {
        fprintf(
            stderr,
            "FAIL %s parity: frame %07X head_pop %u tail_pop %u\n",
            name,
            (unsigned)frame,
            (unsigned)head_bits,
            (unsigned)tail_bits);
        return 1;
    }
    return 0;
}

static int expect_bits(const char* name, uint8_t fc, uint16_t cn, const char* bits) {
    if(strlen(bits) != 26) {
        fprintf(stderr, "FAIL %s: bit string is %zu chars, want 26\n", name, strlen(bits));
        return 1;
    }
    return expect_frame(name, fc, cn, frame_from_bits(bits));
}

int main(void) {
    int failed = 0;

    failed += expect_frame("readme FC 167 CN 34052", 167, 34052, 0x14F0A08u);
    failed += expect_parity("readme FC 167 CN 34052", 167, 34052);

    // README's second worked example is also Proxmark H10301 vector 0.
    failed += expect_bits(
        "proxmark H10301 FC 170 CN 43690", 170, 43690, "01010101010101010101010101");
    failed += expect_parity("proxmark H10301 FC 170 CN 43690", 170, 43690);
    failed += expect_bits(
        "proxmark H10301 FC 85 CN 21845", 85, 21845, "00101010101010101010101011");
    failed += expect_parity("proxmark H10301 FC 85 CN 21845", 85, 21845);

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
