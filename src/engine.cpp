
#include "engine.hpp"

namespace tic_tac_toe {
namespace engine {
    Coordinate::Coordinate() = default;

    Coordinate::Coordinate(Type row, Type col)
        : r(row)
        , c(col)
    {}

    Coordinate::Coordinate(Type i)
        : r(i / 3)
        , c(i % 3)
    {}

    Cell get_cell(Player player) {
        assert(player == Player::O || player == Player::X);
        return player == Player::O ? Cell::O : Cell::X;
    }

    Player nott(Player p) {
        return p == Player::O ? Player::X : Player::O;
    }

    Coordinate::Type index(Coordinate coord) {
        return coord.c + coord.r * 3;
    }

    Cell get_cell(const Board& board, Coordinate coord) {
        return board.cell[coord.r][coord.c];
    }

    Cell& get_cell(Board& board, Coordinate coord) {
        return board.cell[coord.r][coord.c];
    }

    void set_cell(Board& board, Coordinate coord, Player p) {
        board.cell[coord.r][coord.c] = get_cell(p);
    }

    void detect_win(Board& board) {
        GameEnd result = GameEnd::None;
        U16 winning_cells = 0;

        // rows
        for (U8 r = 0; r < 3; ++r) {
            const Cell cell = get_cell(board, Coordinate{r, 0});
            if (cell != Cell::Empty && cell == get_cell(board, Coordinate{r, 1}) && cell == get_cell(board, Coordinate{r, 2})) {
                assert(cell == Cell::O || cell == Cell::X);
                result = cell == Cell::O ? GameEnd::OWin : GameEnd::XWin;
                winning_cells |= (1 << (r * 3)) | (1 << ((r * 3) + 1)) | (1 << ((r * 3) + 2));
            }
        }

        // cols
        for (U8 c = 0; c < 3; ++c) {
            const Cell cell = get_cell(board, Coordinate{0, c});
            if (cell != Cell::Empty && cell == get_cell(board, Coordinate{1, c}) && cell == get_cell(board, Coordinate{2, c})) {
                assert(cell == Cell::O || cell == Cell::X);
                const GameEnd new_result = cell == Cell::O ? GameEnd::OWin : GameEnd::XWin;
                assert(result == GameEnd::None || result == new_result);
                result = new_result;
                winning_cells |= (1 << c) | (1 << (c + (1 * 3))) | (1 << (c + (2 * 3)));
            }
        }

        // diagonal 1
        {
            const Cell cell = get_cell(board, Coordinate{0, 0});
            if (cell != Cell::Empty && cell == get_cell(board, Coordinate{1, 1}) && cell == get_cell(board, Coordinate{2, 2})) {
                assert(cell == Cell::O || cell == Cell::X);
                const GameEnd new_result = cell == Cell::O ? GameEnd::OWin : GameEnd::XWin;
                assert(result == GameEnd::None || result == new_result);
                result = new_result;
                winning_cells |= (1 << 0) | (1 << (1 + (1 * 3))) | (1 << (2 + (2 * 3)));
            }
        }

        // diagonal 2
        {
            const Cell cell = get_cell(board, Coordinate{0, 2});
            if (cell != Cell::Empty && cell == get_cell(board, Coordinate{1, 1}) && cell == get_cell(board, Coordinate{2, 0})) {
                assert(cell == Cell::O || cell == Cell::X);
                const GameEnd new_result = cell == Cell::O ? GameEnd::OWin : GameEnd::XWin;
                assert(result == GameEnd::None || result == new_result);
                result = new_result;
                winning_cells |= (1 << ((0 * 3) + 2)) | (1 << ((1 * 3) + 1)) | (1 << ((2 * 3) + 0));
            }
        }

        if (result == GameEnd::None && board.history_next_index >= 9) {
            result = GameEnd::Draw;
        } else {
            board.win_cell_count = 0;
            for (U32 i = 0; i < 9; ++i) {
                if (winning_cells & (1 << i)) {
                    board.win_cell[board.win_cell_count] = Coordinate(i);
                    ++board.win_cell_count;
                }
            }
        }

        board.game_end = result;
    }

    void play_move(Board& board, Coordinate coord, bool is_redo) {
        if (board.game_end == GameEnd::None && get_cell(board, coord) == Cell::Empty) {
            assert(board.history_next_index < 9);

            set_cell(board, coord, board.next_turn);
            if (board.next_turn == Player::O) {
                board.next_turn = Player::X;
            } else {
                board.next_turn = Player::O;
            }
            board.history[board.history_next_index] = coord;
            ++board.history_next_index;
            board.ai_best_moves_count = 0;
            assert(board.history_next_index <= 9);
            if (!is_redo) {
                board.history_count = board.history_next_index;
            }
        }

        detect_win(board);
    }

    bool is_valid(Coordinate coord) {
        return coord.r < 3 && coord.c < 3;
    }

    Coordinate invalid_coordinate() {
        return Coordinate(3, 3);
    }

    bool can_undo(const Board& board) {
        return board.history_next_index > 0;
    }

    bool can_redo(const Board& board) {
        return board.history_next_index < board.history_count;
    }

    bool undo(Board& board) {
        assert(can_undo(board));
        Coordinate coord = board.history[board.history_next_index - 1];
        assert(get_cell(board, coord) != Cell::Empty);
        get_cell(board, coord) = Cell::Empty;
        --board.history_next_index;
        board.game_end = GameEnd::None;
        board.next_turn = nott(board.next_turn);
        board.win_cell_count = 0;
        board.ai_best_moves_count = 0;
    }

    bool redo(Board& board) {
        assert(can_redo(board));
        Coordinate coord = board.history[board.history_next_index];
        play_move(board, coord, true);
    }

    void play_computer_move(Board& board) {
        assert(board.ai_best_moves_count > 0);
        const U8 index = util::random(0, board.ai_best_moves_count);
        const Coordinate coord = board.ai_best_moves[index];
        play_move(board, coord);
    }

    Coordinate get_random_move(const Board& board) {
        const U8 num_possible_moves = 9 - board.history_next_index;
        assert(num_possible_moves > 0);
        const U8 potential_move_index = num_possible_moves == 1 ? 0 : util::random(0, num_possible_moves);
        U8 potential_move = 0;
        for (U8 i = 0; i < 9; ++i) {
            const Coordinate coord = Coordinate(i);
            if (get_cell(board, coord) == Cell::Empty) {
                if (potential_move == potential_move_index) {
                    return coord;
                } else {
                    ++potential_move;
                }
            }
        }

        assert(false);
        return invalid_coordinate();
    }
} // namespace engine
} // namespace tic_tac_toe
