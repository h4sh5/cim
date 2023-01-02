# cim

ncurses vim-like editor 

this is a mostly toy text editor project made to learn ncurses; don't expect it to work well.

it can kinda edit and output text. Built with laziness and hackability in mind; should be easy to hack on and serve as an educational example of how to implement a text editor in C and ncurses

need to install ncurses library to compile it (on ubuntu/debian, `apt install libncurses-dev`; on fedora, `dnf install ncurses-devel`; on alpine, `pkg add curses-dev curses-libs`

## usage

arrow key bindings; w to save and q to exit (no `:` command mode yet)

```
make
./cim
```


## TODO

- [ ] implement proper paging / scrolling 
- [ ] fix cursor movements when scrolling