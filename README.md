# fftools

Manipulate Call of Duty fastfile files.

## Description

This project is for working with Call of Duty: World at War fastfiles on linux.
The current goal is to support unpacking existing ff files, packing new ff
files, and listing the contents of ff files. In the future I'd like to add the
ability to edit script files interactively using the default terminal editor.
Far off goals are to add support for editing map files.

### Dependencies

* zlib
* make

### Installing

Install dependencies
```
sudo pacman -S zlib make
```

Build
```
$ git clone https://github.com/devon-g/fftools.git
$ make
```

### Executing

Run
```
$ ./build/fftools --help
```

## Acknowledgements

* [readme](https://gist.github.com/PurpleBooth/109311bb0361f32d87a2/)
* [makefile](https://spin.atomicobject.com/makefile-c-projects/)
* [zlib reference code](https://zlib.net/zpipe.c)
