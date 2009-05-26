#include <iostream>
#include <vector>
#include <set>
#include <map>

#include "grid.hpp"
#include "boxlinereduction.hpp"
#include "hintconsumer.hpp"

BoxLineReductionHint::BoxLineReductionHint(Grid &grid, int value,
        int first_block, int second_block, int third_block,
        int rowcol_to_remove1, int rowcol_to_remove2, int rowcol_to_keep,
        bool isrow) :
    grid(grid), value(value), first_block(first_block), second_block(
            second_block), third_block(third_block), rowcol_to_remove1(
            rowcol_to_remove1), rowcol_to_remove2(rowcol_to_remove2),
            rowcol_to_keep(rowcol_to_keep) {

}

void BoxLineReductionHint::apply() {

}

void BoxLineReductionHint::print_description(std::ostream &out) const {
    out << "boxline: ";
    if (isrow) {
        int row = rowcol_to_remove1 / 3 * 3;
        out << "block1(" << row << "," << first_block * 3 << ") ";
        out << "block2(" << row << "," << second_block * 3 << ") ";
        out << "block3(" << row << "," << third_block * 3 << ") ";
        out << "row to remove 1: " << rowcol_to_remove1;
        out << "row to remove 2: " << rowcol_to_remove2;
        out << "row to keep: " << rowcol_to_keep;
    } else {
        int col = rowcol_to_remove1 / 3 * 3;
        out << "block1(" << first_block * 3 << "," << col << ") ";
        out << "block2(" << second_block * 3 << "," << col << ") ";
        out << "block3(" << third_block * 3 << "," << col << ") ";
        out << "col to remove 1: " << rowcol_to_remove1;
        out << "col to remove 2: " << rowcol_to_remove2;
        out << "col to keep: " << rowcol_to_keep;
    }
}

void BoxLineReductionHintProducer::find_hints(Grid &grid,
        HintConsumer &consumer) {
    find_row_lines(grid, consumer);
    if (!consumer.wants_more_hints())
        return;
    find_col_lines(grid, consumer);
}

void BoxLineReductionHintProducer::find_row_lines(Grid &grid,
        HintConsumer &consumer) const {
    for (int start_row = 0; start_row < 3; ++start_row) {
        find_row_line(grid, start_row, consumer);
        if (!consumer.wants_more_hints())
            return;
    }
}

void BoxLineReductionHintProducer::fill_intersection(const std::set<int> first,
        const std::set<int> &second, std::vector<int> &intersection,
        std::vector<int> &difference, std::vector<int> &rowunion) const {
    std::set_intersection(first.begin(), first.end(), second.begin(),
            second.end(), std::back_inserter(intersection));
    std::set_difference(first.begin(), first.end(), second.begin(),
            second.end(), std::back_inserter(difference));
    std::set_union(first.begin(), first.end(), second.begin(), second.end(),
            std::back_inserter(rowunion));
}

void BoxLineReductionHintProducer::process_sets(Grid &grid, int row, int value,
        std::vector<std::set<int> > rowsets, bool isrow, HintConsumer &consumer) const {
    for (int i = 0; i < 3; ++i) {
        for (int j = i + 1; j < 3; ++j) {
            std::vector<int> intersection;
            std::vector<int> difference;
            std::vector<int> rowunion;
            fill_intersection(rowsets[i], rowsets[j], intersection, difference,
                    rowunion);
            if (intersection.size() == 2) {
                for (int k = 0; k < 3; ++k) {
                    if (k != i && k != j) {
                        if (rowsets[k].find(difference.front())
                                != rowsets[k].end()) {
                            std::vector<int> kintersection;
                            std::set_intersection(intersection.begin(),
                                    intersection.end(), rowsets[k].begin(),
                                    rowsets[k].end(), std::back_inserter(
                                            kintersection));
                            if (kintersection.size() > 0) {
                                if (!consumer.consume_hint(
                                        new BoxLineReductionHint(grid, value,
                                                i, j, k, intersection[0],
                                                intersection[1], difference[0],
                                                isrow)))
                                    return;
                            }
                        }
                    }
                }
            }
        }
    }
}

void BoxLineReductionHintProducer::find_row_line(Grid &grid, int row_idx,
        HintConsumer &consumer) const {
    int start_row = 3 * row_idx;

    for (int value = 1; value < 10; ++value) {
        for (int row = start_row; row < start_row + 3; ++row) {
            std::vector<std::set<int> > rowsets(3);
            for (int col_idx = 0; col_idx < 3; ++col_idx) {
                find_row_col_block(grid, row, col_idx, value, rowsets[col_idx]);
            }
            process_sets(grid, row, value, rowsets, true, consumer);
            if (!consumer.wants_more_hints())
                return;
        }
    }
}

void BoxLineReductionHintProducer::find_row_col_block(Grid &grid, int row,
        int col_idx, int value, std::set<int> &rowset) const {
    int start_col = 3 * col_idx;

    for (int col = start_col; col < start_col + 3; ++col) {
        int idx = row * 9 + col;
        Cell &cell = grid[idx];
        if (!cell.has_value()) {
            if (cell.has_choice(value)) {
                rowset.insert(row);
            }
        }
    }
}

void BoxLineReductionHintProducer::find_col_lines(Grid &grid,
        HintConsumer &consumer) const {
    for (int col_idx = 0; col_idx < 3; ++col_idx) {
        find_col_line(grid, col_idx, consumer);
        if (!consumer.wants_more_hints())
            return;
    }
}

void BoxLineReductionHintProducer::find_col_line(Grid &grid, int col_idx,
        HintConsumer &consumer) const {
    int start_col = col_idx * 3;

    for (int value = 1; value < 10; ++value) {
        for (int col = start_col; col < start_col + 3; ++col) {
            std::vector<std::set<int> > colsets(3);
            for (int row_idx = 0; row_idx < 3; ++row_idx) {
                find_col_row_block(grid, col, row_idx, value, colsets[row_idx]);
            }
            process_sets(grid, col, value, colsets, false, consumer);
            if (!consumer.wants_more_hints())
                return;
        }
    }
}

void BoxLineReductionHintProducer::find_col_row_block(Grid &grid, int col,
        int row_idx, int value, std::set<int> &colset) const {
    int start_row = row_idx * 3;
    for (int row = start_row; row < start_row + 3; ++row) {
        int idx = row * 9 + col;
        Cell &cell = grid[idx];
        if (!cell.has_value() && cell.has_choice(value))
            colset.insert(col);
    }
}
