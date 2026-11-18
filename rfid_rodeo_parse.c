// SPDX-License-Identifier: AGPL-3.0-or-later
//
// RFID Rodeo - file parsing + HID H10301 Wiegand math.
// Derived from RFID Rodeo by connor-sec (AGPL-3.0):
//   https://github.com/connor-sec/RFID_Rodeo

#include "rfid_rodeo_app.h"
#include <flipper_format/flipper_format.h>
#include <toolbox/path.h>

// Count set bits
static uint32_t rr_popcount(uint32_t x) {
    uint32_t c = 0;
    while(x) {
        c += x & 1u;
        x >>= 1;
    }
    return c;
}

// Build the 26-bit HID H10301 Wiegand frame.
//   bit 0      : even parity over data bits 1-12  (FC[8] + top 4 bits of CN)
//   bits 1-8   : facility code (8 bits, MSB first)
//   bits 9-24  : card number   (16 bits, MSB first)
//   bit 25     : odd parity over data bits 13-24  (bottom 12 bits of CN)
// Packed MSB-first; printed as 7 hex nibbles (zero padded to 28 bits).
uint32_t rfid_rodeo_h10301_frame(uint8_t fc, uint16_t cn) {
    uint32_t even = (rr_popcount(fc) + rr_popcount((uint32_t)((cn >> 12) & 0xF))) & 1u;
    uint32_t odd = (rr_popcount((uint32_t)(cn & 0xFFF)) & 1u) ^ 1u;
    return ((uint32_t)even << 25) | ((uint32_t)fc << 17) | ((uint32_t)cn << 1) | odd;
}

RfidRodeoParseStatus rfid_rodeo_parse_file(RfidRodeoApp* app) {
    const char* path = furi_string_get_cstr(app->file_path);

    // Remember the file name (without extension) for the save path / display.
    FuriString* name = furi_string_alloc();
    path_extract_filename_no_ext(path, name);
    strncpy(app->key_name, furi_string_get_cstr(name), RFID_RODEO_NAME_SIZE - 1);
    app->key_name[RFID_RODEO_NAME_SIZE - 1] = '\0';
    furi_string_free(name);

    RfidRodeoParseStatus status = RfidRodeoParseBadFile;

    FlipperFormat* ff = flipper_format_file_alloc(app->storage);
    FuriString* key_type = furi_string_alloc();

    do {
        if(!flipper_format_file_open_existing(ff, path)) break;
        if(!flipper_format_read_string(ff, "Key type", key_type)) break;

        strncpy(app->key_type, furi_string_get_cstr(key_type), RFID_RODEO_KEYT_SIZE - 1);
        app->key_type[RFID_RODEO_KEYT_SIZE - 1] = '\0';

        if(furi_string_cmp_str(key_type, "H10301") != 0) {
            // Valid Flipper RFID file, but not the one firmware-verified format.
            status = RfidRodeoParseUnsupported;
            break;
        }

        // H10301 payload is exactly 3 bytes.
        if(!flipper_format_read_hex(ff, "Data", app->data, 3)) break;

        app->fc = app->data[0];
        app->cn = (uint16_t)(((uint16_t)app->data[1] << 8) | app->data[2]);
        app->frame = rfid_rodeo_h10301_frame(app->fc, app->cn);

        snprintf(
            app->command,
            RFID_RODEO_CMD_SIZE,
            "lf hid clone -w H10301 --fc %u --cn %u",
            (unsigned)app->fc,
            (unsigned)app->cn);

        status = RfidRodeoParseOk;
    } while(false);

    furi_string_free(key_type);
    flipper_format_free(ff);
    return status;
}
