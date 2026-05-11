#include "skill.h"
#include "ui_bank.h"   // repaint_all
#include "game.h"      // play_sound 等

// --- 激活雾刃：初始化精确坐标 ---
void activate_fog_blade() {
    if (skill_piece_r == -1) return;

    POINT start_pos = get_pos(skill_piece_r, skill_piece_c);

    fog_blade.is_flying = true;
    fog_blade.cur_x = (float)start_pos.x;
    fog_blade.cur_y = (float)start_pos.y;

    // 设置移动方向：红方向上(-1)，黑方向下(1)
    fog_blade.direction = (board[skill_piece_r][skill_piece_c].color == CHESS_RED) ? -1 : 1;

    // 每一帧移动的像素距离（数值越大速度越快，建议 12.0 - 18.0）
    float speed = 15.0f;
    fog_blade.frame_count = 0;

    show_jack_form = true; // 变为杰克形态

    TCHAR soundPath[512];
    _stprintf_s(soundPath, _T("%sres/sounds/技能释放.mp3"), g_exeDir); // 假设你有这个音效
    play_sound(soundPath);
    if (game_mode != MODE_AI)g_shake_strength = 15; // 起手蓄力震动
    show_jack_form = true;

    // 强制刷新一次，让玩家看到“蓄力”感
    repaint_all();
    Sleep(50);
}

// --- 核心更新：实现平滑滑行和碰撞检测 ---
void update_fog_blade() {
    if (!fog_blade.is_flying) return;

    // 1. 像素位移更新
    float move_pixel = 15.0f; // 每一帧移动的像素
    fog_blade.cur_y += (fog_blade.direction * move_pixel);

    // 2. 将当前的像素坐标换算回逻辑棋盘坐标 (r, c)
    // 偏移半个格子的距离以便更精准地检测中心点碰撞
    int cr = -1, cc = -1;
    bool on_grid = click_to_board((int)fog_blade.cur_x, (int)fog_blade.cur_y, cr, cc);

    // 3. 边界检测：滑出屏幕
    if (fog_blade.cur_y < TOP_MARGIN - GRID_SIZE || fog_blade.cur_y > TOP_MARGIN + GRID_SIZE * 10) {
        goto stop_flight;
    }

    // 4. 碰撞检测：如果当前格子有棋子，且不是出发点
    if (on_grid && board[cr][cc].color != CHESS_EMPTY && (cr != skill_piece_r || cc != skill_piece_c)) {

        // --- 命中逻辑开始 ---
        fog_blade.is_flying = false;
        show_jack_form = false;

        // 触发强震动反馈 (前面提到的震动功能)
        if (game_mode != MODE_AI)g_shake_strength = 20;

        // 记录历史记录 (为了支持悔棋)
        StepRecord rec;
        rec.from_r = skill_piece_r; rec.from_c = skill_piece_c;
        rec.to_r = cr; rec.to_c = cc;
        rec.old_target = board[cr][cc];
        rec.old_turn = turn; rec.old_game_over = game_over;
        rec.last_fr = last_from_r; rec.last_fc = last_from_c;
        rec.last_tr = last_to_r; rec.last_tc = last_to_c;
        rec.has_last = has_last_step;
        rec.move_type = MOVE_FOG;
        rec.old_show_jack = true;
        rec.old_invisible = false;
        rec.old_skill_r = skill_piece_r; rec.old_skill_c = skill_piece_c;
        rec.old_fog_active = btn_fog_blade.is_active;
        rec.old_invis_active = btn_invisible.is_active;
        move_history.push_back(rec);

        for (int r_effect = 5; r_effect < 50; r_effect += 10) {
            setlinecolor(RGB(255, 255, 255));
            circle((int)fog_blade.cur_x, (int)fog_blade.cur_y, r_effect);
            FlushBatchDraw(); // 强制刷新看一眼火花
        }
        // 判定是否击杀将军
        if (board[cr][cc].type == GENERAL) {
            game_over = true;
            Color winner = (turn == CHESS_RED ? CHESS_RED : CHESS_BLACK);
            _tcscpy_s(game_result, _countof(game_result),
                winner == CHESS_RED ? _T("雾刃处决！红方胜利！") : _T("雾刃处决！黑方胜利！"));

        }
        else {
            // 只是吃掉普通子
            board[cr][cc].color = CHESS_EMPTY;
            board[cr][cc].type = TYPE_NONE;
            board[cr][cc].show = false;

            TCHAR soundPath[512];
            _stprintf_s(soundPath, _T("%sres/sounds/雾刃命中.mp3"), g_exeDir);
            play_sound(soundPath);
        }

        // 技能收尾
        goto end_turn;
    }

    repaint_all();
    return;

stop_flight:
    fog_blade.is_flying = false;
    show_jack_form = false;

end_turn:
    turn = (turn == CHESS_RED ? CHESS_BLACK : CHESS_RED);
    is_selected = false;
    selected_row = selected_col = -1;
    skill_piece_r = skill_piece_c = -1;
    btn_fog_blade.is_active = false;
    btn_invisible.is_active = false;
    check_sound_played = false;  // 重置标记
    try_play_check_sound();

    repaint_all();
}

void activate_invisible() {
    if (skill_piece_r == -1) return;
    invisible_mode = true;
    show_jack_form = true;
    repaint_all();
}

void execute_invisible_move(int to_r, int to_c) {
    if (board[to_r][to_c].color != CHESS_EMPTY) return;
    StepRecord rec;
    rec.from_r = skill_piece_r; rec.from_c = skill_piece_c;
    rec.to_r = to_r; rec.to_c = to_c;
    rec.old_target = board[to_r][to_c];
    rec.old_turn = turn; rec.old_game_over = game_over;
    rec.last_fr = last_from_r; rec.last_fc = last_from_c;
    rec.last_tr = last_to_r; rec.last_tc = last_to_c;
    rec.has_last = has_last_step;
    rec.move_type = MOVE_INVISIBLE;
    rec.old_show_jack = show_jack_form; rec.old_invisible = invisible_mode;
    rec.old_skill_r = skill_piece_r; rec.old_skill_c = skill_piece_c;
    rec.old_fog_active = btn_fog_blade.is_active;
    rec.old_invis_active = btn_invisible.is_active;
    move_history.push_back(rec);

    board[to_r][to_c] = board[skill_piece_r][skill_piece_c];
    board[skill_piece_r][skill_piece_c].color = CHESS_EMPTY;
    board[skill_piece_r][skill_piece_c].type = TYPE_NONE;
    board[skill_piece_r][skill_piece_c].show = false;

    invisible_mode = false; show_jack_form = false;
    turn = (turn == CHESS_RED ? CHESS_BLACK : CHESS_RED);
    is_selected = false; selected_row = -1; selected_col = -1;
    skill_piece_r = skill_piece_c = -1;
    btn_fog_blade.is_active = false; btn_invisible.is_active = false;
    check_sound_played = false;
    try_play_check_sound();

    repaint_all();
    repaint_all();
}