#include "ui_bank.h"
#include <cmath>

// 模拟软阴影：通过绘制多层深色圆环，实现边缘模糊效果
void draw_soft_shadow(int x, int y, int r) {
    setlinecolor(RGB(60, 50, 40));
    circle(x + 2, y + 2, r);
    setlinecolor(RGB(100, 90, 80));
    circle(x + 3, y + 3, r);
}

void draw_last_step() {
    if (!has_last_step) return;
    COLORREF old = getcolor();
    POINT fp = get_pos(last_from_r, last_from_c), tp = get_pos(last_to_r, last_to_c);
    setcolor(RGB(0, 150, 255));
    rectangle(fp.x - GRID_SIZE / 2 + 1, fp.y - GRID_SIZE / 2 + 1, fp.x + GRID_SIZE / 2 - 1, fp.y + GRID_SIZE / 2 - 1);
    setcolor(WHITE);
    rectangle(fp.x - GRID_SIZE / 2 + 3, fp.y - GRID_SIZE / 2 + 3, fp.x + GRID_SIZE / 2 - 3, fp.y + GRID_SIZE / 2 - 3);
    setcolor(RGB(255, 100, 100));
    rectangle(tp.x - GRID_SIZE / 2 + 1, tp.y - GRID_SIZE / 2 + 1, tp.x + GRID_SIZE / 2 - 1, tp.y + GRID_SIZE / 2 - 1);
    setcolor(WHITE);
    rectangle(tp.x - GRID_SIZE / 2 + 3, tp.y - GRID_SIZE / 2 + 3, tp.x + GRID_SIZE / 2 - 3, tp.y + GRID_SIZE / 2 - 3);
    setcolor(old);
}

// 绘制艺术化棋子
void draw_piece_artistic(int x, int y, ChessPiece p, bool selected) {
    if (p.color == CHESS_EMPTY || !p.show) return;
    int r = GRID_SIZE / 2 - 5;
    draw_soft_shadow(x, y, r);
    if (selected) {
        static float pulse = 0; static int dir = 1;
        pulse += 0.04f * dir; if (pulse > 1 || pulse < 0) dir *= -1;
        int glow_size = (int)(pulse * 4);
        setlinecolor(RGB(255, 215, 0));
        for (int i = 0; i < glow_size; i++) circle(x, y, r + 2 + i);
    }
    IMAGE* use_piece = NULL;
    if (p.color == CHESS_RED) {
        switch (p.type) {
        case GENERAL:  use_piece = &img_piece_red_general; break;
        case ADVISOR:  use_piece = &img_piece_red_advisor; break;
        case ELEPHANT: use_piece = &img_piece_red_elephant; break;
        case HORSE:    use_piece = &img_piece_red_horse; break;
        case CHARIOT:  use_piece = &img_piece_red_chariot; break;
        case CANNON:   use_piece = &img_piece_red_cannon; break;
        case SOLDIER:  use_piece = &img_piece_red_soldier; break;
        }
    }
    else {
        switch (p.type) {
        case GENERAL:  use_piece = &img_piece_black_general; break;
        case ADVISOR:  use_piece = &img_piece_black_advisor; break;
        case ELEPHANT: use_piece = &img_piece_black_elephant; break;
        case HORSE:    use_piece = &img_piece_black_horse; break;
        case CHARIOT:  use_piece = &img_piece_black_chariot; break;
        case CANNON:   use_piece = &img_piece_black_cannon; break;
        case SOLDIER:  use_piece = &img_piece_black_soldier; break;
        }
    }
    if (use_piece && use_piece->getwidth() > 0)
        putimage_alpha(x - use_piece->getwidth() / 2, y - use_piece->getheight() / 2, use_piece);
}

