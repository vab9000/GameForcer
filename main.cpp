#include "node.hpp"
#include <iostream>

inline void run_algorithm(const node &root) {
    constexpr auto desired_outcome = outcome::player1;
    constexpr auto player_outcome = player::player1;

    std::cout << "Num states: " << root.num_states() << std::endl;

    const std::vector<std::weak_ptr<node> > leaves = root.get_leaf_nodes_with_outcome(desired_outcome);
    for (auto &leaf: leaves) {
        if (leaf.expired()) continue;

        auto leaf_ptr = leaf.lock();

        if (leaf_ptr->prev()->next().size() == 1) continue;
        if ((leaf_ptr->get_move()->player ? player::player2 : player::player1) == player_outcome) {
            auto it = std::remove_if(leaf_ptr->prev()->next().begin(), leaf_ptr->prev()->next().end(),
                                     [&leaf_ptr](const std::shared_ptr<node> &child) {
                                         if (child.get() != leaf_ptr.get()) {
                                             child->prev()->subtract_outcomes(child->outcomes());
                                             return true;
                                         }
                                         return false;
                                     });
            leaf_ptr->prev()->next().erase(it, leaf_ptr->prev()->next().end());
        } else {
            auto it = std::remove_if(leaf_ptr->prev()->next().begin(), leaf_ptr->prev()->next().end(),
                                     [&leaf_ptr](const std::shared_ptr<node> &child) {
                                         if (child.get() == leaf_ptr.get()) {
                                             child->prev()->subtract_outcomes(child->outcomes());
                                             return true;
                                         }
                                         return false;
                                     });
            leaf_ptr->prev()->next().erase(it, leaf_ptr->prev()->next().end());
        }
    }
    const std::vector<std::weak_ptr<node> > leaves_without_outcome = root.get_leaf_nodes_without_outcome(
        desired_outcome);
    for (auto &leaf: leaves_without_outcome) {
        if (leaf.expired()) continue;

        auto leaf_ptr = leaf.lock();

        if (leaf_ptr->prev()->next().size() == 1) continue;
        if ((leaf_ptr->get_move()->player ? player::player2 : player::player1) != player_outcome) {
            auto it = std::remove_if(leaf_ptr->prev()->next().begin(), leaf_ptr->prev()->next().end(),
                                     [&leaf_ptr](const std::shared_ptr<node> &child) {
                                         if (child.get() != leaf_ptr.get()) {
                                             child->prev()->subtract_outcomes(child->outcomes());
                                             return true;
                                         }
                                         return false;
                                     });
            leaf_ptr->prev()->next().erase(it, leaf_ptr->prev()->next().end());
        } else {
            auto it = std::remove_if(leaf_ptr->prev()->next().begin(), leaf_ptr->prev()->next().end(),
                                     [&leaf_ptr](const std::shared_ptr<node> &child) {
                                         if (child.get() == leaf_ptr.get()) {
                                             child->prev()->subtract_outcomes(child->outcomes());
                                             return true;
                                         }
                                         return false;
                                     });
            leaf_ptr->prev()->next().erase(it, leaf_ptr->prev()->next().end());
        }
    }

    std::cout << "Num states: " << root.num_states() << std::endl;

    const std::vector<std::weak_ptr<node> > leaves_not_none = root.get_all_leaves();
    std::cout << "Num not alone leaves: " << leaves_not_none.size() << std::endl;
    const auto root_outcomes = root.outcomes();
    for (const auto &pair: root_outcomes) {
        std::cout << "Outcome: " << static_cast<int>(pair.first) << ", Count: " << pair.second << std::endl;
    }
}

int main() {
    const node root(nullptr, std::nullopt);
    // std::cout << "Tic Tac Toe Game Tree Created. " << root.num_states() << std::endl;
    run_algorithm(root);

    return 0;
}
