// SPDX-License-Identifier: AGPL-3.0-or-later
//
// RFID Rodeo - Flipper Zero external app.
//
// Reads a saved Flipper RFID dump (.rfid) on-device and outputs the equivalent
// Proxmark3 clone command. Scope: HID H10301 (26-bit), the one firmware-verified
// on-disk mapping. All other key types are reported as unsupported rather than
// guessed.
//
// Derived from RFID Rodeo by connor-sec (v1.4, AGPL-3.0):
//   https://github.com/connor-sec/RFID_Rodeo
// The original is a host-side Wiegand parity calculator; this app performs the
// equivalent H10301 parity math on the Flipper itself.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Affero General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version. See the LICENSE note in this folder.

#include "rfid_rodeo_app.h"
#include "rfid_rodeo_scene.h"

static bool rfid_rodeo_custom_event_cb(void* context, uint32_t event) {
    RfidRodeoApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool rfid_rodeo_back_event_cb(void* context) {
    RfidRodeoApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static RfidRodeoApp* rfid_rodeo_app_alloc(void) {
    RfidRodeoApp* app = malloc(sizeof(RfidRodeoApp));
    memset(app, 0, sizeof(RfidRodeoApp));

    app->file_path = furi_string_alloc();
    app->text = furi_string_alloc();

    // Records / services
    app->gui = furi_record_open(RECORD_GUI);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->storage = furi_record_open(RECORD_STORAGE);

    // Scene manager + view dispatcher
    app->scene_manager = scene_manager_alloc(rfid_rodeo_get_scene_handlers(), app);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, rfid_rodeo_custom_event_cb);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, rfid_rodeo_back_event_cb);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Shared widget view
    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, RfidRodeoViewWidget, widget_get_view(app->widget));

    return app;
}

static void rfid_rodeo_app_free(RfidRodeoApp* app) {
    furi_assert(app);

    view_dispatcher_remove_view(app->view_dispatcher, RfidRodeoViewWidget);
    widget_free(app->widget);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_GUI);

    furi_string_free(app->text);
    furi_string_free(app->file_path);

    free(app);
}

int32_t rfid_rodeo_app(void* p) {
    UNUSED(p);
    RfidRodeoApp* app = rfid_rodeo_app_alloc();

    scene_manager_next_scene(app->scene_manager, RfidRodeoSceneFileSelect);
    view_dispatcher_run(app->view_dispatcher);

    rfid_rodeo_app_free(app);
    return 0;
}
