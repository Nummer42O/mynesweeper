#pragma once

#include "defines.hpp"
#include "debug.hpp"

#include <gtkmm.h>


namespace visuals
{
namespace dialogs
{

class NewGameDialog : public Gtk::Dialog
{
public:
  enum class Type
  {
    WIN, LOSE,
    START, RESTART,

    NO_OPT
  };

  enum class ReturnType
  {
    RESTART, QUIT, UNDO,

    NO_OPT
  };

public:
  /**
   * @brief Create a new dialog for selecting the field size for the next game.
   *
   * @param parent dialog parent window
   */
  NewGameDialog(Gtk::Window &parent);

  /**
   * @brief Run the dialog.
   *
   * @param type The type of information to show about the game state.
   * @param io_rows user selected rows; initial value gets reflected in the scale
   * @param max_rows the upper soft limit for the number of rows
   * @param io_cols user selected columns; initial value gets reflected in the scale
   * @param max_cols the upper soft limit for the number of columns
   *
   * @return the action the user selected
   */
  ReturnType run(
    Type type,
    index_t &io_rows,
    index_t max_rows,
    index_t &io_cols,
    index_t max_cols
  );

private:
  static const std::unordered_map<Type, Glib::ustring> type2text;
  static const int RESPONSE_UNDO = 1;

  MW_DECLARE_LOGGER;

  Gtk::HScale rows_widget, cols_widget;
  Gtk::Label  main_text;
  Gtk::Button *undo_button;
};

}
}
