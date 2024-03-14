Command-line options
====================

The awm binary respects standard Unix-style command-line arguments. This page documents those available.

+------------+------------------------------------------------------------------+
| Option     | Description                                                      |
+============+==================================================================+
| -h         | Print a help message and stop                                    |
+------------+------------------------------------------------------------------+
| -V         | Print the current version of Awm and stop                        |
+------------+------------------------------------------------------------------+
|                                                                               |
+------------+------------------------------------------------------------------+
| -p <path>  | Search the specified base config path (i.e. the awm folder).     |
|            | This overrides the :doc:`standard locations <configuration>`.    |
+------------+------------------------------------------------------------------+
|                                                                               |
+------------+------------------------------------------------------------------+
| -R         | Force older RandR <=1.4 functions if applicable (as opposed to   |
|            | using RandR 1.5 if available)                                    |
+------------+------------------------------------------------------------------+
| -X         | Force very old Xinerama API instead of RandR (not recommended    |
|            | due to missing features, but may be required in some setups)     |
+------------+------------------------------------------------------------------+
