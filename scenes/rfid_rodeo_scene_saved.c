// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../rfid_rodeo_app.h"
#include "../rfid_rodeo_scene.h"

#define RFID_RODEO_SAVE_DIR "/ext/apps_data/rfid_rodeo"

static bool rfid_rodeo_save_command(RfidRodeoApp* app, FuriString* out_path) {
    // Ensure the destination folder exists (one level at a time).
    storage_simply_mkdir(app->storage, "/ext/apps_data");
    storage_simply_mkdir(app->storage, RFID_RODEO_SAVE_DIR);

    furi_string_printf(out_path, "%s/%s.txt", RFID_RODEO_SAVE_DIR, app->key_name);

    File* file = storage_file_alloc(app->storage);
    bool ok = false;
    if(storage_file_open(
           file, furi_string_get_cstr(out_path), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        FuriString* body = furi_string_alloc();
        furi_string_printf(
            body,
            "# RFID Rodeo - Proxmark3 clone command\n"
            "# Source: %s\n"
            "# Key type: %s  FC: %u  CN: %u  Frame(26b): %07lX\n"
            "%s\n",
            app->key_name,
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
    UNUSED(context);
    UNUSED(event);
    return false;
}

void rfid_rodeo_scene_saved_on_exit(void* context) {
    RfidRodeoApp* app = context;
    widget_reset(app->widget);
}
