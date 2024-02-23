#pragma once

#include "window.hpp"
#include "no_moves_left_dialog.hpp"
#include "logic.hpp"
#include "debug.hpp"

#include <memory>


class Application : public Gtk::Application
{
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

  bool newGame(NewGameDialog::Type type);

private:
    std::unique_ptr<Window> window = nullptr;
    std::unique_ptr<Minefield> minefield = nullptr;

    struct
    {
      size_t rows, cols;
    } current_field_size;
};