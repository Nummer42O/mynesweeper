#include "tile.hpp"


namespace logic {
namespace structs {

std::ostream &operator<<(std::ostream &stream, const tile_with_position_t &tile)
{
  // number (1-8), empty (0) or mine (-1)
  switch (tile.type)
  {
    case -1:
      stream << "Mine ";
    case 0:
      stream << "Empty field ";
    default:
      stream << "Field with " << tile.type << " adjacent mines ";
  }

  stream << "@ (row=" << tile.row << "|col=" << tile.col << ')';

  return stream;
}

std::ostream &operator<<(std::ostream &stream, const tile_t &tile)
{
  stream << tile.is_mine ? "Mine(" : "Field(";
  stream << "state=" << tile.is_revealed ? "revealed" : (tile.is_flagged ? "flagged" : "untouched");
  stream \
    << " nr_adj_mines=" << tile.nr_surrounding_mines
    << " nr_adj_flags=" << tile.nr_surrounding_flags
    << " nr_adj_untouched=" << tile.nr_surrounding_untouched
    << ')';

  return stream;
}

std::ostream &operator<<(std::ostream &stream, const tile_position_t &tile_pos)
{
  stream << "Tile @ (row=" << tile_pos.row << "|col=" << tile_pos.col << ')';

  return stream;
}

inline bool operator==(const tile_position_t &left, const tile_position_t &right)
{
  return left.row == right.row && left.col == right.col;
}

inline bool operator<(const tile_position_t &left, const tile_position_t &right)
{
  if (left.row == right.row)
  {
    return left.col < right.col;
  }

  return left.row < right.row;
}

std::ostream &operator<<(std::ostream &stream, const tile_offset_t &tile_offset)
{
  stream << "Offset by (rows=" << tile_offset.rows << "|cols=" << tile_offset.cols << ')';

  return stream;
}

}
}