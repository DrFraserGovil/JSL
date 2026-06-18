.. _vault-internal:

Vault Internals
====================

The following are some helper classes and compile-time magic numbers which are required to define the Vault's internal functioning


Vault Variables
----------------------

.. doxygenvariable:: JSL::IO::internal::BLOCK_SIZE
.. doxygenvariable:: JSL::IO::internal::ARCHIVE_END_BLOCK_COUNTS
.. doxygenvariable:: JSL::IO::internal::END_OF_ARCHIVE
.. doxygenvariable:: JSL::IO::internal::zeroBuffer

    

Helper Classes 
------------------

These are Plain Old Data structs for stroring the structured data we use for the Vault:

.. doxygenstruct:: JSL::IO::Security

.. doxygenstruct:: JSL::IO::internal::TarHeader
