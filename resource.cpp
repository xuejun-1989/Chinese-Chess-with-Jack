// resource.cpp
#include "resource.h"

TCHAR* GetExeDir(TCHAR* buf, size_t size) {
    GetModuleFileName(NULL, buf, (DWORD)size);
    TCHAR* p = _tcsrchr(buf, _T('\\'));
    if (p) *(p + 1) = _T('\0');
    return buf;
}

void init_resources() {
    TCHAR exeDir[MAX_PATH];
    GetExeDir(exeDir, MAX_PATH);
    _tcscpy_s(g_exeDir, exeDir);

    TCHAR path[512];
    auto load = [&](IMAGE& img, const TCHAR* name, int w = 0, int h = 0) {
        _stprintf_s(path, _T("%s%s"), exeDir, name);
        loadimage(&img, path, w, h);
        };

    load(img_fog_active, _T("res/images/fog_blade_active.png"));
    load(img_fog_disable, _T("res/images/fog_blade_disable.png"));
    load(img_invis_active, _T("res/images/invisible_active.png"));
    load(img_invis_disable, _T("res/images/invisible_disable.png"));
    load(img_jack_fog, _T("res/images/jack_fog.png"), 50, 50);
    load(img_jack_invis, _T("res/images/jack_invisible.png"), 50, 50);
    load(img_fog_slash, _T("res/images/fog_slash.png"));
    load(img_undo, _T("res/images/undo.png"));
    load(img_board_bg, _T("res/images/board_bg.png"), WINDOW_WIDTH, WINDOW_HEIGHT);
    // 主菜单图片
    load(img_menu_bg, _T("res/images/menu_bg.png"), WINDOW_WIDTH, WINDOW_HEIGHT);
    load(img_btn_2p, _T("res/images/btn_twoplayer.png"), 300, 80);
    load(img_btn_2p_hover, _T("res/images/btn_twoplayer_hover.png"), 300, 80);
    load(img_btn_ai, _T("res/images/btn_ai.png"), 300, 80);
    load(img_btn_ai_hover, _T("res/images/btn_ai_hover.png"), 300, 80);
    //棋子
    load(img_piece_red_general, _T("res/images/piece_red_general.png"), 0, 0);
    load(img_piece_red_advisor, _T("res/images/piece_red_advisor.png"), 0, 0);
    load(img_piece_red_elephant, _T("res/images/piece_red_elephant.png"), 0, 0);
    load(img_piece_red_horse, _T("res/images/piece_red_horse.png"), 0, 0);
    load(img_piece_red_chariot, _T("res/images/piece_red_chariot.png"), 0, 0);
    load(img_piece_red_cannon, _T("res/images/piece_red_cannon.png"), 0, 0);
    load(img_piece_red_soldier, _T("res/images/piece_red_soldier.png"), 0, 0);
    load(img_piece_black_general, _T("res/images/piece_black_general.png"), 0, 0);
    load(img_piece_black_advisor, _T("res/images/piece_black_advisor.png"), 0, 0);
    load(img_piece_black_elephant, _T("res/images/piece_black_elephant.png"), 0, 0);
    load(img_piece_black_horse, _T("res/images/piece_black_horse.png"), 0, 0);
    load(img_piece_black_chariot, _T("res/images/piece_black_chariot.png"), 0, 0);
    load(img_piece_black_cannon, _T("res/images/piece_black_cannon.png"), 0, 0);
    load(img_piece_black_soldier, _T("res/images/piece_black_soldier.png"), 0, 0);

    img_load_success = (img_fog_active.getwidth() > 0);

    // 半透明遮罩
    img_hover_mask.Resize(75, 75);
    DWORD* buf = GetImageBuffer(&img_hover_mask);
    if (buf) {
        DWORD color = (80 << 24) | (255 << 16) | (255 << 8) | 255;
        int total = 75 * 75;
        for (int i = 0; i < total; i++) buf[i] = color;
    }
}