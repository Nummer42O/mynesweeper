#pragma once

#include "debug.hpp"
#include "defines.hpp"


namespace logic {
namespace structs {

typedef struct
{
  index_t row, col;

  // number (1-8), empty (0) or mine (-1)
  int type;
} tile_with_position_t;
std::ostream &operator<<(std::ostream &stream, const tile_with_position_t &tile);

typedef struct
{
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
} tile_t;
std::ostream &operator<<(std::ostream &stream, const tile_t &tile);

typedef struct
{
  index_t row, col;
} tile_position_t;
std::ostream &operator<<(std::ostream &stream, const tile_position_t &tile_pos);
inline bool operator==(const tile_position_t &left, const tile_position_t &right);
inline bool operator<(const tile_position_t &left, const tile_position_t &right);

typedef struct
{
  index_t rows, cols;
} tile_offset_t;
std::ostream &operator<<(std::ostream &stream, const tile_offset_t &tile_offset);

}
}