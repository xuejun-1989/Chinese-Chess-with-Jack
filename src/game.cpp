#include "game.h"

// ---------- 工具函数实现 ----------
POINT get_pos(int row, int col) {
    POINT p;
    p.x = LEFT_MARGIN + col * GRID_SIZE;
    p.y = TOP_MARGIN + row * GRID_SIZE;
    return p;
}

bool is_point_in_button(int x, int y, Button btn) {
    return x >= btn.x && x <= btn.x + btn.width && y >= btn.y && y <= btn.y + btn.height;
}

void putimage_alpha(int x, int y, IMAGE* pSrcImg) {
    if (!pSrcImg) return;
    HDC hdc = GetImageHDC(NULL);
    HDC hdcSrc = GetImageHDC(pSrcImg);
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    AlphaBlend(hdc, x, y, pSrcImg->getwidth(), pSrcImg->getheight(),
        hdcSrc, 0, 0, pSrcImg->getwidth(), pSrcImg->getheight(), bf);
}

void play_sound(LPCTSTR sound_file) {
    TCHAR cmd[512];
    mciSendString(_T("close sound"), NULL, 0, NULL);
    _stprintf_s(cmd, _T("open \"%s\" type mpegvideo alias sound"), sound_file);
    if (mciSendString(cmd, NULL, 0, NULL) != 0) return;
    mciSendString(_T("play sound from 0"), NULL, 0, NULL);
}

bool click_to_board(int x, int y, int& row, int& col) {
    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            POINT p = get_pos(r, c);
            int dx = x - p.x;
            int dy = y - p.y;
            if (dx * dx + dy * dy < (GRID_SIZE / 2) * (GRID_SIZE / 2)) {
                row = r;
                col = c;
                return true;
            }
        }
    }
    return false;
}

int count_pieces_between(int r1, int c1, int r2, int c2) {
    int count = 0;
    if (r1 == r2) {
        int min_c = min(c1, c2), max_c = max(c1, c2);
        for (int c = min_c + 1; c < max_c; c++)
            if (board[r1][c].color != CHESS_EMPTY) count++;
    }
    else if (c1 == c2) {
        int min_r = min(r1, r2), max_r = max(r1, r2);
        for (int r = min_r + 1; r < max_r; r++)
            if (board[r][c1].color != CHESS_EMPTY) count++;
    }
    return count;
}

bool is_move_valid(int from_r, int from_c, int to_r, int to_c) {
    ChessPiece from = board[from_r][from_c], to = board[to_r][to_c];
    if (to.color == from.color) return false;
    int dr = abs(to_r - from_r), dc = abs(to_c - from_c);

    switch (from.type) {
    case CHARIOT:
        if (dr != 0 && dc != 0) return false;
        return count_pieces_between(from_r, from_c, to_r, to_c) == 0;
    case HORSE:
        if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2))) return false;
        if (dr == 2) { int br = from_r + (to_r > from_r ? 1 : -1); if (board[br][from_c].color != CHESS_EMPTY) return false; }
        else { int bc = from_c + (to_c > from_c ? 1 : -1); if (board[from_r][bc].color != CHESS_EMPTY) return false; }
        return true;
    case CANNON:
        if (dr != 0 && dc != 0) return false;
        { int cnt = count_pieces_between(from_r, from_c, to_r, to_c); return to.color == CHESS_EMPTY ? cnt == 0 : cnt == 1; }
    case GENERAL:
        if (dr > 1 || dc > 1) return false;
        if (dc != 0 && dr != 0) return false;
        if (from.color == CHESS_RED) { if (to_r < 7 || to_r > 9 || to_c < 3 || to_c > 5) return false; }
        else { if (to_r < 0 || to_r > 2 || to_c < 3 || to_c > 5) return false; }
        return true;
    case ADVISOR:
        if (dr != 1 || dc != 1) return false;
        if (from.color == CHESS_RED) { if (to_r < 7 || to_r > 9 || to_c < 3 || to_c > 5) return false; }
        else { if (to_r < 0 || to_r > 2 || to_c < 3 || to_c > 5) return false; }
        return true;
    case ELEPHANT:
        if (dr != 2 || dc != 2) return false;
        if (from.color == CHESS_RED && to_r < 5) return false;
        if (from.color == CHESS_BLACK && to_r > 4) return false;
        {
            int br = from_r + (to_r > from_r ? 1 : -1), bc = from_c + (to_c > from_c ? 1 : -1);
            if (board[br][bc].color != CHESS_EMPTY) return false;
        }
        return true;
    case SOLDIER:
        if (from.color == CHESS_RED) {
            if (from_r > 4) { if (to_r >= from_r || dr > 1 || dc > 0) return false; }
            else { if (dr > 1 || dc > 1) return false; if ((dr == 1 && dc != 0) || (dc == 1 && dr != 0)) if (to_r > from_r) return false; }
        }
        else {
            if (from_r < 5) { if (to_r <= from_r || dr > 1 || dc > 0) return false; }
            else { if (dr > 1 || dc > 1) return false; if ((dr == 1 && dc != 0) || (dc == 1 && dr != 0)) if (to_r < from_r) return false; }
        }
        return true;
    default: return false;
    }
}

