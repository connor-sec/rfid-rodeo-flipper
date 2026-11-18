// SPDX-License-Identifier: AGPL-3.0-or-later
#include "rfid_rodeo_app.h"
#include "rfid_rodeo_scene.h"

// Forward-declare each scene's handlers
#define ADD_SCENE(prefix, name, Name)                              \
    extern void prefix##_scene_##name##_on_enter(void*);           \
    extern bool prefix##_scene_##name##_on_event(void*, SceneManagerEvent); \
    extern void prefix##_scene_##name##_on_exit(void*);
RFID_RODEO_SCENE_LIST
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, Name) prefix##_scene_##name##_on_enter,
static const AppSceneOnEnterCallback rfid_rodeo_on_enter_handlers[] = {
    RFID_RODEO_SCENE_LIST};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, Name) prefix##_scene_##name##_on_event,
static const AppSceneOnEventCallback rfid_rodeo_on_event_handlers[] = {
    RFID_RODEO_SCENE_LIST};
#undef ADD_SCENE

#define ADD_SCENE(prefix, name, Name) prefix##_scene_##name##_on_exit,
static const AppSceneOnExitCallback rfid_rodeo_on_exit_handlers[] = {
    RFID_RODEO_SCENE_LIST};
#undef ADD_SCENE

static const SceneManagerHandlers rfid_rodeo_scene_handlers = {
    .on_enter_handlers = rfid_rodeo_on_enter_handlers,
    .on_event_handlers = rfid_rodeo_on_event_handlers,
    .on_exit_handlers  = rfid_rodeo_on_exit_handlers,
    .scene_num         = RfidRodeoSceneCount,
};

const SceneManagerHandlers* rfid_rodeo_get_scene_handlers(void) {
    return &rfid_rodeo_scene_handlers;
}
