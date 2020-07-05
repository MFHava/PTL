Portable Template Library

(c) Michael Florian Hava

Released under the Boost Software License - Version 1.0, see "LICENSE_1_0.txt" for details.

Introduction
============
The Portable Template Library (PTL) is a header-only library providing "standard library"-style...
* class templates that are guaranteed to be binary stable
  * their binary representation NEVER changes
  * they can be passed between precompiled libraries on platforms that lack a standardised C++ ABI
* function templates that fill "holes" in the algorithms library

Requirements
============ 
The PTL requires a conformant C++17 implementation.
The included unit tests depend on CMake and Boost.Test.

Historical Note
===============
This project was originally developed as part of [CWC](https://github.com/MFHava/CWC) but has since been made independent.
