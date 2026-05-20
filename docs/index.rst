.. JSL documentation master file, created by
   sphinx-quickstart on Thu Aug 19 10:01:35 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

The JSL Library
===============================


The JSL ('Jack Standard Library') is a set of tools developed by me (Jack Fraser-Govil) as a uniform, useful set of boilerplate code I found myself duplicating across numerous C++ projects. It is probably of no use to anyone else, but if you have found your way here, it is probably from one of the projects which utilised this library. 

Overview
--------------

The JSL library is broken up into a series of smaller modules, each of which can be included individually if more precise control is desired. 

	.. toctree::
		docfiles/style
		Async.h <docfiles/async>
		Display.h <docfiles/display>
		FileIO.h <docfiles/fileio>
		Parameters.h <docfiles/parameters>
		Strings.h <docfiles/strings>
		Time.h <docfiles/time>
		Vector.h <docfiles/vectors>
		:maxdepth: 2

	* :ref:`genindex`



Internal Dependencies
++++++++++++++++++++++

.. image:: visualiser/network.png