#pragma once

#include "defines.hpp"
#include "tile.hpp"
#include "debug.hpp"

#include <gtkmm.h>

#include <vector>
#include <unordered_map>
#include <memory>


class Window : public Gtk::Window
{
public:
  typedef sigc::slot<
    void
  > restart_button_callback_t;

public:
  /**
   * @brief Create a window displaying the minefield.
   *
   * @param tile_clicked_callback the callback to relay to all tiles
   */
  Window(
    Tile::callback_t tile_clicked_callback
  );

  /**
   * @brief (Re)bind a callback to the restart button.
   *
   * @param callback the new callback
   */
  void bindRestartButtonCallback(
    restart_button_callback_t callback
  );

  /**
   * @brief Load mine and flag sprites into memory.
   *
   * @returns false if a sprite could not be loaded, true otherwise
   */
  bool loadSprites();

  /**
   * @brief Reveal a tile as a number (1-8), empty (0) or bomb (-1).
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   * @param as type of field
   */
  void revealField(
    size_t row,
    size_t col,
    int as
  );

  /**
   * @brief Undo revelation of tile.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   */
  void undoFieldReveal(
    size_t row,
    size_t col
  );

  /**
   * @brief Toggle tile as flagged.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   */
  void setFieldFlag(
    size_t row,
    size_t col,
    bool flag
  );

  /**
   * @brief Generate/Rearrange and reset all the tiles in the minefield.
   *
   * @param rows vertical field size
   * @param cols horizontal field size
   * @param nr_bombs the maximum number of bombs to be found
   */
  void generateMinefield(
    size_t rows,
    size_t cols,
    size_t nr_bombs
  );

  /**
   * @brief Reset the current minefield without changing its size.
   */
  void resetMinefield();

  /**
   * @brief Check the soft maximum rows and columns for the current window size.
   */
  field_size_t getMaxFieldSize();

private:
  /**
   * @brief Update the mine counter label/display.
   */
  void setMinesDisplay();

  /**
   * @brief Get the @ref Tile from the field grid widgets children.
   *
   * @note Does not check if the pointer is valid.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   *
   * @return a pointer to the tile
   */
  inline Tile *getTile(size_t row, size_t col);

private:
  MW_DECLARE_LOGGER;

  size_t  current_mines = 0ul,
          current_max_mines;
  field_size_t current_field_size;

  // 0: bomb, 1: empty, x (2-9): field nr x-2
  std::array<Glib::RefPtr<Gdk::Pixbuf>, 10ul> reveal_sprites;
  struct
  {
    Glib::RefPtr<Gdk::Pixbuf> normal;
    Glib::RefPtr<Gdk::Pixbuf> highlighted;
  } untouched_sprites, flagged_sprites;

  Gtk::Grid field_widget;
  Gtk::Label nr_bombs_widget;
  Gtk::Button restart_widget;

  Tile::callback_t shared_tile_clicked_callback;
  sigc::connection restart_button_callback_connection;
};