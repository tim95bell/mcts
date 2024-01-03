
#include "mcts.hpp"
#include "engine.hpp"
#include "util.hpp"

namespace tic_tac_toe {
namespace mcts {
    struct Node {
        engine::Coordinate coord;
        double score; // numerator
        double visit_count; // denominator
        Node* parent;
        Node* children[9];
        U8 children_count;
        engine::Player perspective;
    };

    static Node& select(engine::Board& board, Node& node) {
        static const double c = sqrt(2.0);

        if (board.game_end != engine::GameEnd::None) {
            return node;
        }

        if (node.visit_count == 0.0 && node.parent != nullptr) {
            return node;
        }

        if (node.children_count == 0) {
            // create a new node for every possible move
            for (U8 i = 0; i < 9; ++i) {
                const engine::Coordinate coord(i);
                if (get_cell(board, coord) == engine::Cell::Empty) {
                    node.children[node.children_count] = static_cast<Node*>(malloc(sizeof(Node)));
                    Node& child = *node.children[node.children_count];
                    child.coord = coord;
                    child.score = 0.0;
                    child.visit_count = 0.0;
                    child.parent = &node;
                    child.children_count = 0;
                    child.perspective = nott(node.perspective);
                    ++node.children_count;
                }
            }

            if (node.children_count == 0) {
                // this should not be able to happen, otherwise board.game_end should not be GameEnd::None
                assert(false);
                return node;
            }

            const U8 index = node.children_count == 1 ? 0 : util::random(0, node.children_count);
            Node& result = *node.children[index];
            play_move(board, result.coord);
            return result;
        }

        {
            U8 children_with_no_visits = 0;
            for (U8 i = 0; i < node.children_count; ++i) {
                Node& child = *node.children[i];
                if (child.visit_count == 0.0) {
                    ++children_with_no_visits;
                }
            }

            if (children_with_no_visits > 0) {
                const U8 index = children_with_no_visits == 1 ? 0 : util::random(0, children_with_no_visits);
                U8 no_visit_index = 0;
                for (U8 i = 0; i < node.children_count; ++i) {
                    Node& child = *node.children[i];
                    if (child.visit_count == 0.0) {
                        if (no_visit_index == index) {
                            engine::play_move(board, child.coord);
                            return child;
                        } else {
                            ++no_visit_index;
                        }
                    }
                }
            }
        }

        double highest_uct;
        U8 highest_count = 1;
        U8 highest_indices[9];

        {
            Node& child = *node.children[0];
            highest_uct = child.score / child.visit_count + c * sqrt(log(child.parent->visit_count) / child.visit_count);
            highest_indices[0] = 0;
        }

        for (U8 i = 1; i < node.children_count; ++i) {
            Node& child = *node.children[i];
            const double uct = child.score / child.visit_count + c * sqrt(log(child.parent->visit_count) / child.visit_count);
            if (uct > highest_uct) {
                highest_uct = uct;
                highest_count = 1;
                highest_indices[0] = i;
            } else if (uct == highest_uct) {
                highest_indices[highest_count] = i;
                ++highest_count;
            }
        }

        assert(highest_count > 0);
        const U8 index = highest_count == 1 ? 0 : util::random(0, highest_count);

        assert(highest_indices[index] < node.children_count);
        Node& child = *node.children[highest_indices[index]];
        engine::play_move(board, child.coord);
        return select(board, child);
    }

    static double get_score(engine::Player perspective, engine::GameEnd state) {
        assert(state != engine::GameEnd::None);

        if (perspective == engine::Player::O) {
            if (state == engine::GameEnd::OWin) {
                return 1.0;
            }

            if (state == engine::GameEnd::XWin) {
                return 0.0;
            }
        } else {
            assert(perspective == engine::Player::X);

            if (state == engine::GameEnd::XWin) {
                return 1.0;
            }

            if (state == engine::GameEnd::OWin) {
                return 0.0;
            }
        }

        return 0.5;
    }

    static engine::GameEnd simulate(engine::Board& board) {
        while (board.game_end == engine::GameEnd::None) {
            engine::play_move(board, engine::get_random_move(board));
        }

        return board.game_end;
    }

    static void backprop(Node& node, engine::GameEnd result) {
        ++node.visit_count;
        if (node.parent) {
            node.score += get_score(node.parent->perspective, result);
        }
        //node.score += get_score(node.perspective, result);

        if (node.parent) {
            backprop(*node.parent, result);
        }
    }

    void generate_computer_moves(engine::Board& board) {
        if (board.game_end != engine::GameEnd::None) {
            return;
        }

        engine::Board board_copy;
        memcpy(&board_copy, &board, sizeof(board));

        Node root_node{};
        root_node.perspective = board.next_turn;

        for (U32 i = 0; i < 1000 * 1000 * 10; ++i) {
            Node& node = select(board, root_node);
            const engine::GameEnd result = simulate(board);
            backprop(node, result);
            memcpy(&board, &board_copy, sizeof(board));
        }

        // do one more iteration but skip simulation and the outcome of the simulation is irrelevant.
        // because the score result of the last iteration is ignored anyway.
        // since the best move is selected purely on visit count.
        {
            Node& node = select(board, root_node);
            backprop(node, engine::GameEnd::Draw);
            memcpy(&board, &board_copy, sizeof(board));
        }

        assert(root_node.children_count > 0);

        double highest_visit_count = 0.0;
        for (U8 i = 0; i < root_node.children_count; ++i) {
            if (root_node.children[i]->visit_count > highest_visit_count) {
                highest_visit_count = root_node.children[i]->visit_count;
            }
        }

        U8 nodes_with_highest_visit_count_count = 0;
        for (U8 i = 0; i < root_node.children_count; ++i) {
            if (root_node.children[i]->visit_count == highest_visit_count) {
                Node* tmp = root_node.children[nodes_with_highest_visit_count_count];
                root_node.children[nodes_with_highest_visit_count_count] = root_node.children[i];
                root_node.children[i] = tmp;
                ++nodes_with_highest_visit_count_count;
            }
        }

        assert(nodes_with_highest_visit_count_count > 0);

        board.ai_best_moves_count = nodes_with_highest_visit_count_count;
        for (U32 i = 0; i < nodes_with_highest_visit_count_count; ++i) {
            board.ai_best_moves[i] = root_node.children[i]->coord;
        }
    }
} // namespace mcts
} // namespace tic_tac_toe
