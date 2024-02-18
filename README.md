# <img src="res/wm_logo_colours.svg" height=40>

**A window manager**

![Lines of code](https://www.aschey.tech/tokei/github.com/kosude/awm)


A highly configurable and lightweight X (xcb) window manager for use on Linux systems.

The aim is to have a basic floating window manager (eventually with compositing support) that behaves in a way somewhat similar to Windows, with the
difference that it runs on an operating system that doesn't hate you.


## Configuration

Currently, there is one config file expected (although not required): `awm.conf`. It must have that filename, and be located in one of the following
(from highest priority to lowest):
 - `~/.local/config/awm/conf/`
 - `~/.awm/conf/`
 - `/etc/awm/conf/`

Otherwise, you can specify a base configuration folder (which must contain a subdirectory named `conf`) with the `-p` argument. Run `awm -h` for more
information.


### Plugins

awm is modular via its plugins system. Official plugins are provided, although awm can work without them either.

#### Location

Plugins are automatically located and loaded by awm at initialisation. In the future, a command-line argument as well as a standard location will be
implemented (likely to be XDG-compliant, i.e. `$HOME/.config/awm/`) to be searched for window manager configuration. For the time being, though,
the **current working directory** is used. Create a folder in the working directory called `plugins/`, and put plugins there.

#### Specification

Plugins are expected as **dynamic libraries**, compiled for the same architecture as awm. That is to say, 32-bit plugins won't work in 64-bit awm, and
vice versa.


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

#### Multihead testing
You can use a command such as `Xephyr :1 +xinerama -screen 800x600+0+0 -screen 800x600+800+0` to test multiple monitor support with Xephyr. An
important thing to note is that RandR doesn't seem to work properly in such a configuration (even if it is enabled with `+extension RANDR`) - invoke
Awm with the `-X` parameter to fix this by using Xinerama instead.