void draw_check_vignette() {
    if (!is_checked(turn)) return;
    float pulse = (float)(sin(GetTickCount() / 400.0) * 0.5 + 0.5);
    int r_val = (int)(60 + pulse * 120);
    for (int i = 0; i < 15; i++) {
        int layer_r = r_val - (i * 8);
        if (layer_r < 0) layer_r = 0;
        setlinecolor(RGB(layer_r, 0, 0));
        setlinestyle(PS_SOLID, 1);
        rectangle(i, i, WINDOW_WIDTH - 1 - i, WINDOW_HEIGHT - 1 - i);
    }
}

void draw_undo_vignette() {
    if (g_undo_effect_strength <= 0) return;
    float pulse = (float)(sin(GetTickCount() / 400.0) * 0.5 + 0.5);
    int r_val = (int)(60 + pulse * 120);               // 基础亮度
    for (int i = 0; i < 15; i++) {
        int layer_r = r_val - (i * 8);
        if (layer_r < 0) layer_r = 0;
        // 紫色：红=蓝=layer_r，绿=0（可根据喜好微调）
        setlinecolor(RGB(layer_r, 0, layer_r));
        setlinestyle(PS_SOLID, 1);
        rectangle(i, i, WINDOW_WIDTH - 1 - i, WINDOW_HEIGHT - 1 - i);
    }
}

bool is_in_circle_button(int mx, int my, const SkillButton& btn) {
    int cx = btn.x + btn.w / 2, cy = btn.y + btn.h / 2, r = btn.w / 2;
    int dx = mx - cx, dy = my - cy;
    return dx * dx + dy * dy <= r * r;
}

