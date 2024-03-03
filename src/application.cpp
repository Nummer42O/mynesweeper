#include "application.hpp"

#include <iostream>
#include <sigc++/sigc++.h>
#include <thread>
#include <chrono>
#include <iomanip>


Application::Application():
  Gtk::Application("image_manipulator.main", Gio::APPLICATION_ALLOW_REPLACEMENT)
{
  MW_SET_CLASS_ORIGIN;
  MW_SET_FUNC_SCOPE;
}


/* #region Application internal */

void Application::on_activate()
{
  MW_SET_FUNC_SCOPE;

  this->window = std::make_unique<Window>(
    sigc::mem_fun3(*this, &Application::clickedCallback)
  );
  this->window->bindRestartButtonCallback(
    sigc::mem_fun0(*this, &Application::startGame)
  );

  this->new_game_dialog = std::make_shared<NewGameDialog>(*(this->window));
  this->no_moves_left_dialog = std::make_shared<NoMovesLeftDialog>(*(this->window));

  this->add_window(*(this->window));

  this->window->show_all();
  this->window->maximize();
}

/* #endregion */
/* #region Dialog Handlers */

NewGameDialog::ReturnType Application::showNewGame(NewGameDialog::Type type, index_t &o_rows, index_t &o_cols)
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

/* #endregion */
/* #region Mouse Callbacks */

void Application::clickedCallback(bool is_reveal, index_t row, index_t col)
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

void Application::revealCallback(index_t row, index_t col)
{
  MW_SET_FUNC_SCOPE;

  std::vector<Minefield::tile_with_position_t> cascade;
  bool has_hit_mine;
  if (!this->minefield->revealTile(row, col, cascade, has_hit_mine)) return;

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
        this->minefield->undoTileReveal(tile.row, tile.col);
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

    this->checkGameStatus();
  }
}

void Application::flagCallback(index_t row, index_t col)
{
  MW_SET_FUNC_SCOPE;

  bool is_flagged;
  if (!this->minefield->toggleTileFlag(row, col, is_flagged)) return;

  this->window->setFieldFlag(row, col, is_flagged);

  this->checkGameStatus();
}

/* #endregion */
/* #region Game Control */

void Application::startGame()
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
    [&]() -> void
    {
      this->newGame(NewGameDialog::Type::START);
    }
  );

# ifdef MW_DEBUG
  this->window->setTileDebugCallback(
    sigc::mem_fun2(*(this->minefield), &Minefield::getTileString)
  );
# endif //defined(DEBUG)
}

bool Application::newGame(NewGameDialog::Type type)
{
  MW_SET_FUNC_SCOPE;

  index_t  rows = this->current_field_size.rows,
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

void Application::checkGameStatus()
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
      this->minefield->revealTilesForUser(cascade);

      for (const Minefield::tile_with_position_t &tile: cascade);
    }
    else if (response == NoMovesLeftDialog::ReturnType::RESTART)
    {
      this->newGame(NewGameDialog::Type::RESTART);
    }
  }
}

/* #endregion */