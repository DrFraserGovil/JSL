.. _vault-write:

Writing to a Vault
------------------------
 
The VaultWriter Streams
=========================

The VaultWriter functions essentially as a ``map`` of streams to individual files within the vault. The majority of these files are *buffered*, the data is stored in-memory until the vault closes. This is because the vault format requires that the full size of the data be known, and written into the relevant header block. 

Since this may lead to an excess memory burden, we also implement

.. dropdown:: The Stream Interface
   :color: secondary
   :icon: code 
    
	.. doxygenclass:: JSL::IO::VaultWriter::Stream 
	
.. dropdown:: The Buffered Stream
	:color: primary
	:icon: ellipsis 
	 
	.. doxygenclass:: JSL::IO::VaultWriter::BufferedStream
	 
.. dropdown:: The Direct Stream
	:color: primary
	:icon: file 
	 
	.. doxygenclass:: JSL::IO::VaultWriter::DirectStream
	 
The VaultWriter Class
=========================

.. .. jsl-class:: JSL::IO::VaultWriter
.. jsl-meta::
	:file: IO/Vault/VaultWriter.h
	:namespace: JSL::IO

.. doxygenclass:: JSL::IO::VaultWriter
	:members: VaultWriter,~VaultWriter, OpenVault, NewFile,Close, operator[], operator<<, Settings, SetPolicy

.. dropdown:: Internal Implementation
	:color: secondary
	:icon: gear

	.. doxygenclass:: JSL::IO::VaultWriter
		:no-link:
		:members-only:
		:private-members:
		:members: False	
	
