# Description
This repository contains an implementation for the BlockTree data structure described [here](https://ieeexplore.ieee.org/document/7149265). In addition to the paper's proposal, this code contains some improvements, and heuristics to play with.
# Installation Guide
First clone the repo with its submodule:
```
 git clone --recurse-submodules https://github.com/elarielcl/BlockTrees.git
 ```
 This project uses the [sdsl](https://github.com/elarielcl/sdsl-lite) library, so previous to compile it you should install the [sdsl](https://github.com/elarielcl/sdsl-lite). To install it:
 ```
 cd BlockTrees/external/sdsl-lite
 ./install.sh .
 ```
 
This project is a CMake project. To build this project with some runnables you should do

```
cd ../..
mkdir build
cd build
cmake ..
make
```

Finally when you want to create a new executable, you need to add it to the CMakeLists.txt files and link the libraries `blocktree`, `sdsl`, `divsufsort`,  `divsufsort64`, as shown in the following example:
```
add_executable(main main.cpp)
...
target_link_libraries(main ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libsdsl.a)
target_link_libraries(main ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort.a)
target_link_libraries(main ${CMAKE_SOURCE_DIR}/external/sdsl-lite/lib/libdivsufsort64.a)
```

 # Usage Guide
 This code contains some examples of executables from where you can see the different versions of BlockTrees and constructions implemented.
 ## Example
 Let's suppose we want to build a BlockTree following the ideas on its original paper, so we do:
 ```
 ...
 std::string input = "AACCCTGCTGCTGATCGGATCGTAGC";
 int r = 2; //The arity of the BlockTree
 int mll = 16; // The max length that a BlockTree's leaf could represent
 
 BlockTree* bt = new BlockTree(input, r, mll); // This creates the BlockTree object
 bt->process_back_pointers(); // This method builds the BackPointers in the BlockTree
bt->clean_unnecessary_expansions(); // This method removes the expansion of InternalBlocks that are unnecesary (this is an state-of-the-art space improvement, also called pruning)
  ...
 ```
  in case you want to build the heuristic_bt version instead you can replace the last two lines with
 ```
 bt->process_back_pointers_heuristic();
 ```
 .. now you have a proper BlockTree answering access queries(`bt->access(i)`), if you want to give it ``bt->rank(c,i) & bt->select(c,j)`` support you should do:
 ```
 for (int c: characters)
     bt->add_rank_select_support(c);
 ```
 So the above is a pointer-based implementation of BlockTrees, if you want to have a more compact representation (using bitvectors to represent the tree) you should do:
 ```
 ...
 CBlockTree *cbt = new CBlockTree(bt); // Builds a more compact BlockTree representation
 cbt->access(i);
 cbt->select(c,i);
 cbt->print_statistics(); // It prints the space used for the different components of the tree
 ...
 ```
 ... and never forget to delet your trash ;)
 ```
 ...
 delete bt;
 delete cbt;
 ...
 ```
 ### Additional
 - You can change `BlockTree` to `PBlockTree` and `CBlockTree` to `PCBlockTree` and get a faster and even more compact version of the tree (it uses padding in the input sequence).
 - There are other implementations of `rank & select`as well as implementation using compressed int/bitvectors
 - There are many others constructions which are heuristics as well as constructions using a string with a bigger alphabet (``std::basic_string<int64_t>``)
 # Contact
 Any error, improvement or suggestion you can write me to `elarielcl` in Gmail. 