void repaint_all() {
    int offsetX = 0, offsetY = 0;
    if (g_shake_strength > 0) {
        offsetX = (rand() % g_shake_strength) - (g_shake_strength / 2);
        offsetY = (rand() % g_shake_strength) - (g_shake_strength / 2);
    }

    BeginBatchDraw();
    setorigin(offsetX, offsetY);
    cleardevice();
    if (img_board_bg.getwidth() > 0) putimage(0, 0, &img_board_bg);
    draw_last_step();

    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            if (g_anim.is_moving && r == last_to_r && c == last_to_c) continue;
            POINT pos = get_pos(r, c);
            bool is_sel = (is_selected && selected_row == r && selected_col == c);
            draw_piece_artistic(pos.x, pos.y, board[r][c], is_sel);
        }
    }

    if (is_selected && !fog_blade.is_flying) {
        for (int r = 0; r < ROW_NUM; r++) {
            for (int c = 0; c < COL_NUM; c++) {
                if (is_move_valid(selected_row, selected_col, r, c) && is_move_safe(selected_row, selected_col, r, c)) {
                    POINT p = get_pos(r, c);
                    setlinecolor(RGB(0, 255, 127));
                    setlinestyle(PS_SOLID, 2);
                    circle(p.x, p.y, 6);
                }
            }
        }
    }
    // 杰克变身形态绘制（放在合法走法标记之后，将军特效之前）
    if (show_jack_form && skill_piece_r != -1) {
        POINT pos = get_pos(skill_piece_r, skill_piece_c);
        if (fog_blade.is_flying && img_jack_fog.getwidth() > 0) {
            // 雾刃飞行中显示杰克雾形态
            putimage_alpha(pos.x - img_jack_fog.getwidth() / 2,
                pos.y - img_jack_fog.getheight() / 2,
                &img_jack_fog);
        }
        else if (invisible_mode && img_jack_invis.getwidth() > 0) {
            // 隐身模式显示杰克隐身形态
            putimage_alpha(pos.x - img_jack_invis.getwidth() / 2,
                pos.y - img_jack_invis.getheight() / 2,
                &img_jack_invis);
        }
        // 如果既无飞行也无隐身，可以显示默认杰克形态（如果你有单独的默认图片也可添加）
    }

    draw_check_vignette();
    if (g_undo_effect_strength > 0) draw_undo_vignette();

    if (g_anim.is_moving) {
        g_anim.t += 0.12f;
        if (g_anim.t >= 1.0f) g_anim.is_moving = false;
        else {
            float ease = 1.0f - pow(1.0f - g_anim.t, 3);
            float cur_px = g_anim.cur_x + (g_anim.target_x - g_anim.cur_x) * ease;
            float cur_py = g_anim.cur_y + (g_anim.target_y - g_anim.cur_y) * ease;
            draw_piece_artistic((int)cur_px, (int)cur_py, g_anim.piece, false);
        }
    }

    // 文字 UI
    LOGFONT f;
    gettextstyle(&f);
    f.lfHeight = 36; f.lfWeight = FW_BOLD; f.lfQuality = ANTIALIASED_QUALITY;
    _tcscpy_s(f.lfFaceName, _T("微软雅黑"));
    settextstyle(&f);
    setbkmode(TRANSPARENT);

    const TCHAR* txt = game_over ? game_result : (turn == CHESS_RED ? _T("红方回合") : _T("黑方回合"));
    int tx = (WINDOW_WIDTH - textwidth(txt)) / 2;
    int ty = 710;

    settextcolor(RGB(50, 40, 30)); outtextxy(tx + 2, ty + 2, txt);
    settextcolor(RGB(255, 245, 220)); outtextxy(tx, ty, txt);

    f.lfHeight = 30; f.lfWeight = FW_NORMAL; settextstyle(&f);
    if (game_over) {
        settextcolor(RGB(200, 200, 200));
        outtextxy((WINDOW_WIDTH - textwidth(_T("按ESC键退出程序"))) / 2, ty + 45, _T("按ESC键退出程序"));
    }
    else {
        const TCHAR* mode_txt = (game_mode == MODE_TWO_PLAYER ? _T("模式：双人对战") : _T("模式：人机对战"));
        settextcolor(RGB(80, 70, 60));
        outtextxy((WINDOW_WIDTH - textwidth(mode_txt)) / 2, ty + 45, mode_txt);

        if (is_checked(turn)) {
            float pulse = (float)(sin(GetTickCount() / 400.0) * 0.5 + 0.5);
            settextcolor(RGB((int)(150 + pulse * 105), 0, 0));
            const TCHAR* warn_txt = _T("危！已被将军！");
            outtextxy((WINDOW_WIDTH - textwidth(warn_txt)) / 2, 122, warn_txt);
        }
    } // 此处补全了 else 的括号

    setorigin(0, 0); // 回归坐标原点绘制固定按钮

    // 按钮绘制
    if (img_undo.getwidth() > 0) {
        putimage_alpha(undo_btn.x, undo_btn.y, &img_undo);
        if (undo_btn.hover) putimage_alpha(undo_btn.x, undo_btn.y, &img_hover_mask);
    }

    // 技能按钮
    if (img_load_success) {
        IMAGE* use_f = btn_fog_blade.is_active ? &img_fog_active : &img_fog_disable;
        putimage_alpha(btn_fog_blade.x, btn_fog_blade.y, use_f);
        if (btn_fog_blade.is_hover && btn_fog_blade.is_active) putimage_alpha(btn_fog_blade.x, btn_fog_blade.y, &img_hover_mask);

        IMAGE* use_i = btn_invisible.is_active ? &img_invis_active : &img_invis_disable;
        putimage_alpha(btn_invisible.x, btn_invisible.y, use_i);
        if (btn_invisible.is_hover && btn_invisible.is_active) putimage_alpha(btn_invisible.x, btn_invisible.y, &img_hover_mask);
    }

    // 雾刃动画
    if (fog_blade.is_flying) {
        if (img_fog_slash.getwidth() > 0) {
            putimage_alpha((int)fog_blade.cur_x - img_fog_slash.getwidth() / 2,
                (int)fog_blade.cur_y - img_fog_slash.getheight() / 2,
                &img_fog_slash);
        }
        for (int j = 0; j < 15; j++) {
            int rx = rand() % 50 - 25, ry = rand() % 50 - 25;
            putpixel((int)fog_blade.cur_x + rx, (int)fog_blade.cur_y + ry, RGB(100, 0, 0));
        }
    }

    FlushBatchDraw();
}