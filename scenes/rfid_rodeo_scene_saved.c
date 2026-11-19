// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../rfid_rodeo_app.h"
#include "../rfid_rodeo_scene.h"

#include <string.h>

#define RFID_RODEO_SAVE_DIR "/ext/apps_data/rfid_rodeo"
#define RFID_RODEO_LFRFID_PREFIX "/ext/lfrfid/"
#define RFID_RODEO_EXT_PREFIX "/ext/"
// Leave room for the numeric suffix storage_get_next_filename may append.
#define RFID_RODEO_SAVE_STEM_MAX 120
#define RFID_RODEO_SAVE_NAME_MAX 200

static char rfid_rodeo_stem_char(char c) {
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' ||
       c == '-' || c == '_') {
        return c;
    }
    return '_';
}

// Save stem is the path relative to /ext/lfrfid (else /ext, else the whole
// path), extension removed, separators turned into underscores. Two keys
// that share a basename but live in different folders therefore get
// different .txt names.
static void rfid_rodeo_save_stem(const char* path, FuriString* stem) {
    const char* rel = path ? path : "";
    if(strncmp(rel, RFID_RODEO_LFRFID_PREFIX, sizeof(RFID_RODEO_LFRFID_PREFIX) - 1) == 0) {
        rel += sizeof(RFID_RODEO_LFRFID_PREFIX) - 1;
    } else if(strncmp(rel, RFID_RODEO_EXT_PREFIX, sizeof(RFID_RODEO_EXT_PREFIX) - 1) == 0) {
        rel += sizeof(RFID_RODEO_EXT_PREFIX) - 1;
    } else if(rel[0] == '/') {
        rel++;
    }

    size_t len = strlen(rel);
    if(len >= 5 && strcmp(rel + (len - 5), ".rfid") == 0) {
        len -= 5;
    }

    furi_string_reset(stem);
    for(size_t i = 0; i < len && furi_string_size(stem) < RFID_RODEO_SAVE_STEM_MAX; i++) {
        char c = rfid_rodeo_stem_char(rel[i]);
        if(c == '_' && (furi_string_empty(stem) ||
                        furi_string_get_char(stem, furi_string_size(stem) - 1) == '_')) {
            continue;
        }
        furi_string_push_back(stem, c);
    }

    while(furi_string_size(stem) > 0) {
        char tail = furi_string_get_char(stem, furi_string_size(stem) - 1);
        if(tail != '_' && tail != '.') break;
        furi_string_left(stem, furi_string_size(stem) - 1);
    }

    if(furi_string_empty(stem)) {
        furi_string_set_str(stem, "key");
    }
}

static bool rfid_rodeo_save_command(RfidRodeoApp* app, FuriString* out_path) {
    // Ensure the destination folder exists (one level at a time).
    storage_simply_mkdir(app->storage, "/ext/apps_data");
    storage_simply_mkdir(app->storage, RFID_RODEO_SAVE_DIR);

    FuriString* stem = furi_string_alloc();
    FuriString* next = furi_string_alloc();
    rfid_rodeo_save_stem(furi_string_get_cstr(app->file_path), stem);
    // "name", then "name1", "name2", ... if that file is already there.
    // FSOM_CREATE_NEW below still refuses to truncate if the helper hands
    // back a name that exists (its max_len fallback).
    storage_get_next_filename(
        app->storage,
        RFID_RODEO_SAVE_DIR,
        furi_string_get_cstr(stem),
        ".txt",
        next,
        RFID_RODEO_SAVE_NAME_MAX);
    furi_string_printf(out_path, "%s/%s.txt", RFID_RODEO_SAVE_DIR, furi_string_get_cstr(next));
    furi_string_free(next);
    furi_string_free(stem);

    File* file = storage_file_alloc(app->storage);
    bool ok = false;
    if(storage_file_open(file, furi_string_get_cstr(out_path), FSAM_WRITE, FSOM_CREATE_NEW)) {
        FuriString* body = furi_string_alloc();
        furi_string_printf(
            body,
            "# RFID Rodeo - Proxmark3 clone command\n"
            "# Source: %s\n"
            "# Key type: %s  FC: %u  CN: %u  Frame(26b): %07lX\n"
            "%s\n",
            furi_string_get_cstr(app->file_path),
            app->key_type,
            (unsigned)app->fc,
            (unsigned)app->cn,
            (unsigned long)app->frame,
            app->command);
        size_t len = furi_string_size(body);
        ok = storage_file_write(file, furi_string_get_cstr(body), len) == len;
        furi_string_free(body);
    }
    storage_file_close(file);
    storage_file_free(file);
    return ok;
}

void rfid_rodeo_scene_saved_on_enter(void* context) {
    RfidRodeoApp* app = context;
    widget_reset(app->widget);

    FuriString* path = furi_string_alloc();
    bool ok = rfid_rodeo_save_command(app, path);

    FuriString* t = app->text;
    furi_string_reset(t);
    if(ok) {
        furi_string_printf(
            t,
            "\e#Saved\e#\n"
            "Command written to:\n"
            "%s\n\n"
            "%s\n\n"
            "Back = pick another file.",
            furi_string_get_cstr(path),
            app->command);
    } else {
        furi_string_printf(
            t,
            "\e#Save failed\e#\n"
            "Could not write to:\n"
            "%s\n\n"
            "Back = pick another file.",
            furi_string_get_cstr(path));
    }
    furi_string_free(path);

    widget_add_text_scroll_element(app->widget, 0, 0, 128, 64, furi_string_get_cstr(t));
    view_dispatcher_switch_to_view(app->view_dispatcher, RfidRodeoViewWidget);
}

bool rfid_rodeo_scene_saved_on_event(void* context, SceneManagerEvent event) {
    RfidRodeoApp* app = context;
    if(event.type == SceneManagerEventTypeBack) {
        // Stack is browser -> result -> saved. One Back would otherwise land
        // on the result screen. Drop back to the browser so this matches
        // the "Back = pick another file" line on this screen.
        return scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, RfidRodeoSceneFileSelect);
    }
    return false;
}

void rfid_rodeo_scene_saved_on_exit(void* context) {
    RfidRodeoApp* app = context;
    widget_reset(app->widget);
}
