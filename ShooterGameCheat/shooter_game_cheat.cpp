// FnTest.cpp: 定义 DLL 应用程序的导出函数。
//

#include <string>
#include <Windows.h>
#include "game/game.h"
#include "game/adapt_data.h"
#include "detours/detours.h"
#include "utils.h"

namespace ShooterGameCheat{

#define ENEMY_TEXT_COLOR  FLinearColor{0.9f, 0.9f, 0.15f, 0.95f }
#define ENEMY_AMMO_COLOR  FLinearColor{0.95f, 0.0f, 0.0f, 0.95f}
#define ENEMY_RANGE_COLOR  FLinearColor{ 0.0f, 0.0f, 1.0f, 1.0f }
#define ENEMY_DEFAULT_COLOR  FLinearColor{0.4f, 0.4f, 0.4f, 1.0f}
#define ENEMY_HEALTH_COLOR  FLinearColor{1.0f, 0.0f, 0.0f, 1.0f}
#define ENEMY_SHIELD_COLOR  FLinearColor{ 0.2f, 0.7f, 0.975f, 1.0f }
#define MENU_HIGHLIGHT_COLOR FLinearColor{1.0f, 1.0f, 1.0f, 0.95f}

const float kHeadshotMinDistance = 5000.0f;
const float kFieldOfView = 3.0f;
const float kMaxAimbotDistance = 10000.0f;

using namespace Game;
using namespace Utils;

void aimbot()
{
    static uint8_t* target_player = nullptr;

    uint8_t* player_controller = get_local_player_controller(g_world);
    uint8_t* acknowledged_pawn = *(uint8_t**)(player_controller + GAME_ACKNOWLEDEGED_PAWN_OFFSET);
    if (player_controller == nullptr || acknowledged_pawn == nullptr)
    {
        return;
    }

    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
    {
        if (target_player == nullptr)
        {
            target_player = get_closest_visible_player(g_world);
        }

        float pitch = *(float*)(player_controller + GAME_CONTROLROTATION_OFFSET);
        float yaw = *(float*)(player_controller + GAME_CONTROLROTATION_OFFSET + 4);
    }
    else
    {
        target_player = nullptr;
    }

    if (target_player != nullptr)
    {
        FVector local_pos, target_pos;
        float pitch, yaw;

        get_local_player_location((uint8_t*)player_controller, &local_pos);
        get_pawn_location((uint8_t*)target_player, &target_pos);

        cal_new_rotation(local_pos, target_pos, pitch, yaw);
        set_local_player_rotation((uint8_t*)player_controller, pitch, yaw);
    }
}

void esp(uint8_t *hud)
{
    if (*g_world == nullptr)
    {
        return;
    }

    uint8_t *owning_game_instance = *(uint8_t**)(*g_world + GAME_GAMEINSTANCE_OFFSET);
    uint8_t *local_players = *(uint8_t**)(owning_game_instance + GAME_LOCALPLAYER_OFFSET);
    uint8_t *local_player = *(uint8_t**)(local_players + 0);

    uint8_t *persistent_level = *(uint8_t**)(*g_world + GAME_PERSISTENT_LEVEL_OFFSET);
    if (persistent_level != nullptr && local_player != nullptr)
    {
        uint8_t* player_controller = get_local_player_controller(g_world);
        uint8_t *local_pawn = *(uint8_t**)(player_controller + GAME_ACKNOWLEDEGED_PAWN_OFFSET);

        auto ptr = (TArray<uint8_t*>*)get_all_actors(g_world);
        TArray<uint8_t*> actors = *ptr;
        for (size_t i = 0; i < actors.Num(); i++)
        {
            uint8_t* actor = actors[i];
            if (actor == nullptr)
            {
                continue;
            }

            std::string name = get_object_name(actor);
            if (name.find("awn_C_") == std::string::npos)
            {
                continue;
            }

            int health = (int)get_pawn_health(actor);
            if (actor == local_pawn || health < 0)
            {
                continue;
            }

            FVector pawn_pos;
            FVector2D screen_pos;
            get_pawn_location(actor, &pawn_pos);

            if (!world_to_screen(player_controller, pawn_pos, &screen_pos))
            {
                continue;
            }

            FVector2D box_size = get_box_size(player_controller, pawn_pos);
            float start_x = screen_pos.X - box_size.X / 2;
            float start_y = screen_pos.Y - box_size.Y / 2;

            // 绘制方框
            draw_box(hud, start_x, start_y, box_size.X, box_size.Y, ENEMY_HEALTH_COLOR);

            //// 绘制人物名称和血量
            //wchar_t buf[100] = { 0 };
            //FString player_name = get_pawn_name(actor);
            //wsprintf(buf, L"%s|%d", player_name.c_str(), health);
            //draw_text(hud, buf, start_x - 3, start_y + box_size.Y + 1, ENEMY_HEALTH_COLOR);
        }

        aimbot();
    }
}

FVector *__fastcall get_damage_detour(void *_this, FVector *result, FVector *aimdir)
{
    FVector *output = g_get_damage_start_location_func(_this, result, aimdir);
    uint8_t* actor = get_closest_visible_player(g_world);

    if (actor != nullptr)
    {
        get_pawn_location(actor, result);
        FVector local_pos;
        uint8_t* player_controller = get_local_player_controller(g_world);
        get_local_player_location((uint8_t*)player_controller, &local_pos);

        aimdir->X = local_pos.X - result->X;
        aimdir->Y = local_pos.Y - result->Y;
        aimdir->Z = local_pos.Z - result->Z;
    }

    return output;
}

void __fastcall draw_hud_detour(void *this_)
{
    uint8_t* canvas = *(uint8_t**)((uint8_t*)this_ + GAME_CANVAS_OFFSET);
    if (canvas)
    {
        esp((uint8_t*)this_);
    }

    g_draw_hud_func(this_);
}

DWORD WINAPI init(LPVOID parm)
{
    Sleep(1000 * 5);
    
    uint64_t game_base = (uint64_t)GetModuleHandleW(L"ShooterClient-Win64-Shipping.exe");
    
    g_world = (uint8_t**)(game_base + GAME_GWORLD);
    g_engine = (uint8_t**)(game_base + GAME_GENGINE);
    g_w2s_func = (WorldToScreenType)(game_base + GAME_WORLD_TO_SCREEN_OFFSET);
    g_draw_hud_func = (DrawHUDType)(game_base + GAME_DRAW_HUD_OFFSET);
    g_get_damage_start_location_func = (GetCameraDamageStartLocationType)(game_base + GAME_GET_CAMERADAMAGESTARTLOCATION_OFFSET);
    g_line_of_sight_to_func = (LineOfSightToType)(game_base + GAME_LINEOFSIGHTTO_OFFSET);
    g_get_viewport_size_func = (GetViewportSizeType)(game_base + GAME_GET_VIEWPORT_SIZE);
    g_draw_text_func = (DrawTextType)(game_base + GAME_DRAWTEXT_OFFSET);
    g_draw_rect_func = (DrawRectType)(game_base + GAME_DRAW_RECT_OFFSET);
    g_draw_line_func = (DrawLineType)(game_base + GAME_DRAW_LINE_OFFSET);

    uint64_t name_ptr = *(uint64_t*)(game_base + GAME_NAMES_PTR);
    FName::GNames = (TNameEntryArray*)(name_ptr);

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    // 透视、自瞄
    DetourAttach(&(void*&)g_draw_hud_func, draw_hud_detour);
    // 子弹跟踪
    DetourAttach(&(void*&)g_get_damage_start_location_func, get_damage_detour);
    DetourTransactionCommit();

    return 0;
}
}