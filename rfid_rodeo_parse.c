// SPDX-License-Identifier: AGPL-3.0-or-later
//
// RFID Rodeo - Flipper RFID key file parsing.
// H10301 frame math lives in rfid_rodeo_h10301.c (host-testable).
// Derived from RFID Rodeo by connor-sec (AGPL-3.0):
//   https://github.com/connor-sec/RFID_Rodeo

#include "rfid_rodeo_app.h"
#include <flipper_format/flipper_format.h>
#include <toolbox/path.h>

// Momentum documentation/file_formats/LfRfidFileFormat.md (mntm-012).
#define RFID_RODEO_FILETYPE "Flipper RFID key"
#define RFID_RODEO_FILE_VERSION 1u

RfidRodeoParseStatus rfid_rodeo_parse_file(RfidRodeoApp* app) {
    const char* path = furi_string_get_cstr(app->file_path);

    // Basename without extension, for the result screen. The save file name
    // is built from the full path so same-named keys in different folders
    // do not collide (see rfid_rodeo_save_stem).
    FuriString* name = furi_string_alloc();
    path_extract_filename_no_ext(path, name);
    strncpy(app->key_name, furi_string_get_cstr(name), RFID_RODEO_NAME_SIZE - 1);
    app->key_name[RFID_RODEO_NAME_SIZE - 1] = '\0';
    furi_string_free(name);

    RfidRodeoParseStatus status = RfidRodeoParseBadFile;

    FlipperFormat* ff = flipper_format_file_alloc(app->storage);
    FuriString* filetype = furi_string_alloc();
    FuriString* key_type = furi_string_alloc();

    do {
        if(!flipper_format_file_open_existing(ff, path)) break;

        // flipper_format_file_open_existing does not check the header.
        uint32_t version = 0;
        if(!flipper_format_read_header(ff, filetype, &version)) break;
        if(furi_string_cmp_str(filetype, RFID_RODEO_FILETYPE) != 0) break;
        if(version != RFID_RODEO_FILE_VERSION) break;

        if(!flipper_format_read_string(ff, "Key type", key_type)) break;

        strncpy(app->key_type, furi_string_get_cstr(key_type), RFID_RODEO_KEYT_SIZE - 1);
        app->key_type[RFID_RODEO_KEYT_SIZE - 1] = '\0';

        if(furi_string_cmp_str(key_type, "H10301") != 0) {
            // Valid Flipper RFID key file, but not the format verified on Momentum.
            status = RfidRodeoParseUnsupported;
            break;
        }

        // read_hex fills `data_size` bytes and returns success even when the
        // line has more values (flipper_format_stream_read_value_line only
        // fails when the line is shorter). Count first and reject anything
        // other than exactly 3 bytes instead of keeping a prefix.
        uint32_t data_count = 0;
        if(!flipper_format_get_value_count(ff, "Data", &data_count)) break;
        if(data_count != 3) break;
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
    furi_string_free(filetype);
    flipper_format_free(ff);
    return status;
}
