#include "application.hpp"

#include <iostream>
#include <sigc++/sigc++.h>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sysexits.h>


Application::Application():
  Gtk::Application("image_manipulator.main", Gio::APPLICATION_ALLOW_REPLACEMENT)
{
  MW_SET_CLASS_ORIGIN;
  MW_SET_FUNC_SCOPE;
}

void Application::on_activate()
{
  MW_SET_FUNC_SCOPE;

  this->window = std::make_unique<Window>(
    sigc::mem_fun3(*this, &Application::clickedCallback)
  );
  this->window->bindRestartButtonCallback(
    sigc::mem_fun0(*this, &Application::restartButtonCallbackInitial)
  );

  if (!this->window->loadSprites())
  {
    std::exit(EX_NOINPUT);
  }

  this->new_game_dialog = std::make_shared<NewGameDialog>(*(this->window));
  this->no_moves_left_dialog = std::make_shared<NoMovesLeftDialog>(*(this->window));

  this->add_window(*(this->window));

  this->window->show_all();
  this->window->maximize();
}

void Application::restartButtonCallbackInitial()
{
  MW_SET_FUNC_SCOPE;

  NewGameDialog::ReturnType new_game_return_type = this->showNewGame(NewGameDialog::Type::START, current_field_size.rows, current_field_size.cols);

  if (new_game_return_type == NewGameDialog::ReturnType::QUIT)
  {
    this->quit();
  }

  this->minefield = std::make_unique<Minefield>(current_field_size.rows, current_field_size.cols);
  this->window->generateMinefield(current_field_size.rows, current_field_size.cols, this->minefield->getNrMines());

  this->window->bindRestartButtonCallback(
    sigc::mem_fun0(*this, &Application::restartButtonCallback)
  );
}

void Application::restartButtonCallback()
{
  MW_SET_FUNC_SCOPE;

  this->newGame(NewGameDialog::Type::RESTART);
}

NewGameDialog::ReturnType Application::showNewGame(NewGameDialog::Type type, size_t &o_rows, size_t &o_cols)
{
  MW_SET_FUNC_SCOPE;

  field_size_t theoretical_max = this->window->getMaxFieldSize();
  NewGameDialog::ReturnType dialog_response = this->new_game_dialog->run(type,
    o_rows, theoretical_max.rows,
    o_cols, theoretical_max.cols
  );
  this->new_game_dialog->hide();

  return dialog_response;
}

NoMovesLeftDialog::ReturnType Application::showNoMovesLeft()
{
  NoMovesLeftDialog::ReturnType dialog_response = this->no_moves_left_dialog->run();
  this->no_moves_left_dialog->hide();

  return dialog_response;
}

bool Application::newGame(NewGameDialog::Type type)
{
  MW_SET_FUNC_SCOPE;

  size_t  rows = this->current_field_size.rows,
          cols = this->current_field_size.cols;
  NewGameDialog::ReturnType new_game_return_type = this->showNewGame(type, rows, cols);
  if (new_game_return_type == NewGameDialog::ReturnType::UNDO)
  {
    return true;
  }

  if (new_game_return_type == NewGameDialog::ReturnType::QUIT)
  {
    this->quit();

    return false;
  }

  MW_LOG(debug) << "current: rows=" << this->current_field_size.rows << " cols=" << this->current_field_size.cols;
  MW_LOG(debug) << "new:     rows=" << rows                          << " cols=" << cols;
  if (this->current_field_size.rows == rows && this->current_field_size.cols == cols)
  {
    this->minefield->reset();
    this->window->resetMinefield();
  }
  else
  {
    this->minefield->resize(rows, cols);
    this->window->generateMinefield(rows, cols, this->minefield->getNrMines());

    this->current_field_size = {rows, cols};
  }

  return false;
}

void Application::checkGameWon()
{
  if (this->minefield->checkGameWon())
  {
    this->newGame(NewGameDialog::Type::WIN);
  }
  else if (!this->minefield->checkHasAvailableMoves())
  {
    NoMovesLeftDialog::ReturnType response = this->showNoMovesLeft();

    if (response == NoMovesLeftDialog::ReturnType::YES)
    {
      Minefield::cascade_t cascade;
      this->minefield->revealFieldsForUser(cascade);

      for (const Minefield::tile_with_position_t &tile: cascade);
    }
    else if (response == NoMovesLeftDialog::ReturnType::RESTART)
    {
      this->newGame(NewGameDialog::Type::RESTART);
    }
  }
}

void Application::clickedCallback(bool is_reveal, size_t row, size_t col)
{
  MW_SET_FUNC_SCOPE;

  if (is_reveal)
  {
    this->revealCallback(row, col);
  }
  else
  {
    this->flagCallback(row, col);
  }
}

void Application::revealCallback(size_t row, size_t col)
{
  MW_SET_FUNC_SCOPE;

  std::vector<Minefield::tile_with_position_t> cascade;
  bool has_hit_mine;
  if (!this->minefield->activateField(row, col, cascade, has_hit_mine)) return;

  if (has_hit_mine)
  {
    for (const Minefield::tile_with_position_t &tile : cascade)
    {
      this->window->revealField(tile.row, tile.col, tile.type);
      if (tile.type == -1) break;
    }

    if (this->newGame(NewGameDialog::Type::LOSE))
    {
      for (const Minefield::tile_with_position_t &tile : cascade)
      {
        this->minefield->undoFieldActivation(tile.row, tile.col);
        this->window->undoFieldReveal(tile.row, tile.col);
      }
    }
  }
  else
  {
    for (const Minefield::tile_with_position_t &tile : cascade)
    {
      this->window->revealField(tile.row, tile.col, tile.type);
    }

    this->checkGameWon();
  }
}

void Application::flagCallback(size_t row, size_t col)
{
  MW_SET_FUNC_SCOPE;

  bool is_flagged;
  if (!this->minefield->toggleFieldFlag(row, col, is_flagged)) return;

  this->window->setFieldFlag(row, col, is_flagged);

  this->checkGameWon();
}
