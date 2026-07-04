
Parsing Nested Objects
=========================


Parsing of objects of the form ``vector<set<T>>`` or nested structures is fully supported, provided that all inner types are individually supported by the engine. Due to the structural tokenization required to parse nested hierarchies, the input string must adhere to the following layout rules:

#. **Supported Endcaps:** Elements can be grouped using ``()``, ``[]``, or ``{}``. These delimiters must match perfectly; asymmetric pairings (e.g., ``(a,b}``) will trigger a runtime parsing failure.
#. **Optional Outer Envelopes:** Top-level containers do not require an outermost wrapper. Both explicitly enveloped structures (e.g., ``[[a,b],[c,d]]``) and raw sibling sequences (e.g., ``[a,b],[c,d]``) are supported out-of-the-box.
#. **Delimiter Propagation:** Outer layers are tokenized purely by evaluating balanced bracket boundaries. Any characters residing *outside* of a balanced bracket pair in an outer layer are skipped. Consequently, expressions like ``[[a,b] @ [c,d]]`` and ``[[a,b],[c,d]]`` yield identical results regardless of the provided delimiter sequence. Tokenization of the innermost values falls back to the specified ``delimiter`` sequence. For example:
   
   .. code-block:: cpp

      // Passes the "-" delimiter directly down to the innermost integers
      auto result = ParseTo<std::vector<std::vector<int>>>("[ [1-2-3], [4-5-6] ]", "-");

