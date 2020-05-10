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
    std::ifstream t("/home/elarielcl/test/BlockTrees/einstein");
    std::stringstream buffer;
    buffer << t.rdbuf();
    input = buffer.str();
    std::cout << input.length() << std::endl;

    std::unordered_set<int> characters;
    for (char c: input) {
        characters.insert(c);
    }

    PBlockTree* bt = new PBlockTree(input, 2, 32);
    bt->process_back_pointers();
    bt->clean_unnecessary_expansions();
    bt->check();

    std::cout << bt->number_of_nodes() << std::endl;
    PCBlockTree* cbt = new PCBlockTree(bt);
    cbt->access(0);

    delete bt;
    delete cbt;
    return 0;
}

