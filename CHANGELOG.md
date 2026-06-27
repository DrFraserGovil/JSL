# Changelog

<!-- All notable changes to this project will be documented in this file. -->

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased Version]

### [Pending Changes]

* TODO: Documentation needs to be fully completed 
* TODO: Rename the Parameter module to CLI  
* TODO: Check the Parameter is optimally written; the new packaging system is pretty good, but needs work
* TODO: Check the Async library. There's almost certainly an overkill of sockets in there, and the parpool could be revived
* TODO: The concepts could be streamlined and made a bit clearer as to what they do. There's also the big load of awful tempaltes found in one of the parsers that might need to be excised 
* TODO: Inconsistent vector <-> container verbiage needs to be fixed (or better delineated in Vectors.h

### Added

* CLI Interface and parameter aggregator to replace the disgusting MACRO interface previously suggested
* New Async library for parallel computing and IPC
* Vault/Archiver system for TAR support
* More robust unit testing

#### Windows Compatibility 

1.  Lots of pragma guards and redefinitions of linux-specific calls. These can be seen mostly in Async.h and Terminal.h
2.  Tested against MSVC compiler & some minor tweaks made to the algorithms (and the test suite): all tests currently pass on both gcc and windows. 
1.  

 <!-- HACK: There's some Gemini-created code hanging around in the Windows portions. This could do with being hand reviewed -->


### Changed

* Moved away from a header-only implementation, and shifted to a compiled library setup
* **Documentation Overhaul.** The documentation has been overhauled 
* **Consistent style guide** Explicitly written and enforced
* **Renaming and Restructuring** Many of the internal submodules and functionshave been renamed or moved into different module. **BACKWARDS COMPATIBILITY WITH PRIOR VERSIONS IS VIOLETED IN ALMOST ALL CASES** 
* Log module has been revamped to be cleaner and more consistent, and have 'boxing' capabilities 

### Removed

* **Plot.h submodule**. This was deemed too broad in scope for the JSL; it has been split out into its own library



## 1.0 & 2.0 (Prehistory)

The 1.0 and 2.0 versions of JSL were created prior to me understanding why semantic versioning was important, as such, they do not exist as `stable releases', and instead track eras-of-usage. JSL 1.0 is that associated with my early PhD work (2018-2021), with JSL 2.0 being an overhaul introduced after I discovered template metaprogramming. 

JSL 3.0 will mark a shift to a more rigorous naming and documentation schedule.
