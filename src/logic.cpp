#include "logic.hpp"

#include <stdexcept>
#include <random>
#include <algorithm>
#include <queue>
#include <cassert>
#include <numeric>
#include <array>

#include <iostream>
#include <iomanip>


Minefield::tile_t Minefield::default_tile = Minefield::tile_t{};

const std::array<Minefield::tile_offset_t, 8ul> Minefield::offsets = {{
  {-1, -1}, {-1,  0}, {-1,  1},
  { 0, -1},           { 0,  1},
  { 1, -1}, { 1,  0}, { 1,  1},
}};


Minefield::Minefield(size_t rows, size_t cols):
  rows(rows), cols(cols)
{
  MW_SET_CLASS_ORIGIN;
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "new with rows=" << rows << " cols=" << cols;

  this->field_size = rows * cols;
  this->field = std::vector<Minefield::tile_t>(this->field_size);

  this->nr_of_mines = this->calculateNrOfMines();

  this->activateField = sigc::mem_fun4(*this, &Minefield::activateFieldInitial);
  this->field_inizialized = false;
}

void Minefield::resize(size_t rows, size_t cols)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "resize with rows=" << rows << " cols=" << cols;

  this->rows = rows;
  this->cols = cols;

  this->field_size = rows * cols;
  this->field.resize(this->field_size);

  this->nr_of_mines = this->calculateNrOfMines();

  this->activateField = sigc::mem_fun4(*this, &Minefield::activateFieldInitial);
  this->field_inizialized = false;
}

void Minefield::reset()
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "reset";

  this->activateField = sigc::mem_fun4(*this, &Minefield::activateFieldInitial);
  this->field_inizialized = false;
}

bool Minefield::undoFieldActivation(size_t row, size_t col)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "undoing field activation at row=" << row << " col=" << col;

  bool tile_is_valid;
  tile_t &tile = this->getTile(tile_position_t{row, col}, tile_is_valid);
  if (!tile_is_valid)
  {
    MW_LOG_INVALID_TILE;

    return false;
  }

  tile.is_revealed = false;

  return true;
}

bool Minefield::toggleFieldFlag(size_t row, size_t col, bool &o_is_flagged)
{
  MW_SET_FUNC_SCOPE;

  if (!this->field_inizialized)
  {
    MW_LOG(warning) << "field not initialized yet";

    return false;
  }

  bool tile_is_valid;
  tile_t &tile = this->getTile(tile_position_t{row, col}, tile_is_valid);
  if (!tile_is_valid)
  {
    MW_LOG_INVALID_TILE;

    return false;
  }

  if (tile.is_revealed)
  {
    return false;
  }

  tile.is_flagged = !tile.is_flagged;
  o_is_flagged = (tile.is_flagged);

  return true;
}

bool Minefield::checkGameWon()
{
  MW_SET_FUNC_SCOPE;

  // for (const tile_t &current_tile: this->field)
  // {
  //   if (current_tile.is_revealed) continue;

  //   if (current_tile.is_mine && !current_tile.is_flagged) return false;
  // }

  // return true;

  size_t  revealed_tiles_count          = 0ul,
          correctly_flagged_mines_count = 0ul;
  const size_t non_mine_tiles_count = this->rows * this->cols - this->nr_of_mines;

  for (const tile_t &current_tile: this->field)
  {
    if (current_tile.is_revealed)
    {
      revealed_tiles_count++;
    }
    else if (current_tile.is_flagged && current_tile.is_mine)
    {
      correctly_flagged_mines_count++;
    }
  }

  return (revealed_tiles_count == non_mine_tiles_count) || (correctly_flagged_mines_count == this->nr_of_mines);
}

