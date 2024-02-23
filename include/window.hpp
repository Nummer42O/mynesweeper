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
  Window(
    Tile::callback_t tile_clicked_callback
  );

  void bindRestartButtonCallback(
    sigc::slot<void> &&callback
  );

  /**
   * Load mine and flag sprites into memory.
   */
  bool loadSprites();

  /**
   * @brief Reveal a field as a number (1-8), empty (0) or bomb (-1).
   *
   * @param row: row of the target tile
   * @param col: column of the target tile
   * @param as: type of field
   */
  void revealField(
    size_t row,
    size_t col,
    int as
  );

  /**
   * @brief Undo revelation of field.
   *
   * @param row: row / y coordinate
   * @param col: column / x coordinate
   */
  void undoFieldReveal(
    size_t row,
    size_t col
  );

  /**
   * @brief Flag/Unflag a field as possible bomb.
   *
   * @param row: row of the target tile
   * @param col: column of the target tile
   */
  void setFieldFlag(
    size_t row,
    size_t col,
    bool flag
  );

  void generateMinefield(
    size_t rows,
    size_t cols,
    size_t nr_bombs
  );

  void resetMinefield();

  void setMinesDisplay(
    size_t nr_revealed_mines
  );

  /**
   * Calculate how many rows and columns can be put in the current window.
   */
  field_size_t getMaxFieldSize();

private:

  size_t getFieldPosition(
    size_t row,
    size_t col
  );

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