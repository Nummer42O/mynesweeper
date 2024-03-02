#include "logic.hpp"

#include <stdexcept>
#include <random>
#include <algorithm>
#include <queue>
#include <cassert>
#include <numeric>
#include <array>
#include <set>

#include <iostream>
#include <iomanip>


Minefield::tile_t Minefield::default_tile = Minefield::tile_t{};

const std::array<Minefield::tile_offset_t, 8ul> Minefield::offsets = {{
  {-1l, -1l}, { 0l, -1l}, { 1l, -1l},
  {-1l,  0l},             { 1l,  0l},
  {-1l,  1l}, { 0l,  1l}, { 1l,  1l},
}};

const std::array<Minefield::tile_offset_t, 4ul> Minefield::directions = {{
              { 0l, -1l},
  {-1l,  0l},             { 1l,  0l},
              { 0l,  1l},
}};


inline bool operator==(const Minefield::tile_position_t &left, const Minefield::tile_position_t &right)
{
  return left.row == right.row && left.col == right.col;
}

inline bool operator<(const Minefield::tile_position_t &left, const Minefield::tile_position_t &right)
{
  if (left.row == right.row)
  {
    return left.col < right.col;
  }

  return left.row < right.row;
}


/* #region minefield generation */

Minefield::Minefield(index_t rows, index_t cols) : current_field_size{rows, cols}
{
  MW_SET_CLASS_ORIGIN;
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "new with rows=" << rows << " cols=" << cols;

  this->field_size = rows * cols;
  this->field = std::vector<Minefield::tile_t>(this->field_size);

  this->nr_of_mines = this->calculateNrOfMines();

  this->field_inizialized = false;
}

void Minefield::resize(index_t rows, index_t cols)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "resize with rows=" << rows << " cols=" << cols;

  this->current_field_size = {rows, cols};

  this->field_size = rows * cols;
  this->field.resize(this->field_size);

  this->nr_of_mines = this->calculateNrOfMines();

  this->field_inizialized = false;
}

void Minefield::reset()
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "reset";

  this->field_inizialized = false;
}

void Minefield::initFields(index_t row, index_t col)
{
  MW_SET_FUNC_SCOPE;

  // setup random device
  std::random_device random_device;
  std::mt19937 mersenne_twister(random_device());
  std::uniform_int_distribution<size_t> random_direction_idx(0ul, 3ul);

  // reset field
  std::fill(this->field.begin(), this->field.end(), tile_t());

  // generate initial patch
  std::set<tile_position_t> initial_patch = {{tile_position_t{row, col}}};
  while (initial_patch.size() < MIN_INITIAL_FIELDS)
  {
    std::set<tile_position_t> initial_patch_copy(initial_patch);
    for (const tile_position_t &tile: initial_patch_copy)
    {
      const tile_offset_t &offset = this->directions[random_direction_idx(mersenne_twister)];
      initial_patch.insert(tile_position_t{tile.row + offset.rows, tile.col + offset.cols});
    }
  }

# ifdef MW_DEBUG
  std::stringstream patch;
  patch << "initial patch: ";
  for (const tile_position_t& tile_pos: initial_patch)
  {
    patch << '(' << tile_pos.row << ", " << tile_pos.col << ") ";
  }
  MW_LOG(debug) << patch.str();
# endif //defined(MW_DEBUG)

  // preparation for sampling
  std::vector<tile_position_t> \
    possible_mine_positions(this->field_size - initial_patch.size()),
    mine_positions;
  index_t tile_position_index = 0l;
  std::generate(
    possible_mine_positions.begin(), possible_mine_positions.end(),
    [&]()
    {
      tile_position_t tile_position;
      do
      {
        assert(tile_position_index < this->field_size);

        tile_position = tile_position_t{
          /*rows = */ tile_position_index / this->current_field_size.cols,
          /*cols = */ tile_position_index % this->current_field_size.cols
        };
        tile_position_index++;
      }
      while (initial_patch.contains(tile_position));

      return tile_position;
    }
  );

  // sample mine indices
  mine_positions.reserve(this->nr_of_mines);
  std::sample(
    possible_mine_positions.begin(), possible_mine_positions.end(),
    std::back_inserter(mine_positions),
    this->nr_of_mines,
    mersenne_twister
  );

# ifdef MW_DEBUG
  std::stringstream mines;
  mines << "mines: ";
  for (const tile_position_t& tile_pos: mine_positions)
  {
    mines << '(' << tile_pos.row << ", " << tile_pos.col << ") ";
  }
  MW_LOG(debug) << mines.str();
# endif //defined(MW_DEBUG)

  // apply mine indices
  for (const tile_position_t &tile_pos : mine_positions)
  {
    this->getTile(tile_pos.row, tile_pos.col).is_mine = true;

    for (const tile_offset_t &offset : this->offsets)
    {
      index_t \
        current_row = tile_pos.row + offset.rows,
        current_col = tile_pos.col + offset.cols;

      if (!this->tilePositionValid(current_row, current_col)) continue;
      tile_t &current_tile = this->getTile(current_row, current_col);

      current_tile.nr_surrounding_mines++;
    }
  }
}

