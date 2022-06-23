.. progress-bar

####################
Progress Bar
####################


When running long computations, it is useful to have a visual way of identifying the progress of the code, without relying heavily on vast streams of data. The ProgressBar provides a simple, intuitive way of tracking the status of code, with support for tracking multiple process simultaneously. 

Class Definition
**********************

.. doxygenclass:: JSL::ProgressBar
	:private-members:
	:protected-members:


Example Usage
******************

Simple Case
---------------

In the simplest case, we can omit the template arguments, and run the progress bar in the following way:

.. code-block:: c++
	
	#include <iostream>
	#include "JSL.h"

	void timeWastingFunction(int i)
	{
		// code that takes some time to complete.....
	}

   	int main(int argc, char * argv[])
   	{
		int N = 100;
		JSL::ProgressBar pb(N);

		for (int i = 0; i < N; ++i)
		{
			timeWastingFunction(i);
			pb.Update(i);
		}
		return 0;
   	}

This would output the following bar, which overwrites itself as i advances::

	[                           ]  //at i = 0
	[##############             ]  //at i = 50
	[###########################] // at code end


Multi-Tiered Case
----------------------


In the case of multiple (potentially nested) processes which must be completed, we can add additional bars:

.. code-block:: c++
	
	#include <iostream>
	#include "JSL.h"

	void tieredTimeWastingFunction(int i, int j)
	{
		// code that takes some time to complete.....
	}

   	int main(int argc, char * argv[])
   	{
		int N = 10;
		int N2 = 1000;
		JSL::ProgressBar<2,False,'@'> pb(N,N2);
		pb.SetNames({"First Loop", "Second Loop"});
		for (int i = 0; i < N; ++i)
		{
			for (int j = 0; j < N2; ++j)
			{
				tieredTimeWastingFunction(i,j);
				pb.Update(i,j);
			}
		}
		return 0;
   	}

At i=j=0 this produces::

	First Loop  [                     ]
	Second Loop [                     ]

As the inner loop progresses::

	First Loop  [                     ]
	Second Loop [@@@@@@@@@            ]

When the inner loop completes, the bar resets itself, and the outer loop updates:

As the inner loop progresses::

	First Loop  [@@@@                 ]
	Second Loop [                     ]

No Delete Case
----------------------

In the case that, for example, the output of the code is being piped to file, printing ANSI escape codes leads to nonsensical output. In this case, the No-Delete mode is a much safer (if more limited) way to run.

.. code-block:: c++
	
	#include <iostream>
	#include "JSL.h"

	void timeWastingFunction(int i)
	{
		// code that takes some time to complete.....
	}

   	int main(int argc, char * argv[])
   	{
		int N = 100;
		JSL::ProgressBar<1,False> pb(N);

		for (int i = 0; i < N; ++i)
		{
			timeWastingFunction(i);
			pb.Update(i);
		}
		return 0;
   	}

In this case the output appears very similar to the Simple Case above, but note that the output is printed *sequentially*, rather than being deleted and then rewritten::

	[                             //at i = 0
	[##############               //at i = 50
	[###########################] // at code end