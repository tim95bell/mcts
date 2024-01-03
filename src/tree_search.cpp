
#include "tree_search.hpp"
#include "engine.hpp"
#include "util.hpp"
#include <cassert>

namespace tic_tac_toe {
namespace tree_search {
    enum struct Score {
        Draw,
        OWins,
        XWins
    };

    struct ScoreAndCoord {
        engine::Coordinate coord;
        Score score;
    };

    Score get_score(engine::Board& board);

    U8 get_child_scores(engine::Board& board, ScoreAndCoord result[9]) {
        assert(board.game_end == engine::GameEnd::None);
        U8 count = 0;
        for (U8 i = 0; i < 9; ++i) {
            const engine::Coordinate coord(i);
            if (engine::get_cell(board, coord) == engine::Cell::Empty) {
                engine::play_move(board, coord);
                result[count] = ScoreAndCoord{coord, get_score(board)};
                ++count;
                engine::undo(board);
            }
        }
        return count;
    }

    U8 get_best_child_scores(engine::Board& board, ScoreAndCoord result[9]) {
        const U8 count = get_child_scores(board, result);

        if (count <= 1) {
            return count;
        }

        const Score win_score = board.next_turn == engine::Player::O ? Score::OWins : Score::XWins;

        U8 use_count = 0;
        for (U8 i = 0; i < count; ++i) {
            if (result[i].score == win_score) {
                if (i != use_count) {
                    ScoreAndCoord tmp = result[use_count];
                    result[use_count] = result[i];
                    result[i] = tmp;
                }
                ++use_count;
            }
        }

        if (use_count == 0) {
            for (U8 i = 0; i < count; ++i) {
                if (result[i].score == Score::Draw) {
                    if (i != use_count) {
                        result[use_count] = result[i];
                    }
                    ++use_count;
                }
            }
        }

        if (use_count == 0) {
            use_count = count;
        }

        return use_count;
    }

    U8 get_best_child_moves(engine::Board& board, engine::Coordinate result[9]) {
        ScoreAndCoord scores[9];
        const U8 count = get_best_child_scores(board, scores);
        for (U8 i = 0; i < count; ++i) {
            result[i] = scores[i].coord;
        }
        return count;
    }

    ScoreAndCoord get_best_child_score(engine::Board& board) { 
        ScoreAndCoord score[9]{};
        const U8 count = get_best_child_scores(board, score);

        if (count == 1) {
            return score[0];
        }

        const U8 index = util::random(0, count);
        return score[index];
    }

    Score get_score(engine::Board& board) {
        if (board.game_end == engine::GameEnd::Draw) {
            return Score::Draw;
        }

        if (board.game_end == engine::GameEnd::XWin) {
            assert(board.next_turn == engine::Player::O);
            return Score::XWins;
        }

        if (board.game_end == engine::GameEnd::OWin) {
            assert(board.next_turn == engine::Player::X);
            return Score::OWins;
        }

        return get_best_child_score(board).score;
    }

    void generate_computer_moves(engine::Board& board) {
        if (board.game_end != engine::GameEnd::None) {
            return;
        }
        
        const U8 history_count_copy = board.history_count;
        engine::Coordinate history_copy[9];
        for (U8 i = 0; i < history_count_copy; ++i) {
            history_copy[i] = board.history[i];
        }

        board.ai_best_moves_count = get_best_child_moves(board, board.ai_best_moves);

        board.history_count = history_count_copy;
        for (U8 i = 0; i < history_count_copy; ++i) {
            board.history[i] = history_copy[i];
        }
    }
} // namespace tree_search
} // namespace tic_tac_toe
