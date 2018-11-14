#include <iostream>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unordered_set>

#include <compressed/PCBlockTree.h>
#include <compressed/CBlockTree.h>

int main() {

    std::string input;
    std::ifstream t("/home/sking32/Thesis/Software/BlockTree/cmake-build-debug/in");
    std::stringstream buffer;
    buffer << t.rdbuf();
    input = buffer.str();

    std::unordered_set<int> characters;
    for (char c: input) {
        characters.insert(c);
    }


    for (int r = 2; r<=8; r*=2) {
        for (int max_leaf_length = 4; max_leaf_length <= 128; max_leaf_length*=2) {

            std::cout << "r=" << r << ", mll=" << max_leaf_length << std::endl << std::endl;

            BlockTree *heuristic_concatenate_bt = new BlockTree(input, r, max_leaf_length);
            heuristic_concatenate_bt->process_back_pointers_heuristic_concatenate();

            for (int c: characters)
                heuristic_concatenate_bt->add_rank_select_support(c);
            std::cout << "heuristic_concatenate_bt" << std::endl;
            heuristic_concatenate_bt->print_statistics();

            CBlockTree *heuristic_concatenate_cbt = new CBlockTree(heuristic_concatenate_bt);
            std::cout << "heuristic_concatenate_cbt" << std::endl;
            heuristic_concatenate_cbt->print_statistics();
            std::cout << std::endl;
            std::cout << std::endl;

            delete heuristic_concatenate_bt;
            delete heuristic_concatenate_cbt;


            BlockTree *heuristic_bt = new BlockTree(input, r, max_leaf_length);
            heuristic_bt->process_back_pointers_heuristic();

            for (int c: characters)
                heuristic_bt->add_rank_select_support(c);
            std::cout << "heuristic_bt" << std::endl;
            heuristic_bt->print_statistics();

            CBlockTree *heuristic_cbt = new CBlockTree(heuristic_bt);
            std::cout << "heuristic_cbt" << std::endl;
            heuristic_cbt->print_statistics();
            std::cout << std::endl;
            std::cout << std::endl;

            delete heuristic_bt;
            delete heuristic_cbt;




            BlockTree *liberal_heuristic_bt = new BlockTree(input, r, max_leaf_length);
            liberal_heuristic_bt->process_back_pointers_liberal_heuristic();

            for (int c: characters)
                liberal_heuristic_bt->add_rank_select_support(c);
            std::cout << "liberal_heuristic_bt" << std::endl;
            liberal_heuristic_bt->print_statistics();

            CBlockTree *liberal_heuristic_cbt = new CBlockTree(liberal_heuristic_bt);
            std::cout << "liberal_heuristic_cbt" << std::endl;
            liberal_heuristic_cbt->print_statistics();
            std::cout << std::endl;
            std::cout << std::endl;

            delete liberal_heuristic_bt;
            delete liberal_heuristic_cbt;


            for (int c = 2; c<=16; c*=2) {
                BlockTree *conservative_heuristic_c_bt = new BlockTree(input, r, max_leaf_length);
                conservative_heuristic_c_bt->process_back_pointers_conservative_heuristic(c);

                for (int c: characters)
                    conservative_heuristic_c_bt->add_rank_select_support(c);
                std::cout << "conservative_heuristic_" << c << "_bt" << std::endl;
                conservative_heuristic_c_bt->print_statistics();

                CBlockTree *conservative_heuristic_c_cbt = new CBlockTree(conservative_heuristic_c_bt);
                std::cout << "conservative_heuristic_" << c <<"_cbt" << std::endl;
                conservative_heuristic_c_cbt->print_statistics();
                std::cout << std::endl;
                std::cout << std::endl;

                delete conservative_heuristic_c_bt;
                delete conservative_heuristic_c_cbt;
            }

            BlockTree *conservative_heuristic_bt = new BlockTree(input, r, max_leaf_length);
            conservative_heuristic_bt->process_back_pointers_conservative_heuristic();

            for (int c: characters)
                conservative_heuristic_bt->add_rank_select_support(c);
            std::cout << "conservative_heuristic_bt" << std::endl;
            conservative_heuristic_bt->print_statistics();

            CBlockTree *conservative_heuristic_cbt = new CBlockTree(conservative_heuristic_bt);
            std::cout << "conservative_heuristic_cbt" << std::endl;
            conservative_heuristic_cbt->print_statistics();
            std::cout << std::endl;
            std::cout << std::endl;

            delete conservative_heuristic_bt;
            delete conservative_heuristic_cbt;



            BlockTree *conservative_optimized_heuristic_bt = new BlockTree(input, r, max_leaf_length);
            conservative_optimized_heuristic_bt->process_back_pointers_conservative_optimized_heuristic();

            for (int c: characters)
                conservative_optimized_heuristic_bt->add_rank_select_support(c);
            std::cout << "conservative_optimized_heuristic_bt" << std::endl;
            conservative_optimized_heuristic_bt->print_statistics();

            CBlockTree *conservative_optimized_heuristic_cbt = new CBlockTree(conservative_optimized_heuristic_bt);
            std::cout << "conservative_optimized_heuristic_cbt" << std::endl;
            conservative_optimized_heuristic_cbt->print_statistics();
            std::cout << std::endl;
            std::cout << std::endl;

            delete conservative_optimized_heuristic_bt;
            delete conservative_optimized_heuristic_cbt;


            BlockTree *bt = new BlockTree(input, r, max_leaf_length);
            bt->process_back_pointers();
            for (int c: characters)
                bt->add_rank_select_support(c);
            std::cout << "bt_no_clean" << std::endl;
            bt->print_statistics();

            CBlockTree *cbt = new CBlockTree(bt);
            std::cout << "cbt_no_clean" << std::endl;
            cbt->print_statistics();
            std::cout << std::endl;
            delete cbt;

            bt->clean_unnecessary_expansions();
            for (int c: characters)
                bt->add_rank_select_support(c);
            std::cout << "bt" << std::endl;
            bt->print_statistics();


            cbt = new CBlockTree(bt);
            std::cout << "cbt" << std::endl;
            cbt->print_statistics();
            std::cout << std::endl;

            delete bt;
            delete cbt;
        }
    }





    for (int r = 2; r<=8; r*=2) {
        for (int max_leaf_length = 4; max_leaf_length <= 128; max_leaf_length*=2) {

            std::cout << "r=" << r << ", mll=" << max_leaf_length << std::endl << std::endl;

            PBlockTree *heuristic_concatenate_bt = new PBlockTree(input, r, max_leaf_length);
            heuristic_concatenate_bt->process_back_pointers_heuristic_concatenate();

            for (int c: characters)
                heuristic_concatenate_bt->add_rank_select_support(c);
            std::cout << "padding_heuristic_concatenate_bt" << std::endl;
            heuristic_concatenate_bt->print_statistics();

            PCBlockTree *heuristic_concatenate_cbt = new PCBlockTree(heuristic_concatenate_bt);
            std::cout << "padding_heuristic_concatenate_cbt" << std::endl;
            heuristic_concatenate_cbt->print_statistics();
            std::cout << std::endl;
            std::cout << std::endl;

            delete heuristic_concatenate_bt;
            delete heuristic_concatenate_cbt;


            PBlockTree *heuristic_bt = new PBlockTree(input, r, max_leaf_length);
            heuristic_bt->process_back_pointers_heuristic();

            for (int c: characters)
                heuristic_bt->add_rank_select_support(c);
            std::cout << "padding_heuristic_bt" << std::endl;
            heuristic_bt->print_statistics();

            PCBlockTree *heuristic_cbt = new PCBlockTree(heuristic_bt);
            std::cout << "padding_heuristic_cbt" << std::endl;
            heuristic_cbt->print_statistics();
            std::cout << std::endl;
            std::cout << std::endl;

            delete heuristic_bt;
            delete heuristic_cbt;




            PBlockTree *liberal_heuristic_bt = new PBlockTree(input, r, max_leaf_length);
            liberal_heuristic_bt->process_back_pointers_liberal_heuristic();

            for (int c: characters)
                liberal_heuristic_bt->add_rank_select_support(c);
            std::cout << "padding_liberal_heuristic_bt" << std::endl;
            liberal_heuristic_bt->print_statistics();

            PCBlockTree *liberal_heuristic_cbt = new PCBlockTree(liberal_heuristic_bt);
            std::cout << "padding_liberal_heuristic_cbt" << std::endl;
            liberal_heuristic_cbt->print_statistics();
            std::cout << std::endl;
            std::cout << std::endl;

            delete liberal_heuristic_bt;
            delete liberal_heuristic_cbt;


            for (int c = 2; c<=16; c*=2) {
                PBlockTree *conservative_heuristic_c_bt = new PBlockTree(input, r, max_leaf_length);
                conservative_heuristic_c_bt->process_back_pointers_conservative_heuristic(c);

                for (int c: characters)
                    conservative_heuristic_c_bt->add_rank_select_support(c);
                std::cout << "padding_conservative_heuristic_" << c << "_bt" << std::endl;
                conservative_heuristic_c_bt->print_statistics();

                PCBlockTree *conservative_heuristic_c_cbt = new PCBlockTree(conservative_heuristic_c_bt);
                std::cout << "padding_conservative_heuristic_" << c <<"_cbt" << std::endl;
                conservative_heuristic_c_cbt->print_statistics();
                std::cout << std::endl;
                std::cout << std::endl;

                delete conservative_heuristic_c_bt;
                delete conservative_heuristic_c_cbt;
            }

            PBlockTree *conservative_heuristic_bt = new PBlockTree(input, r, max_leaf_length);
            conservative_heuristic_bt->process_back_pointers_conservative_heuristic();

            for (int c: characters)
                conservative_heuristic_bt->add_rank_select_support(c);
            std::cout << "padding_conservative_heuristic_bt" << std::endl;
            conservative_heuristic_bt->print_statistics();

            PCBlockTree *conservative_heuristic_cbt = new PCBlockTree(conservative_heuristic_bt);
            std::cout << "padding_conservative_heuristic_cbt" << std::endl;
            conservative_heuristic_cbt->print_statistics();
            std::cout << std::endl;
            std::cout << std::endl;

            delete conservative_heuristic_bt;
            delete conservative_heuristic_cbt;



            PBlockTree *conservative_optimized_heuristic_bt = new PBlockTree(input, r, max_leaf_length);
            conservative_optimized_heuristic_bt->process_back_pointers_conservative_optimized_heuristic();

            for (int c: characters)
                conservative_optimized_heuristic_bt->add_rank_select_support(c);
            std::cout << "padding_conservative_optimized_heuristic_bt" << std::endl;
            conservative_optimized_heuristic_bt->print_statistics();

            PCBlockTree *conservative_optimized_heuristic_cbt = new PCBlockTree(conservative_optimized_heuristic_bt);
            std::cout << "padding_conservative_optimized_heuristic_cbt" << std::endl;
            conservative_optimized_heuristic_cbt->print_statistics();
            std::cout << std::endl;
            std::cout << std::endl;

            delete conservative_optimized_heuristic_bt;
            delete conservative_optimized_heuristic_cbt;


            PBlockTree *bt = new PBlockTree(input, r, max_leaf_length);
            bt->process_back_pointers();
            for (int c: characters)
                bt->add_rank_select_support(c);
            std::cout << "padding_bt_no_clean" << std::endl;
            bt->print_statistics();

            PCBlockTree *cbt = new PCBlockTree(bt);
            std::cout << "padding_cbt_no_clean" << std::endl;
            cbt->print_statistics();
            std::cout << std::endl;
            delete cbt;

            bt->clean_unnecessary_expansions();
            for (int c: characters)
                bt->add_rank_select_support(c);
            std::cout << "padding_bt" << std::endl;
            bt->print_statistics();


            cbt = new PCBlockTree(bt);
            std::cout << "cbt" << std::endl;
            cbt->print_statistics();
            std::cout << std::endl;

            delete bt;
            delete cbt;
        }
    }
    return 0;
}