/* #endregion */
/* #region field manipulation */

bool Minefield::revealTile(index_t row, index_t col, cascade_t &o_revealed_fields, bool & o_has_revealed_mine)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "activating at row=" << row << " col=" << col;

  if (!this->tilePositionValid(row, col))
  {
    MW_LOG_INVALID_TILE;

    return false;
  }

  if (!this->field_inizialized)
  {
    this->initFields(row, col);
    this->field_inizialized = true;

#   ifdef MW_DEBUG
    for (index_t row = 0l; row < this->current_field_size.rows; row++)
    {
      for (index_t col = 0l; col < this->current_field_size.cols; col++)
      {
        Minefield::tile_t tile = this->getTile(row, col);

        std::cout << std::setw(2) << (tile.is_mine ? -1 : tile.nr_surrounding_mines) << ' ';
      }
      std::cout << '\n';
    }
    std::cout.flush();
#   endif // defined(MW_DEBUG)
  }

  this->revealTileInternal(row, col, o_revealed_fields, o_has_revealed_mine);

  return true;
}

bool Minefield::undoTileReveal(index_t row, index_t col)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "undoing field activation at row=" << row << " col=" << col;

  if (!this->tilePositionValid(row, col))
  {
    MW_LOG_INVALID_TILE;

    return false;
  }
  tile_t &tile = this->getTile(row, col);

  tile.is_revealed = false;

  return true;
}

bool Minefield::toggleTileFlag(index_t row, index_t col, bool &o_is_flagged)
{
  MW_SET_FUNC_SCOPE;

  if (!this->field_inizialized)
  {
    MW_LOG(warning) << "field not initialized yet";

    return false;
  }

  if (!this->tilePositionValid(row, col))
  {
    MW_LOG_INVALID_TILE;

    return false;
  }
  tile_t &tile = this->getTile(row, col);

  if (tile.is_revealed)
  {
    return false;
  }

  tile.is_flagged = !tile.is_flagged;
  o_is_flagged = (tile.is_flagged);

  return true;
}

void Minefield::revealTilesForUser(cascade_t &o_revealed_fields)
{
  /**
   * 1. generate list of all unrevealed tiles adjacent to revealed ones
   * 2. iterate over those and check if revealing those would solve the problem
   * 3. if none of those solve the problem, reveal any of them, remove it from list and repeat from 2.
   */
}

void Minefield::revealTileInternal(index_t row, index_t col, std::vector<tile_with_position_t> &o_revealed_fields, bool &o_has_revealed_mine)
{
  MW_SET_FUNC_SCOPE;

  const tile_t &tile = this->getTile(row, col);

  o_has_revealed_mine = false;

  // initialize queue for field cascade
  o_revealed_fields.clear();
  std::queue<tile_position_t> field_queue;
  if (tile.is_revealed && this->checkMineCountSatisfied(row, col))
  {
    // tile is revealed and is surrounded by as many flagged fields as there are mines around it

    index_t \
      above = row - 1l,
      below = row + 1l,
      left  = col - 1l,
      right = col + 1l;

    field_queue.push(tile_position_t{above, left});
    field_queue.push(tile_position_t{above, col});
    field_queue.push(tile_position_t{above, right});
    field_queue.push(tile_position_t{row,   left});
    field_queue.push(tile_position_t{row,   right});
    field_queue.push(tile_position_t{below, left});
    field_queue.push(tile_position_t{below, col});
    field_queue.push(tile_position_t{below, right});
  }
  else if (!tile.is_revealed && !tile.is_flagged)
  {
    // tile is not yet revealed

    field_queue.push(tile_position_t{row, col});
  }
  else
  {
    // not enough adjacent flagged fields or field itself is flagged

    MW_LOG(warning) << "can not activate field";
    MW_LOG(debug) << "is revealed: "  << tile.is_revealed
                  << " is flagged: " << tile.is_flagged
                  << " bomb count satisfied: " << this->checkMineCountSatisfied(row, col);

    return;
  }

  // by default we don't expect to hit a mine, if we hit one this will be overwritten
  do
  {
    // get queue element
    tile_position_t current_tile_pos = std::move(field_queue.front());
    field_queue.pop();

    if (!this->tilePositionValid(current_tile_pos.row, current_tile_pos.col))
    {
      // field does not exist
      continue;
    }
    tile_t &current_tile = this->getTile(current_tile_pos.row, current_tile_pos.col);

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
      index_t above = current_tile_pos.row - 1l,
             below = current_tile_pos.row + 1l,
             left = current_tile_pos.col - 1l,
             right = current_tile_pos.col + 1l;

      field_queue.push(tile_position_t{above, left});
      field_queue.push(tile_position_t{above, current_tile_pos.col});
      field_queue.push(tile_position_t{above, right});
      field_queue.push(tile_position_t{current_tile_pos.row, left});
      field_queue.push(tile_position_t{current_tile_pos.row, right});
      field_queue.push(tile_position_t{below, left});
      field_queue.push(tile_position_t{below, current_tile_pos.col});
      field_queue.push(tile_position_t{below, right});
    }
  } while (!field_queue.empty());

  return;
}

