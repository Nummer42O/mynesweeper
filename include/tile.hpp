#pragma once

#include "defines.hpp"
#include "debug.hpp"

#include <gtkmm.h>


class Tile : public Gtk::EventBox
{
public:
  typedef sigc::slot<void, bool /*is_reveal*/, size_t /*row*/, size_t /*col*/> callback_t;

private:
  typedef Glib::RefPtr<Gdk::Pixbuf> sprite_t;

public:
  Tile(
    callback_t click_callback /*,
    callback_t button_callback*/
  );

  void revealAs(
    const sprite_t &sprite
  );
  void flag(
    const sprite_t &normal,
    const sprite_t &highlighted
  );
  void reset(
    const sprite_t &normal,
    const sprite_t &highlighted
  );

  void setPosition(size_t row, size_t col);

private:
  bool enter_notify_callback(GdkEventCrossing *);
  bool leave_notify_callback(GdkEventCrossing *);
  bool button_release_callback(GdkEventButton *button);
  // bool key_release_callback(GdkEventKey *key);

private:
  MW_DECLARE_LOGGER;

  size_t row, col;
  callback_t click_callback;

  bool revealed = false;
  Glib::RefPtr<Gdk::Pixbuf> normal_sprite, highlighted_sprite;

  Gtk::Image image;
};