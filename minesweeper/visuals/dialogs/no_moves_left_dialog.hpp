#pragma once

#include "defines.hpp"
#include "debug.hpp"

#include <gtkmm.h>


namespace visuals
{
namespace dialogs
{

class NoMovesLeftDialog: public Gtk::Dialog
{
public:
  enum class ReturnType
  {
    YES, NO, RESTART,

    NO_OPT
  };

public:
  /**
   * @brief Create a new dialog for asking the user if they want help.
   *
   * @param parent dialog parent window
   */
  NoMovesLeftDialog(Gtk::Window &parent);

  /**
   * @brief Run the dialog.
   *
   * @return the action the user selected 
   */
  ReturnType run();

private:
  static const int RESPONSE_RESTART = 1;

  MW_DECLARE_LOGGER;
};

}
}