POINT find_general_pos(Color color) {
    for (int r = 0; r < ROW_NUM; r++)
        for (int c = 0; c < COL_NUM; c++)
            if (board[r][c].color == color && board[r][c].type == GENERAL)
                return { c, r };
    return { -1, -1 };
}

bool is_checked(Color color) {
    POINT general_pos = find_general_pos(color);
    if (general_pos.x == -1) return false;
    Color enemy_color = (color == CHESS_RED ? CHESS_BLACK : CHESS_RED);
    for (int r = 0; r < ROW_NUM; r++)
        for (int c = 0; c < COL_NUM; c++)
            if (board[r][c].color == enemy_color && is_move_valid(r, c, general_pos.y, general_pos.x))
                return true;
    POINT enemy_general_pos = find_general_pos(enemy_color);
    if (enemy_general_pos.x == general_pos.x) {
        int min_r = min(general_pos.y, enemy_general_pos.y), max_r = max(general_pos.y, enemy_general_pos.y);
        bool has_piece_between = false;
        for (int r = min_r + 1; r < max_r; r++)
            if (board[r][general_pos.x].color != CHESS_EMPTY) { has_piece_between = true; break; }
        if (!has_piece_between) return true;
    }
    return false;
}

bool is_move_safe(int from_r, int from_c, int to_r, int to_c) {
    Color current_color = board[from_r][from_c].color;
    ChessPiece old_target = board[to_r][to_c];
    board[to_r][to_c] = board[from_r][from_c];
    board[from_r][from_c].color = CHESS_EMPTY;
    board[from_r][from_c].type = TYPE_NONE;
    board[from_r][from_c].show = false;
    bool safe = !is_checked(current_color);
    board[from_r][from_c] = board[to_r][to_c];
    board[to_r][to_c] = old_target;
    return safe;
}

bool is_general_alive(Color color) {
    for (int r = 0; r < ROW_NUM; r++)
        for (int c = 0; c < COL_NUM; c++)
            if (board[r][c].color == color && board[r][c].type == GENERAL)
                return true;
    return false;
}

// ---------- 游戏初始化 ----------
void init_game() {
    for (int r = 0; r < ROW_NUM; r++)
        for (int c = 0; c < COL_NUM; c++)
            board[r][c] = { CHESS_EMPTY, TYPE_NONE, false };

#define SET(r, c, col, ty) board[r][c] = { col, ty, true };
    // 黑方
    SET(0, 0, CHESS_BLACK, CHARIOT); SET(0, 1, CHESS_BLACK, HORSE); SET(0, 2, CHESS_BLACK, ELEPHANT);
    SET(0, 3, CHESS_BLACK, ADVISOR); SET(0, 4, CHESS_BLACK, GENERAL); SET(0, 5, CHESS_BLACK, ADVISOR);
    SET(0, 6, CHESS_BLACK, ELEPHANT); SET(0, 7, CHESS_BLACK, HORSE); SET(0, 8, CHESS_BLACK, CHARIOT);
    SET(2, 1, CHESS_BLACK, CANNON); SET(2, 7, CHESS_BLACK, CANNON);
    SET(3, 0, CHESS_BLACK, SOLDIER); SET(3, 2, CHESS_BLACK, SOLDIER); SET(3, 4, CHESS_BLACK, SOLDIER);
    SET(3, 6, CHESS_BLACK, SOLDIER); SET(3, 8, CHESS_BLACK, SOLDIER);
    // 红方
    SET(9, 0, CHESS_RED, CHARIOT); SET(9, 1, CHESS_RED, HORSE); SET(9, 2, CHESS_RED, ELEPHANT);
    SET(9, 3, CHESS_RED, ADVISOR); SET(9, 4, CHESS_RED, GENERAL); SET(9, 5, CHESS_RED, ADVISOR);
    SET(9, 6, CHESS_RED, ELEPHANT); SET(9, 7, CHESS_RED, HORSE); SET(9, 8, CHESS_RED, CHARIOT);
    SET(7, 1, CHESS_RED, CANNON); SET(7, 7, CHESS_RED, CANNON);
    SET(6, 0, CHESS_RED, SOLDIER); SET(6, 2, CHESS_RED, SOLDIER); SET(6, 4, CHESS_RED, SOLDIER);
    SET(6, 6, CHESS_RED, SOLDIER); SET(6, 8, CHESS_RED, SOLDIER);
#undef SET

    btn_fog_blade = { 577, 392, 75, 75, _T(""), false, false };
    btn_invisible = { 577, 514, 75, 75, _T(""), false, false };
    fog_blade = { false, -1, -1, 0, 0 };
    invisible_mode = false;
    skill_piece_r = skill_piece_c = -1;
    show_jack_form = false;
    selected_row = selected_col = -1;
    is_selected = false;
    turn = CHESS_RED;
    game_over = false;
    memset(game_result, 0, sizeof(game_result));
    srand((unsigned int)time(NULL));
    move_history.clear();
    last_from_r = last_from_c = last_to_r = last_to_c = -1;
    has_last_step = false;
}

