# <img src="res/wm_logo_colours.svg" height=40>

**A window manager**

![Lines of code](https://www.aschey.tech/tokei/github.com/kosude/awm)


A lightweight X (xcb) window manager for use on Linux systems.

The aim is to have a basic floating window manager (eventually with compositing support) that behaves in a way somewhat similar to Windows, with the
difference that it runs on an operating system that doesn't hate you.


## Documentation

I would like to base a lot of this project on user configuration and customisability and, once more of that is implemented, I'll set up a proper
place for documentation. Since awm is still in its early stages of development, though, you can find any important documentation in this README for
the time being.


### Plugins

awm is modular via its plugins system. Official plugins are provided, although awm can work without them either.

#### Location

Plugins are automatically located and loaded by awm at initialisation. In the future, a command-line argument as well as a standard location will be
implemented (likely to be XDG-compliant, i.e. `$HOME/.config/awm/`) to be searched for window manager configuration. For the time being, though,
the **current working directory** is used. Create a folder in the working directory called `plugins/`, and put plugins there.

#### Specification

Plugins are expected as **dynamic libraries**, compiled for the same architecture as awm.
