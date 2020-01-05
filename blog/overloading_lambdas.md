[Gudmundur F. Adalsteinsson](https://github.com/gummif) -- 2020-01-05 -- [Blog index](http://gummif.github.io/blog)

# Overloading Lambdas and Sharing State


*Creating function overload sets with lambdas and library solutions to share state between overloads.*

### Introduction

A common problem people face when using `std::visit` is that it takes a `std::variant` and a single visitor function object, and therefore is's not straight-forward to pass lambdas to it like you can with almost any other higher-order algorithm in the standard library. There exists a [documented pattern](https://en.cppreference.com/w/cpp/utility/variant/visit) (also available in [Boost.Hana](https://www.boost.org/doc/libs/1_71_0/libs/hana/doc/html/group__group-functional.html)) that uses C++17 to solve the problem with an `overload` class:

```c++
template<class... Ts> struct overload : Ts...
{
    using Ts::operator()...;
};
template<class... Ts> overload(Ts...) -> overload<Ts...>;
```

A similar problem is related to callbacks to async (or sync) functions that can fail with an error or return a result value (see [p1678](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1678r0.pdf) (Callback and Composition) for more details). These callbacks or handlers often need to share state (think some non-trivial resources), so there is something to be gained in both space and time compared to the simple `overload` class above.

Overload sets have been called the [atoms of C++ API design](https://youtu.be/2UmDvg5xv1U?t=284). But any [solution](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1170r0.html) to passing overload sets around don't solve the problem we are facing here, i.e. constructing overloaded lambda closures.

### Overloading `operator()`

For an example, we will start with a function `foo` that takes a function object that either takes two `int` or a string-like object. The function object will capture a variable of type `A` by reference and an integer, and will print them via member functions of `A`.

```c++
template<class F>
void foo(int i, F f)
{
    if (i > 0)
        f(i, 2);
    else
        f(std::string_view("error"));
}

struct A
{
    void print_int(int i) const 
    {
        std::cout << "got int " << i << std::endl;
    }
    void print_string(std::string_view str) const 
    {
        std::cout << "got string " << str << std::endl;
    }
};
```

We will then execute the examples below like so:

```c++
A a;
exampleN(a, 2);
```

### Using `if constexpr`

We can use C++17's `if constexpr` syntax to write a single lambda to call the correct function. The size of the overload is 16 bytes (a reference plus an `int`) which is as small is it gets.

```c++
template<class T>
void example1(A& a, T c)
{
    auto f = [&a, c](auto&& ...args) {
        auto ma = [c](int x, int y) { return x + y * c; };
        if constexpr (sizeof...(args) == 1)
            a.print_string(args...);
        else
            a.print_int(ma(args...));
    };
    std::cout << "example1 size " << sizeof f << std::endl;
    foo(38, f);
    foo(-1, f);
}
```
```sh
example1 size 16
got int 42
got string error
```

### Using `overload`

Now we use the `overload` class, which makes the code readable, but the size is now 24 bytes, since the reference to `a` is captured twice.

```c++
template<class... Ts> struct overload : Ts...
{
    using Ts::operator()...;
};
template<class... Ts> overload(Ts...) -> overload<Ts...>;

template<class T>
void example2(A& a, T c)
{
    auto f = overload{
        [&a, c](int x, int y) {
            a.print_int(x + y * c);
        },
        [&a](std::string_view str) {
            a.print_string(str);
        }
    };
    std::cout << "example2 size " << sizeof f << std::endl;
    foo(38, f);
    foo(-1, f);
}
```

### Local struct

To minimize the size of the overload we can define and construct a struct locally, bringing us back to 16 bytes at the expense of readability. 

```c++
template<class T>
void example3(A& a, T c)
{
    struct {
        A& a;
        T c;
        void operator()(int x, int y) const { a.print_int(x + y * c); }
        void operator()(std::string_view str) const { a.print_string(str); }
    } f{a, c};
    std::cout << "example3 size " << sizeof f << std::endl;
    foo(38, f);
    foo(-1, f);
}
```

### Local struct with a template

We can not define template function inside function scope (why not?), so if we need to overload generic functions we need to move the struct definition outside the function. A bit less readable than the previous example.

```c++
template<class T>
struct Op {
    A& a;
    T c;
    void operator()(int x, int y) const { a.print_int(x + y * c); }
    template<class Str>
    void operator()(const Str& str) const { a.print_string(str); }
};
template<class T>
void example4(A& a, T c)
{
    Op<T> f{a, c};
    std::cout << "example4 size " << sizeof f << std::endl;
    foo(38, f);
    foo(-1, f);
}
```
### Using `stateful_overload`

We can come up with a library solution similar to `overload` that explicitly captures state of any type. 

```c++
template<class S, class F>
struct stateful_overload : S, F
{
    template<class ...Args>
    constexpr auto operator()(Args&& ...args)
        noexcept(noexcept(static_cast<F&>(*this)(static_cast<S&>(*this), std::forward<Args>(args)...)))
        -> decltype(static_cast<F&>(*this)(static_cast<S&>(*this), std::forward<Args>(args)...))
    {
        return static_cast<F&>(*this)(static_cast<S&>(*this), std::forward<Args>(args)...); 
    }
    // overload for &, const&, && and const&& to be fully generic
};
template<class S, class ...Ts> stateful_overload(S, Ts...) -> stateful_overload<S, overload<Ts...>>;

template<class T>
void example5(A& a, T c)
{
    auto f = stateful_overload{
        std::pair<A&, T>{a, c},
        [](auto& s, int x, int y) {
            s.first.print_int(x + y * s.second);
        },
        [](auto& s, std::string_view str) {
            s.first.print_string(str);
        }
    };
    std::cout << "example5 size " << sizeof f << std::endl;
    foo(38, f);
    foo(-1, f);
}
```

A capture of a single variable would make this more readable (or if structured bindings were allowed in the parameters list). If we capture a single variable by reference the code might look like this

```c++
auto f = stateful_overload{
    std::ref(a),
    [](A& a, int x, int y) {
        a.print_int(x + y);
    },
    [](A& a, std::string_view str) {
        a.print_string(str);
    }
};
```

Capturing `this` is not quite as simple as with lambdas.
```c++
class B
{
public:
    void pub()
    {
        auto f = stateful_overload{
            this,
            [](auto s, int x, int y) {
                s->priv(x + y);
            },
            [](auto s, std::string_view str) {
                s->priv(str);
            }
        };
    
        foo(38, f);
    }
private:
    template<class T> void priv(T x) {}
};
```

### Using `bind_front` and `overload`

You might have noticed that the `stateful_overload` solution looks a lot like partial currying. In C++20 we will get `std::bind_front` that does exactly that.

```c++
template<class T>
void example6(A& a, T c)
{
    auto f = std::bind_front(overload{
            [](auto& a, auto c, int x, int y) {
                a.print_int(x + y * c);
            },
            [](auto& a, auto c, std::string_view str) {
                a.print_string(str);
            }
        }, std::ref(a), c);
    std::cout << "example6 size " << sizeof f << std::endl;
    foo(38, f);
    foo(-1, f);
}
```
```sh
example6 size 24
got int 42
got string error
```

It looks like it's a quality of implementation issues (gcc 9.2) since the size is now 24 bytes instead of the expected 16 bytes. If we take the (simplified) definiton for the return type of `bind_front` and add `[[no_unique_address]]` in front of the captured function object we save 8 bytes:

```c++
template<typename _Fd, typename... _BoundArgs>
struct _Bind_front
{
    template<typename _Fn, typename... _Args>
    _Bind_front(_Fn&& __fn, _Args&&... __args)
        : _M_fd(std::forward<_Fn>(__fn))
        , _M_bound_args(std::forward<_Args>(__args)...)
    {}
    // ...

    [[no_unique_address]] _Fd _M_fd;
    std::tuple<_BoundArgs...> _M_bound_args;
};
```

I'm not sure if the specification allows this, maybe a more knowledgeable reader can answer that. 


### Conclusion

The preferred method to construct function overloads and share state is the combination of two already available library solutions: `bind_front` and `overload`. The only drawback is the (possibly) non-optimal size. If that is a problem you can always go for the custom `stateful_overload` or local struct methods.

| Method    | Minimal size | Shared state | Complexity |
|-----------|---------|-----------|-----------|
| `if constexpr` | **Yes** | **Yes** | High |
| `overload` | No | No | Low |
| local struct | **Yes** | **Yes** | High |
| `stateful_overload` | **Yes** | **Yes** | Low |
| `bind_front` + `overload` | No | **Yes** | Low |
