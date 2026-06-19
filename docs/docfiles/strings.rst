.. _strings:

String Handlers
============================

Strings can be famously tricksy to work with: there's a dozen different interfaces which all use slightly different variations on: some use ``char*``, other use ``string_view``, others implicitly cast to string via ``operator std::string``, whilst others use ``to_chars``. 

This is made even more delightful as the support from some functions is compiler dependent (`as of 2026`_, ``from_chars`` is still not supported for non-integral types by the clang and apple-clang compilers).


.. _as of 2026: https://en.cppreference.com/cpp/compiler_support/17#C.2B.2B17_library_features
 
The aim of this module is to provide a consistent interface for string manipulation, parsing and generation.


.. toctree::
    :hidden:

    strings/cases
    strings/folding
    strings/manipulate
    strings/makestring
    strings/parseto
    strings/join
    strings/null
    

.. list-table::
    :header-rows: 1
    :widths: 20,80
    :class: no-wrap

    * - Submodule
      - Contents

    * - :ref:`Cases.h <stringcase>` 
      - Provides case-independent equality checking and case-conversion functions.
    
    * - :ref:`Stitch.h <joinstring>` 
      - Provide functions to join together arbitrary containers of objects into a serialised string
         
    * - :ref:`MakeFrom.h <makestring>`
      - Provide a consistent interface to construct strings from a variety of types 

    * - :ref:``

    * - :ref:`Wrap.h <textwrap>` 
      - Provides a function to compute the 'true size' of a string, and 'fold' strings to fit a certain character length 
         