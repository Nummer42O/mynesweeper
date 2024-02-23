#pragma once

#include "window.hpp"
#include "new_game_dialog.hpp"
#include "no_moves_left_dialog.hpp"
#include "logic.hpp"
#include "debug.hpp"

#include <memory>


class Application : public Gtk::Application
{
private:
  enum class NewGameReturnType
  {
    RESTART, QUIT, UNDO
  };

public:
  /**
  * Instanciate Gtk::Application to "image_manipulator.main".
  */
  Application();

private:
  MW_DECLARE_LOGGER;

  /**
   * Callback to be activated when Application::activate gets called.
   * This adds the main window and initializes it.
   *
   * (internal)
   */
  void on_activate();

  void clickedCallback(
    bool is_reveal,
    size_t row,
    size_t col
  );

  void revealCallback(
    size_t row,
    size_t col
  );

  void flagCallback(
    size_t row,
    size_t col
  );

  void restartButtonCallbackInitial();
  void restartButtonCallback();

  /**
   * Shows dialog to make new field.
   *
   * @param type: what text to display in the dialog
   * @param o_rows: output parameter for new field rows
   * @param o_cols: output parameter for new field columns
   *
   * @returns wether to restart or quit, if @ref `type` is `NewGameDialog::Type::LOSE` then undo is also an option
   */
  NewGameReturnType showNewGame(
    NewGameDialog::Type type,
    size_t &o_rows,
    size_t &o_cols
  );

  bool newGame(NewGameDialog::Type type);

private:
  std::unique_ptr<Window> window = nullptr;
  std::unique_ptr<Minefield> minefield = nullptr;
  std::shared_ptr<NewGameDialog> new_game_dialog = nullptr;

  field_size_t current_field_size;
};