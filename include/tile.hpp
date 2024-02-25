#pragma once

#include "defines.hpp"
#include "debug.hpp"

#include <gtkmm.h>


class Tile : public Gtk::EventBox
{
public:
  typedef sigc::slot<
    void,
    bool /*is_reveal*/,
    size_t /*row*/,
    size_t /*col*/
  > callback_t;

private:
  typedef Glib::RefPtr<Gdk::Pixbuf> sprite_t;

public:
  /**
   * @brief Create a minefield tile.
   *
   * @note Does still need a sprite.
   *
   * @param click_callback callback for when the tile gets clicked
   */
  Tile(
    callback_t click_callback
  );

  /**
   * @brief Reveal this tile as a mined, empty or numbered tile.
   *
   * @param sprite the sprite showing what this tile is revealed as
   */
  void revealAs(
    const sprite_t &sprite
  );

  /**
   * @brief Mark this tile as flagged.
   *
   * @param normal the normal flagged tile sprite
   * @param highlighted the flagged tile sprite if the mouse hovers
   */
  void flag(
    const sprite_t &normal,
    const sprite_t &highlighted
  );

  /**
   * @brief Unmark/Reset this tile.
   *
   * @param normal the normal untouched tile sprite
   * @param highlighted the untouched tile sprite if the mouse hovers
   */
  void reset(
    const sprite_t &normal,
    const sprite_t &highlighted
  );

  /**
   * @brief Set the position associated with this tile.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   */
  void setPosition(size_t row, size_t col);

private:
  /**
   * @brief Callback for when the pointer enters the tile area.
   *
   * @param <unused>
   *
   * @return false to propagate the event further
   */
  bool enterNotifyCallback(GdkEventCrossing *);

  /**
   * @brief Callback for when the pointer leaves the tile area.
   *
   * @param <unused>
   *
   * @return false to propagate the event further
   */
  bool leaveNotifyCallback(GdkEventCrossing *);

  /**
   * @brief Callback for when the tile gets released by the pointer.
   *
   * @param <unused>
   *
   * @return false to propagate the event further
   */
  bool buttonReleaseCallback(GdkEventButton *button);

  // bool keyReleaseCallback(GdkEventKey *key);

private:
  MW_DECLARE_LOGGER;

  size_t row, col;
  callback_t click_callback;

  bool revealed = false;
  Glib::RefPtr<Gdk::Pixbuf> normal_sprite, highlighted_sprite;

  Gtk::Image image;
};