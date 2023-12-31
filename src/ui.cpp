
#include "ui.hpp"
#include "engine.hpp"
#include "util.hpp"
#include "mcts.hpp"
#include "tree_search.hpp"
#include <cassert>
#include <raylib.h>

#define SEARCH_TYPE_MCTS 0
#define SEARCH_TYPE_TREE 1
#define SEARCH_TYPE SEARCH_TYPE_MCTS
static_assert(SEARCH_TYPE == SEARCH_TYPE_MCTS || SEARCH_TYPE == SEARCH_TYPE_TREE);

namespace tic_tac_toe {
namespace ui {
    struct State {
        engine::Board board;
        U32 board_top_left_x;
        U32 board_top_left_y;
    };

    static constexpr U32 window_width = 640;
    static constexpr U32 window_height = 480;
    static constexpr U32 smallest_dimension = (window_width < window_height ? window_width : window_height);
    static constexpr U32 margin = smallest_dimension * 0.05;
    static constexpr U32 board_size = smallest_dimension - margin * 2;
    static constexpr U32 cell_margin = (board_size / 3) * 0.05;
    static constexpr U32 cell_size = (board_size - cell_margin * 2) / 3;
    static const Color cell_color{120, 140, 170, 255};
    static const Color background_color{200, 215, 230, 255};
    static const Color piece_color = background_color;
    static const Color win_color{50, 150, 50, 255};
    static const Color draw_color{150, 150, 50, 255};

    void init() {
        InitWindow(window_width, window_height, "TicTacToe");
        SetTargetFPS(60);
    }

    Vector2 get_cell_screen_pos(const State& state, engine::Coordinate coord) {
        return Vector2{
            (float)state.board_top_left_x + coord.c * (cell_size + cell_margin),
            (float)state.board_top_left_y + coord.r * (cell_size + cell_margin)
        };
    }

    void draw_o(float x, float y, float size, Color color = piece_color) {
        DrawRing(Vector2{x + size / 2.0f, y + size / 2.0f}, (size / 3), (size / 2), 0, 360, 100, color);
    }

    void draw_o(const State& state, engine::Coordinate coord, Color color = piece_color) {
        const float margin = cell_size * 0.1;
        const Vector2 pos = get_cell_screen_pos(state, coord);
        draw_o(pos.x + margin, pos.y + margin, cell_size * 0.8, color);
    }

    void draw_x(float x, float y, float size, Color color = piece_color) {
        float margin = size * 0.1;
        DrawLineEx(Vector2{x + margin, y + margin}, Vector2{x + size - margin, y + size - margin}, size / 5.0f, color);
        DrawLineEx(Vector2{x + size - margin, y + margin}, Vector2{x + margin, y + size - margin}, size / 5.0f, color);
    }

    void draw_x(const State& state, engine::Coordinate coord, Color color = piece_color) {
        const float margin = cell_size * 0.1;
        const Vector2 pos = get_cell_screen_pos(state, coord);
        draw_x(pos.x + margin, pos.y + margin, cell_size * 0.8, color);
    }

    void draw_piece(const State& state, engine::Cell cell, engine::Coordinate coord, Color color = piece_color) {
        assert(cell == engine::Cell::Empty || cell == engine::Cell::O || cell == engine::Cell::X);
        if (cell == engine::Cell::O) {
            draw_o(state, coord, color);
        } else if (cell == engine::Cell::X) {
            draw_x(state, coord, color);
        }
    }

    void draw_cell(const State& state, engine::Coordinate coord) {
        const Vector2 pos = get_cell_screen_pos(state, coord);
        DrawRectangle(pos.x, pos.y, cell_size, cell_size, cell_color);

        if (get_cell(state.board, coord) == engine::Cell::Empty) {
            bool is_ai_best_move = false;
            for (U8 i = 0; i < state.board.ai_best_moves_count; ++i) {
                if (state.board.ai_best_moves[i].r == coord.r && state.board.ai_best_moves[i].c == coord.c) {
                    is_ai_best_move = true;
                    break;
                }
            }

            if (is_ai_best_move) {
                const Color color{piece_color.r, piece_color.g, piece_color.b, 65};
                if (state.board.next_turn == engine::Player::O) {
                    draw_o(state, coord, color);
                } else {
                    draw_x(state, coord, color);
                }
            }
        } else {
            Color color = piece_color;
            if (state.board.game_end == engine::GameEnd::OWin || state.board.game_end == engine::GameEnd::XWin) {
                for (U32 i = 0; i < state.board.win_cell_count; ++i) {
                    if (coord.r == state.board.win_cell[i].r && coord.c == state.board.win_cell[i].c) {
                        color = win_color;
                        break;
                    }
                }
            } else if (state.board.game_end == engine::GameEnd::Draw) {
                color = draw_color;
            }

            draw_piece(state, state.board.cell[coord.r][coord.c], coord, color);
        }
    }

    void draw_next_turn_player(const State& state) {
        const float x_margin = state.board_top_left_x * 0.1f;
        const float size = state.board_top_left_x * 0.8;
        if (state.board.next_turn == engine::Player::O) {
            draw_o(x_margin, margin, size, cell_color);
        } else {
            draw_x(x_margin, margin, size, cell_color);
        }
    }

