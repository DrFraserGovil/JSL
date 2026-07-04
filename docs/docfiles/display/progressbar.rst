.. _progress:

Progress Bars
=====================

We provide two different means of rendering progress bars:

* :cpp:class:`Dynamic Bars <JSL::Display::Progress::Bar>`, which utilise ANSI sequences to dynamically modify the text already piped to the stream, giving the impression of an animated display.
* :cpp:class:`Static Bars <JSL::Display::Progress::Static>`, which are much more limited, but which are appropriate for non-ANSI-capable outputs (such as files) 

Both of these use the same interface, provided by the ProgressEngine parent class. 

Main Interface
-------------------
.. jsl-class:: JSL::Display::Progress::internal::ProgressEngine
	:file: Display/ProgressBar.h


Child Classes
------------------

.. dropdown:: Dynamic Bar
	:icon: pulse

	.. jsl-class:: JSL::Display::Progress::Bar
		:file: Display/ProgressBar.h

.. dropdown:: Static Bar
	:icon: stopwatch

	.. jsl-class:: JSL::Display::Progress::Static
		:file: Display/ProgressBar.h
		