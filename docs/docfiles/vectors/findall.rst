.. _findall:

Finding-All Functions
============================

These functions do not only find the first occurence of a result, but instead return the indices of all matching values.

.. jsl-meta::
	:file: Vector/SearchAll.h
	:namespace: JSL/Vector

.. doxygenfunction:: JSL::Vector::FindAll(const R &vec, const T &target)

.. doxygenfunction:: JSL::Vector::FindAll(const R &vec, double target, double tolerance)

.. doxygenfunction:: JSL::Vector::FindAllWhere
