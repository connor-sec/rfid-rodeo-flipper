// SPDX-License-Identifier: AGPL-3.0-or-later
//
// RFID Rodeo (Flipper app) - app-wide definitions.
// Derived from RFID Rodeo by connor-sec:
//   https://github.com/connor-sec/RFID_Rodeo  (v1.4, AGPL-3.0)
// See LICENSE note in this folder.

#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/widget.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

// ── Scene / View IDs ──────────────────────────────────────────────────────────

typedef enum {
    RfidRodeoSceneFileSelect = 0, // open the dialogs file browser
    RfidRodeoSceneResult,         // show parsed credential + Proxmark command
    RfidRodeoSceneSaved,          // confirmation after saving the command
    RfidRodeoSceneCount,
} RfidRodeoScene;

typedef enum {
    RfidRodeoViewWidget = 0, // shared widget for result + saved scenes
} RfidRodeoView;

// Custom events emitted by widget button callbacks
typedef enum {
    RfidRodeoEventSave = 0, // "Save" pressed on result screen
    RfidRodeoEventPick,     // "Pick file" pressed (go back to browser)
} RfidRodeoCustomEvent;

// ── Parse result ──────────────────────────────────────────────────────────────

typedef enum {
    RfidRodeoParseOk = 0,       // H10301, 3-byte payload, command built
    RfidRodeoParseUnsupported,  // valid file, Key type != H10301
    RfidRodeoParseBadFile,      // could not read file / fields
} RfidRodeoParseStatus;

#define RFID_RODEO_CMD_SIZE  96
#define RFID_RODEO_NAME_SIZE 64
#define RFID_RODEO_KEYT_SIZE 32

// ── Application struct ─────────────────────────────────────────────────────────

typedef struct {
    // GUI
    Gui*            gui;
    ViewDispatcher* view_dispatcher;
    SceneManager*   scene_manager;
    Widget*         widget;

    // Services
    DialogsApp*     dialogs;
    Storage*        storage;

    // Selected file path (full /ext/... path) returned by the browser
    FuriString*     file_path;

    // Parse results
    RfidRodeoParseStatus status;
    char     key_type[RFID_RODEO_KEYT_SIZE];
    char     key_name[RFID_RODEO_NAME_SIZE]; // file name without extension
    uint8_t  data[3];                        // raw payload bytes (H10301)
    uint8_t  fc;                             // facility code
    uint16_t cn;                             // card number
    uint32_t frame;                          // 26-bit Wiegand frame
    char     command[RFID_RODEO_CMD_SIZE];   // Proxmark clone command

    // Scrollable text buffer for the result/saved widget
    FuriString* text;
} RfidRodeoApp;

// Implemented in rfid_rodeo_parse.c
RfidRodeoParseStatus rfid_rodeo_parse_file(RfidRodeoApp* app);
uint32_t rfid_rodeo_h10301_frame(uint8_t fc, uint16_t cn);

// Implemented in rfid_rodeo_scene.c
const SceneManagerHandlers* rfid_rodeo_get_scene_handlers(void);
