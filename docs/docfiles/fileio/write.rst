.. _file-write:

Writing To File
====================

Streaming to File
++++++++++++++++++++++++

The standard way to write to file in modern C++ is to use an ``ofstream``, and stream the content into the file. We provide a basic wrapper around this:

.. jsl:: JSL::IO::openStream
    :file: FileIO/FileWriters.h
    :command: doxygenfunction

Subsequent data can then be streamed as normal:

.. code-block:: cpp

    auto f = JSL::IO::openStream("file.txt");
    //equivalent to calling f =  std::ofstream("file.txt"); if (!f.is_open()) throw....

    f << "Hello world\n";
    int x = 99;
    f << x << " Red Balloons, floating in the summer sky\n";
    f.close();

Initialising & Basic Writing
+++++++++++++++++++++++++++++

We provide some basic functions which replicate this behaviour, but with additional error handling in the case where the file cannot be opened.

.. jsl:: JSL::IO::initialise
    :file: FileIO/FileWriters.h
    :command: doxygenfunction


.. doxygenfunction:: JSL::IO::writeString


Containers & Pseudo-Tables
+++++++++++++++++++++++++++++++


It is common to want to save a vector (or other STL-container) to file as in a row/column format. The following functions are designed to make this as easy as possible

.. doxygenfunction:: JSL::IO::writeRows(const std::filesystem::path &filename, WriteOptions opts, const Ts&... vecs)

.. doxygenfunction:: JSL::IO::writeRows(const std::filesystem::path &filename, WriteOptions opts, const Outer<Inner, Args...> &contentMatrix)


WriteOptions
//////////////

.. doxygenstruct:: JSL::IO::WriteOptions

.. dropdown:: Helper Functions 
    :color: primary
    :icon: sliders

    .. jsl:: JSL::IO::internal::checkFile
        :file: FileIO/TemplateWriters.h
        :command: doxygenfunction

    .. doxygenfunction:: JSL::IO::internal::mismatchError


    
Examples
++++++++++

.. code-block:: cpp

    #include <JSL.h>
    void printFile(std::string file)
    {
        LOG(INFO) << JSL::IO::getFile(file); //copies a file into a string
    }

    int main(int argc,char**argv)
    {
        std::vector<char> a = {73,76,89,74,83,76};

        std::set<int> b = {6,1,2,3,5,4}; //note the out of order... 
        
        std::string file = "test.txt";
        JSL::IO::writeRows(file,a,b);
        printFile(file);


        std::vector<std::tuple<int,std::string,double>> table{
            {1,"hello",1e-99},
            {2,"world",0.00}
        };
        
        JSL::IO::writeRows(file,{.ColumnDelimiter="-"},table);
        printFile(file);
    }

Output:

.. code-block:: shell-session

    [INFO]  I 1
            L 2
            Y 3
            J 4
            S 5
            L 6
    [INFO]  1-hello-1e-99
            2-world-0

