#include "new_game_dialog.hpp"


const std::unordered_map<NewGameDialog::Type, Glib::ustring> NewGameDialog::type2text {
  {Type::START, "Lets get started, pick a size:"},
  {Type::RESTART, "Wanna try again? No problem, pick a size:"},
  {Type::WIN, "You won, congratulations!"},
  {Type::LOSE, "Misson failed. We'll get em' next time."}
};


NewGameDialog::NewGameDialog(Gtk::Window &parent):
  Dialog("New Game", parent, Gtk::DIALOG_DESTROY_WITH_PARENT | Gtk::DIALOG_MODAL)
{
  MW_SET_CLASS_ORIGIN;
  MW_SET_FUNC_SCOPE;

  Gtk::Box *content_area = this->get_content_area();

  this->main_text.set_margin_left(50);
  this->main_text.set_margin_right(50);
  content_area->pack_start(this->main_text, Gtk::PACK_EXPAND_WIDGET, 10u);

  Gtk::Grid *base = Gtk::make_managed<Gtk::Grid>();
  base->set_border_width(SPACING);
  base->set_column_spacing(SPACING);
  base->set_column_spacing(SPACING);
  base->set_column_homogeneous(false);
  base->set_row_homogeneous(true);
  content_area->pack_start(*base, Gtk::PACK_EXPAND_WIDGET);

  Gtk::Label *rows_label = Gtk::make_managed<Gtk::Label>("Rows: ", Gtk::ALIGN_START);
  base->attach(*rows_label, 0, 0);
  this->rows_widget.set_digits(0);
  this->rows_widget.set_round_digits(0);
  this->rows_widget.set_slider_size_fixed(true);
  this->rows_widget.set_hexpand(true);
  base->attach(this->rows_widget, 1, 0);

  Gtk::Label *cols_label = Gtk::make_managed<Gtk::Label>("Cols: ", Gtk::ALIGN_START);
  base->attach(*cols_label, 0, 1);
  this->cols_widget.set_digits(0);
  this->cols_widget.set_round_digits(0);
  this->cols_widget.set_slider_size_fixed(true);
  this->cols_widget.set_hexpand(true);
  base->attach(this->cols_widget, 1, 1);

  content_area->show_all();

  this->add_button(Gtk::Stock::OK,    Gtk::RESPONSE_OK);
  this->undo_button = this->add_button(Gtk::Stock::UNDO,  this->RESPONSE_UNDO);
  this->add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
}

int NewGameDialog::run(Type type, size_t &io_rows, size_t max_rows, size_t &io_cols, size_t max_cols)
{
  MW_SET_FUNC_SCOPE;

  this->main_text.set_text(this->type2text.at(type));

  this->rows_widget.set_range(MIN_FIELD_ROWS, (max_rows > MIN_FIELD_ROWS ? static_cast<double>(max_rows) : MIN_FIELD_ROWS));
  this->rows_widget.set_value(static_cast<double>(io_rows));

  this->cols_widget.set_range(MIN_FIELD_ROWS, (max_cols > MIN_FIELD_COLS ? static_cast<double>(max_cols) : MIN_FIELD_COLS));
  this->cols_widget.set_value(static_cast<double>(io_cols));

  if (type == Type::LOSE)
  {
    this->undo_button->set_sensitive(true);
  }
  else
  {
    this->undo_button->set_sensitive(false);
  }

  int response = Gtk::Dialog::run();

  io_rows = static_cast<size_t>(this->rows_widget.get_value());
  io_cols = static_cast<size_t>(this->cols_widget.get_value());

  return response;
}