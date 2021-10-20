## uLib - a modern, generic C library.

`uLib` is a modern C library featuring generic and type-safe data structures.

### Usage

If you are using [CMake](https://cmake.org) as your build system, you can add `uLib` as
a subproject, then link against the `ulib` target.

### Documentation

Documentation for the project is provided in form of docstrings in public header files.
You can also generate HTML docs via CMake, though you will also need
[Doxygen](http://www.doxygen.nl). For usage examples, refer to the tests of each data structure.

### CMake targets

- `ulib`: library target, which you can link against.
- `ulib-docs`: generates documentation via Doxygen.

### Author

[Ivano Bilenchi](https://ivanobilenchi.com)

### License

`uLib` is available under the MIT license. See the [LICENSE](./LICENSE) file for more info.
