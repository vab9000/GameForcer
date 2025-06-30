#include "node.hpp"

void prune_non_optimal(node &root) {
    constexpr auto desired_outcome = outcome::player1;
    constexpr auto player_choice = player::player2;

    std::vector<std::weak_ptr<node> > leaves = root.get_all_leaves();
    for (auto &leaf: leaves) {
        if (leaf.expired()) continue;

        auto leaf_ptr = leaf.lock();

        std::vector<std::shared_ptr<node> > &siblings = leaf_ptr->prev()->next();
        if (siblings.size() == 1) continue;
        player turn = leaf_ptr->get_move()->player ? player::player2 : player::player1;

        if ((leaf_ptr->get_outcome() == desired_outcome && turn == player_choice) ||
            (leaf_ptr->get_outcome() != desired_outcome && turn != player_choice)) {

            std::erase_if(siblings, [&leaf_ptr](const std::shared_ptr<node> &child) {
                if (child.get() != leaf_ptr.get()) {
                    child->prev()->subtract_outcomes(child->outcomes());
                    return true;
                }
                return false;
            });
        } else if ((leaf_ptr->get_outcome() == desired_outcome && turn != player_choice) ||
                   (leaf_ptr->get_outcome() != desired_outcome && turn == player_choice)) {
            leaf_ptr->prev()->subtract_outcomes(leaf_ptr->outcomes());
            std::erase_if(siblings,[&leaf_ptr](std::shared_ptr<node> &child) {
                return child.get() == leaf_ptr.get();
            });
        }
    }
}

void simplify_outcomes(node &root) {
    if (root.outcomes().size() == 1) {
        root.make_leaf();
    }
    for (auto child: root.next()) {
        simplify_outcomes(*child.get());
    }
}

int main() {
    node root(nullptr, std::nullopt);
    
    // node *n = &root;
    // while (n->next().size() != 0) {
    //     std::cout << n->next().size() << " possible moves\n";
    //     print_board(n->get_board_state());
    //     n = (n->next().back()).get();
    // }
    // print_board(n->get_board_state());
    //
    root.print_info();
    while (true) {
        std::cout << "Pruning...\n" << std::endl;
        prune_non_optimal(root);
        root.print_info();


        if (root.outcomes().size() == 1) break;

        std::cout << "Simplifying...\n" << std::endl;
        simplify_outcomes(root);
        root.print_info();
    }
    // root.export_tree_to_html(root);
    return 0;
}
