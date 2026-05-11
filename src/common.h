#pragma once
#include <Windows.h>
#include <mmsystem.h>
#include <graphics.h>
#include <conio.h>
#include <cstring>
#include <tchar.h>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "msimg32.lib")

#define GRID_SIZE 54
#define LEFT_MARGIN 84
#define TOP_MARGIN 204
#define ROW_NUM 10
#define COL_NUM 9
#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 800
#define _CRT_SECURE_NO_WARNINGS

enum Color { CHESS_RED, CHESS_BLACK, CHESS_EMPTY };
enum Type { GENERAL, ADVISOR, ELEPHANT, HORSE, CHARIOT, CANNON, SOLDIER, TYPE_NONE };
enum GameMode { MODE_TWO_PLAYER, MODE_AI, MODE_NONE };
enum MoveType { MOVE_NORMAL, MOVE_FOG, MOVE_INVISIBLE };

struct ChessPiece { Color color; Type type; bool show; };
struct ChessMove { int from_r, from_c, to_r, to_c, score; };
struct Button { int x, y, width, height; TCHAR text[32]; bool is_hover; GameMode mode; };
struct UndoButton { int x, y, w, h; TCHAR text[16]; bool hover; };
struct SkillButton { int x, y, w, h; TCHAR text[16]; bool is_active, is_hover; };

// 统一的 FogBladeState 定义
struct FogBladeState {
    bool is_flying;
    float cur_x, cur_y;
    int current_r, current_c, direction, frame_count;
};

struct AnimState {
    bool is_moving;
    float cur_x, cur_y;
    float target_x, target_y;
    ChessPiece piece;
    float t;
};

struct StepRecord {
    int from_r, from_c, to_r, to_c;
    ChessPiece old_target;
    Color old_turn;
    bool old_game_over;
    int last_fr, last_fc, last_tr, last_tc;
    bool has_last;
    MoveType move_type;
    bool old_show_jack, old_invisible;
    int old_skill_r, old_skill_c;
    bool old_fog_active, old_invis_active;
};

// ===== 全局变量声明 =====
extern ChessPiece board[ROW_NUM][COL_NUM];
extern int selected_row, selected_col;
extern bool is_selected;
extern Color turn;
extern GameMode game_mode;
extern bool game_over;
extern TCHAR game_result[64];
extern int last_from_r, last_from_c, last_to_r, last_to_c;
extern bool has_last_step;
extern UndoButton undo_btn;
extern std::vector<StepRecord> move_history;
extern TCHAR g_exeDir[MAX_PATH];
extern SkillButton btn_fog_blade, btn_invisible;
extern FogBladeState fog_blade;
extern AnimState g_anim;
extern bool invisible_mode;
extern int skill_piece_r, skill_piece_c;
extern bool show_jack_form;
extern int g_shake_strength;
extern bool check_sound_played;

extern IMAGE img_fog_active, img_fog_disable, img_invis_active, img_invis_disable;
extern IMAGE img_jack_fog, img_jack_invis, img_fog_slash;
extern IMAGE img_hover_mask, img_undo, img_board_bg;
extern IMAGE img_menu_bg;           // 主菜单背景图
extern IMAGE img_btn_2p;            // 双人对战按钮（普通）
extern IMAGE img_btn_2p_hover;      // 双人对战按钮（悬停）
extern IMAGE img_btn_ai;            // 人机对战按钮（普通）
extern IMAGE img_btn_ai_hover;      // 人机对战按钮（悬停）
extern IMAGE img_piece_red_general;
extern IMAGE img_piece_red_advisor;
extern IMAGE img_piece_red_elephant;
extern IMAGE img_piece_red_horse;
extern IMAGE img_piece_red_chariot;
extern IMAGE img_piece_red_cannon;
extern IMAGE img_piece_red_soldier;
extern IMAGE img_piece_black_general;
extern IMAGE img_piece_black_advisor;
extern IMAGE img_piece_black_elephant;
extern IMAGE img_piece_black_horse;
extern IMAGE img_piece_black_chariot;
extern IMAGE img_piece_black_cannon;
extern IMAGE img_piece_black_soldier;
extern bool img_load_success;

// ===== 函数声明 =====
POINT get_pos(int row, int col);
void putimage_alpha(int x, int y, IMAGE* pSrcImg);
void play_sound(LPCTSTR sound_file);
bool click_to_board(int x, int y, int& row, int& col);
bool is_move_valid(int from_r, int from_c, int to_r, int to_c);
bool is_move_safe(int from_r, int from_c, int to_r, int to_c);
bool is_checked(Color color);
bool is_general_alive(Color color);