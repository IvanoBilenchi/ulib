========
Building
========

You can find uLib's code on its `git repository <git_url_>`_. If you are using `CMake`_
as your build system, you can import it via the ``add_subdirectory`` command, then link against
the ``ulib`` target. You can control the library type by setting the ``ULIB_LIBRARY_TYPE``
CMake variable, which can be either ``STATIC`` (default), ``SHARED`` or ``OBJECT``.

CMake targets
=============

- ``ulib``: builds the library.
- ``ulib-tests``: builds the unit tests.
- ``ulib-docs``: generates the documentation (requires `Doxygen`_ and optionally `Sphinx`_).

========
Examples
========

For usage examples, refer to the unit tests.

.. _CMake: https://cmake.org
.. _Doxygen: https://doxygen.nl
.. _Sphinx: https://www.sphinx-doc.org
