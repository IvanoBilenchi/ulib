========
Building
========

You can find uLib's code on its `git repository <git_url_>`_. If you are using `CMake`_
as your build system, you can use the ``add_subdirectory`` command, then link against
any of uLib's library targets. Alternatively, build one of the library targets
and then link against it. Some of uLib's targets can be enabled or disabled via CMake options.

CMake targets
=============

- ``ulib-static``: static library (``ULIB_STATIC`` option, default ``ON``).
- ``ulib-shared``: dynamic library (``ULIB_SHARED`` option, default ``ON``).
- ``ulib-object``: object library (``ULIB_OBJECT`` option, default ``OFF``).
- ``ulib-docs``: generates the documentation (requires `Doxygen`_ and optionally `Sphinx`_).

========
Examples
========

For usage examples, refer to the unit tests.

.. _CMake: https://cmake.org
.. _Doxygen: https://doxygen.nl
.. _Sphinx: https://www.sphinx-doc.org
