#include "tile.hpp"

#ifdef MW_DEBUG
#include <sstream>
#include <iomanip>
#endif // defined(MW_DEBUG)


namespace visuals
{
namespace widgets
{

Tile::Tile(callback_t click_callback, const state_sprites_t &flagged, const state_sprites_t &untouched):
  EventBox(),
  click_callback(click_callback),
  flagged_sprites(flagged), untouched_sprites(untouched)
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

  this->state = TileState::REVEALED;
}

void Tile::flag()
{
  MW_SET_FUNC_SCOPE;

  //! @note is highlighted because we expect the mouse pointer to be over the tile anyways
  this->image.set(this->flagged_sprites.highlighted);

  this->state = TileState::FLAGGED;
}

void Tile::reset()
{
  MW_SET_FUNC_SCOPE;

  this->image.set(this->untouched_sprites.normal);

  this->state = TileState::UNTOUCHED;
}

void Tile::setPosition(index_t row, index_t col)
{
  this->row = row;
  this->col = col;

# ifdef MW_DEBUG
  this->set_has_tooltip();
# endif //defined(MW_DEBUG)
}

# ifdef MW_DEBUG
void Tile::setInformationCallback(const sigc::slot<std::string, index_t, index_t> &callback)
{
  this->signal_query_tooltip().connect(
    [&](int, int, bool, const Glib::RefPtr<Gtk::Tooltip> &tooltip)
    {
      std::stringstream text;
      text << '(' << std::setw(2) << row << " | " << std::setw(2) << col << ")\n" << callback(this->row, this->col);
      tooltip->set_text(text.str());

      return true;
    }
  );
}
# endif // defined(MW_DEBUG)

bool Tile::enterNotifyCallback(GdkEventCrossing *)
{
  MW_SET_FUNC_SCOPE;

  if (this->state == TileState::FLAGGED)
  {
    this->image.set(this->flagged_sprites.highlighted);
  }
  else if (this->state == TileState::UNTOUCHED)
  {
    this->image.set(this->untouched_sprites.highlighted);
  }

  return false;
}

bool Tile::leaveNotifyCallback(GdkEventCrossing *)
{
  MW_SET_FUNC_SCOPE;

  if (this->state == TileState::FLAGGED)
  {
    this->image.set(this->flagged_sprites.normal);
  }
  else if (this->state == TileState::UNTOUCHED)
  {
    this->image.set(this->untouched_sprites.normal);
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

}
}
