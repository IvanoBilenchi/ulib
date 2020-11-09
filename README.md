## uLib - a modern, generic C library.

### Author

[Ivano Bilenchi](https://ivanobilenchi.com)

### Description

`uLib` is a modern C library featuring generic and type-safe data structures.
It is currently made of:

- [uVec](https://github.com/IvanoBilenchi/uvec), a generic vector.
- [uHash](https://github.com/IvanoBilenchi/uhash), a generic hash map/set.
- [uFlags](https://github.com/IvanoBilenchi/uflags), a collection of bitmask manipulation primitives.

### Usage

If you are using [CMake](https://cmake.org) as your build system, you can add `uLib` as
a subproject, then link against the `ulib` target. `uLib` data structures are standalone and 
header-only, so you can also include individual headers in your project.

### Documentation

Documentation for the project is provided in form of docstrings in public header files.
You can also generate HTML docs via CMake, though you will also need
[Doxygen](http://www.doxygen.nl). For usage examples, refer to the tests of each data structure.

### CMake targets

- `ulib`: interface library target, which you can link against.
- `ulib-docs`: generates documentation via Doxygen.

### License

`uLib` is available under the MIT license. See the [LICENSE](./LICENSE) file for more info.
