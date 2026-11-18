// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../rfid_rodeo_app.h"
#include "../rfid_rodeo_scene.h"
#include <gui/modules/widget_elements/widget_element.h>

static void rfid_rodeo_result_button_cb(GuiButtonType result, InputType type, void* context) {
    RfidRodeoApp* app = context;
    if(type != InputTypeShort) return;
    if(result == GuiButtonTypeRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, RfidRodeoEventSave);
    } else if(result == GuiButtonTypeLeft) {
        view_dispatcher_send_custom_event(app->view_dispatcher, RfidRodeoEventPick);
    }
}

void rfid_rodeo_scene_result_on_enter(void* context) {
    RfidRodeoApp* app = context;
    widget_reset(app->widget);

    FuriString* t = app->text;
    furi_string_reset(t);

    if(app->status == RfidRodeoParseOk) {
        furi_string_printf(
            t,
            "\e#%s\e#\n"
            "Key type: %s\n"
            "FC: %u   CN: %u\n"
            "Payload: %02X %02X %02X\n"
            "Frame(26b): %07lX\n"
            "\e#Proxmark:\e#\n"
            "%s\n",
            app->key_name,
            app->key_type,
            (unsigned)app->fc,
            (unsigned)app->cn,
            app->data[0],
            app->data[1],
            app->data[2],
            (unsigned long)app->frame,
            app->command);

        widget_add_text_scroll_element(app->widget, 0, 0, 128, 52, furi_string_get_cstr(t));
        widget_add_button_element(
            app->widget, GuiButtonTypeLeft, "Pick", rfid_rodeo_result_button_cb, app);
        widget_add_button_element(
            app->widget, GuiButtonTypeRight, "Save", rfid_rodeo_result_button_cb, app);
    } else if(app->status == RfidRodeoParseUnsupported) {
        furi_string_printf(
            t,
            "\e#Unsupported: H10301 only\e#\n"
            "File: %s\n"
            "Key type: %s\n\n"
            "EM4100, Indala26, AWID,\n"
            "Pyramid and others store\n"
            "credential data differently\n"
            "on disk. Auto-converting\n"
            "them is not safe, so this\n"
            "tool refuses to guess.\n",
            app->key_name,
            app->key_type);

        widget_add_text_scroll_element(app->widget, 0, 0, 128, 52, furi_string_get_cstr(t));
        widget_add_button_element(
            app->widget, GuiButtonTypeLeft, "Pick", rfid_rodeo_result_button_cb, app);
    } else {
        furi_string_printf(
            t,
            "\e#Could not read file\e#\n"
            "File: %s\n\n"
            "Need a Flipper RFID key\n"
            "header (Filetype and\n"
            "Version: 1). H10301 Data\n"
            "must be exactly 3 bytes.\n",
            app->key_name);

        widget_add_text_scroll_element(app->widget, 0, 0, 128, 52, furi_string_get_cstr(t));
        widget_add_button_element(
            app->widget, GuiButtonTypeLeft, "Pick", rfid_rodeo_result_button_cb, app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, RfidRodeoViewWidget);
}

bool rfid_rodeo_scene_result_on_event(void* context, SceneManagerEvent event) {
    RfidRodeoApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == RfidRodeoEventSave && app->status == RfidRodeoParseOk) {
            scene_manager_next_scene(app->scene_manager, RfidRodeoSceneSaved);
            return true;
        }
        if(event.event == RfidRodeoEventPick) {
            // Go back to the file browser (replace this scene).
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, RfidRodeoSceneFileSelect);
            return true;
        }
    }
    return false;
}

void rfid_rodeo_scene_result_on_exit(void* context) {
    RfidRodeoApp* app = context;
    widget_reset(app->widget);
}
