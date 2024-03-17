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


std::ostream &operator<<(std::ostream &stream, const Minefield::tile_t &tile)
{
  stream << std::boolalpha << "{"
    "is mine: " << tile.is_mine << "; "
    "nr surrounding mines: " << (int)tile.nr_surrounding_mines << "; "
    "is flagged: " << tile.is_flagged << "; "
    "is revealed: " << tile.is_revealed << "; "
    "nr surrounding flags: " << (int)tile.nr_surrounding_flags << "; "
    "nr surrounding untouched: " << (int)tile.nr_surrounding_untouched << '}';

  return stream;
}

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

Minefield::Minefield(index_t rows, index_t cols):
  current_field_size{rows, cols}
{
  MW_SET_CLASS_ORIGIN;
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "new with rows=" << rows << " cols=" << cols;

  this->field_size = rows * cols;
  this->field = std::vector<Minefield::tile_t>(this->field_size);

  this->nr_of_mines = this->calculateNrOfMines();

  this->field_initialized = false;
}

void Minefield::resize(index_t rows, index_t cols)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "resize with rows=" << rows << " cols=" << cols;

  this->current_field_size = {rows, cols};

  this->field_size = rows * cols;
  this->field.resize(this->field_size);

  this->nr_of_mines = this->calculateNrOfMines();

  this->field_initialized = false;
}

void Minefield::reset()
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "reset";

  this->field_initialized = false;
}

void Minefield::initFields(index_t row, index_t col)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "initialize field from row=" << row << " col=" << col;

  // setup random device
  std::random_device random_device;
  std::mt19937 mersenne_twister(random_device());
  std::uniform_int_distribution<size_t> random_direction_idx(0ul, 3ul);

  // reset field
  index_t tile_position_idx = 0l;
  std::generate(
    this->field.begin(), this->field.end(),
    [&]()
    {
      assert(tile_position_idx < this->field_size);

      tile_t tile;
      const index_t \
        row = tile_position_idx / this->current_field_size.cols,
        col = tile_position_idx % this->current_field_size.cols;
      uint8_t untouched_tiles_correction = 0u;
      if (row == 0l || row == this->current_field_size.rows - 1) untouched_tiles_correction += 3u;
      if (col == 0l || col == this->current_field_size.cols - 1) untouched_tiles_correction += 3u;
      if (untouched_tiles_correction == 6u) untouched_tiles_correction = 5u;
      tile.nr_surrounding_untouched -= untouched_tiles_correction;

      tile_position_idx++;

      return tile;
    }
  );

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
  //! TODO: add set stream operator implementation
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
  tile_position_idx = 0l;
  std::generate(
    possible_mine_positions.begin(), possible_mine_positions.end(),
    [&]()
    {
      tile_position_t tile_position;
      do
      {
        assert(tile_position_idx < this->field_size);

        tile_position = tile_position_t{
          /*rows = */ tile_position_idx / this->current_field_size.cols,
          /*cols = */ tile_position_idx % this->current_field_size.cols
        };
        tile_position_idx++;
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

      if (!this->checkTilePositionValid(current_row, current_col)) continue;
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

  MW_LOG(trace) << "reveal @ row=" << row << " col=" << col;

  if (!this->checkTilePositionValid(row, col))
  {
    MW_LOG_INVALID_TILE;

    return false;
  }

  if (!this->field_initialized)
  {
    this->initFields(row, col);
    this->field_initialized = true;
  }

  this->revealTileInternal(row, col, o_revealed_fields, o_has_revealed_mine);

  return true;
}

bool Minefield::undoTileReveal(index_t row, index_t col)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "undoing field activation at row=" << row << " col=" << col;

  if (!this->checkTilePositionValid(row, col))
  {
    MW_LOG_INVALID_TILE;

    return false;
  }
  tile_t &tile = this->getTile(row, col);

  tile.is_revealed = false;
  this->forSurroundingMines(
    row, col,
    [](tile_t &surrounding_tile, const void *)
    {
      surrounding_tile.nr_surrounding_untouched++;
    }
  );

  return true;
}

bool Minefield::toggleTileFlag(index_t row, index_t col, bool &o_is_flagged)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "toggling flag @ row=" << row << " col=" << col;

  if (!this->checkTilePositionValid(row, col))
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

  MW_LOG(debug) << "flag is now flagged: " << std::boolalpha << o_is_flagged;

  this->forSurroundingMines(
    row, col,
    [](tile_t &surrounding_tile, const void *user_data)
    {
      //! NOTE: (int)false = 0 -> offset = -1; (int)true = 1 -> 1
      const int8_t offset = 2 * (*static_cast<const int8_t *>(user_data)) - 1;
      surrounding_tile.nr_surrounding_flags += offset;
      surrounding_tile.nr_surrounding_untouched -= offset;
    },
    &o_is_flagged
  );

  return true;
}

