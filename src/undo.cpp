#include "undo.h"
#include "ui_bank.h"   // repaint_all
#include "game.h"

void undo_move() {
    if (move_history.empty() || game_over) return;
    StepRecord rec = move_history.back();
    move_history.pop_back();

    if (rec.move_type == MOVE_FOG) {
        board[rec.to_r][rec.to_c] = rec.old_target;
    }
    else {
        board[rec.from_r][rec.from_c] = board[rec.to_r][rec.to_c];
        board[rec.to_r][rec.to_c] = rec.old_target;
    }
    turn = rec.old_turn;
    game_over = rec.old_game_over;
    show_jack_form = rec.old_show_jack;
    invisible_mode = rec.old_invisible;
    skill_piece_r = rec.old_skill_r;
    skill_piece_c = rec.old_skill_c;
    btn_fog_blade.is_active = rec.old_fog_active;
    btn_invisible.is_active = rec.old_invis_active;
    last_from_r = rec.last_fr; last_from_c = rec.last_fc;
    last_to_r = rec.last_tr; last_to_c = rec.last_tc;
    has_last_step = rec.has_last;

    TCHAR soundPath[512];
    _stprintf_s(soundPath, _T("%sres/sounds/悔棋.mp3"), g_exeDir);
    play_sound(soundPath);

    check_sound_played = false;
    try_play_check_sound();
    repaint_all();
}

bool is_in_undo_btn(int x, int y) {
    return x >= undo_btn.x && x <= undo_btn.x + undo_btn.w
        && y >= undo_btn.y && y <= undo_btn.y + undo_btn.h;
}