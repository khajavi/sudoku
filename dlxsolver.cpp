#include <vector>
#include <iostream>
#include <sstream>
#include <cassert>
#include <fstream>
#include "dlxsolver.hpp"
#include "grid.hpp"

DlxSudokuSolver::DlxSudokuSolver() {

}

DlxSudokuSolver::~DlxSudokuSolver() {

}

void DlxSudokuSolver::solution_found(const std::vector<Node *> &rows) {
    assert(rows.size() == 81);
    std::cout << std::endl;
    Grid grid;
    for (std::vector<Node *>::const_iterator i = rows.begin(); i != rows.end(); ++i) {
        int idx = (*i)->get_row();
        int value = idx % 9 + 1;
        idx /= 9;
        grid[idx].set_value(value);
        grid[idx].clear_choices();
    }
    grid.print(std::cout);
    std::cout << std::endl;
}

void DlxSudokuSolver::solve(Grid &grid) {
    NodeFactory node_factory;
    Solver solver(&node_factory, this);
    init_solver(node_factory, solver, grid);
    solver.solve();
}

void DlxSudokuSolver::init_solver(NodeFactory &node_factory, Solver &solver,
        Grid &grid) {
    /*
     * ncolumns: all cells + each row/col/block * each possible value (9)
     */
    const int ncolumns = 81 + 3 * 81;
    std::vector<Column *> columns;

    for (int i = 0; i < ncolumns; ++i) {
        columns.push_back(solver.add_column(i));
    }

    for (int idx = 0; idx < 81; ++idx) {
        const Cell &cell = grid[idx];
        if (cell.has_value()) {
            add_nodes(cell, cell.get_value(), columns, node_factory);
        } else {
            for (int value = 1; value < 10; ++value) {
                if (cell.has_choice(value)) {
                    add_nodes(cell, value, columns, node_factory);
                }
            }
        }
    }
}

void DlxSudokuSolver::add_nodes(const Cell &cell, int value, std::vector<
        Column *> &columns, NodeFactory &node_factory) {
    int row = cell.get_row();
    int col = cell.get_col();
    int row_idx = 81 * row + 9 * col + value - 1;
    assert(row_idx >= 0 && row_idx < 729);

    int col_idx = cell.get_idx();
    assert(col_idx >= 0 && col_idx < 381);
    Node *pfirst = node_factory.create_child(columns[col_idx], row_idx);

    col_idx = 81 + row * 9 + value - 1;
    assert(col_idx >= 0 && col_idx < 381);
    Node *node = node_factory.create_child(columns[col_idx], row_idx);
    pfirst->add_sibling(node);

    col_idx = 2 * 81 + col * 9 + value - 1;
    assert(col_idx >= 0 && col_idx < 381);
    node = node_factory.create_child(columns[col_idx], row_idx);
    pfirst->add_sibling(node);

    /**
     * block_idx = 0..8
     */
    int block_idx = (row / 3 * 3) + (col / 3);
    assert(block_idx >= 0 && block_idx < 9);
    col_idx = 3 * 81 + block_idx * 9 + value - 1;
    assert(col_idx >= 0 && col_idx < 381);
    node = node_factory.create_child(columns[col_idx], row_idx);
    pfirst->add_sibling(node);
}

void solve(const std::string &s) {
    std::istringstream in(s);
    Grid grid;
    grid.load(in);
    grid.print(std::cout);
    std::cout << std::endl;
    DlxSudokuSolver solver;
    solver.solve(grid);
}

int main(int argc, char *argv[]) {
    std::string filename("top1465.txt");

    if (argc > 1) {
        filename = argv[1];
    }

    std::ifstream in(filename.c_str());
    if (!in) {
        std::cerr << "cannot open file " << filename << std::endl;
        return 1;
    }
    std::string line;
    while (getline(in, line)) {
        solve(line);
    }
    return 0;
}