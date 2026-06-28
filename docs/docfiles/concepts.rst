.. _concepts:

Concepts
=========

Concepts, introduced in C++20 provide a more advanced method of constraining the types that a template can accept, by checking certain compile time properties. 

Concepts reduce the need to rely on `SFINAE`_ principles, and have the distinct advantage of providing nicer error messages than standard templates (which are notorious for being unreadable). 

   .. _SFINAE: https://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error 


We provide the following concepts, all nested within the ``JSL::Concept`` namespace

Simple Types
++++++++++++++

   .. doxygenconcept:: JSL::Concept::DurationLike
   .. doxygenconcept:: JSL::Concept::Integer
   .. doxygenconcept:: JSL::Concept::Numeric
   .. doxygenconcept:: JSL::Concept::OptionalLike
   .. doxygenconcept:: JSL::Concept::SharedPtr
   .. doxygenconcept:: JSL::Concept::SmartPtr
   .. doxygenconcept:: JSL::Concept::StringLike
   .. doxygenconcept:: JSL::Concept::TupleLike
   .. doxygenconcept:: JSL::Concept::UniquePtr

Vectors & Ranges
+++++++++++++++++

   .. doxygenconcept:: JSL::Concept::IndexableRange
   .. doxygenconcept:: JSL::Concept::Iterable
   .. doxygenconcept:: JSL::Concept::NonStringRange
   .. doxygenconcept:: JSL::Concept::SearchableRange
   .. doxygentypedef:: JSL::Concept::RangeInternalType