/* #endregion */
/* #region status checks */

bool Minefield::checkGameWon()
{
  MW_SET_FUNC_SCOPE;

  index_t
      revealed_tiles_count = 0l,
      correctly_flagged_mines_count = 0l;
  const index_t non_mine_tiles_count = this->field_size - this->nr_of_mines;

  for (const tile_t &current_tile : this->field)
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

  for (index_t row = 0l; row < this->current_field_size.rows; row++)
  {
    for (index_t col = 0l; col < this->current_field_size.cols; col++)
    {
      tile_t current_tile = this->getTile(row, col);

      if (!current_tile.is_revealed || (current_tile.is_revealed && current_tile.nr_surrounding_mines == 0))
        continue;

      uint8_t surrounding_flags = 0u,
              surrounding_covered = 0u;
      {
        index_t above = row - 1l,
               below = row + 1l,
               left = col - 1l,
               right = col + 1l;
        std::array<tile_position_t, 8ul> surrounding_tiles{
            tile_position_t{above, left},
            tile_position_t{above, col},
            tile_position_t{above, right},
            tile_position_t{row, left},
            tile_position_t{row, right},
            tile_position_t{below, left},
            tile_position_t{below, col},
            tile_position_t{below, right},
        };

        for (const tile_position_t &current_tile_pos : surrounding_tiles)
        {
          if (!this->tilePositionValid(current_tile_pos.row, current_tile_pos.col)) continue;
          const tile_t &surrounding_tile = this->getTile(current_tile_pos.row, current_tile_pos.col);

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

      const bool flagged_all_mines = (surrounding_flags == current_tile.nr_surrounding_mines);
      if ((flagged_all_mines && surrounding_covered > 0) ||
          (!flagged_all_mines && surrounding_covered == (current_tile.nr_surrounding_mines - surrounding_flags)))
      {
        MW_LOG(debug) << "row=" << row << "col=" << col << " has available moves";
        return true;
      }
    }
  }

  MW_LOG(warning) << "No more available moves.";

  return false;
}

bool Minefield::checkMineCountSatisfied(index_t row, index_t col)
{
  MW_SET_FUNC_SCOPE;

  if (!this->tilePositionValid(row, col))
  {
    MW_LOG_INVALID_TILE;

    return false;
  }
  const tile_t &tile = this->getTile(row, col);

  uint8_t mine_count = 0u;

  //! TODO: maybe rewrite without loop - the array size won't change anyways
  index_t above = row - 1l,
         below = row + 1l,
         left  = col - 1l,
         right = col + 1l;
  std::array<tile_position_t, 8ul> surrounding_tiles{
      tile_position_t{above, left},
      tile_position_t{above, col},
      tile_position_t{above, right},
      tile_position_t{row,   left},
      tile_position_t{row,   right},
      tile_position_t{below, left},
      tile_position_t{below, col},
      tile_position_t{below, right},
  };

  for (const tile_position_t &current_tile_pos : surrounding_tiles)
  {
    if (!this->tilePositionValid(current_tile_pos.row, current_tile_pos.col)) continue;
    const tile_t &current_tile = this->getTile(current_tile_pos.row, current_tile_pos.col);

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

/* #endregion */
/* #region getters */

const index_t &Minefield::getNrMines()
{
  MW_SET_FUNC_SCOPE;

  return this->nr_of_mines;
}

index_t Minefield::calculateNrOfMines()
{
  MW_SET_FUNC_SCOPE;

  return DEFAULT_BOMB_FACTOR * this->field_size;
}

inline bool Minefield::tilePositionValid(index_t row, index_t col)
{
  MW_SET_FUNC_SCOPE;

  return (
    row >= 0l && row < this->current_field_size.rows &&
    col >= 0l && col < this->current_field_size.cols
  );
}

Minefield::tile_t &Minefield::getTile(index_t row, index_t col)
{
  MW_SET_FUNC_SCOPE;

  return this->field[row * this->current_field_size.cols + col];
}

/* #endregion */