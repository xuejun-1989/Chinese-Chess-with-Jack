#include "common.h"
#include "resource.h"
#include "game.h"
#include "ui_bank.h"
#include "skill.h"
#include "ai.h"
#include "undo.h"

int main() {
    init_resources();
    select_game_mode();
    init_game();
    play_bgm();
    repaint_all();
    ExMessage msg;

    while (true) {
        // 1. 统一的动画与状态更新区（逻辑帧）
        bool need_redraw = false;

        if (g_shake_strength > 0 || g_anim.is_moving || fog_blade.is_flying) {
            if (g_shake_strength > 0) g_shake_strength--; // 【在这里扣减，每圈只减1】

            if (fog_blade.is_flying) update_fog_blade();

            need_redraw = true;
        }
        repaint_all();
        if (game_over) {
            if (_kbhit() && _getch() == 27) break;
            continue;
        }
        if (fog_blade.is_flying) {
            update_fog_blade(); Sleep(20); continue;
        }

        if (is_checked(turn)) {
            bool has_legal = false;
            for (int fr = 0; fr < ROW_NUM; fr++) for (int fc = 0; fc < COL_NUM; fc++) {
                if (board[fr][fc].color != turn) continue;
                for (int tr = 0; tr < ROW_NUM; tr++) for (int tc = 0; tc < COL_NUM; tc++) {
                    if (is_move_valid(fr, fc, tr, tc) && is_move_safe(fr, fc, tr, tc)) { has_legal = true; goto end_check; }
                }
            }
        end_check:
            if (!has_legal) {
                game_over = true;
                if (turn == CHESS_RED) _tcscpy_s(game_result, _T("游戏结束！黑方胜利！红方被将死！"));
                else                   _tcscpy_s(game_result, _T("游戏结束！红方胜利！黑方被将死！"));
                repaint_all(); continue;
            }
        }

        if (game_mode == MODE_AI && turn == CHESS_BLACK) { ai_move(); continue; }

        if (peekmessage(&msg, EM_MOUSE)) {
            undo_btn.hover = is_in_undo_btn(msg.x, msg.y);
            btn_fog_blade.is_hover =
                is_in_circle_button(msg.x, msg.y, btn_fog_blade);

            btn_invisible.is_hover =
                is_in_circle_button(msg.x, msg.y, btn_invisible);

            if (msg.message == WM_LBUTTONDOWN) {
                if (invisible_mode) {
                    if (btn_invisible.is_hover) {
                        invisible_mode = false; show_jack_form = false; repaint_all(); continue;
                    }
                    int cr, cc; if (click_to_board(msg.x, msg.y, cr, cc) && board[cr][cc].color == CHESS_EMPTY)
                        execute_invisible_move(cr, cc);
                    continue;
                }
                if (is_in_undo_btn(msg.x, msg.y)) { undo_move(); continue; }
                if (btn_fog_blade.is_active && btn_fog_blade.is_hover) {
                    if (is_checked(turn)) {
                        MessageBox(GetHWnd(), _T("当前状态不可使用该技能！"), _T("提示"), MB_OK | MB_ICONWARNING);
                        continue;
                    }
                    if (MessageBox(GetHWnd(), _T("是否要使用超模雾刃？"), _T("确认"), MB_YESNO | MB_ICONQUESTION) == IDYES)
                        activate_fog_blade();
                    continue;
                }
                if (btn_invisible.is_active && btn_invisible.is_hover) {
                    if (is_checked(turn)) {
                        MessageBox(GetHWnd(), _T("当前状态不可使用该技能！"), _T("提示"), MB_OK | MB_ICONWARNING);
                        continue;
                    }
                    activate_invisible(); continue;
                }

                int cr, cc;
                if (click_to_board(msg.x, msg.y, cr, cc)) {
                    if (!is_selected) {
                        if (board[cr][cc].color == turn) {
                            selected_row = cr; selected_col = cc; is_selected = true;
                            skill_piece_r = cr; skill_piece_c = cc;
                            btn_fog_blade.is_active = true;
                            btn_invisible.is_active = (board[cr][cc].type == CHARIOT || board[cr][cc].type == SOLDIER);
                            repaint_all();
                        }
                    }
                    else {
                        if (board[cr][cc].color == turn) {
                            selected_row = cr; selected_col = cc; skill_piece_r = cr; skill_piece_c = cc;
                            btn_fog_blade.is_active = true;
                            btn_invisible.is_active = (board[cr][cc].type == CHARIOT || board[cr][cc].type == SOLDIER);
                            repaint_all();
                        }
                        else {
                            if (is_move_valid(selected_row, selected_col, cr, cc) && is_move_safe(selected_row, selected_col, cr, cc)) {
                                move_piece(selected_row, selected_col, cr, cc);
                                btn_fog_blade.is_active = false; btn_invisible.is_active = false;
                                skill_piece_r = skill_piece_c = -1;
                            }
                            is_selected = false; selected_row = -1; selected_col = -1;
                            repaint_all();
                        }
                    }
                }
            }
            repaint_all();
            need_redraw = true;
        }
        if (need_redraw || is_checked(turn)) {
            repaint_all();
        }
        if (_kbhit() && _getch() == 27) break;
    }
    stop_bgm();
    EndBatchDraw();
    closegraph();
    return 0;
}