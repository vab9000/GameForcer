#include "node.hpp"
#include <iostream>
#include <ranges>

inline bool prune_player_options(const std::vector<std::weak_ptr<node> > &leaves, const player outcome_player,
                                 const bool same_player) {
    bool modified = false;
    for (auto &leaf: leaves) {
        if (leaf.expired()) continue;

        auto leaf_ptr = leaf.lock();

        if (leaf_ptr->prev()->next().size() == 1) continue;
        if (same_player
                ? (leaf_ptr->get_move()->player ? player::player2 : player::player1) == outcome_player
                : (leaf_ptr->get_move()->player ? player::player2 : player::player1) != outcome_player) {
            std::erase_if(leaf_ptr->prev()->next(),
                          [&leaf_ptr, &modified](const std::shared_ptr<node> &child) {
                              if (child.get() != leaf_ptr.get()) {
                                  child->prev()->subtract_outcomes(child->outcomes());
                                  modified = true;
                                  return true;
                              }
                              return false;
                          });
        } else {
            std::erase_if(leaf_ptr->prev()->next(),
                          [&leaf_ptr, &modified](const std::shared_ptr<node> &child) {
                              if (child.get() == leaf_ptr.get()) {
                                  child->prev()->subtract_outcomes(child->outcomes());
                                  modified = true;
                                  return true;
                              }
                              return false;
                          });
        }
    }
    return modified;
}

inline bool prune_tree(const node &root, const outcome desired_outcome, const player outcome_player) {
    bool modified = false;
    const std::vector<std::weak_ptr<node> > leaves_with_outcome = root.get_leaf_nodes_with_outcome(desired_outcome);
    if (prune_player_options(leaves_with_outcome, outcome_player, true)) {
        modified = true;
    }

    const std::vector<std::weak_ptr<node> > leaves_without_outcome = root.get_leaf_nodes_without_outcome(
        desired_outcome);
    if (prune_player_options(leaves_without_outcome, outcome_player, false)) {
        modified = true;
    }
    return modified;
}

inline bool fold_tree(node &root) {
    bool modified = false;
    for (const auto &child: root.next()) {
        if (child->get_outcome() != outcome::none || child->next().size() == 1) {
            if (fold_tree(*child)) {
                modified = true;
            }
            continue;
        }
        const auto &outcomes = child->outcomes();
        int num_outcomes = 0;
        auto child_outcome = outcome::none;
        for (const auto &[outcome, count]: outcomes) {
            if (count > 0) {
                child_outcome = outcome;
                num_outcomes++;
            }
        }
        if (num_outcomes == 1) {
            child->get_outcome() = child_outcome;
            for (const auto &child_child: child->next()) {
                child->subtract_outcomes(child_child->outcomes());
            }
            child->next().clear();
            child->add_outcome(child_outcome);
            modified = true;
        } else {
            if (fold_tree(*child)) {
                modified = true;
            }
        }
    }

    return modified;
}

inline void run_algorithm(node &root) {
    constexpr auto desired_outcome = outcome::player1;
    constexpr auto outcome_player = player::player1;

    bool modified = true;
    while (modified) {
        modified = false;
        if (prune_tree(root, desired_outcome, outcome_player)) {
            modified = true;
        }
        if (fold_tree(root)) {
            modified = true;
        }
        std::cout << "Num states: " << root.num_states() << std::endl;
        const std::vector<std::weak_ptr<node> > leaves = root.get_all_leaves();
        std::cout << "Num leaves: " << leaves.size() << std::endl;
        for (const auto &root_outcomes = root.outcomes(); const auto &[outcome, count]: root_outcomes) {
            std::cout << "Outcome: " << static_cast<int>(outcome) << ", Count: " << count << std::endl;
        }
    }

    std::cout << "Num states: " << root.num_states() << std::endl;

    const std::vector<std::weak_ptr<node> > leaves = root.get_all_leaves();
    std::cout << "Num leaves: " << leaves.size() << std::endl;
    for (const auto &root_outcomes = root.outcomes(); const auto &[outcome, count]: root_outcomes) {
        std::cout << "Outcome: " << static_cast<int>(outcome) << ", Count: " << count << std::endl;
    }
    print_board(root.get_first_leaf()->get_board_state());
}

int main() {
    node root(nullptr, std::nullopt);
    run_algorithm(root);

    return 0;
}
