# Documentation

Start with the [ELI5 tutorial](../README.md#getting-started).

## Installation

Drop the `include/avakar/corecursive.h` file somewhere into your include path.
If you use CMake, you can also fetch this repo as a dependency.

    include(FetchContent)
    FetchContent_Declare(
        avakar_corecursive
        GIT_REPOSITORY https://github.com/avakar/corecursive
        GIT_TAG master
        GIT_SHALLOW YES
        )
    FetchContent_MakeAvailable(avakar_corecursive)
    # Link avakar::corecursive into your project

## Class `corecursive<T>`

When a transformed function is called, its body is not immediately executed.
Instead, an object of type `corecursive<T>` is returned.
Call one of the following functions executes the body.

* `corecursive<T>::get()`: executes the function and returns its return value.
  If the function exits via an exception, the exception is propagated to the caller.
  The function may only be called once. If it is called more than once,
  the behavior is undefined.

* `corecursive<T>::operator T()`: same as `get()`. Allows the object
  to be used in most places where the call to the untransformed function would work.
  For example, the following code is well-formed and stores the result of `get()`.

      int x = fact(5);

* `corecursive<T>::~corecursive()`: if `get()` wasn't already called,
  call it and discard the result. If `get()` throws an exception,
  it is propagated from the destructor. Note that the destructor is
  marked `noexcept(false)`.

The object is movable, but non-copyable.

## Custom Allocators

Currently, the global `operator new` is used to allocate function frames,
but allocator support is forthcoming.
