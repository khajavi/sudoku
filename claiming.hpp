#ifndef CLAIMING_HPP
#define CLAIMING_HPP

#include "indirecthint.hpp"

#include <vector>

class Cell;
class ClaimingRowHint : public IndirectHint {
    std::vector<Cell *> cells;
    int                 row;
    int                 block_idx;
    int                 value;
    Grid                &grid;
public:
    ClaimingRowHint(std::vector<Cell *> &cells,int row, int block_idx, int value, Grid &grid);
    void print_description(std::ostream &out) const;
};

class ClaimingColumnHint : public IndirectHint {
    std::vector<Cell *> cells;
    int                 col;
    int                 block_idx;
    int                 value;
    Grid                &grid;
public:
    ClaimingColumnHint(std::vector<Cell *> &cells,int row, int block_idx, int value, Grid &grid);
    void print_description(std::ostream &out) const;
};

class ClaimingHintProducer : public HintProducer {
public:
    void find_hints(Grid &grid, HintConsumer &consumer);
private:
    int find_block_by(int start_row, int start_col) const;
    void find_row_hint(int row, Grid &grid, HintConsumer &consumer) const;
    void find_col_hint(int col, Grid &grid, HintConsumer &consumer) const;
    bool check_block(const std::vector<Cell *> &cells, int block_idx, int value, Grid &grid) const;
    ClaimingRowHint *create_row_hint(std::vector<Cell *> &cells,int row, int block_idx, int value, Grid &grid) const;
    ClaimingColumnHint *create_column_hint(std::vector<Cell *> &cells,int row, int block_idx, int value, Grid &grid) const;
};

#endif
