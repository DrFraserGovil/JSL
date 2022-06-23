.. gnuplot

#################
gnuplot Interface
#################

Having spent lots of time working with both MATLAB and matplotlib, I have become extremely used to having the ability to plot data from within the confines of the code which generated it -- something that C++ finds lacking. 

I often found myself saving data to file, writing a custom gnuplot script, and then running that -- which was a lot of hassle when it had to be done repeatedly. 

Behind the scenes, that is all that this interface does -- writes data to file, then constructssa gnuplot plotter file, calls it, then cleans up after itself. However, it is written in such a way as to be simple to use and intuitive to those with experience with MATLAB and matplotlib.

.. toctree::
	gp-main
	gp-axis
	properties
	compile-flags
	:caption: Members:
