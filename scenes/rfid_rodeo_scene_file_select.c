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
        //
        // TODO(device, Momentum mntm-012): confirm cancel on first launch
        // leaves the app, and does not sit on a blank screen. This was not
        // built or run on a Flipper.
        //
        // On first launch, scene_manager_next_scene() runs this on_enter
        // before view_dispatcher_run(). view_dispatcher_stop() posts
        // FuriEventLoopFlagStop on the loop created in view_dispatcher_alloc()
        // (applications/services/gui/view_dispatcher.c, furi/core/event_loop.c
        // at tag mntm-012). furi_event_loop_run() waits with
        // ulBitsToClearOnEntry = 0, so a stop posted before run stays pending
        // and run() should return immediately. Bad USB's file-select scene
        // uses the same stop-from-on_enter pattern. If a device test shows a
        // blank screen, do not call stop until the dispatcher is running:
        // skip view_dispatcher_run() when the first browser is cancelled.
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
