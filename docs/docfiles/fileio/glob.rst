.. _glob:

Globbing
===========

Users of unix-like systems will be familiar with using ``glob`` syntax to match filenames. We provide a means to convert glob strings into their regex-equivalents. 

This is more or less ends up being a cross-platform implementation of ``fnmatch``, which is part of UNIX-like distributions, but not windows.

The Syntax
----------------

We attempt to adhere as close to the `glob specification used in GNU/Linux <https://man7.org/linux/man-pages/man7/glob.7.html>`_ as possible, whilst extending it to expose the possibility of using the internal regular expressions. 

Control Characters
+++++++++++++++++++

Except whilst inside groups or escaped, the characters ``*`` and ``?`` are replaced by ``.*`` and ``.`` respectively, meaning they match arbitrary string sequences (``*``) and arbitrary single characters (``?``)

Regex control characters (``.+^$(){}\``) are automatically escaped; so ``|`` is replaced with the literal ``\|``, causing it to match with the literal ``|``. 

This escaping can be prevented by escaping within the glob string: ``\|`` in the glob string is converted to ``|`` in the regular expression. Any character following a backslash is inserted as a literal. 

Groups
++++++

Groups are denoted by brackets ``[...]``, and behave largely the same in regex and glob syntaxes, with the following exceptions:
* Replacement of ``*`` and ``?`` is disabled (these instead match their literal characters)
* If the first character in the group is ``!`` it acts as a negator to the entire group (replaced with ``^`` in the regex)
* If the first character in the group is ``]``, it is treated as a literal, without needing to escape. 

The Functions
---------------

.. jsl:: JSL::IO::makeGlob
	:command: doxygenfunction
	:file: FileIO/glob.h

.. doxygenfunction:: JSL::IO::globToRegex

Example Usage
---------------------

.. code-block:: cpp

	void testGlob(std::vector<std::string> & files, std::regex pattern) {
	
		for(auto & f : files)
		{
			if (std::regex_match(f,pattern))
			{
				std::cout << "\t-" << f << std::endl;
			}
		}
	}

	int main(int argc,char**argv)
	{
		std::vector<std::string> files = {"a.cpp", "b.h", "c.txt", "dir/subdir/d.cpp","dir/subdir/e.txt","dir/subdir/e.tst","dir/subdir/f.t?t"};

		std::string glob1 = "*.cpp";
		std::string converted = JSL::IO::makeGlob(glob1);

		std::cout << "Matching " << glob1 << " (" << converted << ")" <<std::endl;
		testGlob(files,std::regex(converted));

		std::string glob2 = "*/*.t[!x?]t";
		std::cout << "Matching " << glob2 << " (" << JSL::IO::makeGlob(glob2) << "):" << std::endl;
		testGlob(files,JSL::IO::globToRegex(glob2));
		
		//escaping allows regex to be mixed in with globbing:
		std::string glob3 = R"(*/\(d\.\*\|e*\))";
		std::cout << "Matching " << glob3 << " (converted to: " << JSL::IO::makeGlob(glob3) <<")" << std::endl;
		testGlob(files,JSL::IO::globToRegex(glob3));

	}

Output:

.. code-block:: shell-session

	Matching *.cpp (converted to ^.*\.cpp$)
		-a.cpp
		-dir/subdir/d.cpp
	Matching */*.t[!x?]t (converted to ^.*/.*\.t[^x?]t$):
		-dir/subdir/e.tst
	Matching */\(d\.\*\|e*\) (converted to: ^.*/(d.*|e.*)$)
		-dir/subdir/d.cpp
		-dir/subdir/e.txt
		-dir/subdir/e.tst