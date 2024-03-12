User configuration
==================


Config locations
----------------

Awm configuration is expected at a few specific paths. If there are multiple configurations, priority works in the order of the list below (top
is prioritised the most).

 - ``~/.config/awm/`` *(recommended for user configs)*
 - ``~/.awm/``
 - ``/etc/awm/``

If none of these are ideal, you can override any config searching with the ``-p`` command-line argument.

.. note::
    For the rest of this page, the base path (that is, the ``awm/`` folder) is replaced with ``<base>``.


awm.conf
--------

The ``awm.conf`` file is expected in the ``<base>/conf/`` folder, and contains options for controlling Awm's behaviour via various settings. This file
is to be written in a standard `INI <https://en.wikipedia.org/wiki/INI_file>`_ format.

TODO: keys?
