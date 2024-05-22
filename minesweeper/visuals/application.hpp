#pragma once

#include "visuals/window.hpp"
#include "visuals/dialogs/new_game_dialog.hpp"
#include "visuals/dialogs/no_moves_left_dialog.hpp"
#include "logic/logic.hpp"
#include "debug.hpp"

#include <memory>


class Application : public Gtk::Application
{
public:
  /**
   * @brief Instanciate Gtk::Application to "image_manipulator.main".
   */
  Application();

private:
  /* #region Application internal */

  /**
   * @brief Adds the main window and dialog windows to application and initializes them.
   *
   * Internal callback to be activated when Application::activate gets called.
   */
  void on_activate();

  /* #endregion */
  /* #region Dialog Handlers */

  /**
   * @brief Shows dialog for choosing to restart with new field dimensions, undo the last move or close the game.
   *
   * @note Only if parameter `type` is @ref `NewGameDialog::Type::LOSE`, undo is an available option. Otherwise it will be grayed out.
   *
   * @param type what text to display in the dialog
   * @param o_rows output parameter for new field rows
   * @param o_cols output parameter for new field columns
   *
   * @returns wether to restart, undo or quit
   */
  NewGameDialog::ReturnType showNewGame(
    NewGameDialog::Type type,
    index_t &o_rows,
    index_t &o_cols
  );

  /**
   * @brief Shows dialog asking the user if they want help.
   *
   * @returns wether to help, not help or restart instead
   */
  NoMovesLeftDialog::ReturnType showNoMovesLeft();

  /* #endregion */
  /* #region Mouse Callbacks */

  /**
   * @brief General callback for a mouse click, internally calling the reveal or flag callback.
   *
   * @param is_reveal switch between resulting callbacks
   * @param row the row of the clicked tile
   * @param col the column of the clicked tile
   */
  void clickedCallback(
    bool is_reveal,
    index_t row,
    index_t col
  );

  /**
   * @brief Attempts to reveal the selected field and possibly ensuing cascade and checks the game status
   *
   * @param row the row of the clicked tile
   * @param col the column of the clicked tile
   */
  void revealCallback(
    index_t row,
    index_t col
  );

  /**
   * @brief Attempts to flag the selected field and checks the game status.
   *
   * @param row the row of the clicked tile
   * @param col the column of the clicked tile
   */
  void flagCallback(
    index_t row,
    index_t col
  );

  /* #endregion */
  /* #region Game Control */

  /**
   * @brief Initial callback for the restart button upon game startup.
   *
   * Will run the @ref `NewGameDialog` with @ref `NewGameDialog::Type::START`.
   */
  void startGame();

  /**
   * @brief Acquire new game mode and handle it.
   *
   * @param type the reason for the new game
   *
   * @returns wether the user backpedals and requested an undo or not
   */
  bool newGame(NewGameDialog::Type type);

  /**
   * @brief Check and handle if the game if won or no more moves are available.
   */
  void checkGameStatus();

  /* #endregion */

private:
  MW_DECLARE_LOGGER;

  std::unique_ptr<Window> window = nullptr;
  std::unique_ptr<Minefield> minefield = nullptr;

  std::shared_ptr<NewGameDialog> new_game_dialog = nullptr;
  std::shared_ptr<NoMovesLeftDialog> no_moves_left_dialog = nullptr;

  field_size_t current_field_size;
};