
#include "mcts.hpp"
#include "engine.hpp"
#include "util.hpp"

namespace tic_tac_toe {
namespace mcts {
    struct Node {
        Node() = default;

        ~Node() {
            for (U8 i = 0; i < children_count; ++i) {
                delete children[i];
            }
        }

        engine::Coordinate coord;
        double score; // numerator
        double visit_count; // denominator
        Node* parent;
        Node* children[9];
        U8 children_count;
        engine::Player perspective;
    };

    template <typename FilterFunction>
    static Node* random_child(Node& node, FilterFunction filter) {
        U8 count = 0;
        U8 indices[9];

        for (U8 i = 0; i < node.children_count; ++i) {
            if (filter(*node.children[i])) {
                indices[count] = i;
                ++count;
            }
        }
        
        if (count == 0) {
            return nullptr;
        }

        return node.children[indices[util::random(0, count)]];
    }

    static double uct(double score, double visit_count, double parent_visit_count) {
        static const double c = sqrt(2.0);
        return score / visit_count + c * sqrt(log(parent_visit_count) / visit_count);
    }

    template <typename ValueFunction>
    static Node* random_child_with_highest_value(Node& node, ValueFunction value_function) {
        if (node.children_count == 0) {
            return nullptr;
        }

        double highest_value;
        U8 highest_count = 1;
        U8 highest_indices[9];

        {
            Node& child = *node.children[0];
            highest_value = value_function(child);
            highest_indices[0] = 0;
        }

        for (U8 i = 1; i < node.children_count; ++i) {
            Node& child = *node.children[i];
            const double value = value_function(child);
            if (value > highest_value) {
                highest_value = value;
                highest_count = 1;
                highest_indices[0] = i;
            } else if (value == highest_value) {
                highest_indices[highest_count] = i;
                ++highest_count;
            }
        }

        assert(highest_count > 0);
        const U8 index = highest_count == 1 ? 0 : util::random(0, highest_count);

        assert(highest_indices[index] < node.children_count);
        return node.children[highest_indices[index]];
    }
    
    template <typename ResultType, typename ValueType>
    static U8 children_with_highest_value(Node& node, ResultType result[9], ValueType (*value_function)(Node& child), ResultType (*child_to_result)(Node& child)) {
        if (node.children_count == 0) {
            return 0;
        }

        ValueType highest_value;
        U8 highest_count = 1;
        U8 highest_indices[9];

        {
            Node& child = *node.children[0];
            highest_value = value_function(child);
            highest_indices[0] = 0;
        }

        for (U8 i = 1; i < node.children_count; ++i) {
            Node& child = *node.children[i];
            const ValueType value = value_function(child);
            if (value > highest_value) {
                highest_value = value;
                highest_count = 1;
                highest_indices[0] = i;
            } else if (value == highest_value) {
                highest_indices[highest_count] = i;
                ++highest_count;
            }
        }

        for (U8 i = 0; i < highest_count; ++i) {
            result[i] = child_to_result(*node.children[highest_indices[i]]);
        }

        return highest_count;
    }

    static Node& select(engine::Board& board, Node& node) {
        // if game is over at this node, select this node
        if (board.game_end != engine::GameEnd::None) {
            return node;
        }

        // if this node has not been visited, select this node
        if (node.visit_count == 0.0 && node.parent != nullptr) {
            // this should not be possible
            assert(false);
            return node;
        }

        // if this node has no children, create all possible children, and randomly select one of them
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
                    child.perspective = other(node.perspective);
                    ++node.children_count;
                }
            }

            if (node.children_count == 0) {
                // this should not be able to happen, otherwise board.game_end should not be GameEnd::None
                assert(false);
                return node;
            }

            const U8 index = util::random(0, node.children_count);
            Node& result = *node.children[index];
            play_move(board, result.coord);
            return result;
        }

        {
            // if some children have no visits, one of them should be visited next
            // check seperately instead of calculating uct to avoid dividing by zero
            Node* child = random_child(node, [](Node& child) {
                return child.visit_count == 0.0;
            });

            if (child) {
                engine::play_move(board, child->coord);
                return *child;
            }
        }

        {
            Node* child = random_child_with_highest_value(node, [](Node& child) {
                return uct(child.score, child.visit_count, child.parent->visit_count);
            });

            assert(child);
            engine::play_move(board, child->coord);
            return select(board, *child);
        }
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

        for (U32 i = 0; i < 1000 * 100; ++i) {
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

        board.ai_best_moves_count = children_with_highest_value<engine::Coordinate, double>(root_node, board.ai_best_moves, [](Node& child) {
            return child.visit_count;
        }, [](Node& child) {
            return child.coord;
        });

        assert(board.ai_best_moves_count > 0);
    }
} // namespace mcts
} // namespace tic_tac_toe
