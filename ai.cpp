#include "ai.h"
#include "game.h"      // move_piece 等
#include "ui_bank.h"   // repaint_all

enum PieceValue { VAL_GENERAL = 10000, VAL_CHARIOT = 900, VAL_CANNON = 450, VAL_HORSE = 400, VAL_ADVISOR = 200, VAL_ELEPHANT = 200, VAL_SOLDIER = 100 };

static int getPieceValue(Type t) {
    switch (t) {
    case GENERAL: return VAL_GENERAL; case CHARIOT: return VAL_CHARIOT;
    case CANNON: return VAL_CANNON; case HORSE: return VAL_HORSE;
    case ADVISOR: return VAL_ADVISOR; case ELEPHANT: return VAL_ELEPHANT;
    case SOLDIER: return VAL_SOLDIER; default: return 0;
    }
}
// 走法排序：吃子价值大的优先，MVV-LVA 思路简化版
static int moveScore(const ChessMove& m) {
    // 如果没有吃子，给一个基础分 -100（确保正常走子不会被完全忽略）
    if (board[m.to_r][m.to_c].color == CHESS_EMPTY)
        return -100;
    // 吃子得分 = 目标棋子价值 - 攻击棋子价值 (LVA 修正)
    int attackerVal = getPieceValue(board[m.from_r][m.from_c].type);
    int targetVal = getPieceValue(board[m.to_r][m.to_c].type);
    return targetVal - attackerVal;
}

static bool move_sort(const ChessMove& a, const ChessMove& b) {
    return moveScore(a) > moveScore(b);
}

// 棋子位置价值表（让AI知道：马跳中心更好，卒过河更强）
// 位置价值表：黑方视角（红方使用时镜像翻转）
const int horse_pst[10][9] = {
    { -5,-10, -5, -5, -5, -5, -5,-10, -5 },
    { -5,  0,  5, 10, 10, 10,  5,  0, -5 },
    { -5,  5, 10, 15, 20, 15, 10,  5, -5 },
    { -5, 10, 15, 20, 25, 20, 15, 10, -5 },
    { -5, 10, 15, 20, 30, 20, 15, 10, -5 },
    { -5,  5, 10, 15, 20, 15, 10,  5, -5 },
    { -5,  0,  5, 10, 10, 10,  5,  0, -5 },
    { -5,-10, -5, -5, -5, -5, -5,-10, -5 },
    { -5,-10, -5, -5, -5, -5, -5,-10, -5 },
    { -5,-10, -5, -5, -5, -5, -5,-10, -5 }
};

const int chariot_pst[10][9] = {
    { -2, -2, -2, -2, -2, -2, -2, -2, -2 },
    {  2,  6,  6,  8,  8,  8,  6,  6,  2 },
    {  4,  8, 10, 12, 14, 12, 10,  8,  4 },
    {  6, 12, 14, 16, 18, 16, 14, 12,  6 },
    {  8, 16, 18, 20, 22, 20, 18, 16,  8 },
    {  6, 12, 14, 16, 18, 16, 14, 12,  6 },
    {  4,  8, 10, 12, 14, 12, 10,  8,  4 },
    {  2,  6,  6,  8,  8,  8,  6,  6,  2 },
    {  0,  4,  4,  6,  6,  6,  4,  4,  0 },
    { -2, -2, -2, -2, -2, -2, -2, -2, -2 }
};

const int cannon_pst[10][9] = {
    { 0, 0, 2, 2, 4, 2, 2, 0, 0 },
    { 0, 2, 4, 4, 6, 4, 4, 2, 0 },
    { 2, 4, 6, 6, 8, 6, 6, 4, 2 },
    { 4, 6, 8, 10,12,10, 8, 6, 4 },
    { 6, 8,10, 12,14,12,10, 8, 6 },
    { 4, 6, 8, 10,12,10, 8, 6, 4 },
    { 2, 4, 6, 6, 8, 6, 6, 4, 2 },
    { 0, 2, 4, 4, 6, 4, 4, 2, 0 },
    { 0, 0, 2, 2, 4, 2, 2, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

const int advisor_pst[10][9] = {
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,15,20,15,0,0,0 },
    { 0,0,0,10,15,10,0,0,0 },
    { 0,0,0,5,10,5,0,0,0 }
};

const int elephant_pst[10][9] = {
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,10,0,10,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,5,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0 }
};

const int soldier_pst[10][9] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 5,10,20,30,40,30,20,10, 5 },  // 过河兵价值暴增
    { 10,20,30,40,50,40,30,20,10 },
    { 20,30,40,50,60,50,40,30,20 },
    { 30,40,50,60,70,60,50,40,30 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 }   // 底线兵往回走没用
};

// 通用位置分获取（传入棋子颜色和类型，返回额外分值）
int get_pos_value(int r, int c, Color color, Type type) {
    int val = 0;
    int row = (color == CHESS_BLACK) ? r : 9 - r; // 黑方直接取，红方镜像
    switch (type) {
    case HORSE:   val = horse_pst[row][c]; break;
    case CHARIOT: val = chariot_pst[row][c]; break;
    case CANNON:  val = cannon_pst[row][c]; break;
    case ADVISOR: val = advisor_pst[row][c]; break;
    case ELEPHANT:val = elephant_pst[row][c]; break;
    case SOLDIER: val = soldier_pst[row][c]; break;
    default: break;
    }
    return val;
}

