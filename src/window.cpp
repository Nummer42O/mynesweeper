#include "window.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <exception>

#define _GET_TILE_WIDGET(row, col) static_cast<Tile *>(this->field_widget.get_child_at(col, row))


Window::Window(Tile::callback_t tile_clicked_callback):
  shared_tile_clicked_callback(tile_clicked_callback)
{
  MW_SET_CLASS_ORIGIN;
  MW_SET_FUNC_SCOPE;

  this->set_title(PROGRAM_NAME);
  this->set_icon_name(EXECUTABLE_NAME ".png");

  // add base box to be able to add multiple widgets
  Gtk::Box *base = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, SPACING);
  base->set_homogeneous(false);
  base->set_border_width(SPACING);
  this->add(*base);

  // add grid in scrolled window for field tiles
  Gtk::ScrolledWindow *field_scroll_widget = Gtk::make_managed<Gtk::ScrolledWindow>();
  field_scroll_widget->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  base->pack_start(*field_scroll_widget, Gtk::PACK_EXPAND_WIDGET);

  Gtk::Box *v_allign_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
  field_scroll_widget->add(*v_allign_box);

  Gtk::Box *h_allign_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
  v_allign_box->pack_start(*h_allign_box, Gtk::PACK_EXPAND_PADDING);

  // this->field_widget = Gtk::Grid();
  this->field_widget.set_row_spacing(SPACING);
  this->field_widget.set_column_spacing(SPACING);
  h_allign_box->pack_start(this->field_widget, Gtk::PACK_EXPAND_PADDING);

  // add base box for buttons and info
  Gtk::Box *controls_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, SPACING);
  base->pack_end(*controls_box, Gtk::PACK_SHRINK);

  // add controls
  this->nr_bombs_widget = Gtk::Label("/", Gtk::ALIGN_CENTER);
  controls_box->pack_start(this->nr_bombs_widget, Gtk::PACK_SHRINK);
  this->restart_widget = Gtk::Button("Start new game");
  controls_box->pack_end(this->restart_widget, Gtk::PACK_SHRINK);
}

void Window::bindRestartButtonCallback(sigc::slot<void> &&callback)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "changing restart button callback";

  if (!restart_button_callback_connection.empty())
  {
    restart_button_callback_connection.disconnect();
  }
  restart_button_callback_connection = std::move(this->restart_widget.signal_clicked().connect(callback, true));
}

bool Window::loadSprites()
{
  MW_SET_FUNC_SCOPE;

  try
  {
    this->reveal_sprites.at(0ul) = Gdk::Pixbuf::create_from_file(SPRITE_DIRECTORY "/mine.bmp");
    this->reveal_sprites.at(1ul) = Gdk::Pixbuf::create_from_file(SPRITE_DIRECTORY "/revealed.bmp");
    for (size_t idx = 1ul; idx < 9; idx++)
    {
      this->reveal_sprites.at(idx + 1ul) = Gdk::Pixbuf::create_from_file(SPRITE_DIRECTORY "/" + std::to_string(idx) + ".bmp");
    }

    untouched_sprites.normal = Gdk::Pixbuf::create_from_file(SPRITE_DIRECTORY "/untouched_normal.bmp");
    untouched_sprites.highlighted = Gdk::Pixbuf::create_from_file(SPRITE_DIRECTORY "/untouched_highlighted.bmp");

    flagged_sprites.normal = Gdk::Pixbuf::create_from_file(SPRITE_DIRECTORY "/flag_normal.bmp");
    flagged_sprites.highlighted = Gdk::Pixbuf::create_from_file(SPRITE_DIRECTORY "/flag_highlighted.bmp");
  }
  catch (const Glib::FileError &error)
  {
    MW_LOG(error) << "FileError: " << error.what();

    return false;
  }
  catch (const Gdk::PixbufError &error)
  {
    MW_LOG(error) << "PixbufError: " << error.what();

    return false;
  }

  return true;
}

void Window::revealField(size_t row, size_t col, int as)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "revealing field at row=" << row << " col=" << col;

  _GET_TILE_WIDGET(row, col)->revealAs(this->reveal_sprites.at(as + 1));
}