void Minefield::revealTilesForUser(cascade_t &o_revealed_fields)
{
  MW_SET_FUNC_SCOPE;

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
  if (tile.is_revealed && tile.nr_surrounding_mines == tile.nr_surrounding_flags)
  {
    MW_LOG(trace) << "tile is revealed and satisfied";

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
    MW_LOG(trace) << "tile is not available for reveal";

    field_queue.push(tile_position_t{row, col});
  }
  else
  {
    MW_LOG(trace) << "nothing to reveal";
    MW_LOG(debug) << tile;

    return;
  }

  // by default we don't expect to hit a mine, if we hit one this will be overwritten
  do
  {
    // get queue element
    tile_position_t current_tile_pos = std::move(field_queue.front());
    field_queue.pop();

    MW_LOG(trace) << "processing queue tile @ row=" << current_tile_pos.row << " col=" << current_tile_pos.col;

    if (!this->checkTilePositionValid(current_tile_pos.row, current_tile_pos.col)) continue;
    tile_t &current_tile = this->getTile(current_tile_pos.row, current_tile_pos.col);

    // check if we need to evaluate further
    if (current_tile.is_flagged || current_tile.is_revealed) continue;

    // should be revealed now
    current_tile.is_revealed = true;
    this->forSurroundingMines(
      current_tile_pos.row, current_tile_pos.col,
      [](tile_t &surrounding_tile, const void *)
      {
        surrounding_tile.nr_surrounding_untouched--;
      }
    );

    // check if we hit a mine
    if (current_tile.is_mine)
    {
      MW_LOG(trace) << "hit a mine";

      o_has_revealed_mine = true;
      o_revealed_fields.push_back(tile_with_position_t{current_tile_pos.row, current_tile_pos.col, -1});

      break;
    }
    else
    {
      MW_LOG(trace) << "hit normal tile";
      o_revealed_fields.push_back(tile_with_position_t{current_tile_pos.row, current_tile_pos.col, current_tile.nr_surrounding_mines});
    }

    // if we hit an "empty" (no adjacent mines) reveal, add all adjacent fields to the queue
    if (current_tile.nr_surrounding_mines == 0)
    {
      MW_LOG(trace) << "adding new elements to queue";

      index_t \
        above = current_tile_pos.row - 1l,
        below = current_tile_pos.row + 1l,
        left  = current_tile_pos.col - 1l,
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

void Minefield::forSurroundingMines(index_t row, index_t col, for_surrounding_tiles_callback_t callback, const void *user_data)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "iterating around row=" << row << " col=" << col;

  for (const tile_offset_t& offset: this->offsets)
  {
    const index_t \
      current_row = row + offset.rows,
      current_col = col + offset.cols;

    if (!this->checkTilePositionValid(current_row, current_col)) continue;

    MW_LOG(trace) << "surrounding tile @ row=" << current_row << " col=" << current_col;

    callback(this->getTile(current_row, current_col), user_data);
  }
}

/* #endregion */
/* #region status checks */

bool Minefield::checkGameWon()
{
  MW_SET_FUNC_SCOPE;

  //! TODO: CONTINUE HERE (use the newly introduces runtime variables on the tiles)

  index_t \
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

  // for (const tile_t &tile: this->field)
  for (index_t idx = 0l; idx < this->field_size; idx++)
  {
    const tile_t &tile = this->field[idx];

    if (
      tile.is_flagged   ||
      !tile.is_revealed ||
      (tile.is_revealed && tile.nr_surrounding_mines == 0)
    ) continue;

    MW_LOG(debug) << tile;

    /**
     * NOTE: a tile has moves available, when...
     *  ...a satisfied mine has still non-revealed fields
     *  ...a non-satisfied mine has the same number unrevealed adjacent tiles as remaining unflagged adjacent mines
    */
    const bool tile_satisfied = (tile.nr_surrounding_flags == tile.nr_surrounding_mines);
    if (
      (tile_satisfied && tile.nr_surrounding_untouched > 0) ||
      (!tile_satisfied && tile.nr_surrounding_untouched == (tile.nr_surrounding_mines - tile.nr_surrounding_flags))
    )
    {
      MW_LOG(debug) << "moves available @ row=" << idx / this->current_field_size.cols << " cols=" << idx % this->current_field_size.cols;
      return true;
    }
  }

  MW_LOG(warning) << "No more available moves.";

  return false;
}

inline bool Minefield::checkTilePositionValid(index_t row, index_t col)
{
  return (
    row >= 0l && row < this->current_field_size.rows &&
    col >= 0l && col < this->current_field_size.cols
  );
}

/* #endregion */
/* #region getters */

const index_t &Minefield::getNrMines()
{
  return this->nr_of_mines;
}

#ifdef MW_DEBUG
std::string Minefield::getTileString(index_t row, index_t col)
{
  MW_SET_FUNC_SCOPE;

  if (!this->field_initialized) return "";
  const tile_t &tile = this->getTile(row, col);

  std::stringstream text;
  text << \
    "type: " << (tile.is_mine ? "mine" : std::to_string(tile.nr_surrounding_mines)) << "\n"
    "state: " << (tile.is_flagged ? "flagged" : (tile.is_revealed ? "revealed" : "untouched")) << "\n"
    "adjacent flags: " << (int)tile.nr_surrounding_flags << "\n"
    "adjacent untouched: " << (int)tile.nr_surrounding_untouched;

  return text.str();
}
#endif // defined(MW_DEBUG)

inline index_t Minefield::calculateNrOfMines()
{
  return DEFAULT_BOMB_FACTOR * this->field_size;
}

inline bool Minefield::tilePositionValid(index_t row, index_t col)
{
  return (
    row >= 0l && row < this->current_field_size.rows &&
    col >= 0l && col < this->current_field_size.cols
  );
}

inline Minefield::tile_t &Minefield::getTile(index_t row, index_t col)
{
  return this->field[row * this->current_field_size.cols + col];
}

/* #endregion */