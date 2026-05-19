.. _nullstring:

The Null String
=====================

For both the :ref:`makestring` and :ref:`parseto` modules, there are objects (``std::optional`` and smart pointers) which have the potential to not hold their associated value, and instead be in an uninitialised or invalid state. 

In these cases, we serialise and parse the output via the preprocessor directive ``JSL_NULL_STRING``. This has the default value ``"-none-"``, but can be manually defined by the user by defining this quantity prior to the inclusion of the JSL library. 

.. doxygendefine:: JSL_NULL_STRING
