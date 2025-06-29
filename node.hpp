#pragma once
#include <vector>
#include <array>
#include <optional>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

enum class outcome : char {
    none,
    player1,
    player2,
    draw
};

using move = struct {
    bool player: 1;
    unsigned char x: 2;
    unsigned char y: 2;
};

enum class player : char {
    none,
    player1 = 'X',
    player2 = 'O',
};

using board_state = std::array<std::array<player, 3>, 3>;

inline void print_board(const board_state &state) {
    for (const auto &row: state) {
        for (const auto &cell: row) {
            if (cell == player::none) {
                std::cout << '.';
            } else {
                std::cout << static_cast<char>(cell);
            }
        }
        std::cout << '\n';
    }
    std::cout << "-----\n";
}

inline player is_winning(const board_state &state) {
    for (int i = 0; i < 3; ++i) {
        if (state[i][0] != player::none && state[i][0] == state[i][1] && state[i][1] == state[i][2]) return state[i][0];
        // Row
        if (state[0][i] != player::none && state[0][i] == state[1][i] && state[1][i] == state[2][i]) return state[0][i];
        // Row
    }
    if (state[0][0] != player::none && state[0][0] == state[1][1] && state[1][1] == state[2][2]) return state[0][0];
    // Diagonal
    if (state[0][2] != player::none && state[0][2] == state[1][1] && state[1][1] == state[2][0]) return state[0][2];
    // Diagonal
    return player::none;
}

inline bool is_full(const board_state &state) {
    for (const auto &row: state) {
        for (const auto &cell: row) {
            if (cell == player::none) return false;
        }
    }
    return true;
}

class node {
    node *prev_;
    std::vector<std::shared_ptr<node> > next_;
    outcome outcome_;
    std::optional<move> move_;
    std::unordered_map<outcome, int> outcomes_;

public:
    node(node *prev, const std::optional<move> move) : prev_(prev), move_(move) {
        const board_state state = get_board_state();
        if (const player winner = is_winning(state); winner == player::none) {
            if (is_full(state)) {
                outcome_ = outcome::draw;
                outcomes_[outcome::draw] += 1;
                return;
            }
            outcome_ = outcome::none;
            gen_next_moves();
            for (const auto &child: next_) {
                for (const auto &child_outcome: child->outcomes()) {
                    outcomes_[child_outcome.first] += child_outcome.second;
                }
            }
        } else if (winner == player::player1) {
            outcome_ = outcome::player1;
            outcomes_[outcome::player1] += 1;
        } else if (winner == player::player2) {
            outcome_ = outcome::player2;
            outcomes_[outcome::player2] += 1;
        } else {
            outcome_ = outcome::draw;
            outcomes_[outcome::draw] += 1;
        }
    }

    void print_next() const {
        std::cout << "size: " << next_.size() << "\n";
        for (const auto &child: next_) {
            std::cout << child.get() << "\n";
        }
        std::cout << std::endl;
    }

    void gen_next_moves() {
        const bool next_player = move_.has_value() ? !move_->player : false;
        const board_state state = get_board_state();
        for (unsigned char x = 0; x < 3; ++x) {
            for (unsigned char y = 0; y < 3; ++y) {
                if (state[x][y] == player::none) {
                    move next_move{next_player, x, y};
                    next_.emplace_back(std::make_shared<node>(this, next_move));
                }
            }
        }
    }

    [[nodiscard]] board_state get_board_state() const {
        board_state state{};
        if (!move_.has_value()) return state;
        const node *current = this;
        while (current) {
            state[current->move_->x][current->move_->y] = current->move_->player ? player::player2 : player::player1;
            current = current->prev_;
        }
        return state;
    }

    [[nodiscard]] int num_states() const {
        int count = 1;
        for (const auto &child: next_) {
            count += child->num_states();
        }
        return count;
    }

    [[nodiscard]] const node *get_first_leaf() const {
        if (outcome_ != outcome::none) {
            return this;
        }
        return next_.front()->get_first_leaf();
    }

    const std::unordered_map<outcome, int> &outcomes() const {
        return outcomes_;
    }

    void subtract_outcomes(const std::unordered_map<outcome, int> &other) {
        for (const auto &pair: other) {
            outcomes_[pair.first] -= pair.second;
        }
        if (prev_ == nullptr) return;
        prev_->subtract_outcomes(other);
    }

    std::vector<std::weak_ptr<node> > get_leaf_nodes_with_outcome(const outcome target_outcome) const {
        std::vector<std::weak_ptr<node> > nodes;

        for (const auto &child: next_) {
            if (child->outcome_ == target_outcome) {
                nodes.push_back(child);
            }
            auto child_nodes = child->get_leaf_nodes_with_outcome(target_outcome);
            nodes.insert(nodes.end(), child_nodes.begin(), child_nodes.end());
        }
        return nodes;
    }

    std::vector<std::weak_ptr<node> > get_leaf_nodes_without_outcome(const outcome target_outcome) const {
        std::vector<std::weak_ptr<node> > nodes;

        for (const auto &child: next_) {
            if (child->outcome_ != target_outcome && child->outcome_ != outcome::none) {
                nodes.push_back(child);
            }
            auto child_nodes = child->get_leaf_nodes_with_outcome(target_outcome);
            nodes.insert(nodes.end(), child_nodes.begin(), child_nodes.end());
        }
        return nodes;
    }

    std::vector<std::weak_ptr<node> > get_all_leaves() const {
        std::vector<std::weak_ptr<node> > leaves;

        for (const auto &child: next_) {
            if (child->outcome_ != outcome::none) {
                leaves.push_back(child);
            }
            auto child_leaves = child->get_all_leaves();
            leaves.insert(leaves.end(), child_leaves.begin(), child_leaves.end());
        }

        return leaves;
    }

    node *prev() const {
        return prev_;
    }

    const std::vector<std::shared_ptr<node> > &next() const {
        return next_;
    }

    std::vector<std::shared_ptr<node> > &next() {
        return next_;
    }

    std::optional<move> get_move() const {
        return *move_;
    }
};