// ---------- 走棋 ----------
void move_piece(int from_r, int from_c, int to_r, int to_c) {
    // 1. 在逻辑变动前，记录动画起始点
    POINT start_pt = get_pos(from_r, from_c);
    POINT end_pt = get_pos(to_r, to_c);

    g_anim.is_moving = true;
    g_anim.piece = board[from_r][from_c]; // 拷贝棋子镜像
    g_anim.cur_x = (float)start_pt.x;
    g_anim.cur_y = (float)start_pt.y;
    g_anim.target_x = (float)end_pt.x;
    g_anim.target_y = (float)end_pt.y;
    g_anim.t = 0;

    // --- 新增：根据是否吃子设置震动强度 ---
    if (board[to_r][to_c].color != CHESS_EMPTY) {
        g_shake_strength = 12; // 吃子：震动剧烈
    }
    else {
        g_shake_strength = 4;  // 普通落子：轻微震动
    }
    StepRecord rec;
    rec.from_r = from_r; rec.from_c = from_c;
    rec.to_r = to_r; rec.to_c = to_c;
    rec.old_target = board[to_r][to_c];
    rec.old_turn = turn;
    rec.old_game_over = game_over;
    rec.last_fr = last_from_r; rec.last_fc = last_from_c;
    rec.last_tr = last_to_r; rec.last_tc = last_to_c;
    rec.has_last = has_last_step;
    rec.move_type = MOVE_NORMAL;
    rec.old_show_jack = show_jack_form;
    rec.old_invisible = invisible_mode;
    rec.old_skill_r = skill_piece_r;
    rec.old_skill_c = skill_piece_c;
    rec.old_fog_active = btn_fog_blade.is_active;
    rec.old_invis_active = btn_invisible.is_active;
    move_history.push_back(rec);

    last_from_r = from_r; last_from_c = from_c;
    last_to_r = to_r; last_to_c = to_c;
    has_last_step = true;

    bool is_eat = (board[to_r][to_c].color != CHESS_EMPTY);
    board[to_r][to_c] = board[from_r][from_c];
    board[from_r][from_c].color = CHESS_EMPTY;
    board[from_r][from_c].type = TYPE_NONE;
    board[from_r][from_c].show = false;

    TCHAR soundPath[512];
    if (is_eat) {
        _stprintf_s(soundPath, _T("%sres/sounds/吃子.mp3"), g_exeDir);
        play_sound(soundPath);
    }
    else {
        _stprintf_s(soundPath, _T("%sres/sounds/落子.mp3"), g_exeDir);
        play_sound(soundPath);
    }

    if (!is_general_alive(CHESS_RED)) {
        game_over = true;
        _tcscpy_s(game_result, _T("游戏结束！黑方胜利！"));
    }
    else if (!is_general_alive(CHESS_BLACK)) {
        game_over = true;
        _tcscpy_s(game_result, _T("游戏结束！红方胜利！"));
    }

    if (!game_over) turn = (turn == CHESS_RED) ? CHESS_BLACK : CHESS_RED;
    check_sound_played = false;  // 新回合重置标记
    try_play_check_sound();      // 检测并播放将军音效
}

