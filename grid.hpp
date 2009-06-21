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

#ifndef GRID_HPP_
#define GRID_HPP_

#include <vector>
#include <iosfwd>

/*!
* \brief
* maintains a list of choices (possible values) for a given cell.
*/
class Choices {
    bool choices[10];
    int num_choices;
public:
    /*!
    * \brief
    * constructor
    */
    Choices();

    /*!
    * \brief
    * copy constructor
    */
    Choices(const Choices &others);

    /*!
    * \brief
    * assignment operator
    * \return this
    */
    Choices & operator =(const Choices &others);

    /*!
    * \brief returns the number of choices left
    * returns the number of choices left
    */
    int get_num_choices() const;

    /*!
    * \brief
    * returns true, if the value given is a valid choice
    * \param value the value to be tested
    * \return true, if the value given is a valid choice
    */
    bool has_choice(int value) const;

    /*!
    * \brief adds a choice to the list of valid choices
    * \param value the value to be added
    */
    void add_choice(int value);


    void remove_choice(int value);
    void clear();
    int first_choice() const;
};

class Cell {
    int idx;
    int value;
    Choices choices;
public:
    Cell(int idx);
    Cell(const Cell &other);
    Cell & operator =(const Cell &other);

    int get_value() const;
    bool has_value() const;
    void set_value(int value);

    int get_row() const;
    int get_col() const;
    int get_block_idx() const;
    int get_idx() const;

    bool has_choice(int value) const;
    void add_choice(int value);
    void remove_choice(int value);
    void clear_choices();

    int get_num_choices() const;
    void print_choices(std::ostream &out) const;
    int first_choice() const;

    const Choices &get_choices() const;
};

class Grid {
public:
    typedef std::vector<Cell>::iterator iterator;
    typedef std::vector<Cell>::const_iterator const_iterator;
private:
    std::vector<Cell> cells;
public:
    Grid();
    Grid(const Grid &other);
    Grid & operator =(const Grid &other);
    void init_cells();
    void load(std::istream &in);
    void print(std::ostream &out) const;
    void print_choices(std::ostream &out) const;
    void print_status(std::ostream &out) const;
    int get_to_do() const;
    int get_num_choices() const;
    void cleanup_choices();
    void cleanup_choice(Cell &cell);
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    Cell & operator[](int idx);
    const Cell & operator[](int idx) const;
};

inline Choices::Choices() :
num_choices(9) {
    std::fill(choices, choices + 10, true);
}

inline Choices::Choices(const Choices &other) :
num_choices(other.num_choices) {
    std::copy(other.choices, other.choices + 10, choices);
}

inline Choices &Choices::operator =(const Choices &other) {
    if (this == &other)
        return *this;

    std::copy(other.choices, other.choices + 10, choices);
    num_choices = other.num_choices;

    return *this;
}

inline int Choices::get_num_choices() const {
    return num_choices;
}

inline bool Choices::has_choice(int value) const {
    return choices[value];
}

inline void Choices::add_choice(int value) {
    if (choices[value] == false) {
        choices[value] = true;
        ++num_choices;
    }
}

inline void Choices::remove_choice(int value) {
    if (choices[value] == true) {
        choices[value] = false;
        --num_choices;
    }
}

inline void Choices::clear() {
    std::fill(choices, choices + 10, false);
    num_choices = 0;
}

inline int Choices::first_choice() const {
    if (num_choices == 0)
        return 0;

    for (int i = 1; i < 10; ++i) {
        if (choices[i])
            return i;
    }

    return 0;
}

inline Cell::Cell(int idx) :
idx(idx), value(0), choices() {
}

inline Cell::Cell(const Cell &other) :
idx(other.idx), value(other.value), choices(other.choices) {
}

inline Cell &Cell::operator =(const Cell &other) {
    if (this == &other)
        return *this;

    idx = other.idx;
    value = other.value;
    choices = other.choices;

    return *this;
}

inline int Cell::get_value() const {
    return value;
}

inline bool Cell::has_value() const {
    return value > 0;
}

inline void Cell::set_value(int value) {
    this->value = value;
}

inline int Cell::get_row() const {
    return idx / 9;
}

inline int Cell::get_col() const {
    return idx % 9;
}

inline int Cell::get_block_idx() const {
    return get_row() / 3 * 3 + get_col() / 3;
}

inline int Cell::get_idx() const {
    return idx;
}

inline bool Cell::has_choice(int value) const {
    return choices.has_choice(value);
}

inline void Cell::add_choice(int value) {
    choices.add_choice(value);
}

inline void Cell::remove_choice(int value) {
    choices.remove_choice(value);
}

inline void Cell::clear_choices() {
    choices.clear();
}

inline int Cell::get_num_choices() const {
    return choices.get_num_choices();
}

inline int Cell::first_choice() const {
    return choices.first_choice();
}

inline const Choices &Cell::get_choices() const {
    return choices;
}

inline Grid::Grid() {
    init_cells();
}

inline Grid::Grid(const Grid &other) {
    cells = other.cells;
}

inline Grid &Grid::operator =(const Grid &other) {
    if (this == &other)
        return *this;

    cells = other.cells;

    return *this;
}

inline void Grid::init_cells() {
    cells.clear();
    for (int i = 0; i < 81; ++i) {
        cells.push_back(Cell(i));
    }
}

inline int Grid::get_to_do() const {
    int todo = 0;

    for (int i = 0; i < 81; ++i)
        if (!cells[i].has_value())
            ++todo;

    return todo;
}

inline int Grid::get_num_choices() const {
    int result = 0;
    for (int i = 0; i < 81; ++i)
        result += cells[i].get_num_choices();
    return result;
}

inline Grid::iterator Grid::begin() {
    return cells.begin();
}

inline Grid::iterator Grid::end() {
    return cells.end();
}

inline Grid::const_iterator Grid::begin() const {
    return cells.begin();
}

inline Grid::const_iterator Grid::end() const {
    return cells.end();
}

inline Cell &Grid::operator[](int idx) {
    return cells[idx];
}

inline const Cell & Grid::operator[](int idx) const {
    return cells[idx];
}

inline void Grid::cleanup_choices() {
    for (iterator i = begin(); i != end(); ++i) {
        cleanup_choice(*i);
    }
}

#endif
