.. _forlinein:

Per-Line File Processing
==========================


Often, a file is being read in not purely for storage, but because some form of operation is being performed on it. In such a case it is wasteful to store the entire string, and then postprocess it. The ``forLineIn`` routine and its descendents provide a way to operate on a file on a line-by-line basis. 

.. doxygenfunction:: JSL::IO::forLineIn(const std::filesystem::path fileName, std::function<void(std::string_view)> lineProcessor)

.. doxygenfunction:: JSL::IO::forSplitLineIn

Early Stopping
++++++++++++++++

Without breaking the usual program flow (such as with throwing errors), the forLineIn routine is guaranteed to visit every line in the file - even if the macro attempts to return. 

We provide an interface which allows the processor to indicate an early return is desirable:
 
.. doxygenfunction:: JSL::IO::forLineIn(const std::filesystem::path fileName, std::function<void(std::string_view)> lineProcessor, bool&earlyStop)

This holds a reference to a boolean flag: if it is ever `true`, then the file is closed without scanning further. 

.. code-block:: cpp

   // DOES NOT WORK
   std::sting file = "1000-line-file-on-disk";
   int i = 0;
   JSL::IO::forLineIn(file,[](auto line)
   {
        i +=1;
        if (i > 10) return;     // a well meaning but flawed attempt to return
   });
   std::cout << i << std::endl; // outputs 1000
    
   // DOES WORK
   i = 0;
   bool earlyReturn = false;
   JSL::IO::forLineIn(file,[&](auto line, earlyReturn) //have to capture by reference
   {
        i +=1;
        earlyReturn = (i > 10);  // set the flag within the macro
   });
   std::cout << i << std::endl; // outputs 11

Direct Conversion
++++++++++++++++++++++

The file-read-in process is directly integrated with the :ref:`Parsing Library <parseto>`, allowing direct conversion of input data into a typed instance.

The first two overloads convert the entire line into the provided type, whilst the third overload allows arbitrary variadic types, and hence can convert the input file into a table.

.. doxygenfunction:: JSL::IO::forConvertedLineIn(const std::filesystem::path fileName, std::function<void(T)> callback)
.. doxygenfunction:: JSL::IO::forConvertedLineIn(const std::filesystem::path fileName, std::string_view delimiter, std::function<void(T)> callback)
.. doxygenfunction:: JSL::IO::forConvertedLineIn(const std::filesystem::path fileName, std::string_view delimiter, Func callback)
