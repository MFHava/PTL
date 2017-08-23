Portable Template Library

(c) Michael Florian Hava

Released under the Boost Software License - Version 1.0, see "LICENSE_1_0.txt" for details.

Introduction
============
The Portable Template Library (PTL) is a header-only library providing "standard library"-style class templates that are guaranteed to be binary stable (their binary representation NEVER changes) - this property enables them to be portable between different compiler(version)s and therefore suitable for the definition of C++-based interfaces of precompiled libraries on platforms that lack a standardised C++ ABI.

Requirements
============ 
The PTL requires only a conformant C++11 implementation and has no external depedencies.
The included unit tests depend on CMake and the the Boost.Test.

Historical Note
===============
This project was originally developed as part of C++ with Components (https://github.com/MFHava/CWC) but has since been made independent.