    void draw_game_end(const State& state) {
        const float margin = state.board_top_left_x * 0.1f;
        const float size = state.board_top_left_x * 0.8;
        const float y = margin + size + margin * 3;
        if (state.board.game_end == engine::GameEnd::OWin) {
            draw_o(margin, y, size, win_color);
        } else if (state.board.game_end == engine::GameEnd::XWin) {
            draw_x(margin, y, size, win_color);
        } else if (state.board.game_end == engine::GameEnd::Draw) {
            draw_o(margin, y, size, draw_color);
            draw_x(margin, y, size, draw_color);
        }
    }

    void draw_history(const engine::Cell board[3][3], engine::Coordinate coord, float x, float y, float size, Color cell_color) {
        const U32 margin = util::max((size / 3.0f) * 0.05f, 1);
        const U32 cell_size = (size - margin * 2.0f) / 3.0f;
        const Color not_most_recent_move_cell_color{cell_color.r, cell_color.g, cell_color.b, 100};
        for (U32 r = 0; r < 3; ++r) {
            for (U32 c = 0; c < 3; ++c) {
                const float cell_x = x + c * (cell_size + margin);
                const float cell_y = y + r * (cell_size + margin);
                const Color color = coord.r == r && coord.c == c ? cell_color : not_most_recent_move_cell_color;
                DrawRectangle(cell_x, cell_y, cell_size, cell_size, color);
                const engine::Cell cell = board[r][c];
                U32 margin = cell_size * 0.1;
                if (cell == engine::Cell::O) {
                    draw_o(cell_x + margin, cell_y + margin, cell_size - (margin * 2), piece_color);
                } else if (cell == engine::Cell::X) {
                    draw_x(cell_x + margin, cell_y + margin, cell_size - (margin * 2), piece_color);
                }
            }
        }
    }

    void draw_history(const State& state) {
        const U32 y = margin;
        const U32 height = window_height - margin * 2;
        const U32 item_margin = height * 0.012;
        const U32 item_height = (height - item_margin * 8) / 9;
        const U32 min_x_margin = state.board_top_left_x * 0.05;
        const U32 max_width = state.board_top_left_x - min_x_margin * 2;
        const U32 item_width = util::min(max_width, item_height);
        const U32 x_margin = (state.board_top_left_x - item_width) / 2;
        const U32 x = window_width - state.board_top_left_x + x_margin;
        engine::Cell board[3][3]{};
        engine::Player player = engine::Player::O;
        for (U32 i = 0; i < state.board.history_count; ++i) {
            const engine::Coordinate coord = state.board.history[i];
            board[coord.r][coord.c] = engine::get_cell(player);
            player = engine::other(player);
            const U32 item_y = y + i * (item_height + item_margin);
            const Color color = i == (state.board.history_next_index - 1) ? Color{170, 140, 120, 255} : cell_color;
            draw_history(board, coord, x, item_y, item_width, color);
        }
    }

    engine::Coordinate get_cell_for_screen_pos(const State& state, Vector2 pos) {
        for (engine::Coordinate::Type r = 0; r < 3; ++r) {
            for (engine::Coordinate::Type c = 0; c < 3; ++c) {
                const Vector2 cell_pos = get_cell_screen_pos(state, engine::Coordinate(r, c));
                if (pos.x > cell_pos.x && pos.x < cell_pos.x + cell_size && pos.y > cell_pos.y && pos.y < cell_pos.y + cell_size) {
                    return engine::Coordinate(r, c);
                }
            }
        }
        
        return engine::invalid_coordinate();
    }

    void run() {
        srand(420);
        State state{};
        state.board_top_left_x = (window_width - board_size) / 2;
        state.board_top_left_y = (window_height - board_size) / 2;

        init();

        while (!WindowShouldClose()) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                const Vector2 pos = GetMousePosition();
                const engine::Coordinate coord = get_cell_for_screen_pos(state, pos);
                if (is_valid(coord)) {
                    play_move(state.board, coord);
                }
            }

            if (IsKeyPressed(KEY_LEFT)) {
                if (!IsKeyPressed(KEY_RIGHT)) {
                    if (can_undo(state.board)) {
                        undo(state.board);
                    }
                }
            } else if (IsKeyPressed(KEY_RIGHT)) {
                if (can_redo(state.board)) {
                    redo(state.board);
                }
            } else if (IsKeyPressed(KEY_DOWN)) {
                if (state.board.ai_best_moves_count == 0) {
#if SEARCH_TYPE == SEARCH_TYPE_MCTS
                    mcts::generate_computer_moves(state.board);
#elif SEARCH_TYPE == SEARCH_TYPE_TREE
                    tree_search::generate_computer_moves(state.board);
#endif
                } else {
                    play_computer_move(state.board);
                }
            }

            BeginDrawing();
            {
                ClearBackground(background_color);

                for (engine::Coordinate::Type r = 0; r < 3; ++r) {
                    for (engine::Coordinate::Type c = 0; c < 3; ++c) {
                        draw_cell(state, engine::Coordinate(r, c));
                    }
                }

                draw_next_turn_player(state);
                draw_game_end(state);
                draw_history(state);
            }
            EndDrawing();
        }

        CloseWindow();
    }
} // namespace ui
} // namespace tic_tac_toe
