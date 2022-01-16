
#include "game.h"
#include "adapt_data.h"
#include "../utils.h"
#undef max
TNameEntryArray* FName::GNames = nullptr;

namespace Game {

uint8_t** g_engine = nullptr;
uint8_t** g_world = nullptr;

WorldToScreenType g_w2s_func = nullptr;
DrawHUDType g_draw_hud_func = nullptr;
GetCameraDamageStartLocationType g_get_damage_start_location_func = nullptr;
LineOfSightToType g_line_of_sight_to_func = nullptr;
GetViewportSizeType g_get_viewport_size_func = nullptr;
DrawTextType g_draw_text_func = nullptr;
DrawRectType g_draw_rect_func = nullptr;
DrawLineType g_draw_line_func = nullptr;

const double PI = 3.1415926535;

uint8_t *get_local_player_controller(uint8_t** world)
{
    if (*world == nullptr)
    {
        return nullptr;
    }

    uint8_t *owning_game_instance = *(uint8_t**)(*world + GAME_GAMEINSTANCE_OFFSET);
    uint8_t *local_players = *(uint8_t**)(owning_game_instance + GAME_LOCALPLAYER_OFFSET);
    uint8_t *local_player = *(uint8_t**)(local_players + 0);
    uint8_t *player_controller = *(uint8_t**)(local_player + GAME_PLAYERCONTROLLER_OFFSET);

    return player_controller;
}


// 获取玩家控制的pawn对象指针
uint8_t* get_local_pawn(uint8_t** world)
{
    if (!*world)
    {
        return nullptr;
    }

    uint8_t *owning_game_instance = *(uint8_t**)(*world + GAME_GAMEINSTANCE_OFFSET);
    uint8_t* local_players = *(uint8_t**)(owning_game_instance + GAME_LOCALPLAYER_OFFSET);
    uint8_t* local_player = *(uint8_t**)(local_players + 0);
    uint8_t *player_controller = *(uint8_t**)(local_player + GAME_PLAYERCONTROLLER_OFFSET);
    uint8_t *local_pawn = *(uint8_t**)(player_controller + GAME_ACKNOWLEDEGED_PAWN_OFFSET);
    return local_pawn;
}

bool is_local_player(uint8_t **world, uint8_t* actor)
{
    uint8_t *player_controller = get_local_player_controller(world);
    uint8_t *acknowledged_pawn = *(uint8_t**)(player_controller + GAME_ACKNOWLEDEGED_PAWN_OFFSET);
    if (acknowledged_pawn == nullptr)
    {
        return true;
    }
    return (actor == acknowledged_pawn);
}

uint8_t* get_pawn_state(uint8_t *pawn)
{
    uint8_t **state_addr = (uint8_t**)(pawn + GAME_PAWN_PLAYERSTATE_OFFSET);
    if (state_addr && !IsBadReadPtr(state_addr, sizeof(uint8_t*)))
    {
        return *state_addr;
    }

    return 0;
}

float get_pawn_health(uint8_t *pawn)
{
    float* heath_addr = (float*)(pawn + GAME_PAWN_HEALTH_OFFSET);
    if (heath_addr)
    {
        return *heath_addr;
    }

    return 0;
}

FString get_pawn_name(uint8_t *pawn)
{
    uint8_t* player_state = get_pawn_state(pawn);
    if (player_state)
    {
        return *(FString*)(player_state + GAME_PAWN_NAME);
    }

    return L"";
}

bool get_pawn_location(uint8_t *pawn, FVector *location)
{
    uint8_t *root_commoent = *(uint8_t**)(pawn + GAME_PAWN_ROOTCOMMENT);
    if (root_commoent)
    {
        FVector relative_location = *(FVector*)(root_commoent + GAME_PAWN_LOCATION);
        location->X = relative_location.X;
        location->Y = relative_location.Y;
        location->Z = relative_location.Z;
        return true;
    }

    return false;
}

void get_local_player_location(uint8_t *controller, FVector *location)
{
    uint8_t *local_pawn = *(uint8_t**)(controller + GAME_ACKNOWLEDEGED_PAWN_OFFSET);
    if (local_pawn)
    {
        get_pawn_location(local_pawn, location);
    }
}

uint8_t *get_all_actors(uint8_t **world)
{
    if (*world == nullptr)
    {
        return nullptr;
    }

    uint8_t *persistent_level = *(uint8_t**)(*world + GAME_PERSISTENT_LEVEL_OFFSET);
    uint8_t *actors = (uint8_t*)(persistent_level + GAME_ALL_ACTORS_OFFSET);
    return actors;
}

std::vector<uint8_t*> get_other_pawns(uint8_t** world)
{
    uint8_t *persistent_level = *(uint8_t**)(*world + GAME_PERSISTENT_LEVEL_OFFSET);
    uint8_t *actors = (uint8_t*)(persistent_level + GAME_ALL_ACTORS_OFFSET);
    TArray<uint8_t*> actor_array = *(TArray<uint8_t*>*)actors;

    std::vector<uint8_t*> pawns;
    for (size_t i = 0; i < actor_array.Num(); i++)
    {
        uint8_t* actor = actor_array[i];
        if (actor == nullptr)
        {
            continue;
        }

        std::string name = get_object_name(actor);
        if (name.find("awn_C_") == std::string::npos)
        {
            continue;
        }

        pawns.push_back(actor);
    }

    return pawns;
}

std::string get_object_name(uint8_t *obj)
{
    FName *f_name = (FName*)(obj + GAME_OBJ_NAME_OFFSET);
    if (IsBadReadPtr(f_name, sizeof(FName)))
    {
        return "";
    }

    std::string name(f_name->GetName());
    if (f_name->Number > 0)
    {
        name += '_' + std::to_string(f_name->Number);
    }

    auto pos = name.rfind('/');
    if (pos == std::string::npos)
    {
        return name;
    }

    return name.substr(pos + 1);
}

void draw_text(uint8_t *hud, const FString& text, float x, float y, const FLinearColor& color)
{
    uint8_t *font = *(uint8_t**)(*g_engine + GAME_MEDIUM_FONT);
    g_draw_text_func(hud, text, color, x, y, font, 1.0, false);
}

void draw_box(uint8_t *hud, float x, float y, float width, float height, FLinearColor color)
{
    g_draw_line_func(hud, x, y, x + width, y, color, 1);
    g_draw_line_func(hud, x, y, x, y + height, color, 1);
    g_draw_line_func(hud, x + width, y, x + width, y + height, color, 1);
    g_draw_line_func(hud, x, y + height, x + width, y + height, color, 1);
}

uint8_t* get_closest_visible_player(uint8_t** world)
{
    std::vector<uint8_t*> actors = get_other_pawns(world);
    uint8_t* player_controller = get_local_player_controller(world);

    int screen_size_x, screen_size_y;
    g_get_viewport_size_func((uint8_t*)player_controller, &screen_size_x, &screen_size_y);

    FVector2D center_screen{ (float)screen_size_x / 2, (float)screen_size_y / 2 };

    uint8_t* closest_player = nullptr;
    float distance = 300.0f;
    for (auto &candidate : actors)
    {
        FVector pawn_pos;
        get_pawn_location((uint8_t*)candidate, &pawn_pos);

        FVector2D screen_pos;
        if (world_to_screen(player_controller, pawn_pos, &screen_pos))
        {
            float tmp_distance = Utils::get_distance_2d(center_screen, screen_pos);
            if (tmp_distance < 24.0f)
            {
                closest_player = candidate;
                break;
            }
            else if (tmp_distance < distance)
            {
                closest_player = candidate;
                distance = tmp_distance;
            }
        }
    }

    return closest_player;
}
//
//uint8_t* get_closest_visible_player(float max_distance, bool ignore_walls)
//{
//    FVector local_pos;
//    uint8_t *player_controller = get_local_player_controller();
//
//    if (player_controller != nullptr)
//    {
//        get_local_player_location((uint8_t*)player_controller, &local_pos);
//    }
//    else
//    {
//        return nullptr;
//    }
//
//    uint8_t* closest_player = nullptr;
//    std::vector<uint8_t*> candidates;
//    TArray<uint8_t*> actors = *(TArray<uint8_t*>*)get_all_actors();
//
//    for (int i = 0; i < actors.Num(); i++)
//    {
//        uint8_t* actor = actors[i];
//        if (actor == nullptr || *(uint8_t**)(actor + GAME_PAWN_ROOTCOMMENT) == nullptr || is_local_player(actor))
//        {
//            continue;
//        }
//
//        if (get_object_name(actor).find("awn_C_") == std::string::npos)
//        {
//            continue;
//        }
//
//        FVector zero{ 0.0f, 0.0f, 0.0f };
//        if (!ignore_walls && !g_line_of_sight_to_func((uint8_t*)player_controller, actor, zero, false))
//        {
//            continue;
//        }
//
//        if (get_pawn_health(actor) <= 0.0f)
//        {
//            continue;
//        }
//
//        candidates.push_back(actor);
//    }
//
//    float distance = std::numeric_limits<float>::max();
//    if (!ignore_walls)
//    {
//        for (auto candidate : candidates)
//        {
//            FVector pawn_pos;
//            get_pawn_location((uint8_t*)candidate, &pawn_pos);
//            float cur_dist = Utils::get_distance_3d(local_pos, pawn_pos);
//
//            if (cur_dist < distance && cur_dist < max_distance)
//            {
//                distance = cur_dist;
//                closest_player = candidate;
//            }
//        }
//    }
//
//    int screen_size_x, screen_size_y;
//    g_get_viewport_size_func((uint8_t*)player_controller, &screen_size_x, &screen_size_y);
//
//    FVector2D center_screen{ (float)screen_size_x / 2, (float)screen_size_y / 2 };
//
//    distance = 300.0f;
//    for (auto candidate : candidates)
//    {
//        FVector pawn_pos;
//        get_pawn_location((uint8_t*)candidate, &pawn_pos);
//
//        FVector2D screen_pos;
//        if (g_w2s_func(player_controller, pawn_pos, screen_pos))
//        {
//            float dist = Utils::get_distance_2d(center_screen, screen_pos);
//            if (dist < 24.0f)
//            {
//                closest_player = candidate;
//                break;
//            }
//            else if (dist < distance)
//            {
//                closest_player = candidate;
//                distance = dist;
//            }
//        }
//    }
//
//    return closest_player;
//}

void cal_new_rotation(FVector local_pos, FVector target_pos, float &pitch, float &yaw)
{
    float dx = target_pos.X - local_pos.X;
    float dy = target_pos.Y - local_pos.Y;
    float dz = target_pos.Z - local_pos.Z;
    const float pi_2 = 1.5707963f;

    const float yaw_range = 360.0;
    const float pitch_range = 90.0;

    pitch = atan2(dz, sqrt(dx * dx + dy * dy)) * (pitch_range / pi_2);
    yaw = atan2(dy, dx) * yaw_range / 4 / pi_2;
    if (pitch < 0.0f)
    {
        pitch += 360.0;
    }

    if (yaw < 0.0f)
    {
        yaw += 360.0;
    }
}

void set_local_player_rotation(uint8_t *controller, float &pitch, float &yaw)
{
    *(float*)(controller + GAME_CONTROLROTATION_OFFSET) = pitch;
    *(float*)(controller + GAME_CONTROLROTATION_OFFSET + 4) = yaw;
}

void get_local_player_rotation(uint8_t **world, float *pitch, float *yaw)
{
    uint8_t *owning_game_instance = *(uint8_t**)(*world + GAME_GAMEINSTANCE_OFFSET);
    uint8_t *local_players = *(uint8_t**)(owning_game_instance + GAME_LOCALPLAYER_OFFSET);
    uint8_t *local_player = *(uint8_t**)(local_players + 0);
    uint8_t *player_controller = *(uint8_t**)(local_player + GAME_PLAYERCONTROLLER_OFFSET);

    *pitch = *(float*)(player_controller + GAME_CONTROLROTATION_OFFSET);
    *yaw = *(float*)(player_controller + GAME_CONTROLROTATION_OFFSET + 4);
}

FVector matrix_transform(FVector target_pos, FVector rotation, FVector camera_pos)
{
    
    float rad_pitch = (rotation.X * float(PI) / 180.f);
    float rad_yaw = (rotation.Y * float(PI) / 180.f);
    float rad_roll = (rotation.Z * float(PI) / 180.f);

    float sp = sinf(rad_pitch);
    float cp = cosf(rad_pitch);
    float sy = sinf(rad_yaw);
    float cy = cosf(rad_yaw);

    FVector axis_x, axis_y, axis_z;

    axis_x = FVector(-sy, cy, 0);
    axis_y = FVector(-sp * cy, -sp * sy, cp);
    axis_z = FVector(cp * cy, cp * sy, sp);

    FVector delta = target_pos - camera_pos;
    FVector transformed = FVector(delta.Dot(axis_x), delta.Dot(axis_y), delta.Dot(axis_z));
    return transformed;
}

FVector2D get_box_size(uint8_t *player_controller, FVector target_pos)
{
    int window_width, window_height;
    g_get_viewport_size_func((uint8_t*)player_controller, &window_width, &window_height);

    uint8_t *camera_mgr = *(uint8_t**)(player_controller + GAME_PLAYER_CAMERA_MANAGER);
    FVector camera_angle = *(FVector*)(camera_mgr + GAME_CAMERACACHEPRIVATE + GAME_CAMERA_POV + GAME_CAMERA_ROTATION);
    FVector camera_location = *(FVector*)(camera_mgr + GAME_CAMERACACHEPRIVATE + GAME_CAMERA_POV + GAME_CAMERA_LOCATION);
    float camera_fov = *(float*)(camera_mgr + GAME_CAMERACACHEPRIVATE + GAME_CAMERA_POV + GAME_CAMERA_FOV);

    FVector transformed = matrix_transform(target_pos, camera_angle, camera_location);

    float pawn_width = 100;
    float pawn_height = 300;

    float tmp_fov = tanf(camera_fov * (float)PI / 360.f);
    float depth = window_width / 2.0f / tmp_fov;

    float box_width = pawn_width / transformed.Z * depth;
    float box_height = pawn_height / transformed.Z * depth;

    return FVector2D(box_width, box_height);
}

bool world_to_screen(uint8_t *player_controller, FVector target_pos, FVector2D* screen_pos)
{
    int window_width, window_height;
    g_get_viewport_size_func((uint8_t*)player_controller, &window_width, &window_height);

    uint8_t *camera_mgr = *(uint8_t**)(player_controller + GAME_PLAYER_CAMERA_MANAGER);
    FVector camera_angle = *(FVector*)(camera_mgr + GAME_CAMERACACHEPRIVATE + GAME_CAMERA_POV + GAME_CAMERA_ROTATION);
    FVector camera_location = *(FVector*)(camera_mgr + GAME_CAMERACACHEPRIVATE + GAME_CAMERA_POV + GAME_CAMERA_LOCATION);
    float camera_fov = *(float*)(camera_mgr + GAME_CAMERACACHEPRIVATE + GAME_CAMERA_POV + GAME_CAMERA_FOV);

    FVector transformed = matrix_transform(target_pos, camera_angle, camera_location);

    if (transformed.Z < 0.f)
    {
        // 说明在背后，就没必要显示了
        return false;
    }

    float screen_center_x = window_width / 2.0f;
    float screen_center_y = window_height / 2.0f;

    float tmp_fov = tanf(camera_fov * (float)PI / 360.f);
    screen_pos->X = screen_center_x + transformed.X * (screen_center_x / tmp_fov) / transformed.Z;
    screen_pos->Y = screen_center_y - transformed.Y * (screen_center_x / tmp_fov) / transformed.Z;
  
    return true;
}
}