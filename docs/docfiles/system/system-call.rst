.. system-call

################
System Call
################

The basis of any good system interface is the ability to actualy make such calls. Anoyingly, the C++ system interface has some odd requirements (C strings only, for example), this function overrides some of those requirements and removes a lot of boilerplate code.

.. doxygenfunction:: JSL::systemCall