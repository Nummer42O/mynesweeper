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
    index_t /*row*/,
    index_t /*col*/
  > callback_t;

private:
  enum class TileState {
    REVEALED, FLAGGED, UNTOUCHED,

    NO_OPT
  };

public:
  /**
   * @brief Create a minefield tile.
   *
   * @note Does still need a sprite.
   *
   * @param click_callback callback for when the tile gets clicked
   * @param flagged the normal and highlighted tile sprite for the flagged state
   * @param untouched the normal and highlighted tile sprite for the untouched state
   */
  Tile(
    callback_t click_callback,
    const state_sprites_t &flagged,
    const state_sprites_t &untouched
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
   */
  void flag();

  /**
   * @brief Unmark/Reset this tile.
   */
  void reset();

  /**
   * @brief Set the position associated with this tile.
   *
   * @param row row / y coordinate
   * @param col column / x coordinate
   */
  void setPosition(index_t row, index_t col);

# ifdef MW_DEBUG
  void setInformationCallback(const sigc::slot<std::string, index_t, index_t> &callback);
# endif //!defined(MW_DEBUG)

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

  Gtk::Image image;
  callback_t click_callback;

  index_t row, col;
  TileState state = TileState::UNTOUCHED;

  state_sprites_t flagged_sprites, untouched_sprites;
};