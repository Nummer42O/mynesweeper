# Mynesweeper

It's just another little mineweeper clone by a bored student. <br>
Like the description says, it:
- is probably not perfect.
- will maybe never be finished.
- is made for fun.

Fell absolutely free to post Issues and PRs, but no guarantees.

## TODO:

- [ ] replace field iterations with generator
- [ ] use output operators
- [ ] restructuring of logic
- [ ] add "auto" mode just for the sake of it
- [ ] add "no more moves" warning -> reveal certain fields on random to push game forward
- [ ] add easy, medium, hard quickselect buttons to new game dialog
- [ ] add additional entry for new size
- [X] check if it might be better to use new game dialog from application
- [ ] save preferences (window size, window mode, window location, last field size, etc)
  <!-- ref: https://developer-old.gnome.org/glibmm/stable/classGio_1_1Settings.html -->
- [ ] make .deb package
- [ ] keyboard callbacks
- ~~[ ] restart button should initially be called start~~
- [ ] start new game dialog automatically
- [X] change new game dialog "Col: " to "Column: " and maybe adopt formatting/arrangement
- [X] check performance of `Minefield::activateField` as member function pointer vs if statement
- [X] make tiles hold their own copies of flagged and normal the sprite pointers
- [X] show flag icon with mine counter
- [X] redo intiial reveal callback to generate left out pattern instead of randomly trying
- [ ] recheck initial reveal callback because of inconsistencies
- [X] constantly keep track of unrevealed tiles and "the border" so it does not get recalculated from scratch every step

## Requirements

- Boost: 1.73
- gtkmm: 3.0
- glibmm: 2.4
<!-- giomm -->
- PkgConfig: 0.29

## Installation

> **Note**: If you are unsure where to put the source files, I recommend `/usr/local/src` in GNU/Linux systems.

```bash
git clone https://github.com/Nummer42O/mynesweeper.git
mkdir mynesweeper/build
cd mynesweeper/build
cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -S .. \
  -B .
make
sudo make install
```

## Removal

```bash
cd mynesweeper/build
sudo make uninstall
```