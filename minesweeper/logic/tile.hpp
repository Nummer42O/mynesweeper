#pragma once

#include "debug.hpp"
#include "defines.hpp"


namespace logic {
namespace tile {

class Tile
{
public:
  typedef struct
  {
    index_t row, col;
    int type;

    static const int \
      TYPE_EMPTY = 0,
      TYPE_MINE  = -1;
  } tile_export_t;
  friend std::ostream &operator<<(std::ostream &stream, const tile_export_t &tile);

  typedef struct
  {
    index_t row, col;
  } tile_position_t;
  friend std::ostream &operator<<(std::ostream &stream, const tile_position_t &tile_pos);
  friend inline bool operator==(const tile_position_t &left, const tile_position_t &right);
  friend inline bool operator<(const tile_position_t &left, const tile_position_t &right);

  typedef struct
  {
    index_t rows, cols;
  } tile_offset_t;
  friend std::ostream &operator<<(std::ostream &stream, const tile_offset_t &tile_offset);

public:
  friend inline std::ostream &operator<<(std::ostream &stream, const Tile &tile);
  inline tile_export_t exportTileAt(const tile_position_t &tile_position);

private:
  // semi constant
  bool is_mine = false;
  uint8_t nr_surrounding_mines = 0u;

  // toggleable
  bool \
    is_flagged  = false,
    is_revealed = false;

  // runtime
  uint8_t \
    nr_surrounding_flags      = 0u,
    nr_surrounding_untouched  = 8u;
};

}
}