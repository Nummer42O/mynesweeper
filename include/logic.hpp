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
    size_t row, col;

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
    bool is_flagged = false;
    bool is_revealed = false;

    bool is_mine = false;
    uint8_t nr_surrounding_mines = 0;
  } tile_t;

  typedef struct
  {
    size_t row, col;
  } tile_position_t;

  typedef struct
  {
    int64_t rows, cols;
  } tile_offset_t;

  typedef sigc::slot<
      bool,
      size_t /*row*/, size_t /*col*/,
      std::vector<tile_with_position_t> & /*o_revealed_fields*/,
      bool & /*o_has_revealed_mine*/
      >
      activate_field_callback_t;

public:
  /* #region minefield generation */

  /**
   * Create a new minefield of given size and initialize it.
   *
   * @param rows: new minefield height
   * @param cols: new minefield width
   */
  Minefield(
      size_t rows,
      size_t cols);

  /**
   * Resizes the current field to the new size if it differs.
   * Will always reinitialize all fields.
   *
   * @param rows: new minefield height
   * @param cols: new minefield width
   */
  void resize(
      size_t rows,
      size_t cols);

  /**
   * Reininitialize the mine field with the same size as before.
   */
  void reset();

  /* #endregion */
  /* #region field manipulation */

  /**
   * bool activateField(
   *   size_t row,
   *   size_t col,
   *   cascade_t &o_revealed_fields,
   *   bool &o_has_revealed_mine
   * );
   */

  /**
   * @brief Attempts to undo revelation of field.
   *
   * @param row: row / y coordinate
   * @param col: column / x coordinate
   *
   * @return true if the position was valid, false otherwise
   */
  bool undoFieldActivation(
      size_t row,
      size_t col);

  /**
   * Flag or unflag the field as suspected mine.
   *
   * @param row: row / y coordinate
   * @param col: column / x coordinate
   * @param o_is_flagged: wether to activate or deactivate flag
   *
   * @returns true if o_is_flagged is valid, false otherwise
   */
  bool toggleFieldFlag(
      size_t row,
      size_t col,
      bool &o_is_flagged);

  void revealFieldsForUser(
      cascade_t &o_revealed_fields);

  /* #endregion */
  /* #region status checks */

  /**
   * @brief Check if all non-mine fields are revealed.
   *
   * @returns true if the check succeded, false otherwise
   */
  bool checkGameWon();

  /**
   * @brief Checks if there are any "reasonable" moves left.
   *
   * @returns true if the check succeded, false otherwise
   */
  bool checkHasAvailableMoves();

  /* #endregion */
  /* #region getters */

  /**
   * Get the number of mines for the current field.
   *
   * @returns nr of mines
   */
  const size_t &getNrMines();

  /* #endregion */

private:
  /* #region minefield generation */

  /**
   * Assign random states to the fields.
   */
  void initFields();

  /* #endregion */
  /* #region field manipulation */

  /**
   * Reveals the first field and will set of a reveal cascade.
   *
   * @param row: row / y coordinate
   * @param col: column / x coordinate
   * @param o_revealed_fields: vector of tiles that got revealed in cascade
   * @param o_has_revealed_mine: wether @ref `o_revealed_fields` contains a mine or not
   *
   * @returns true if the position was valid, false otherwise
   */
  bool activateFieldInitial(
      size_t row,
      size_t col,
      std::vector<tile_with_position_t> &o_revealed_fields,
      bool &o_has_revealed_mine);

  /**
   * Attempts to reveal the field and may set of a reveal cascade.
   *
   * @param row: row / y coordinate
   * @param col: column / x coordinate
   * @param o_revealed_fields: vector of tiles that got revealed in cascade
   * @param o_has_revealed_mine: wether @ref `o_revealed_fields` contains a mine or not
   *
   * @returns true if the position was valid, false otherwise
   */
  bool activateFieldMain(
      size_t row,
      size_t col,
      std::vector<tile_with_position_t> &o_revealed_fields,
      bool &o_has_revealed_mine);

  /* #endregion */
  /* #region status checks */

  /**
   * @brief Checks wether all mines sourrnding the tile are flagged.
   *
   * @note It is not checked, wether this check itself makes sense or not.
   *
   * @param pos: position of the tile in the field
   *
   * @returns true if the check succeded, false otherwise
   */
  bool checkMineCountSatisfied(
      const tile_position_t &pos);

  /* #endregion */
  /* #region getters */

  /**
   * Function to calculate number of mines from field count.
   *
   * @returns number of mines
   */
  size_t calculateNrOfMines();

  // TODO: get tile position

  /**
   * Checks if the indices are in bounds and returns Tile if so.
   *
   * @param pos: position of the tile in the field
   * @param o_is_valid: true if the position is valid, false otherwise
   *
   * @returns the selected tile
   */
  tile_t &getTile(
      const tile_position_t &pos,
      bool &o_is_valid);

  /* #endregion */

public:
  activate_field_callback_t activateField;

private:
  MW_DECLARE_LOGGER;

  field_size_t current_field_size;
  size_t nr_of_mines;

  size_t field_size;
  std::vector<tile_t> field;
  bool field_inizialized;

  static tile_t default_tile;
  static const std::array<tile_offset_t, 8ul> offsets;
};