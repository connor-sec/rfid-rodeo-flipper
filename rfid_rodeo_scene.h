// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <gui/scene_manager.h>

// Single scene list. ADD_SCENE(prefix, name, id) is the only place scenes are
// named: this header turns it into the enum, and rfid_rodeo_scene.c turns the
// same list into the handler tables. Do not declare the enum by hand.
#define RFID_RODEO_SCENE_LIST                      \
    ADD_SCENE(rfid_rodeo, file_select, FileSelect) \
    ADD_SCENE(rfid_rodeo, result, Result)          \
    ADD_SCENE(rfid_rodeo, saved, Saved)

#define ADD_SCENE(prefix, name, id) RfidRodeoScene##id,
typedef enum {
    RFID_RODEO_SCENE_LIST
    RfidRodeoSceneCount,
} RfidRodeoScene;
#undef ADD_SCENE