bool Minefield::checkHasAvailableMoves()
{
  MW_SET_FUNC_SCOPE

  for (size_t row = 0ul; row < this->rows; row++)
  {
    for (size_t col = 0ul; col < this->cols; col++)
    {
      tile_t current_tile = this->field[row * this->cols + col];

      if (!current_tile.is_revealed || (current_tile.is_revealed && current_tile.nr_surrounding_mines == 0)) continue;

      uint8_t surrounding_flags   = 0u,
              surrounding_covered = 0u;
      {
        size_t  above = row - 1ul,
                below = row + 1ul,
                left  = col - 1ul,
                right = col + 1ul;
        std::array<tile_position_t, 8ul> surrounding_tiles {
          tile_position_t{above,  left  },
          tile_position_t{above,  col   },
          tile_position_t{above,  right },
          tile_position_t{row,    left  },
          tile_position_t{row,    right },
          tile_position_t{below,  left  },
          tile_position_t{below,  col   },
          tile_position_t{below,  right },
        };

        for (const tile_position_t &current_tile_position: surrounding_tiles)
        {
          bool tile_is_valid;
          const tile_t &surrounding_tile = this->getTile(current_tile_position, tile_is_valid);
          if (!tile_is_valid) continue;

          if (surrounding_tile.is_flagged)
          {
            surrounding_flags++;
          }
          else if (!surrounding_tile.is_revealed)
          {
            surrounding_covered++;
          }
        }
      }

      // TODO: continue here, this logic needs to be redone
      const bool flagged_all_mines = (surrounding_flags == current_tile.nr_surrounding_mines);
      if ((flagged_all_mines  && surrounding_covered > 0) ||
          (!flagged_all_mines && surrounding_covered == (current_tile.nr_surrounding_mines - surrounding_flags)))
      {
        MW_LOG(debug) << "row=" << row << "col=" << col << " has available moves";
        return true;
      }
    }
  }

  return false;
}

const size_t &Minefield::getNrMines()
{
  MW_SET_FUNC_SCOPE;

  return this->nr_of_mines;
}

bool Minefield::activateFieldInitial(size_t row, size_t col, std::vector<tile_with_position_t> &o_revealed_fields, bool &o_has_revealed_mine)
{
  MW_SET_FUNC_SCOPE

  MW_LOG(trace) << "activating at row=" << row << " col=" << col;

  // don't even bother if the position is invalid
  if (!(row < this->rows && col < this->cols))
  {
    MW_LOG_INVALID_TILE;

    return false;
  }

  // reinitialize until enough fields are revealed without hitting a bomb
  do
  {
    this->initFields();

    {
      // initialize queue for field cascade
      o_revealed_fields.clear();
      std::queue<tile_position_t> field_queue;
      field_queue.push(tile_position_t{row, col});

      // by default we don't expect to hit a mine, if we hit one this will be overwritten
      o_has_revealed_mine = false;
      do
      {
        // get queue element
        tile_position_t current_tile_pos = std::move(field_queue.front());
        field_queue.pop();

        // check tile validity
        bool tile_is_valid;
        tile_t &current_tile = this->getTile(current_tile_pos, tile_is_valid);
        if (!tile_is_valid)
        {
          // field does not exist

          continue;
        }

        // check if we need to evaluate further
        if (current_tile.is_revealed)
        {
          // already "revealed" thus not interesting

          continue;
        }

        // should be revealed now
        current_tile.is_revealed = true;

        // check if we hit a mine
        if (current_tile.is_mine)
        {
          // signal that a mine is revealed and stop since further evaluation is not needed

          o_has_revealed_mine = true;
          o_revealed_fields.push_back(tile_with_position_t{current_tile_pos.row, current_tile_pos.col, -1});

          break;
        }
        else
        {
          o_revealed_fields.push_back(tile_with_position_t{current_tile_pos.row, current_tile_pos.col, current_tile.nr_surrounding_mines});
        }

        // if we hit an "empty" (no adjacent mines) reveal, add all adjacent fields to the queue
        if (current_tile.nr_surrounding_mines == 0)
        {
          size_t  above = current_tile_pos.row - 1ul,
                  below = current_tile_pos.row + 1ul,
                  left  = current_tile_pos.col - 1ul,
                  right = current_tile_pos.col + 1ul;

          field_queue.push(tile_position_t{above,                 left});
          field_queue.push(tile_position_t{above,                 current_tile_pos.col});
          field_queue.push(tile_position_t{above,                 right});
          field_queue.push(tile_position_t{current_tile_pos.row,  left});
          field_queue.push(tile_position_t{current_tile_pos.row,  right});
          field_queue.push(tile_position_t{below,                 left});
          field_queue.push(tile_position_t{below,                 current_tile_pos.col});
          field_queue.push(tile_position_t{below,                 right});
        }
      } while (!field_queue.empty());
    }

    MW_LOG(debug) << "revealed mine: " << std::boolalpha << o_has_revealed_mine << " nr revealed fields: " << o_revealed_fields.size();
  } while (o_has_revealed_mine || (o_revealed_fields.size() < MIN_INITIAL_FIELDS));

  // from this point forth use the "normal" activation function
  this->activateField = sigc::mem_fun4(*this, &Minefield::activateFieldMain);
  this->field_inizialized = true;

# ifdef MW_DEBUG
  bool is_valid;
  for (size_t row = 0ul; row < this->rows; row++)
  {
    for (size_t col = 0ul; col < this->cols; col++)
    {
      Minefield::tile_t tile = this->getTile(Minefield::tile_position_t{row, col}, is_valid);

      std::cout << std::setw(2) << (tile.is_mine ? -1 : tile.nr_surrounding_mines) << ' ';
    }
    std::cout << '\n';
  }
  std::cout.flush();
# endif // defined(MW_DEBUG)

  return true;
}

