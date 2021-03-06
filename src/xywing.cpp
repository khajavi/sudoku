/*
 * Copyright (c) 2009, Ralph Juhnke
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following conditions
 * are met:
 *
 *    * Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *    * Neither the name of "Ralph Juhnke" nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <vector>
#include <bitset>

#include "xywing.hpp"
#include "grid.hpp"
#include "range.hpp"
#include "hintconsumer.hpp"
#include "util.hpp"

XYWingHint::XYWingHint(const Cell &xy, const Cell &yz, const Cell &xz, int x,
        int y, int z) :
    xy(xy), yz(yz), xz(xz), x(x), y(y), z(z) {
}

void XYWingHint::print_description(std::ostream &out) const {
    out << "xy-wing: xy: " << print_row_col(xy.get_idx()) << ", yz: "
            << print_row_col(yz.get_idx()) << ", xz: " << print_row_col(
            xz.get_idx()) << " x=" << x << ", y=" << y << ", z=" << z
            << ". removing: " << print_choices_to_remove(
            get_choices_to_remove());
}

const char *XYWingHint::get_name() const {
    return "XY-wing";
}

void XYWingHintProducer::find_hints(Grid &grid, HintConsumer &consumer) {
    for (Grid::iterator i = grid.begin(); i != grid.end(); ++i) {
        find_xy_wing(*i, grid, consumer);
        if (!consumer.wants_more_hints())
            return;
    }
}

void XYWingHintProducer::fill_cell_values(const Cell &cell, int values[2]) const {
    int idx = 0;
    for (int value = 1; value < 10; ++value) {
        if (cell.has_choice(value)) {
            values[idx] = value;
            ++idx;
        }
    }
}

void XYWingHintProducer::find_xy_wing(Cell &cell, Grid &grid,
        HintConsumer &consumer) const {
    if (cell.get_num_choices() != 2)
        return;

    RangeList::const_index_iterator begin = RANGES.field_begin(cell.get_idx());
    RangeList::const_index_iterator end = RANGES.field_end(cell.get_idx());
    std::vector<Cell *> cells;

    for (RangeList::const_index_iterator i = begin; i != end; ++i) {
        Cell & c = grid[*i];
        if (c.get_num_choices() == 2)
            cells.push_back(&c);
    }

    if (cells.size() < 2)
        return;

    int cell_values[2];
    fill_cell_values(cell, cell_values);

    for (std::vector<Cell *>::iterator i = cells.begin(); i != cells.end(); ++i) {
        Cell &yz = *(*i);
        if (yz.has_choice(cell_values[0]) && !yz.has_choice(cell_values[1])) {
            int yz_values[2];
            fill_cell_values(yz, yz_values);
            int x = cell_values[1];
            int y = cell_values[0];
            int z = yz_values[0] == y ? yz_values[1] : yz_values[0];
            for (std::vector<Cell *>::iterator j = i + 1; j != cells.end(); ++j) {
                Cell &xz = *(*j);
                if (xz.has_choice(x) && xz.has_choice(z)) {
                    XYWingHint *hint = create_xy_hint(cell, yz, xz, x, y, z,
                            grid);
                    if (hint != 0) {
                        if (!consumer.consume_hint(hint))
                            return;
                    }
                }
            }
        } else if (!yz.has_choice(cell_values[0]) && yz.has_choice(
                cell_values[1])) {
            int yz_values[2];
            fill_cell_values(yz, yz_values);
            int x = cell_values[0];
            int y = cell_values[1];
            int z = yz_values[0] == y ? yz_values[1] : yz_values[0];
            for (std::vector<Cell *>::iterator j = i + 1; j != cells.end(); ++j) {
                Cell &xz = *(*j);
                if (xz.has_choice(x) && xz.has_choice(z)) {
                    XYWingHint *hint = create_xy_hint(cell, yz, xz, x, y, z,
                            grid);
                    if (hint != 0) {
                        if (!consumer.consume_hint(hint))
                            return;
                    }
                }
            }
        }
    }
}

XYWingHint *XYWingHintProducer::create_xy_hint(Cell &xy, Cell &yz, Cell &xz,
        int x, int y, int z, Grid &grid) const {
    std::vector<int> cells_to_clear;

    RangeList::const_index_iterator begin = RANGES.field_begin(yz.get_idx());
    RangeList::const_index_iterator end = RANGES.field_end(yz.get_idx());

    for (RangeList::const_index_iterator i = begin; i != end; ++i) {
        Cell &cell = grid[*i];
        if (cell.has_choice(z) && cell.get_idx() != yz.get_idx()
                && xz.get_idx() != cell.get_idx() && shares_region(xz, cell)) {
            cells_to_clear.push_back(cell.get_idx());
        }
    }

    if (cells_to_clear.empty())
        return 0;

    return create_hint(xy, yz, xz, x, y, z, grid, cells_to_clear);
}

bool XYWingHintProducer::shares_region(const Cell &a, const Cell &b) const {
    return a.get_row() == b.get_row() || a.get_col() == b.get_col()
            || a.get_block_idx() == b.get_block_idx();
}

XYWingHint *XYWingHintProducer::create_hint(const Cell &xy, const Cell &yz,
        const Cell &xz, int x, int y, int z, Grid &grid,
        const std::vector<int> &cells_to_clear) const {
    XYWingHint *hint = new XYWingHint(xy, yz, xz, x, y, z);

    for (std::vector<int>::const_iterator i = cells_to_clear.begin(); i
            != cells_to_clear.end(); ++i) {
        if (grid[*i].has_choice(z))
            hint->add_choice_to_remove(&grid[*i], z);
    }

    return hint;
}
