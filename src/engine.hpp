
#pragma once

#include "util.hpp"
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdio>


namespace tic_tac_toe {
namespace engine {
    enum class Player {
        O,
        X
    };

    enum class Cell {
        Empty,
        O,
        X
    };

    enum class GameEnd {
        None,
        Draw,
        OWin,
        XWin
    };

    struct Coordinate {
        using Type = U8;
        Coordinate();
        Coordinate(Type row, Type col);
        Coordinate(Type i);

        Type r;
        Type c;
    };

    struct Board {
        Player next_turn;
        Cell cell[3][3];
        GameEnd game_end;
        U8 history_next_index;
        U8 history_count;
        Coordinate history[9];
        Coordinate win_cell[6];
        U8 win_cell_count;
        Coordinate ai_best_moves[9];
        U8 ai_best_moves_count;
    };

    Cell get_cell(Player player);
    Player other(Player p); 
    Coordinate::Type index(Coordinate coord);
    Cell get_cell(const Board& board, Coordinate coord);
    Cell& get_cell(Board& board, Coordinate coord);
    void set_cell(Board& board, Coordinate coord, Player p);
    void detect_win(Board& board);
    void play_move(Board& board, Coordinate coord, bool is_redo = false);
    bool is_valid(Coordinate coord);
    Coordinate invalid_coordinate();
    bool can_undo(const Board& board);
    bool can_redo(const Board& board);
    bool undo(Board& board);
    bool redo(Board& board);
    void play_computer_move(Board& board);
    Coordinate get_random_move(const Board& board);
} // namespace engine
} // namespace tic_tac_toe
