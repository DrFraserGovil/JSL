.. display

#################
Array Functions
#################

Array Functions is the name given to some various support functions defined on the standard std::vector<T> type -- I have termed them "Array Functions" to prevent name collisions with the JSL::Vector objects.


Locator Functions
**************************

.. doxygenfunction:: JSL::FindXInY(T x, const std::vector<T> & y)

.. doxygenfunction:: JSL::FindXInY(double x, const std::vector<double> & y, double tolerance)

.. doxygenfunction:: JSL::UpperBoundLocator

Sort Functions
********************

.. doxygenfunction:: SortIndices