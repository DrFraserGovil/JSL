.. vectors

######################
Vectors
######################

"Vector" is a funny word. It means a lot of things, to a lot of people. The JSL::Vector object, however, is unambiguously a vector on :math:`\mathbb{R}^n`, the real, Euclidean vector space of dimension :math:`n`. That means that it always contains objects of type ``double`` (though ``int`` castings in and out are more than welcome). 

.. doxygenclass:: JSL::Vector

***********************
Overloaded Operators
***********************

The advantage of a custom vector class is that you can unambiguously overload some common operators, so that the vectors act as you suspect that they should under common understanding. 

Addition
-------------

Addition and subtraction of vectors are defined exactly as one might expect:

.. math::
	\left(\vec{a} \pm \vec{b}\right)_i = a_i \pm b_i


.. doxygenfunction:: JSL::operator+(const Vector &lhs, const Vector &rhs)

.. doxygenfunction:: JSL::operator-(const Vector &lhs, const Vector &rhs)

We also define the addition/subtraction of scalars from vectors, in which case we assume the following syntax:

.. math::
	\left( x \pm \vec{b} \right)_i =  x \pm b_i
	
.. doxygenfunction:: JSL::operator+(const Vector &lhs, const double &scalar)

.. doxygenfunction:: JSL::operator+(const double &scalar, const Vector &rhs)

.. doxygenfunction:: JSL::operator-(const Vector &lhs, const double & scalar)

.. doxygenfunction:: JSL::operator-(const double &scalar, const Vector &rhs)


Multiplication
----------------------

In accordance with the definition of a Vector Space, we define scalar multiplication as:

.. math::
	\left( \alpha \vec{a} \right)_i = \left( \vec{a} \alpha \right)_i= \alpha a_i
	
.. doxygenfunction:: JSL::operator*(const double &scalar, const Vector &rhs)


.. doxygenfunction:: JSL::operator*(const Vector &lhs, const double &scalar)


.. doxygenfunction:: JSL::operator/(const Vector &lhs, const double &scalar)


We also define the following vector multiplication-adjacent operations.

.. doxygenfunction:: JSL::Hadamard(const Vector &lhs, const Vector &rhs)

.. doxygenfunction:: JSL::VectorDotProduct(const Vector &lhs, const Vector &rhs)

.. doxygenfunction:: JSL::VectorCrossProduct(const Vector &lhs, const Vector &rhs)

.. doxygenfunction:: JSL::AngleBetweenVectors(const Vector &lhs, const Vector &rhs)

Misc.
----------

.. doxygenfunction:: JSL::operator<<(std::ostream& os, const Vector & obj)

.. doxygenfunction:: JSL::ElementWiseOperation(Vector input, double (*function)(double))