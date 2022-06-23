.. rm

################################
Remove Files & Directories
################################

Use with extreme care. This exposes the system command ``rm`` to the C++ interface. There are very few checks performed here, so if this goes wrong you can delete files you might not want to!

.. doxygenfunction:: rm
