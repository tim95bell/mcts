
#include <cstdio>
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

    static constexpr U32 window_width = 640;
    static constexpr U32 window_height = 480;
    static constexpr U32 board_size = window_width < window_height ? window_width : window_height;
    static constexpr U32 cell_size = board_size / 3;

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

        return 'O';
    }

    void print(const Board& board) {
        printf("next turn: %c\n", board.next_turn == Player::X ? 'x' : 'y');
        for (U32 y = 0; y < 3; ++y) {
            for (U32 x = 0; x < 3; ++x) {
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
}

using namespace mcts;

int main() {
    Board board{};
    board.cell[0][0] = Cell::X;
    board.cell[1][0] = Cell::O;

    init();

    Color color_1{80, 100, 200, 255};
    Color color_2{80, 200, 100, 255};
    Color color_3{200, 100, 80, 255};
    while (!WindowShouldClose()) {

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            const Vector2 pos = GetMousePosition();
            if (pos.x > 0 && pos.x < cell_size && pos.y > 0 && pos.y < cell_size) {
                float tmp = color_1.r;
                color_1.r = color_1.b;
                color_1.b = tmp;
            }
        }

        BeginDrawing();
        {
            //void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);
            //void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);                // Draw a line
            //void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);                                     // Draw a line (using gl lines)
            //void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);       

            Color background_color{200, 215, 230, 255};
            Color o_color = background_color;
            Color x_color = background_color;

            ClearBackground(background_color);

            DrawRectangle(0, 0, cell_size, cell_size, color_1);
            DrawRectangle(cell_size, cell_size, cell_size, cell_size, color_2);
            DrawRectangle(cell_size * 2, cell_size * 2, cell_size, cell_size, color_3);

            DrawRing(Vector2{cell_size + cell_size / 2, cell_size + cell_size / 2}, (cell_size / 3) * 0.8, (cell_size / 2) * 0.8, 0, 360, 100, o_color);

            DrawLineEx(Vector2{cell_size * 2 + cell_size * 0.1, cell_size * 2 + cell_size * 0.1}, Vector2{cell_size * 2 + cell_size * 0.9, cell_size * 2 + cell_size * 0.9}, 18.0, x_color);
            DrawLineEx(Vector2{cell_size * 2 + cell_size * 0.9, cell_size * 2 + cell_size * 0.1}, Vector2{cell_size * 2 + cell_size * 0.1, cell_size * 2 + cell_size * 0.9}, 18.0, x_color);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
