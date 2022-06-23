.. clear-screen

=========================
Terminal Modifiers
=========================


Terminals are not just static blocks of text - almost all terminals support the use of ANSI escape codes. The problem of course is that these escape codes are both ancient and arcane, and hence difficult to use.

The functions here simply print one or more ANSI escape codes to the standard output. Note that whilst this will result in expected behaviour on almost all terminals designed after 1975, it will result in horrible gobbledegook if piped to a file. I might try to rectify this via isatty(), but that might not work particularly well.


.. doxygenfunction:: JSL::jumpLineUp

.. doxygenfunction:: JSL::clearScreen

.. doxygenfunction:: JSL::deleteLine