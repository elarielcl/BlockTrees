//
// Created by sking32 on 4/7/18.
//
#include <sdsl/suffix_trees.hpp>
#include <iostream>

using namespace sdsl;
using namespace std;


void print_bp(cst_sct3<> cst) {
    auto current = cst.root();
    cout << '(';
    while (true) {
        if (!cst.is_leaf(current)) {
            cout << '(';
            current = (*cst.children(current).begin());
        } else if (current == cst.parent(current)) {
            cout << ')';
            return;
        } else {
            auto parent = cst.parent(current);
            auto iterator = cst.children(parent);
            auto it = iterator.begin();
            while ((*(it++)) != current);

            while (it == iterator.end()) {
                cout << ')';
                current = parent;
                parent = cst.parent(current);
                if (current == parent) {
                    cout << ')';
                    return;
                }
                iterator = cst.children(parent);
                it = iterator.begin();
                while ((*(it++)) != current);
            }
            cout << ")(";
            current = (*it);
        }
    }
}
int main()
{
    string input;
    ifstream t("/home/sking32/Thesis/Software/BlockTree/cmake-build-debug/in");
    stringstream buffer;
    buffer << t.rdbuf();
    input = buffer.str();


    cst_sct3<> cst;

    construct_im(cst, input, 1);

    print_bp(cst);
}

/*
 *def print_topology_2(root, out):
    out.write('(')
    current = root

    while current != None:
        if current.firstChild != None:
            current = current.firstChild
            out.write('(')
        elif current.next != None:
            out.write(')(')
            current = current.next
        else:
            while current.next == None :
                out.write(')')
                current = current.parent
                if current == None:
                    return
            out.write(')(')
            current = current.next



stree = SuffixTree(open('dna1.0','r').read())


out = open('dna1.0.par','w')
print_topology_2(stree.root, open('out','w'))
 */