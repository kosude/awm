# <img src="res/wm_logo_colours.svg" height=40>

**A window manager**

![Lines of code](https://www.aschey.tech/tokei/github.com/kosude/awm)


A lightweight X (xcb) window manager for use on Linux systems.

The aim is to have a basic floating window manager (eventually with compositing support) that behaves in a way somewhat similar to Windows, with the
difference that it runs on an operating system that doesn't hate you.


## Development

Awm can be compiled to either 32-bit or 64-bit using the appropriate CMake toolchain (in [`cmake/tc/`](cmake/tc/)).

The ideal platform for development on Awm is, naturally, Linux (with X.org installed). That said, macOS (at least M1) is also confirmed to work
decently well. In any case, **libxcb** must be installed.

### Testing

The recommended method for testing Awm is via [Xephyr](https://wiki.archlinux.org/title/Xephyr). This is available from most package managers on
Linux. It is possible to get this set up on macOS despite X not being available by default, by installing the `xquartz` package from
[Homebrew](https://formulae.brew.sh/cask/xquartz); make sure it's allowed to run in the background and re-log after installation to start the
XQuartz session. A `Xephyr` binary is provided by this package.

A convenience script is provided at [`tools/run_xephyr.sh`](tools/run_xephyr.sh). This script runs an instance of Awm, as well as xterm, in a Xephyr
session. Valgrind is enabled by default here, but it can be disabled with the `--none` argument.
