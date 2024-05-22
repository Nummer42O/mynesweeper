#pragma once

#include "debug.hpp"
#include "defines.hpp"

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <array>
#include <optional>
#include <sigc++/sigc++.h>


class Minefield
{
public:
  typedef struct
  {
    index_t row, col;

    // number (1-8), empty (0) or mine (-1)
    int type;
  } tile_with_position_t;

  typedef std::vector<tile_with_position_t> cascade_t;

  enum class ToggleFieldFlagStatus
  {
    OK,
    INVALID_TILE,
    ALREADY_REVEALED
  };

private:
  typedef struct
  {
    // semi constant
    bool is_mine = false;
    uint8_t nr_surrounding_mines = 0u;

    // toggleable
    bool \
      is_flagged  = false,
      is_revealed = false;

    // runtime
    uint8_t \
      nr_surrounding_flags      = 0u,
      nr_surrounding_untouched  = 8u;
  } tile_t;
  friend std::ostream &operator<<(std::ostream &stream, const tile_t &tile);

  typedef struct
  {
    index_t row, col;
  } tile_position_t;
  //! TODO: operator<<
  friend inline bool operator==(const tile_position_t &left, const tile_position_t &right);
  friend inline bool operator<(const tile_position_t &left, const tile_position_t &right);

  typedef struct
  {
    int64_t rows, cols;
  } tile_offset_t;

  /**
   * @param tile the currently selected adjacent tile
   * @param user_data custom data given to the callback
   *
   * @returns true if the loop for be exited, false otherwise
   */
  typedef bool (*for_surrounding_tiles_callback_t)(tile_t &tile, void *user_data);

public:
  /* #region minefield generation */

  /**
   * @brief Create the minefield of given size and set it up for on demand initialization.
   *
   * @param rows new minefield height
   * @param cols new minefield width
   */
  Minefield(
    index_t rows,
    index_t cols
  );

  /**
   * @brief Resize the field to the new size if it differs and set it up for on demand initialization.
   *
   * @param rows new minefield height
   * @param cols new minefield width
   */
  void resize(
    index_t rows,
    index_t cols
  );

  /**
   * @brief Set the field up for on demand initialization
   */
  void reset();

  /* #endregion */
  /* #region field manipulation */

  /**
   * @brief Attempt to reveal the field and maybe set of a reveal cascade.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   * @param o_revealed_fields vector of tiles that got revealed in cascade
   * @param o_has_revealed_mine wether @ref `o_revealed_fields` contains a mine or not
   *
   * @returns true if the position was valid, false otherwise
   */
  bool revealTile(
    index_t row,
    index_t col,
    cascade_t &o_revealed_fields,
    bool &o_has_revealed_mine
  );

  /**
   * @brief Attempt to undo revelation of field.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   *
   * @returns true if the position was valid, false otherwise
   */
  bool undoTileReveal(
    index_t row,
    index_t col
  );

  /**
   * @brief Flag or unflag the field as suspected mine.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   * @param o_is_flagged wether to activate or deactivate flag
   *
   * @returns true if o_is_flagged is valid, false otherwise
   */
  bool toggleTileFlag(
    index_t row,
    index_t col,
    bool &o_is_flagged
  );

  /**
   * @brief Help the user by revealing fields until they have available moves again.
   *
   * @param o_revealed_fields vector of tiles that got revealed in cascade
   */
  void revealTilesForUser(
    cascade_t &o_revealed_fields
  );

  /* #endregion */
  /* #region status checks */

  /**
   * @brief Check if all non-mine fields are revealed.
   *
   * @returns true if the check succeded, false otherwise
   */
  bool checkGameWon();

  /**
   * @brief Check if there are any "reasonable" moves left.
   *
   * @returns true if the check succeded, false otherwise
   */
  bool checkHasAvailableMoves();

  /* #endregion */
  /* #region getters */

  /**
   * @brief Get the number of mines for the current field.
   *
   * @returns nr of mines
   */
  const index_t &getNrMines();

# ifdef MW_DEBUG
  std::string getTileString(index_t row, index_t col);
# endif //defined(MW_DEBUG)

  /* #endregion */

private:
  /* #region minefield generation */

  /**
   * @brief Assign random states to the fields.
   *
   * @param row row / y coordinate of the initial patch source tile
   * @param col column / x coordinate of the initial patch source tile
   */
  void initFields(
    index_t row,
    index_t col
  );

  /* #endregion */
  /* #region field manipulation */

  /**
   * @brief Attempt to reveal the field and may set of a reveal cascade.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   * @param o_revealed_fields vector of tiles that got revealed in cascade
   * @param o_has_revealed_mine wether @ref `o_revealed_fields` contains a mine or not
   */
  void revealTileInternal(
    index_t row,
    index_t col,
    std::vector<tile_with_position_t> &o_revealed_fields,
    bool &o_has_revealed_mine
  );

  /**
   * @brief Give all valid surrounding tiles to callback.
   *
   * @note No range checks are performed on the source tile.
   *
   * @param row row / y coordinate of source tile
   * @param col column / x coordinate of source tile
   * @param callback function to apply to surrounding tiles
   * @param user_data data pointer to be reinterpreted by the callback
   */
  void forSurroundingMines(
    index_t row,
    index_t col,
    for_surrounding_tiles_callback_t callback,
    void *user_data = nullptr
  );

  /* #endregion */
  /* #region status checkers */

  /**
   * @brief Check if the given position is inside the field boundaries.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   *
   * @return true if row/col are in bounds, false otherwise
   */
  inline bool checkTilePositionValid(
    index_t row,
    index_t col
  );

  /**
   * @brief Check if a tile has available moves.
   *
   * @note Wether this check makes sense on this tile does not get checked.
   *
   * @param tile reference tile to be checked
   *
   * @return true if moves are available, false otherwise
   */
  static inline bool checkTileHasAvailableMoves(
    const tile_t &tile
  );

  /* #endregion*/
  /* #region getters */

  /**
   * @brief Calculate number of mines from field count.
   *
   * @returns number of mines
   */
  inline index_t calculateNrOfMines();

  /**
   * @brief Get the tile at the specified position.
   *
   * @note No range checks are performed.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   *
   * @returns the selected tile
   */
  inline tile_t &getTile(
    index_t row,
    index_t col
  );

  /* #endregion */

private:
  MW_DECLARE_LOGGER;

  field_size_t current_field_size;
  index_t nr_of_mines;

  index_t field_size;
  std::vector<tile_t> field;
  bool field_initialized = false;

  static tile_t default_tile;
  static const std::array<tile_offset_t, 8ul> offsets;
  static const std::array<tile_offset_t, 4ul> directions;
};