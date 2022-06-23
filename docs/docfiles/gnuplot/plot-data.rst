.. plot-data


#################
Plot Data
#################

A Plot Data is generated each time a new plotting function (either Plot or Scatter) is called. The relevant data is written to file, and the filelocation, along with the specifications for how the data should be displayed, are saved in the PlotData object.

JS::PlotData objects can be modified in two ways -- pre-facto and post-facto. Pre-facto specification relies on passing relevant :ref:`line properties<Plot Properties>` to the Plot or Scatter command which generated it. Alteratively, these same commands return references to the geneated JSL::PlotData object, which can then be modified via the member functions. 

.. doxygenclass:: JSL::PlotData
	:members:
	:private-members: