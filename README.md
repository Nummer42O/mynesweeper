# Mynesweeper

It's just another little mineweeper clone by a bored student. <br>
Like the description says, it:
- is probably not perfect.
- will maybe never be finished.
- is made for fun.

Fell absolutely free to post Issues and PRs, but no guarantees.

## TODO:

- [ ] add "auto" mode just for the sake of it
- [ ] add "no more moves" warning -> reveal certain fields on random to push game forward
- [ ] add easy, medium, hard quickselect buttons to new game dialog
- [ ] add additional entry for new size
- [ ] check if it might be better to use new game dialog from application

## Requirements

- Boost: 1.73
- gtkmm: 3.0
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