bool Minefield::activateFieldMain(size_t row, size_t col, std::vector<tile_with_position_t> &o_revealed_fields, bool &o_has_revealed_mine)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "activating at row=" << row << " col=" << col;

  tile_position_t current_tile_pos{row, col};

  // get source tile
  bool tile_is_valid;
  const tile_t &tile = this->getTile(current_tile_pos, tile_is_valid);

  // check source tile validity
  if (!tile_is_valid)
  {
    MW_LOG_INVALID_TILE;

    return false;
  }

  o_has_revealed_mine = false;

  // initialize queue for field cascade
  o_revealed_fields.clear();
  std::queue<tile_position_t> field_queue;
  if (tile.is_revealed && this->mineCountSatisfied(current_tile_pos))
  {
    // tile is revealed and is surrounded by as many flagged fields as there are mines around it

    size_t  above = current_tile_pos.row - 1ul,
            below = current_tile_pos.row + 1ul,
            left  = current_tile_pos.col - 1ul,
            right = current_tile_pos.col + 1ul;

    field_queue.push(tile_position_t{above,                 left});
    field_queue.push(tile_position_t{above,                 current_tile_pos.col});
    field_queue.push(tile_position_t{above,                 right});
    field_queue.push(tile_position_t{current_tile_pos.row,  left});
    field_queue.push(tile_position_t{current_tile_pos.row,  right});
    field_queue.push(tile_position_t{below,                 left});
    field_queue.push(tile_position_t{below,                 current_tile_pos.col});
    field_queue.push(tile_position_t{below,                 right});
  }
  else if (!tile.is_revealed && !tile.is_flagged)
  {
    // tile is not yet revealed

    field_queue.push(current_tile_pos);
  }
  else
  {
    // not enough adjacent flagged fields or field itself is flagged

    MW_LOG(warning) << "can not activate field";
    MW_LOG(debug) << "is revealed: " << tile.is_revealed << "; is flagged: " << tile.is_flagged << "; bomb count satisfied: " <<  this->mineCountSatisfied(current_tile_pos);

    return true;
  }

  // by default we don't expect to hit a mine, if we hit one this will be overwritten
  do
  {
    // get queue element
    tile_position_t current_tile_pos = std::move(field_queue.front());
    field_queue.pop();

    // check tile validity
    tile_t &current_tile = this->getTile(current_tile_pos, tile_is_valid);
    if (!tile_is_valid)
    {
      // field does not exist

      continue;
    }

    // check if we need to evaluate further
    if (current_tile.is_flagged || current_tile.is_revealed)
    {
      // already "revealed" thus not interesting

      continue;
    }

    // should be revealed now
    current_tile.is_revealed = true;

    // check if we hit a mine
    if (current_tile.is_mine)
    {
      // signal that a mine is revealed and stop since further evaluation is not needed

      o_has_revealed_mine = true;
      o_revealed_fields.push_back(tile_with_position_t{current_tile_pos.row, current_tile_pos.col, -1});

      break;
    }
    else
    {
      o_revealed_fields.push_back(tile_with_position_t{current_tile_pos.row, current_tile_pos.col, current_tile.nr_surrounding_mines});
    }

    // if we hit an "empty" (no adjacent mines) reveal, add all adjacent fields to the queue
    if (current_tile.nr_surrounding_mines == 0)
    {
      size_t  above = current_tile_pos.row - 1ul,
              below = current_tile_pos.row + 1ul,
              left  = current_tile_pos.col - 1ul,
              right = current_tile_pos.col + 1ul;

      field_queue.push(tile_position_t{above,                 left});
      field_queue.push(tile_position_t{above,                 current_tile_pos.col});
      field_queue.push(tile_position_t{above,                 right});
      field_queue.push(tile_position_t{current_tile_pos.row,  left});
      field_queue.push(tile_position_t{current_tile_pos.row,  right});
      field_queue.push(tile_position_t{below,                 left});
      field_queue.push(tile_position_t{below,                 current_tile_pos.col});
      field_queue.push(tile_position_t{below,                 right});
    }
  } while (!field_queue.empty());

  return true;
}

