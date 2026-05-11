// game.h
#pragma once
#include "common.h"

void init_game();
void move_piece(int from_r, int from_c, int to_r, int to_c);
void select_game_mode();
void play_bgm();
void stop_bgm();
void set_bgm_volume(int volume);
void try_play_check_sound();