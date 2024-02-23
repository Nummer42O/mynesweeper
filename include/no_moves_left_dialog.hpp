#pragma once

#include "defines.hpp"
#include "debug.hpp"

#include <gtkmm.h>


class NoMovesLeftDialog: public Gtk::Dialog
{
public:
  enum class ReturnType
  {
    YES, NO, RESTART
  };

public:
  NoMovesLeftDialog(Gtk::Window &parent);

  ReturnType run();

private:
  static const int RESPONSE_RESTART = 1;

  MW_DECLARE_LOGGER;
};