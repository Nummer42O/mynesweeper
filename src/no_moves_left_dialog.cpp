#include "no_moves_left_dialog.hpp"

/**
 * TODO:
 *  - actually implement this
 *  - implement logic function behind this:
 *   1. generate list of all unrevealed tiles adjacent to revealed ones
 *   2. iterate over those and check if revealing those would solve the problem
 *   3. if none of those solve the problem, reveal any of them, remove it from list and repeat from 2.
*/

NoMovesLeftDialog::NoMovesLeftDialog(Gtk::Window &parent):
  Dialog("No Moves Left", parent, Gtk::DIALOG_DESTROY_WITH_PARENT | Gtk::DIALOG_MODAL)
{
  MW_SET_CLASS_ORIGIN;
  MW_SET_FUNC_SCOPE;

  Gtk::Box *content_area = this->get_content_area();

  Gtk::Label *text = Gtk::make_managed<Gtk::Label>("You've got no moves left.\nDo you want some help?");
  text->set_margin_left(50);
  text->set_margin_right(50);
  content_area->pack_start(*text, Gtk::PACK_EXPAND_WIDGET, 10u);

  content_area->show_all();

  this->add_button(Gtk::Stock::YES, Gtk::RESPONSE_YES);
  this->add_button(Gtk::Stock::NO,  Gtk::RESPONSE_NO);
  this->add_button("_Restart",      this->RESPONSE_RESTART);
}

NoMovesLeftDialog::ReturnType NoMovesLeftDialog::run()
{
  int dialog_response = Gtk::Dialog::run();

  ReturnType result;
  switch (dialog_response)
  {
  case Gtk::RESPONSE_YES:
    MW_LOG(trace) << "no moves left dialog returned yes";

    result = ReturnType::YES;
    break;
  case Gtk::RESPONSE_NO:
    MW_LOG(trace) << "no moves left dialog returned no";

    result = ReturnType::NO;
    break;
  case this->RESPONSE_RESTART:
    MW_LOG(trace) << "no moves left dialog returned restart";

    result = ReturnType::RESTART;
    break;
  default:
    MW_LOG(error) << "unkown/-expected dialog return value: " << dialog_response << std::endl;

    result = ReturnType::NO;
    break;
  }

  return result;
}
