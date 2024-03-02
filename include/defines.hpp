#pragma once

#include <cstddef>
#include <glibmm/refptr.h>
#include <gdkmm/pixbuf.h>

/* #region graphical configs */

#define SPACING 5l

#define TILE_SIZE 50l

#define MIN_FIELD_ROWS 10l
#define MIN_FIELD_COLS 10l

/*
#define COLOR_FIELD_1 "ddfac3"
#define COLOR_FIELD_2 "dfeaba"
#define COLOR_FIELD_3 "e1dab2"
#define COLOR_FIELD_4 "e3caaa"
#define COLOR_FIELD_5 "e6baa2"
#define COLOR_FIELD_6 "e8aa9a"
#define COLOR_FIELD_7 "ea9a92"
#define COLOR_FIELD_8 "ed8a8a"
*/

/* #endregion */
/* #region general configs */

#define DEFAULT_BOMB_FACTOR .1563

#define MIN_INITIAL_FIELDS 3l

/* #endregion */
/* #region build context */

#ifndef PROGRAM_NAME
#define PROGRAM_NAME "[UNKNOWN]"
#endif // !defined(PROGRAM_NAME)

#ifndef EXECUTABLE_NAME
#error No executable name defined.
#endif // !defined(EXECUTABLE_NAME)

#ifndef SPRITE_DIRECTORY
#error No sprite directory defined.
#endif // !defined(SPRITE_DIRECTORY)

/* #endregion */
/* #region general resources */

typedef int64_t index_t;

typedef struct
{
  index_t \
    rows = 0l,
    cols = 0l;
} field_size_t;

typedef Glib::RefPtr<Gdk::Pixbuf> sprite_t;
typedef struct {
  sprite_t normal, highlighted;
} state_sprites_t;

/* #endregion */