#pragma once
#include <cassert>
#include <memory>
#include <vector>
#include <array>
#include <optional>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <fstream>

enum class outcome : char {
    none = 0,
    player1 = 1,
    player2 = 2,
    draw = 3
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
        std::cout << std::endl;
    }
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
                for (const auto &[outcome, count]: child->outcomes()) {
                    outcomes_[outcome] += count;
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

    void print_info(void) const {
        std::cout << "States: " << num_states() << "\n"
                  << "Leaves: " << get_all_leaves().size() << "\n";
    
        for (auto outcome : outcomes_) {
            std::cout << "Outcome: " << static_cast<int>(outcome.first) << ", Count: " << outcome.second << "\n";
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

    board_state get_board_state() const {
        board_state state{};
        if (!move_.has_value()) return state;
        const node *current = this;
        while (current) {
            if (!current->move_.has_value()) break;
            state[current->move_->x][current->move_->y] = current->move_->player ? player::player2 : player::player1;
            current = current->prev_;
        }
        return state;
    }

    int num_states() const {
        int count = 1;
        for (const auto &child: next_) {
            count += child->num_states();
        }
        return count;
    }

    const std::unordered_map<outcome, int> &outcomes() const {
        return outcomes_;
    }

    void subtract_outcomes(const std::unordered_map<outcome, int> &other) {
        for (const auto &pair: other) {
            outcomes_[pair.first] -= pair.second;
            if (outcomes_[pair.first] == 0) outcomes_.erase(pair.first);
        }
        if (prev_ == nullptr) return;
        prev_->subtract_outcomes(other);
    }

    void add_outcomes(const std::unordered_map<outcome, int> &other) {
        for (const auto &pair: other) {
            outcomes_[pair.first] += pair.second;
        }
        if (prev_ == nullptr) return;
        prev_->add_outcomes(other);
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

    outcome get_outcome() const {
        return outcome_;
    }

    void make_leaf() {
        assert(outcomes_.size() == 1);
        next_.clear();
        prev_->subtract_outcomes(outcomes_);
        outcome o = outcomes_.begin()->first;
        outcomes_[o] = 1;
        outcome_ = o;
        prev_->add_outcomes(outcomes_);
    }

    std::string to_json(int indent = 0) const {
        std::ostringstream oss;
        std::string indent_str(indent, ' ');
        std::string indent_str_next(indent + 2, ' ');

        oss << indent_str << "{\n";
        oss << indent_str_next << "\"move\": ";
        if (move_) {
            oss << "\"Player " << (move_->player ? "O" : "X")
                << " to (" << static_cast<int>(move_->x)
                << ", " << static_cast<int>(move_->y) << ")\"";
        } else {
            oss << "\"Start\"";
        }
        oss << ",\n";

        oss << indent_str_next << "\"outcome\": \"" << static_cast<int>(outcome_) << "\",\n";
        oss << indent_str_next << "\"children\": [\n";

        for (size_t i = 0; i < next_.size(); ++i) {
            oss << next_[i]->to_json(indent + 4);
            if (i + 1 < next_.size()) {
                oss << ",";
            }
            oss << "\n";
        }

        oss << indent_str_next << "]\n";
        oss << indent_str << "}";
        return oss.str();
    }

    void export_tree_to_html(const node& root, const std::string& filename = "tree.html") {
        std::ofstream file(filename);
        file << R"""(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Tic-Tac-Toe Game Tree</title>
    <style>
        ul { list-style-type: none; padding-left: 20px; }
        li::before { content: ""; padding-right: 5px; cursor: pointer; }
        li.expanded::before { content: "▼"; }
        span.node { cursor: pointer; }
    </style>
</head>
<body>
    <h1>Tic-Tac-Toe Game Tree</h1>
    <div id="tree-container"></div>
    <script>
        const treeData = )""" << root.to_json() << R"""(;

        function createTreeNode(node) {
            const li = document.createElement("li");
            const container = document.createElement("div");
            container.style.display = "flex";
            container.style.alignItems = "center";

            const toggle = document.createElement("span");
            toggle.textContent = "▶";
            toggle.style.cursor = "pointer";
            toggle.style.width = "1em";
            toggle.style.display = node.children.length ? "inline-block" : "none";

            const label = document.createElement("span");
            label.className = "node";
            label.textContent = node.move + " (Outcome: " + node.outcome + ")";
            label.style.marginLeft = "0.5em";

            container.appendChild(toggle);
            container.appendChild(label);
            li.appendChild(container);

            const ul = document.createElement("ul");
            ul.style.display = "none";
            for (const child of node.children) {
                ul.appendChild(createTreeNode(child));
            }
            li.appendChild(ul);

            toggle.onclick = () => {
                const isExpanded = ul.style.display === "block";
                ul.style.display = isExpanded ? "none" : "block";
                toggle.textContent = isExpanded ? "▶" : "▼";
            };

            label.onclick = toggle.onclick;

            return li;
        }

        const treeContainer = document.getElementById("tree-container");
        const ul = document.createElement("ul");
        ul.appendChild(createTreeNode(treeData));
        treeContainer.appendChild(ul);
    </script>
</body>
</html>
        )""";
        file.close();
    }

};
