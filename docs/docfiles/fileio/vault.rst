.. _vault:

File Vault
===============

A common side effect of code bases is the need to save a large number of output files. This can quickly get very messy: sure, it is possible to create a directory to hide them away in, but it can also be easier to pack all these files together into a *Vault*.

A *Vault* is a TAR archive, written directly to the disk. This means that the files can be recovered not only by the VaultReader, but by an external user calling ``tar`` to unpack the results.


.. toctree::
  :maxdepth: 2
    
  vault-write
  vault-read
  vault-internal