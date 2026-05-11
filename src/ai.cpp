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

// 棋子位置价值表（让AI知道：马跳中心更好，卒过河更强）
const int horse_pst[10][9] = {
    {-5,-10,-5,-5,-5,-5,-5,-10,-5},
    {-5, 0, 5, 10, 10, 10, 5, 0, -5},
    {-5, 5, 10, 15, 20, 15, 10, 5, -5}, // 马在中心分数更高
    // ... 剩下的可以根据象棋常识填充 ...
};

int evaluate() {
    int score = 0;
    for (int r = 0; r < ROW_NUM; r++) {
        for (int c = 0; c < COL_NUM; c++) {
            ChessPiece p = board[r][c];
            if (p.color == CHESS_EMPTY) continue;
            int val = getPieceValue(p.type);
            if (p.type == HORSE) val += (p.color == CHESS_BLACK) ? horse_pst[r][c] : horse_pst[9 - r][c];
            if (p.color == CHESS_BLACK) { score += val; score += r * 2; }
            else { score -= val; score -= (9 - r) * 2; }
        }
    }
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
    std::vector<ChessMove> moves; getAllLegalMoves(me, moves);
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
    if (game_over || turn != CHESS_BLACK) return;
    std::vector<ChessMove> moves; getAllLegalMoves(CHESS_BLACK, moves);
    if (moves.empty()) return;
    int bestVal = -999999; ChessMove bestMove = moves[0];
    for (auto& m : moves) {
        ChessPiece oldTar; fakeMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
        int val = alphaBeta(3, -999999, 999999, false);
        undoMove(m.from_r, m.from_c, m.to_r, m.to_c, oldTar);
        if (val > bestVal) { bestVal = val; bestMove = m; }
    }
    move_piece(bestMove.from_r, bestMove.from_c, bestMove.to_r, bestMove.to_c);
    repaint_all();
}