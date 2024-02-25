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

  typedef bool (Minefield::*activate_field_callback_t)(
    size_t /*row*/,
    size_t /*col*/,
    cascade_t & /*o_revealed_fields*/,
    bool & /*o_has_revealed_mine*/
  );

public:
  /* #region minefield generation */

  /**
   * @brief Create the minefield of given size and set it up for on demand initialization.
   *
   * @param rows new minefield height
   * @param cols new minefield width
   */
  Minefield(
    size_t rows,
    size_t cols
  );

  /**
   * @brief Resize the field to the new size if it differs and set it up for on demand initialization.
   *
   * @param rows new minefield height
   * @param cols new minefield width
   */
  void resize(
    size_t rows,
    size_t cols
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
  bool activateField(
    size_t row,
    size_t col,
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
  bool undoFieldActivation(
    size_t row,
    size_t col
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
  bool toggleFieldFlag(
    size_t row,
    size_t col,
    bool &o_is_flagged
  );

  /**
   * @brief Help the user by revealing fields until they have available moves again.
   *
   * @param o_revealed_fields vector of tiles that got revealed in cascade
   */
  void revealFieldsForUser(
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
  const size_t &getNrMines();

  /* #endregion */

private:
  /* #region minefield generation */

  /**
   * @brief Assign random states to the fields.
   */
  void initFields();

  /* #endregion */
  /* #region field manipulation */

  /**
   * @brief Reveal the first field and will set of a reveal cascade.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   * @param o_revealed_fields vector of tiles that got revealed in cascade
   * @param o_has_revealed_mine wether @ref `o_revealed_fields` contains a mine or not
   *
   * @returns true if the position was valid, false otherwise
   */
  bool activateFieldInitial(
    size_t row,
    size_t col,
    std::vector<tile_with_position_t> &o_revealed_fields,
    bool &o_has_revealed_mine
  );

  /**
   * @brief Attempt to reveal the field and may set of a reveal cascade.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   * @param o_revealed_fields vector of tiles that got revealed in cascade
   * @param o_has_revealed_mine wether @ref `o_revealed_fields` contains a mine or not
   *
   * @returns true if the position was valid, false otherwise
   */
  bool activateFieldMain(
    size_t row,
    size_t col,
    std::vector<tile_with_position_t> &o_revealed_fields,
    bool &o_has_revealed_mine
  );

  /* #endregion */
  /* #region status checks */

  /**
   * @brief Check wether all mines sourrnding the tile are flagged.
   *
   * @note It is not checked, wether this check itself makes sense or not.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   *
   * @returns true if the check succeded, false otherwise
   */
  bool checkMineCountSatisfied(
    size_t row,
    size_t col
  );

  /* #endregion */
  /* #region getters */

  /**
   * @brief Calculate number of mines from field count.
   *
   * @returns number of mines
   */
  size_t calculateNrOfMines();

  /**
   * @brief Get the field index from a tile position.
   *
   * @note No range checks are performed.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   *
   * @returns field vector index
   */
  inline size_t getTilePosition(
    size_t row,
    size_t col
  );

  /**
   * @brief Attempt to get the tile at the specified position.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   * @param o_is_valid true if the position is valid, false otherwise
   *
   * @returns the selected tile
   */
  tile_t &getTile(
    size_t row,
    size_t col,
    bool &o_is_valid
  );

  /* #endregion */

private:
  MW_DECLARE_LOGGER;

  activate_field_callback_t activate_field_callback;

  field_size_t current_field_size;
  size_t nr_of_mines;

  size_t field_size;
  std::vector<tile_t> field;
  bool field_inizialized;

  static tile_t default_tile;
  static const std::array<tile_offset_t, 8ul> offsets;
};