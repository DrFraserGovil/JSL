.. matrix

##############
Matrix
##############


Matrix Object
******************

.. doxygenclass:: JSL::Matrix

Operator Overloads
**********************

Equality 
------------

.. doxygenfunction:: operator==(const Matrix & lhs, const Matrix & rhs)

.. doxygenfunction:: operator!=(const Matrix & lhs, const Matrix & rhs)

.. doxygenfunction:: MatrixSizesEqual(const Matrix & m1, const Matrix & m2)

Addition/Subtraction
--------------------------

.. doxygenfunction:: operator+(const Matrix & lhs, const Matrix & rhs)

.. doxygenfunction:: operator+(const Matrix & lhs, const double & scalar)

.. doxygenfunction:: operator+(const double & scalar, const Matrix & rhs)

.. doxygenfunction:: operator-(const Matrix & lhs, const Matrix & rhs)

.. doxygenfunction:: operator-(const Matrix & lhs, const double & scalar) 

.. doxygenfunction:: operator-(const double & scalar, const Matrix & rhs)


Scalar Multiplication/Division
------------------------------------

.. doxygenfunction:: operator*(const Matrix & lhs, const double & scalar)

.. doxygenfunction:: operator*(const double & scalar, const Matrix & rhs)

.. doxygenfunction:: operator/(const Matrix & lhs, const double & scalar)


Matrix and Vector Products
--------------------------------------

.. doxygenfunction:: operator*(const Matrix & lhs, const Matrix & rhs)

.. doxygenfunction:: operator*(const Matrix & lhs, const Vector & rhs)


Streaming
-------------
		
.. doxygenfunction:: operator<<(std::ostream& os, const Matrix & obj) 