void Minefield::initFields()
{
  MW_SET_FUNC_SCOPE;

  // reset field
  std::fill(this->field.begin(), this->field.end(), tile_t());

  // make list of indices
  std::vector<int64_t> field_indices(this->field_size);
  std::iota(field_indices.begin(), field_indices.end(), 0ul);

  // prepare for list of mine indices
  std::vector<int64_t> mine_indies;
  std::random_device random_device;
  std::mt19937 mersenne_twister(random_device());

  // sample mine indices
  std::sample(
    field_indices.begin(), field_indices.end(),
    std::back_inserter(mine_indies),
    this->nr_of_mines,
    mersenne_twister
  );

  // apply mine indices
  for (const int64_t &idx : mine_indies)
  {
    this->field.at(idx).is_mine = true;

    int64_t row = idx / this->cols,
            col = idx % this->cols;

    for (const tile_offset_t offset: this->offsets)
    {
      int64_t current_row = row + offset.rows,
              current_col = col + offset.cols;

      if (current_row < 0 || current_row >= this->rows ||
          current_col < 0 || current_col >= this->cols) continue;

      this->field.at(current_row * this->cols + current_col).nr_surrounding_mines++;
    }
  }
}

size_t Minefield::calculateNrOfMines()
{
  MW_SET_FUNC_SCOPE;

  return DEFAULT_BOMB_FACTOR * this->field_size;
}

Minefield::tile_t &Minefield::getTile(const tile_position_t &pos, bool &o_is_valid)
{
  MW_SET_FUNC_SCOPE;

  o_is_valid = (pos.row < this->rows && pos.col < this->cols);

  if (!o_is_valid)
  {
    return this->default_tile;
  }
  else
  {
    return this->field[pos.row * this->cols + pos.col];
  }
}

bool Minefield::mineCountSatisfied(const tile_position_t &tile_pos)
{
  MW_SET_FUNC_SCOPE;

  bool tile_is_valid;
  const tile_t &tile = this->getTile(tile_pos, tile_is_valid);
  if (!tile_is_valid)
  {
    MW_LOG_INVALID_TILE;

    return false;
  }

  uint8_t mine_count = 0u;

  //! TODO: maybe rewrite without loop - the array size won't change anyways
  size_t above = tile_pos.row - 1ul,
         below = tile_pos.row + 1ul,
         left  = tile_pos.col - 1ul,
         right = tile_pos.col + 1ul;
  std::array<tile_position_t, 8ul> surrounding_tiles {
    tile_position_t{above,        left},
    tile_position_t{above,        tile_pos.col},
    tile_position_t{above,        right},
    tile_position_t{tile_pos.row, left},
    tile_position_t{tile_pos.row, right},
    tile_position_t{below,        left},
    tile_position_t{below,        tile_pos.col},
    tile_position_t{below,        right},
  };

  for (const tile_position_t &current_tile_position: surrounding_tiles)
  {
    const tile_t &current_tile = this->getTile(current_tile_position, tile_is_valid);
    if (!tile_is_valid) continue;

    if (/*current_tile.is_mine &&*/ current_tile.is_flagged)
    {
      if (++mine_count == tile.nr_surrounding_mines)
      {
        return true;
      }
    }
  }

  return false;
}