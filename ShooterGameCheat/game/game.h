#pragma once

#include "base_struct.h"

namespace Game {

using DrawHUDType = void(__fastcall*)(void *);
using WorldToScreenType = bool(__fastcall*)(uint8_t*, FVector&, FVector2D&);
using GetCameraDamageStartLocationType = FVector * (__fastcall*) (void*, FVector*, FVector*);
using LineOfSightToType = bool(__fastcall*)(uint8_t*, uint8_t*, FVector, bool);
using GetViewportSizeType = void(__thiscall*)(uint8_t*, int*, int*);
using DrawTextType = float(__fastcall*)(uint8_t*, FString const&, FLinearColor, float, float, uint8_t*, float, bool);
using DrawRectType = void(__fastcall*)(uint8_t*, FLinearColor, float, float, float, float);
using DrawLineType = void(__fastcall*)(uint8_t*, float, float, float, float, FLinearColor, float);

extern uint8_t** g_engine;
extern uint8_t** g_world;
extern WorldToScreenType g_w2s_func;
extern DrawHUDType g_draw_hud_func;
extern GetCameraDamageStartLocationType g_get_damage_start_location_func;
extern LineOfSightToType g_line_of_sight_to_func;
extern GetViewportSizeType g_get_viewport_size_func;
extern DrawTextType g_draw_text_func;
extern DrawRectType g_draw_rect_func;
extern DrawLineType g_draw_line_func;

uint8_t *get_local_player_controller(uint8_t **world);
bool is_local_player(uint8_t **world, uint8_t* actor);
uint8_t* get_pawn_state(uint8_t *pawn);
float get_pawn_health(uint8_t *pawn);
FString get_pawn_name(uint8_t *pawn);
bool get_pawn_location(uint8_t *pawn, FVector *location);
void get_local_player_location(uint8_t *controller, FVector *location);
uint8_t *get_all_actors(uint8_t **world);
std::string get_object_name(uint8_t *obj);
void draw_text(uint8_t *hud, const FString& text, float x, float y, const FLinearColor& color);
uint8_t* get_closest_visible_player(uint8_t** world);
void cal_new_rotation(FVector local_pos, FVector target_pos, float &pitch, float &yaw);
void set_local_player_rotation(uint8_t *controller, float &pitch, float &yaw);

void draw_box(uint8_t *hud, float a, float b, float c, float d, FLinearColor color);

FVector2D get_box_size(uint8_t *player_controller, FVector target_pos);
bool world_to_screen(uint8_t *player_controller, FVector target_pos, FVector2D* screen_pos);
}