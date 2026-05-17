.. _joinstring:

String Stitching
=====================

These functions use the :ref:`makestring` algorithms to provide a generic interface to stitch together vectors of any input type

.. jsl-meta::
	:file: Strings/Stitch.h
	:namespace: JSL/Strings

.. doxygenfunction:: stitch(const container & vec, size_t begin, size_t end, std::string_view delim)
.. doxygenfunction:: stitch(const container & vec, std::string_view delim)