# Changelog

<!-- All notable changes to this project will be documented in this file. -->

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).


## [3.0.1] - 2026-07-04 (2)

### Added 

* Added the missing readthedocs.yaml file and configured the repo for pushing to readthedocs, which now has a successfully building branch after 280 consectutive failed builds
* Pretty badges on the README

## [3.0.0] - 2026-07-04

### Added

* CLI Interface and parameter aggregator to replace the disgusting MACRO interface previously suggested
    - The macro interface has been preserved for 'small scale' parameter sets of < 10 parameters. Larger scale groups require an Aggregator approach
* Vault/Archiver system for TAR support
* More robust unit testing
* Windows Compatibility 
    - Lots of pragma guards and redefinitions of linux-specific calls. These can be seen mostly in Async.h and Terminal.h
    - Tested against MSVC compiler & some minor tweaks made to the algorithms (and the test suite): all tests currently pass on both gcc and windows. 


### Changed

* **De-headified** Moved away from a header-only implementation, and shifted to a compiled library setup
* **Documentation Overhaul.** The documentation has been overhauled 
* **Consistent style guide.** Explicitly written and enforced
* **Renaming and Restructuring.** Many of the internal submodules and functions have been renamed or moved into different module. **BACKWARDS COMPATIBILITY WITH PRIOR VERSIONS IS VIOLATED IN ALMOST ALL CASES** 
* **Log module revamp.** Interface changed to be cleaner and more consistent, and have 'boxing' capabilities 

### Removed

* **Plot.h submodule**. This was deemed too broad in scope for the JSL; it has been split out into its own library



## 1.0 & 2.0 (Prehistory)

The 1.0 and 2.0 versions of JSL were created prior to me understanding why semantic versioning was important, as such, they do not exist as `stable releases', and instead track eras-of-usage. JSL 1.0 is that associated with my early PhD work (2018-2021), with JSL 2.0 being an overhaul introduced after I discovered template metaprogramming. 

JSL 3.0 will mark a shift to a more rigorous naming and documentation schedule.
