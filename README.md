# corecursive

Single-header C++20 library to remove recursion using coroutines.

## Motivation

Recursive functions are often an easy way to implement an algorithm,
but you must be careful not to overflow your execution stack.
This is especially important if the input to your recursive function
comes from an untrusted source.

Typically, you avoid stack overflows either by

* limiting the depth of the recursion and
  failing the algorithm at some arbitrary depth, or
* manually rewriting the function logic into a state machine.

Now with `co_await`, there is a third option.

## Getting Started

Consider the canonical awful factorial implementation.

    int factorial(int n) {
        if (n <= 1)
            return 1;
        return n * factorial(n - 1);
    }
    assert(factorial(5) == 120);

To make the function iterative, change its return type to `corecursive<int>`
and add `co_await` in front of all recursive calls.

    #include <avakar/corecursive.h>
    using avakar::corecursive;

    corecursive<int> factorial(int n) {
        if (n <= 1)
            co_return 1;
        co_return n * co_await factorial(n - 1);
    }
    assert(factorial(5) == 120);

That's it, your function is no longer recursive and stores its state on the heap.
Read more in the [documentation](docs/index.md).

## License

The library is licensed under the [zero-clause BSD license](LICENSE),
which is almost the same as public domain and is compatible with everything.
