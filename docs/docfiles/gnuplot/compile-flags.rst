.. compile-flags

########################
gnuplot Compiler Flags
########################


The JSL::gnuplot module uses a compiler flag to alter the behaviour of the gnuplot machine::

	#define GNUPLOT_NO_TIDY

If this command is declared before any part of JSL is included, it disables the function CleanupTempFiles(), and leaves the corresponding gnuplot_tmp directory when the code completes. This is done either for the reproducability of a plot, for modifications outside the scope of what the JSL::gnuplot allows, or simply to check for bugs, errors or other causes of unexpected behaviour.  