// ---------- 模式选择 ----------
void select_game_mode() {
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(RGB(240, 230, 200));
    BeginBatchDraw();

    int btn_width = 300, btn_height = 80, btn_x = (WINDOW_WIDTH - btn_width) / 2;
    Button btn_two = { btn_x, 350, btn_width, btn_height, _T("双人对战"), false, MODE_TWO_PLAYER };
    Button btn_ai = { btn_x, 480, btn_width, btn_height, _T("人机对战（你执红方）"), false, MODE_AI };

    ExMessage msg;
    GameMode selected = MODE_NONE;
    while (selected == MODE_NONE) {
        while (peekmessage(&msg, EM_MOUSE)) {
            if (msg.message == WM_MOUSEMOVE) {
                btn_two.is_hover = is_point_in_button(msg.x, msg.y, btn_two);
                btn_ai.is_hover = is_point_in_button(msg.x, msg.y, btn_ai);
            }
            if (msg.message == WM_LBUTTONDOWN) {
                if (is_point_in_button(msg.x, msg.y, btn_two)) selected = MODE_TWO_PLAYER;
                if (is_point_in_button(msg.x, msg.y, btn_ai))  selected = MODE_AI;
            }
        }

        cleardevice();

        // 绘制主菜单背景图
        if (img_menu_bg.getwidth() > 0) {
            putimage(0, 0, &img_menu_bg);
        }

        // 绘制双人对战按钮
        if (btn_two.is_hover && img_btn_2p_hover.getwidth() > 0) {
            putimage_alpha(btn_two.x, btn_two.y, &img_btn_2p_hover);
        }
        else if (img_btn_2p.getwidth() > 0) {
            putimage_alpha(btn_two.x, btn_two.y, &img_btn_2p);
        }
        else {
            // 图片未加载时的文字按钮
            setfillcolor(btn_two.is_hover ? RGB(220, 200, 170) : RGB(245, 235, 210));
            setlinecolor(RGB(120, 50, 20)); setlinestyle(PS_SOLID, 2);
            fillroundrect(btn_two.x, btn_two.y, btn_two.x + btn_two.width, btn_two.y + btn_two.height, 10, 10);
            settextcolor(RGB(80, 30, 10)); settextstyle(28, 0, _T("楷体"));
            int tx = btn_two.x + (btn_two.width - textwidth(btn_two.text)) / 2;
            int ty = btn_two.y + (btn_two.height - textheight(btn_two.text)) / 2;
            outtextxy(tx, ty, btn_two.text);
        }

        // 绘制人机对战按钮
        if (btn_ai.is_hover && img_btn_ai_hover.getwidth() > 0) {
            putimage_alpha(btn_ai.x, btn_ai.y, &img_btn_ai_hover);
        }
        else if (img_btn_ai.getwidth() > 0) {
            putimage_alpha(btn_ai.x, btn_ai.y, &img_btn_ai);
        }
        else {
            // 文字兜底
            setfillcolor(btn_ai.is_hover ? RGB(220, 200, 170) : RGB(245, 235, 210));
            setlinecolor(RGB(120, 50, 20)); setlinestyle(PS_SOLID, 2);
            fillroundrect(btn_ai.x, btn_ai.y, btn_ai.x + btn_ai.width, btn_ai.y + btn_ai.height, 10, 10);
            settextcolor(RGB(80, 30, 10)); settextstyle(28, 0, _T("楷体"));
            int tx = btn_ai.x + (btn_ai.width - textwidth(btn_ai.text)) / 2;
            int ty = btn_ai.y + (btn_ai.height - textheight(btn_ai.text)) / 2;
            outtextxy(tx, ty, btn_ai.text);
        }


        FlushBatchDraw();
        if (_kbhit() && _getch() == 27) { EndBatchDraw(); closegraph(); exit(0); }
    }
    game_mode = selected;
    EndBatchDraw();
    closegraph();
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(RGB(240, 230, 200));
    BeginBatchDraw();
}

void play_bgm() {
    TCHAR path[512];
    _stprintf_s(path, _T("%sres/sounds/bgm.mp3"), g_exeDir);
    TCHAR cmd[512];

    // 先关闭已存在的 bgm（防止重复）
    mciSendString(_T("close bgm"), NULL, 0, NULL);
    _stprintf_s(cmd, _T("open \"%s\" type mpegvideo alias bgm"), path);
    if (mciSendString(cmd, NULL, 0, NULL) != 0) return; // 文件不存在或格式错误则忽略
    // 设置初始音量（默认 600）
    mciSendString(_T("setaudio bgm volume to 600"), NULL, 0, NULL);
    // 循环播放
    mciSendString(_T("play bgm repeat"), NULL, 0, NULL);
}

void stop_bgm() {
    mciSendString(_T("close bgm"), NULL, 0, NULL);
}

// 音量范围：0（静音）～ 1000（最大）
void set_bgm_volume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 1000) volume = 1000;
    TCHAR cmd[64];
    _stprintf_s(cmd, _T("setaudio bgm volume to %d"), volume);
    mciSendString(cmd, NULL, 0, NULL);
}
void try_play_check_sound() {
    if (is_checked(turn) && !check_sound_played) {
        TCHAR path[512];
        _stprintf_s(path, _T("%sres/sounds/将军.mp3"), g_exeDir);
        play_sound(path);
        check_sound_played = true;
    }
}