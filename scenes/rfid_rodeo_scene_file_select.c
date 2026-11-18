// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../rfid_rodeo_app.h"
#include "../rfid_rodeo_scene.h"

#define RFID_RODEO_LFRFID_FOLDER "/ext/lfrfid"
#define RFID_RODEO_FILE_EXTENSION ".rfid"

void rfid_rodeo_scene_file_select_on_enter(void* context) {
    RfidRodeoApp* app = context;

    DialogsFileBrowserOptions options;
    dialog_file_browser_set_basic_options(&options, RFID_RODEO_FILE_EXTENSION, NULL);
    options.base_path = RFID_RODEO_LFRFID_FOLDER;
    options.hide_ext = false;
    options.skip_assets = true;

    // Preselect inside /ext/lfrfid; input/views are owned by the file browser.
    furi_string_set(app->file_path, RFID_RODEO_LFRFID_FOLDER);

    bool selected =
        dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &options);

    if(!selected) {
        // No file chosen -> exit the app.
        view_dispatcher_stop(app->view_dispatcher);
        return;
    }

    app->status = rfid_rodeo_parse_file(app);
    scene_manager_next_scene(app->scene_manager, RfidRodeoSceneResult);
}

bool rfid_rodeo_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void rfid_rodeo_scene_file_select_on_exit(void* context) {
    UNUSED(context);
}
