#include "no_moves_left_dialog.hpp"

/**
 * TODO:
 *  - add to cmake
 *  - check if used in application or window
 *  - actually implement this
 *  - implement logic function behind this:
 *   1. generate list of all unrevealed tiles adjacent to revealed ones
 *   2. iterate over those and check if revealing those would solve the problem
 *   3. if none of those solve the problem, reveal any of them, remove it from list and repeat from 2.
*/