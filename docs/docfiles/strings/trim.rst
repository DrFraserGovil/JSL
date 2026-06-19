.. _trimstring:

Trimming Strings
=======================

The ``string_view`` library is a modern extension to C++ strings, and are a great way to port around 'windows' into an underlying immutable string without needing to allocate large blocks of memory. 

However, they come with a handful of downsides:

a. A ``string_view`` is immutable: text cannot be appended and modifications are limited to slicing
b. The conversion to ``std::string`` is (by design) only explicit. This forces a user to only make copies when they actually want to allocate memory, but adds verbosity
c. Some ordered containers require additional 'transparency' options in order to lexographically sort on the underlying string

For this reason it is often useful to retain 'string versions' of functions, even when the same functionality could be achieved by using a string_view version followed by a series of casts. 

.. note::
	We use a consistent naming system, mimicking the ``string``/``string_view`` difference:

	* ``function(...)``: returns a string 			
	* ``function_view(...)``: returns a ``string_view``
 
    This is an explicit exception to the :ref:`usual style guidelines <style>` employed by the JSL
  


Trimming
----------------

.. jsl-meta::
	:file: Strings/Manipulate.h
	:namespace: JSL::String

.. doxygenfunction:: JSL::String::trim_view(std::string_view sv)

.. doxygenfunction:: JSL::String::trim(std::string_view sv)

Trimming (with Comments)
----------------------------

.. jsl-meta::
	:file: Strings/Manipulate.h
	:namespace: JSL::String

.. doxygenfunction:: JSL::String::trim_view(std::string_view sv,std::string_view commentIndicator)

.. doxygenfunction:: JSL::String::trim(std::string_view sv,std::string_view commentIndicator)
