#pragma once

#include "defines.hpp"
#include "debug.hpp"

#include <gtkmm.h>


class NewGameDialog : public Gtk::Dialog
{
public:
  enum class Type
  {
    WIN, LOSE,
    START, RESTART,
    INVALID
  };

  enum class ReturnType
  {
    RESTART, QUIT, UNDO
  };

public:
  NewGameDialog(Gtk::Window &parent);

  ReturnType run(
    Type type,
    size_t &io_rows,
    size_t max_rows,
    size_t &io_cols,
    size_t max_cols
  );

public:
  static const int RESPONSE_UNDO = 1;

private:
  MW_DECLARE_LOGGER;

  Gtk::HScale rows_widget, cols_widget;
  Gtk::Label main_text;

  Gtk::Button *undo_button;

  static const std::unordered_map<Type, Glib::ustring> type2text;
};