void Window::undoFieldReveal(size_t row, size_t col)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << "undoing field reveal at row=" << row << " col=" << col;

  _GET_TILE_WIDGET(row, col)->reset(this->untouched_sprites.normal, this->untouched_sprites.highlighted);
}

void Window::setFieldFlag(size_t row, size_t col, bool flag)
{
  MW_SET_FUNC_SCOPE;

  MW_LOG(trace) << (flag ? "setting" : "unsetting") << " field at row=" << row << " col=" << col;

  if (flag)
  {
    _GET_TILE_WIDGET(row, col)->flag(this->flagged_sprites.normal, this->flagged_sprites.highlighted);
    this->setMinesDisplay(++this->current_mines);
  }
  else
  {
    _GET_TILE_WIDGET(row, col)->reset(this->untouched_sprites.normal, this->untouched_sprites.highlighted);
    this->setMinesDisplay(--this->current_mines);
  }
}

field_size_t Window::getMaxFieldSize()
{
  MW_SET_FUNC_SCOPE;

  Gtk::Box *base = static_cast<Gtk::Box *>(this->get_child());
  Gtk::ScrolledWindow *scrolled_base = static_cast<Gtk::ScrolledWindow *>(base->get_children().at(0));

  int width  = scrolled_base->get_width(),
      height = scrolled_base->get_height();

  return field_size_t{(height - SPACING) / (SPACING + TILE_SIZE), (width - SPACING) / (SPACING + TILE_SIZE)};
}

size_t Window::getFieldPosition(size_t row, size_t col)
{
  MW_SET_FUNC_SCOPE;

  return row * this->current_field_size.cols + col;
}

void Window::generateMinefield(size_t rows, size_t cols, size_t nr_bombs)
{
  MW_SET_FUNC_SCOPE;

  const size_t  old_tile_count = this->current_field_size.rows * this->current_field_size.cols,
                new_tile_count = rows * cols;

  if (new_tile_count > old_tile_count)
  {
    for (size_t idx = old_tile_count; idx < new_tile_count; idx++)
    {
      Tile *new_tile = Gtk::make_managed<Tile>(this->shared_tile_clicked_callback);
      this->field_widget.add(*new_tile);
    }
  }
  else
  {
    const std::vector<Gtk::Widget *> children = this->field_widget.get_children();
    const auto end_it = children.begin() + (old_tile_count - new_tile_count);
    for (auto it = children.begin(); it != end_it; it++)
    {
      Widget *widget = *it;
      if (widget) this->field_widget.remove(*widget);
    }
  }

  std::vector<Gtk::Widget *> children = this->field_widget.get_children();
  for (size_t idx = 0ul; idx < new_tile_count; idx++)
  {
    const size_t  row = idx / cols,
                  col = idx % cols;

    Tile *tile = static_cast<Tile *>(children.at(idx));
    tile->setPosition(row, col);
    tile->reset(this->untouched_sprites.normal, this->untouched_sprites.highlighted);

    // a little bit of referencing magic so tiles can be reordered without being garbage collected
    tile->reference();
    this->field_widget.remove(*tile);
    this->field_widget.attach(*tile, col, row);
    tile->unreference();
  }

  this->field_widget.set_size_request(TILE_SIZE * cols, TILE_SIZE * rows);
  this->field_widget.show_all_children();

  this->current_max_mines = nr_bombs;
  this->current_field_size = {rows, cols};

  this->current_mines = 0ul;
  this->setMinesDisplay(this->current_mines);
}

void Window::resetMinefield()
{
  MW_SET_FUNC_SCOPE;

  for (Gtk::Widget *widget: this->field_widget.get_children())
  {
    static_cast<Tile *>(widget)->reset(this->untouched_sprites.normal, this->untouched_sprites.highlighted);
  }

  this->current_mines = 0ul;
  this->setMinesDisplay(this->current_mines);
}

void Window::setMinesDisplay(size_t nr_revealed_mines)
{
  MW_SET_FUNC_SCOPE;

  std::ostringstream label_text;
  label_text << std::setw(3) << nr_revealed_mines << '/' << std::setw(3) << this->current_max_mines;

  this->nr_bombs_widget.set_text(label_text.str());
}