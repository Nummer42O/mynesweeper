#include "tile.hpp"

#ifdef MW_DEBUG
#include <sstream>
#include <iomanip>
#endif // defined(MW_DEBUG)


Tile::Tile(callback_t click_callback):
  EventBox(),
  click_callback(click_callback)
{
  MW_SET_CLASS_ORIGIN;
  MW_SET_FUNC_SCOPE;

  this->set_above_child(true);
  this->set_visible_window(true);
  this->signal_enter_notify_event().connect(
    sigc::mem_fun1(*this, &Tile::enterNotifyCallback)
  );
  this->signal_leave_notify_event().connect(
    sigc::mem_fun1(*this, &Tile::leaveNotifyCallback)
  );
  this->signal_button_release_event().connect(
    sigc::mem_fun1(*this, &Tile::buttonReleaseCallback)
  );

  this->add(this->image);
}

void Tile::revealAs(const sprite_t &sprite)
{
  MW_SET_FUNC_SCOPE;

  this->image.set(sprite);

  this->revealed = true;
}

void Tile::flag(const sprite_t &normal, const sprite_t &highlighted)
{
  MW_SET_FUNC_SCOPE;

  this->normal_sprite       = normal;
  this->highlighted_sprite  = highlighted;

  this->image.set(this->highlighted_sprite);
}

void Tile::reset(const sprite_t &normal, const sprite_t &highlighted)
{
  MW_SET_FUNC_SCOPE;

  this->normal_sprite       = normal;
  this->highlighted_sprite  = highlighted;

  this->image.set(this->normal_sprite);

  this->revealed = false;
}

void Tile::setPosition(size_t row, size_t col)
{
  MW_SET_FUNC_SCOPE;

  this->row = row;
  this->col = col;

# ifdef MW_DEBUG
  std::stringstream tooltip_text_stream;
  tooltip_text_stream << '(' << std::setw(2) << row << '|' << std::setw(2) << col << ')';

  this->set_tooltip_text(tooltip_text_stream.str());
# endif // defined(MW_DEBUG)
}

bool Tile::enterNotifyCallback(GdkEventCrossing *)
{
  MW_SET_FUNC_SCOPE;

  if (!this->revealed)
  {
    this->image.set(this->highlighted_sprite);
  }

  return false;
}

bool Tile::leaveNotifyCallback(GdkEventCrossing *)
{
  MW_SET_FUNC_SCOPE;

  if (!this->revealed)
  {
    this->image.set(this->normal_sprite);
  }

  return false;
}

bool Tile::buttonReleaseCallback(GdkEventButton *button)
{
  MW_SET_FUNC_SCOPE;

  if (button->button == GDK_BUTTON_PRIMARY)
  {
    this->click_callback(true, this->row, this->col);
  }
  else if (button->button == GDK_BUTTON_SECONDARY)
  {
    this->click_callback(false, this->row, this->col);
  }

  return false;
}