// 在 ai.cpp 开头添加一个新函数，用于决定当前深度
int get_search_depth() {
    // 统计当前棋盘上棋子总数，子越少深度可以稍大（因为分支少）
    int pieceCount = 0;
    for (int r = 0; r < ROW_NUM; r++)
        for (int c = 0; c < COL_NUM; c++)
            if (board[r][c].color != CHESS_EMPTY) pieceCount++;
    if (pieceCount > 28) return 4;      // 开局：深度4
    if (pieceCount > 16) return 5;      // 中局：深度5
    return 6;                           // 残局：深度6
}

int evaluate() {
    int score = 0;
    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            ChessPiece p = board[r][c];
            if (p.color == CHESS_EMPTY) continue;
            int val = getPieceValue(p.type);
            val += get_pos_value(r, c, p.color, p.type);
            // 额外的机动性/威胁奖励（简单示例）
            if (p.type == CHARIOT || p.type == CANNON) {
                // 车在开阔线加分
                int openCount = 0;
                for (int rr = 0; rr < ROW_NUM; rr++)
                    if (rr != r && board[rr][c].color != CHESS_EMPTY) openCount++;
                if (openCount <= 2) val += 20; // 所在列棋子少，车路通畅
            }
            if (p.color == CHESS_BLACK) {
                score += val;
                score += r * 2;  // 鼓励向前
            }
            else {
                score -= val;
                score -= (9 - r) * 2;
            }
        }
    }
    // 将军惩罚保留
    if (is_checked(CHESS_BLACK)) score -= 800;
    if (is_checked(CHESS_RED))  score += 800;
    return score;
}

static void getAllLegalMoves(Color color, std::vector<ChessMove>& moves) {
    moves.clear();
    for (int fr = 0; fr < ROW_NUM; fr++) for (int fc = 0; fc < COL_NUM; fc++) {
        if (board[fr][fc].color != color) continue;
        for (int tr = 0; tr < ROW_NUM; tr++) for (int tc = 0; tc < COL_NUM; tc++) {
            if (is_move_valid(fr, fc, tr, tc) && is_move_safe(fr, fc, tr, tc)) {
                ChessMove m = { fr, fc, tr, tc, 0 }; moves.push_back(m);
            }
        }
    }
}

static void fakeMove(int fr, int fc, int tr, int tc, ChessPiece& oldTar) {
    oldTar = board[tr][tc]; board[tr][tc] = board[fr][fc];
    board[fr][fc].color = CHESS_EMPTY; board[fr][fc].type = TYPE_NONE; board[fr][fc].show = false;
}
static void undoMove(int fr, int fc, int tr, int tc, ChessPiece& oldTar) {
    board[fr][fc] = board[tr][tc]; board[tr][tc] = oldTar;
}

static int alphaBeta(int depth, int alpha, int beta, bool isMaxTurn) {
    if (depth == 0) return evaluate();
    Color me = isMaxTurn ? CHESS_BLACK : CHESS_RED;
    std::vector<ChessMove> moves;
    getAllLegalMoves(me, moves);
    std::sort(moves.begin(), moves.end(), move_sort);    // ← 新增
    if (moves.empty()) return isMaxTurn ? -100000 : 100000;

    if (isMaxTurn) {
        int best = -999999;
        for (auto& m : moves) {
            ChessPiece oldTar; fakeMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
            int val = alphaBeta(depth - 1, alpha, beta, false);
            best = max(best, val); alpha = max(alpha, best);
            undoMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
            if (beta <= alpha) break;
        }
        return best;
    }
    else {
        int best = 999999;
        for (auto& m : moves) {
            ChessPiece oldTar; fakeMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
            int val = alphaBeta(depth - 1, alpha, beta, true);
            best = min(best, val); beta = min(beta, best);
            undoMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
            if (beta <= alpha) break;
        }
        return best;
    }
}

void ai_move() {
    int depth = get_search_depth(); 
    if (game_over || turn != CHESS_BLACK) return;
    std::vector<ChessMove> moves;
    getAllLegalMoves(CHESS_BLACK, moves);
    std::sort(moves.begin(), moves.end(), move_sort);    
    if (moves.empty()) return;
    int bestVal = -999999; 
    ChessMove bestMove = moves[0];
    for (size_t i = 0; i < moves.size(); i++) {
        auto& m = moves[i];
        ChessPiece oldTar;
        fakeMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
        int val = alphaBeta(depth - 1, -999999, 999999, false);
        undoMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
        if (val > bestVal) {
            bestVal = val;
            bestMove = m;
        }
        // 保留之前的刷新，避免界面卡顿
        if (i % 3 == 0) {
            repaint_all();
            Sleep(1);
        }
    }
    move_piece(bestMove.from_r, bestMove.from_c, bestMove.to_r, bestMove.to_c);
    repaint_all();
}