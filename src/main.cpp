
#include <cstdio>
#include <cassert>
#include <raylib.h>

namespace mcts {
    using U8 = unsigned char;
    static_assert(sizeof(U8) == 1);
    using U16 = unsigned short;
    static_assert(sizeof(U16) == 2);
    using U32 = unsigned;
    static_assert(sizeof(U32) == 4);
    using U64 = unsigned long long;
    static_assert(sizeof(U64) == 8);

    enum class Player {
        O,
        X
    };

    enum class Cell {
        Empty,
        X,
        O
    };

    struct Board {
        Player next_turn;
        Cell cell[3][3];
    };

    struct State {
        Board board;
        U32 board_top_left_x;
        U32 board_top_left_y;
    };

    struct Coordinate {
        using Type = U8;
        Coordinate(Type row, Type col)
            : r(row)
            , c(col)
        {}

        Coordinate(Type i)
            : r(i / 3)
            , c(i % 3)
        {}

        Type r;
        Type c;
    };

    static constexpr U32 window_width = 640;
    static constexpr U32 window_height = 480;
    static constexpr U32 board_size = window_width < window_height ? window_width : window_height;
    static constexpr U32 cell_margin = (board_size / 3) * 0.05;
    static constexpr U32 cell_size = (board_size - cell_margin * 4) / 3;
    static const Color cell_color{120, 140, 170, 255};
    static const Color background_color{200, 215, 230, 255};
    static const Color piece_color = background_color;

#if 0
    struct Node {
        U32 visit;
        U32 win;
        U32 loss;
        Node* parent;
        Node* child_1;
        Node* child_2;
    };
#endif

    char get_char(Cell cell) {
        if (cell == Cell::Empty) {
            return '.';
        }

        if (cell == Cell::X) {
            return 'X';
        }

        assert(cell == Cell::O);
        return 'O';
    }

    void print(const Board& board) {
        printf("next turn: %c\n", board.next_turn == Player::X ? 'x' : 'y');
        for (Coordinate::Type y = 0; y < 3; ++y) {
            for (Coordinate::Type x = 0; x < 3; ++x) {
                printf("\t%c", get_char(board.cell[y][x]));
            }
            printf("\n");
        }
        printf("\n");
    }

    void init() {
        InitWindow(window_width, window_height, "TicTacToe");
        SetTargetFPS(60);
    }

    Vector2 get_cell_screen_pos(const State& state, Coordinate coord) {
        return Vector2{
            (float)state.board_top_left_x + cell_margin + coord.c * (cell_size + cell_margin),
            (float)state.board_top_left_y + cell_margin + coord.r * (cell_size + cell_margin)
        };
    }

    void draw_o(float x, float y, float size, Color color = piece_color) {
        DrawRing(Vector2{x + size / 2.0f, y + size / 2.0f}, (size / 3), (size / 2), 0, 360, 100, color);
    }

    void draw_o(const State& state, Coordinate coord, Color color = piece_color) {
        const float margin = cell_size * 0.1;
        const Vector2 pos = get_cell_screen_pos(state, coord);
        draw_o(pos.x + margin, pos.y + margin, cell_size * 0.8, color);
    }

    void draw_x(float x, float y, float size, Color color = piece_color) {
        DrawLineEx(Vector2{x, y}, Vector2{x + size, y + size}, size / 5.0f, color);
        DrawLineEx(Vector2{x + size, y}, Vector2{x, y + size}, size / 5.0f, color);
    }

    void draw_x(const State& state, Coordinate coord, Color color = piece_color) {
        const float margin = cell_size * 0.1;
        const Vector2 pos = get_cell_screen_pos(state, coord);
        draw_x(pos.x + margin, pos.y + margin, cell_size * 0.8, color);
    }

    void draw_piece(const State& state, Cell cell, Coordinate coord, Color color = piece_color) {
        assert(cell == Cell::Empty || cell == Cell::O || cell == Cell::X);
        if (cell == Cell::O) {
            draw_o(state, coord, color);
        } else if (cell == Cell::X) {
            draw_x(state, coord, color);
        }
    }

    Cell get_cell(Player player) {
        assert(player == Player::O || player == Player::X);
        return player == Player::O ? Cell::O : Cell::X;
    }

    void draw_cell(const State& state, Coordinate coord) {
        const Vector2 pos = get_cell_screen_pos(state, coord);
        DrawRectangle(pos.x, pos.y, cell_size, cell_size, cell_color);
        draw_piece(state, state.board.cell[coord.r][coord.c], coord);
    }

    void draw_next_turn_player(const State& state) {
        const float margin = state.board_top_left_x * 0.1f;
        const float size = state.board_top_left_x * 0.8;
        if (state.board.next_turn == Player::O) {
            draw_o(margin, margin, size, cell_color);
        } else {
            draw_x(margin, margin, size, cell_color);
        }
    }

    Coordinate get_cell_for_screen_pos(const State& state, Vector2 pos) {
        for (Coordinate::Type r = 0; r < 3; ++r) {
            for (Coordinate::Type c = 0; c < 3; ++c) {
                const Vector2 cell_pos = get_cell_screen_pos(state, Coordinate(r, c));
                if (pos.x > cell_pos.x && pos.x < cell_pos.x + cell_size && pos.y > cell_pos.y && pos.y < cell_pos.y + cell_size) {
                    return Coordinate(r, c);
                }
            }
        }
    }

    Coordinate::Type index(Coordinate coord) {
        return coord.c + coord.r * 3;
    }

    Cell get_cell(const Board& board, Coordinate coord) {
        return board.cell[coord.r][coord.c];
    }

    void set_cell(Board& board, Coordinate coord, Player p) {
        board.cell[coord.r][coord.c] = get_cell(p);
    }

    void play_move(Board& board, Coordinate coord) {
        if (get_cell(board, coord) == Cell::Empty) {
            set_cell(board, coord, board.next_turn);
            if (board.next_turn == Player::O) {
                board.next_turn = Player::X;
            } else {
                board.next_turn = Player::O;
            }
        }
    }

    bool is_valid(Coordinate coord) {
        return coord.r < 3 && coord.c < 3;
    }
}

using namespace mcts;

int main() {
    State state{};
    state.board_top_left_x = (window_width - board_size) / 2;
    state.board_top_left_y = (window_height - board_size) / 2;
    state.board.cell[0][0] = Cell::X;
    state.board.cell[1][0] = Cell::O;

    init();

    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            const Vector2 pos = GetMousePosition();
            const Coordinate coord = get_cell_for_screen_pos(state, pos);
            if (is_valid(coord)) {
                play_move(state.board, coord);
            }
        }

        BeginDrawing();
        {
            //void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);
            //void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);                // Draw a line
            //void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);                                     // Draw a line (using gl lines)
            //void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);       


            ClearBackground(background_color);

            for (Coordinate::Type r = 0; r < 3; ++r) {
                for (Coordinate::Type c = 0; c < 3; ++c) {
                    draw_cell(state, Coordinate(r, c));
                }
            }

            draw_next_turn_player(state);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
