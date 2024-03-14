Guidance for contributors
=========================


.. toctree::
    :hidden:

    contributors-resources


Awm is a personal project, but contributions are welcome. This page contains documentation providing guidelines for contributions as well as any
useful resources, which can be found and downloaded from :doc:`this sub-page <contributors-resources>`.


Building
--------

Ideal platform for development on Awm is, naturally, Linux (with X.org installed). That said, macOS is also confirmed to work pretty well. Make sure
libxcb is installed before compilation!

The build process is handled with `CMake <https://cmake.org>`_ - use the ``cmake . -Bbuild/`` command to configure, and ``cmake --build build/`` to
compile. There are a few available options:

+----------+----------------------------------------------------+---------------+
| Option   | Description                                        | Default value |
+==========+====================================================+===============+
| AWM_WM   | Compile the Awm window manager                     | ON            |
+----------+----------------------------------------------------+---------------+
| AWM_DOCS | Compile this HTML documentation                    | OFF           |
|          | (requires Sphinx)                                  |               |
+----------+----------------------------------------------------+---------------+


Toolchains
^^^^^^^^^^

CMake toolchains can be found in the ``cmake/tc/`` directory, and used with the ``-DCMAKE_TOOLCHAIN_FILE=<file.cmake>`` command-line option. This way,
Awm can be cross-compiled to 32- or 64-bit architectures, or with different compilers.


Integration testing
-------------------

It can be a little more involved to test a window manager than a standard user application. For that reason, some advice is given here.

The recommended method for testing Awm is via `Xephyr <https://wiki.archlinux.org/title/Xephyr>`_. This is available from most package managers on
Linux. It is possible to get this set up on macOS despite X not being available by default, by installing the
`xquartz <https://formulae.brew.sh/cask/xquartz>`_ package with Homebrew; make sure it's allowed to run in the background and re-log after
installation to start the XQuartz session. A ``Xephyr`` binary is provided by this package.

A convenience script is provided throguh the ``tools/run_xephyr.sh`` file. This script runs an instance of Awm, as well as xterm, in a Xephyr session.
Valgrind is enabled by default here, but it can be disabled with the ``--none`` argument.


Multihead testing
^^^^^^^^^^^^^^^^^

You can use a command such as ``Xephyr :1 +xinerama -screen 800x600+0+0 -screen 800x600+800+0`` to test multiple monitor support with Xephyr. An
important thing to note is that RandR doesn't seem to work properly in such a configuration (even if it is enabled with ``+extension RANDR``) - invoke
Awm with the ``-X`` parameter to fix this by using Xinerama instead.
