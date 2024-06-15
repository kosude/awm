Guidance for contributors
=========================


.. toctree::
    :hidden:

    contributors-resources


Awm is a personal project, but contributions are welcome. This page contains documentation providing guidelines for contributions as well as any
useful resources, which can be found and downloaded from :doc:`this sub-page <contributors-resources>`.


.. _build process:

Build process
-------------

Ideal platform for development on Awm is, naturally, Linux (with X.org installed). That said, macOS is also confirmed to work pretty well. Before
anything else, ensure that libxcb is installed on the development machine.

The build process is handled with `Meson <https://mesonbuild.com/>`_:

.. code-block:: bash

    # with full optimisations
    $ meson setup $BUILD_DIR -Dbuildtype=release
    $ ninja -C $BUILD_DIR

    # with debug symbols
    # (this is the default when -Dbuildtype is not specified)
    $ meson setup $BUILD_DIR -Dbuildtype=debugoptimized
    $ ninja -C $BUILD_DIR

    # compiling this documentation
    # note the use of '-Dcore=false', which disables compilation of awm proper
    $ meson setup $BUILD_DIR -Ddocs=true -Dcore=false
    $ ninja -C $BUILD_DIR


Available project setup options are listed below. To specify them, use the ``-D`` flag, as seen in the examples above.

+----------+--------------------------------------------------------+---------------+
| Option   | Description                                            | Default value |
+==========+========================================================+===============+
| core     | Compile the Awm window manager                         | true          |
+----------+--------------------------------------------------------+---------------+
| docs     | Compile this HTML documentation                        | false         |
|          | (also requires `Sphinx <https://www.sphinx-doc.org>`_) |               |
+----------+--------------------------------------------------------+---------------+

.. // TODO: cross-compiling information?


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
