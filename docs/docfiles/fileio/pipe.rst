.. _pipe:

Piped Input
=============

A common use-case for applications in a shell environment is piping the output of one command into the next. Being able to capture a piped (or redirected) input therefore enables one to integrate software into a pipeline. 

Functions
+++++++++++++

.. jsl:: JSL::IO::Pipe::IsPiped
	:file: FileIO/pipedInput.h
	:command: doxygenfunction

.. doxygenfunction:: JSL::IO::Pipe::forLineIn

.. doxygenfunction:: JSL::IO::Pipe::forSplitLineIn

.. doxygenfunction:: JSL::IO::Pipe::save

Usage
++++++++++

This examples sorts through a piped input file 

.. code:: cpp

	int main(int argc,char**argv)
	{
		if (JSL::IO::Pipe::IsPiped())
		{
			LOG(INFO) << "Pipe detected";
			int i = 0;
			JSL::IO::Pipe::forLineIn([&](auto line)
			{
				if (i > 0)
				{
					auto person = JSL::String::ParseTo<std::tuple<std::string,int,double,double,double>>(line," ");
					std::string name = std::get<0>(person);
					int age = std::get<1>(person);
					double height = std::get<2>(person);
					if (age > 40)
					{
						LOG(INFO) <<std::setw(10) << name <<"\t" << height<< "cm\n" ;
					}
				}
				++i;
			});
		}
		else
		{
			LOG(INFO) << "No pipe detected";
		}
	}

We save the following data as ``data.txt``:


.. include:: ../data.txt
	:literal:

Giving output (we use a file redirect here, a ``cat data.txt | ./jsltest`` would give the same output)

.. code-block:: shell-session

	>> ./jsltest < data.txt
	[INFO]  Pipe detected
	[INFO]        Dave  175.3cm                                                                                      
	[INFO]       Henry  178.2cm                                                                                      
	[INFO]       Karen  166.7cm                                                                                      
	[INFO]        Nick  176.8cm                                                                                      
	[INFO]      Rachel  167.2cm                                                                                      
	[INFO]         Tom  174.1cm
	>> ./jsltest
	[INFO] No pipe detected