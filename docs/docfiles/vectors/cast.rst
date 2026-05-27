.. _vector_cast:

Vector Casting
=================

Vector Casting is a generalisation of the ``static_cast`` interface, allowing the mass-casting of objects held within a single-element container (such as ``std::vector`` or ``std::set``, but not ``std::map``).

We have also integrated the :ref:`ParseTo <parseto>` and :ref:`MakeFrom <makestring>` functions, allowing these to be treated as mass-castings too. 

.. jsl-meta::
	:file: JSL/Vectors/Cast.H
	:namespace: JSL/Vector

.. have to manually document this as doxygen/sphinx don't like function overloads that only differ by their template signature!

.. cpp:function:: template<typename To, template<typename...> typename Container, typename From, typename... Args> Container<To> Vector::cast(const Container<From, Args...> & src)
	 
	A generalisation of static_cast to STL containers; generates a new object which contains a casting of the original internal type
	
	:warning: We make no guarantees about the reversibility of this process, or that the order of the objects are preserved (i.e. an std::set may sort differently in the To-space than the From-space).

	:tparam To: The destination type held within the output container. Must be implicitly constructible from the origin type. If this is a string-like type, triggers ``JSL::String::makeFrom<From>`` instead of a cast. 
	:tparam From: The origin type held within the original container. If this is a string-like type, triggers ``JSL::String::ParseTo<To>`` instead of a cast.
	:tparam ...Args: A catcher for other types (i.e. iterators) that are often hidden away by STL containers
	:param src: The original container to be cast into a new type.
	:return: A new container containing the same objects as the original, but constructed into a new type.
