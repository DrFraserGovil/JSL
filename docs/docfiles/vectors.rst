.. _vectors:

Vectors
==================

In the context of this module, when 'vectors' is used, we generally mean functions which extend (or in some cases, merely simplify) the behaviour of `STL containers <https://en.cppreference.com/cpp/container>`_ - not mathematical vectors. 

These functions can be broken down into three groups:

.. toctree::
   :hidden:

   vectors/join    
   vectors/linspace
   vectors/search
   vectors/cast

.. list-table::
    :header-rows: 1
    :widths: 20,80
    :class: no-wrap

    * - Submodule
      - Contents

    * - :ref:`Cast.h<vectorcast>`
      - Provide a simple way to convert a ``container<T>`` to ``container<S>``, where ``S`` and ``T`` are easily convertible.
           
    * - :ref:`Join.h<vectorjoin>`
      - Provides methods of joining and concatenating STL containers
         
    * - :ref:`Linspace.h<linspace>`
      - Provides a simple interface to generate an evenly spaced array of values 
         
    * - :ref:`Search.h<searcher>`
      - Provide a clean interface for finding and locating values in an STL container

         
       
