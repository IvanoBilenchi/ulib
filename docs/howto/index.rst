====================
Building from source
====================

uLib can be built and run on **Windows**, **macOS** and **Linux**. We have successfully deployed it
to a wider range of platforms, including tiny microcontrollers, with relatively minor
build system setup. It can be compiled either as a **static** or **dynamic library**.

Requirements
------------

In order to compile the library, you will need at a minimum:

- GCC_, LLVM_ or MSVC_, any fairly recent version.
- CMake_ version 3.13 or later.

If you want to generate the documentation, you will also need:

- Doxygen_ version 1.8 or later.
- *(optional)* Sphinx_ version 2.0 or later, Breathe_ and the `Read The Docs Theme`_.

Sphinx is optional as Doxygen will already generate some form of HTML docs,
though not as fancy as the ones you are viewing.

Downloading the sources
-----------------------

You can find uLib's code on its `git repository <git_url_>`_. Begin by cloning it:

.. parsed-literal::

   git clone |git_url|

Compiling
---------

The following commands allow you to build uLib:

.. code-block:: bash

   # Generate the build system
   cmake -B cmake-build -DCMAKE_BUILD_TYPE=Release

   # [Optional] Edit build settings (library type, optimization options, etc.)
   ccmake cmake-build

   # Build the library
   cmake --build cmake-build --config Release

   # [Optional] Install the library and its headers in <install path>
   cmake --install cmake-build --prefix <install path>

   # [Optional] Build the documentation
   cmake --build cmake-build --target ulib-docs

Linking
-------

If you're using CMake as your build system, you can link against uLib by configuring your
*CMakeLists.txt* file as follows:

.. code-block:: cmake

    # Assuming uLib's source is under "lib/ulib"
    add_subdirectory("lib/ulib")
    target_link_libraries(your-target PRIVATE ulib)

For other build systems or if you are building directly through a compiler, please refer
to their respective documentations. A relatively headache-free way to integrate uLib
involves compiling it and then linking against the built library, making sure
the build system or compiler is aware of uLib's headers.

Zephyr
------

uLib builds for `Zephyr`_ as it does anywhere else, so vendor it and add it to your
application's *CMakeLists.txt*, configuring it through the usual CMake options. It picks up
Zephyr's toolchain on its own:

.. code-block:: cmake

    set(ULIB_CONCURRENCY ON)
    add_subdirectory("lib/ulib")
    target_link_libraries(app PRIVATE ulib)

Concurrency is backed by Zephyr's own kernel objects, so it does not require the POSIX
subsystem. Five things are worth knowing before enabling it:

- **Thread-local storage must be enabled.** Thread identifiers and the random number
  generator rely on it, so set ``CONFIG_THREAD_LOCAL_STORAGE=y`` on any target whose
  architecture supports it. The native simulator is not one of them: there the host
  toolchain provides it.

- **Thread stacks belong to the caller.** Zephyr can only allocate them when
  ``CONFIG_DYNAMIC_THREAD`` is set, which is experimental and pulls in thread monitoring.
  Their size is that of ``ULIB_THREAD_STACK_SIZE``. Without it, hand each thread a stack
  of its own:

  .. code-block:: c

     K_THREAD_STACK_DEFINE(worker_stack, 2048);

     UThread thread;
     uthread(&thread, worker, NULL);
     uthread_set_stack(&thread, worker_stack, K_THREAD_STACK_SIZEOF(worker_stack));
     uthread_start(&thread);

- **Threads must be joined.** Zephyr has neither a detached thread state nor a thread exit
  hook, so a stack that nobody joins on cannot be reclaimed. :func:`uthread_detach` returns
  ``ULIB_ERR_UNSUPPORTED``.

- **Spinning locks need the timeslicer.** :type:`USLock` spins without entering the kernel,
  so on a single CPU a preempted holder is only rescheduled if ``CONFIG_TIMESLICING`` is
  enabled with a nonzero ``CONFIG_TIMESLICE_SIZE``. Prefer :type:`ULock` otherwise.

- **Do not synchronize from an ISR.** Blocking primitives are built on Zephyr mutexes and
  condition variables, which cannot be waited on from interrupt context.

========
Examples
========

For usage examples, refer to the unit tests.

.. _Breathe: https://breathe.readthedocs.io
.. _CMake: https://cmake.org
.. _Doxygen: http://doxygen.nl
.. _GCC: https://gcc.gnu.org
.. _LLVM: https://llvm.org
.. _MSVC: https://visualstudio.microsoft.com
.. _Read The Docs Theme: https://sphinx-rtd-theme.readthedocs.io
.. _Sphinx: http://sphinx-doc.org
.. _Zephyr: https://zephyrproject.org
