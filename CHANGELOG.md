# Changelog

<!-- All notable changes to this project will be documented in this file. -->

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## 3.3.3 2026-09-01

## Changed

* Reverted some breaking changes induced in MSVC compilers (oops)
* Removed some unusued variables that were causing compiler flags

## 3.3.2 2026-09-01

## Changed 

* Added compiler optimisation flags

## [3.3.1] 2026-08-17

## Changed

* Added an interface for modifying the debounce time in both the Watcher::File object and monitoring Watcher::Panopticon

## Bugfix

* Overhauled the unit testing modules for the Async module so that they are less fragile. Your testing suite shouldn't have bugs!

## [3.3.0] 2026-08-16

## New Module : Async

An entirely new Asynchronous Computing module has been added (or, re-added). It has been validated to run on Windows, macOS and Ubuntu.

* The Async::Socket class allows for basic string-based Inter-Process Communication; using Unix Domain Sockets.
* The Async::Pool class allows for streamlined Parallel Task Dispatch, without the excess overhead of spooling up std::thread instances for every new task. Includes support for both void functions and return-value functions either as single-shot dispatches, or (for the largest efficiency gains) by wrapping up tight for-loops. 
* The Async::Watcher submodule defines three objects which monitor different aspects of the system, and fire a callback function whenever a change is detected: 
    * The Async::Watcher::Socket watches for messages to a UDS, and applies a callback to every new string message it recieves
    * The Async::Watcher::Input watches for input passed to the standard input stream (i.e. ``std::cin``), and applies a callback to every complete line it recieves
    * The Async::Watcher::File watches for changes in a watched directory (or its children), and applies a callback to each batch of changed files
* The Async::Panopticon acts as a centralised orchestrator of the Async::Watcher classes, collating them under single master `input manager'
* The new module is fully documented

## Added

* JSL::IO::multiGlob allows multiple globs to be composited into a single regex

## Changed

* The IO::Directory class now tracks file metadata as a separate option
* Rewrote the internal String::wrap logic 
* Removed the fragile tabsize-detection from GlobalConfiguration, and replaced it with an assertion on the tabsize to use
* Updated unit tests
* Multiple minor file changes induced by a renaming effort ``FatalError -> LibraryError`` 


## [3.2.0] 2026-08-06

**Apologies: Due to some parallel development with another project, we skipped robustly documenting changes. This changelog indicates all changes made since 3.1.1. Versions 3.1.2 -> 3.1.4 should be considered vestigial**

## New

* **Cross platform integration testing** A new github action ensures that the JSL compiles and meets all of our unit tests on Ubuntu, macOS and Windows machines. A number of changes to header files / implicit casting operations were made to fulfill this cross-platform promise.
* **ProgressBar Prefixes** Progress bars can now have text as either prefixes or suffixes to give additional context
 
## Changed

* Changed the definition of the TupleLike concept to exclude iterables, resolving a conflict where types std::array were matching both TupleLike and NonStringRange
* Changed the definition of the NonStringRange concept to exclude std::filesystem, which met the criteria under the MSVC implementation

## [3.1.1] - 2026-07-14

### Bugfix

* Fixed an error in the ProgressBar implementation that caused Depth to be inferred from a vector after the contents had been moved out.

### Added

* JSL::Log::Config can now have an Indent setting incremented or decremented, which automatically indents all text by a chosen amount - now you no longer need to add "\t\t\t" to the beginnign of your output streams to get a nice heiararchy!
* Interface::Config can now be configured either from a file, or directly from a vector of strings (useful in the case where the 'file'is actually an IO::Vault, not a true file on disk.
* Aggregator can now be configured without needing a command line parse (useful for re-configurations later in the program lifetime)

### Changed

* Added a compiler flag to the KeyType inference engine in Interface; suppresses unused variable warnings

## [3.1.0] - 2026-07-09


### Added

* The Aggregator class now has an Export method

### Changed

* The Help menu in the interface now adds indentation and visual signal that a section is nested
* The .readthedocs.yaml file is now hidden away in the docs/ repository
 
### Bugfixes

* Added source implementations of header-defined functions in the VaultReader; now the functions can actually be called!
* Typos in some Help menu defalut text fixed
 
## [3.0.3] - 2026-07-05

### Bugfix

* Fixed an error where a raw ASCI sequence was inserted regardless of terminal status when a FormatGroup is piped to a string (replaced with a call to ResetALl which does the correct checks)

### Added

* Aggregator help menu now extends the help menu down the heirarchy so all messages are the same width

## [3.0.2] - 2026-07-05

### Bugfix

* Fixed an error in the String::Wrap where totally empty input wouldn't be wrapped to a blank extent (it was treated as a poorly-overhanging trailing whitespace, which is erased)
* Due to the new opinionated editor, this resulted in a complete file change. The actual bugfix is on line 120.

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
