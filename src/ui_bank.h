#pragma once
#include "common.h"

void draw_chessboard();
void draw_last_step();
void draw_piece(int row, int col);
void repaint_all();
bool is_in_circle_button(int mx, int my, const SkillButton& btn);
void draw_undo_vignette();