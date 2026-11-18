// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

// Scene list for the RFID Rodeo app. ADD_SCENE(prefix, name, Name) is expanded
// by scene_manager helpers into enum values, handler externs and arrays.
#define RFID_RODEO_SCENE_LIST                                 \
    ADD_SCENE(rfid_rodeo, file_select, FileSelect)            \
    ADD_SCENE(rfid_rodeo, result,      Result)                \
    ADD_SCENE(rfid_rodeo, saved,       Saved)

#include <gui/scene_manager.h>
