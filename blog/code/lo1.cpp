#include <iostream>
#include <functional>
#include <memory>

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

template<class S, class F>
struct stateful_overload
{
    [[no_unique_address]] S s;
    [[no_unique_address]] F f;
    template<class ...Fs>
    stateful_overload(S sa, Fs ...fs)
        : s(std::move(sa))
        , f{std::forward<Fs>(fs)...}
    {}

    template<class ...Args>
    constexpr auto operator()(Args&& ...args)
        noexcept(noexcept(f(s, std::forward<Args>(args)...)))
        -> decltype(f(s, std::forward<Args>(args)...))
    {
        return f(s, std::forward<Args>(args)...); 
    }
    // overload for &, const&, && and const&& to be fully generic
};
template<class S, class ...Ts> stateful_overload(S, Ts...) -> stateful_overload<S, overload<Ts...>>;

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
        std::cout << "example5c size " << sizeof f << std::endl;
    
        foo(38, f);
    }
private:
    template<class T>
    void priv(T x) {
        std::cout << "priv" << std::endl;

    }
};

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

    auto g = stateful_overload{
        std::ref(a),
        [](auto& s, int x, int y) {
            s.get().print_int(x + y);
        },
        [](auto& s, std::string_view str) {
            s.get().print_string(str);
        }
    };
    std::cout << "example5b size " << sizeof g << std::endl;
    
    foo(38, g);

    B b;
    b.pub();
    using namespace std::placeholders;
    auto h = std::bind([](auto x, auto y){ return x + y;}, 40, _1);
    std::cout << "example5d size " << sizeof h << " " << h(2) << std::endl;
}

template<typename _Fd, typename... _BoundArgs>
struct _Bind_front
{
    template<typename _Fn, typename... _Args>
    _Bind_front(_Fn&& __fn, _Args&&... __args)
        : _M_fd(std::forward<_Fn>(__fn))
        , _M_bound_args(std::forward<_Args>(__args)...)
    {}

    [[no_unique_address]] _Fd _M_fd;
    std::tuple<_BoundArgs...> _M_bound_args;
};

template<class T>
void example6(A& a, T c)
{
    auto f = std::bind_front(overload{
            [](auto& s, int x, int y) {
                s.first.print_int(x + y * s.second);
            },
            [](auto& s, std::string_view str) {
                s.first.print_string(str);
            }
        }, std::pair<A&, T>{a, c});
    std::cout << "example6 size " << sizeof f << std::endl;
    foo(38, f);
    foo(-1, f);

    auto h = std::bind_front([](auto x, auto y){ return y; }, std::tuple<A&>{a});
    std::cout << "example6b size " << sizeof h << " " << h(2) << std::endl;

    auto ol = overload{
            [](auto& s, int x, int y) {
                s.first.print_int(x + y * s.second);
            },
            [](auto& s, std::string_view str) {
                s.first.print_string(str);
            }
        };
    _Bind_front<decltype(ol), A&, T> bf(ol, a, c);
    std::cout << "example6c size " << sizeof bf << " " << h(2) << std::endl;

}

// TODO another post on type erasing overloads
template<class, class>
class function_overload2;

template<class R1, class ...Args1, class R2, class ...Args2>
class function_overload2<R1(Args1...), R2(Args2...)> {
private:
    struct abstract_fn {
        virtual R1 call1(Args1...) = 0;
        virtual R2 call2(Args2...) = 0;
        virtual ~abstract_fn() = default;
    };

    template<class T>
    struct wrapping_fn : abstract_fn {
        T cb_;
        explicit wrapping_fn(T &&cb) : cb_(std::move(cb)) {}
        R1 call1(Args1... x) override { return cb_(x...); }
        R2 call2(Args2... x) override { return cb_(x...); }
    };

    std::unique_ptr<abstract_fn> ptr_;

public:
    template<class T>
    function_overload2(T t)
        : ptr_{std::make_unique<wrapping_fn<T>>(std::move(t))}
    {
    }
    R1 operator()(Args1... x) {
        return ptr_->call1(x...);
    }
    R2 operator()(Args2... x) {
        return ptr_->call2(x...);
    }
};

template<class T>
void example6_(A& a, T c)
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
    std::cout << "example6a size " << sizeof f << std::endl;
    foo(38, f);
    foo(-1, f);
    function_overload2<void(int, int), void(std::string_view)> g1 = f;
    std::cout << "example6b size " << sizeof g1 << std::endl;
    foo(38, std::move(g1)); // move-only
    function_overload2<void(int, int), void(std::string_view)> g2 = f;
    foo(-1, std::move(g2));
}

template<class H>
void foo_err(int i, H handler)
{
    if (i > 0)
        handler(i, 2);
    else
        handler.on_error(std::string_view("error"));
}

template<class T>
void example7(A& a, T c)
{
    struct {
        A& a;
        T c;
        void operator()(int x, int y) const { a.print_int(x + y * c); }
        void on_error(std::string_view str) const { a.print_string(str); }
    } f{a, c};
    std::cout << "example7 size " << sizeof f << std::endl;
    foo_err(38, f);
    foo_err(-1, f);
}

template<class S, class F, class E>
struct stateful_callback : S, F, E
{
    template<class ...Args>
    auto operator()(Args&& ...args)
        noexcept(noexcept(static_cast<F&>(*this)(static_cast<S&>(*this), std::forward<Args>(args)...)))
        -> decltype(static_cast<F&>(*this)(static_cast<S&>(*this), std::forward<Args>(args)...))
    {
        return static_cast<F&>(*this)(static_cast<S&>(*this), std::forward<Args>(args)...); 
    }
    template<class ...Args>
    auto on_error(Args&& ...args)
        noexcept(noexcept(static_cast<E&>(*this)(static_cast<S&>(*this), std::forward<Args>(args)...)))
        -> decltype(static_cast<E&>(*this)(static_cast<S&>(*this), std::forward<Args>(args)...))
    {
        return static_cast<E&>(*this)(static_cast<S&>(*this), std::forward<Args>(args)...); 
    }
    // overload for &, const&, && and const&& to be fully generic
};
template<class S, class F, class E> stateful_callback(S, F, E) -> stateful_callback<S, F, E>;

template<class T>
void example8(A& a, T c)
{
    auto f = stateful_callback{
        std::pair<A&, T>{a, c},
        [](auto& s, int x, int y) {
            s.first.print_int(x + y * s.second);
        },
        [](auto& s, std::string_view str) {
            s.first.print_string(str);
        }
    };
    std::cout << "example8 size " << sizeof f << std::endl;
    foo_err(38, f);
    foo_err(-1, f);
}

// Creating function overload sets with lambdas, 
// library solutions to share state between overloads 
// and an example of a type-erased overload set.
// Finally a small example applied to named functions.
// https://quuxplusone.github.io/blog/2019/03/18/what-is-type-erasure/ 
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1678r0.pdf
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1660r0.pdf

int main()
{
    A a;
    example1(a, 2);
    example2(a, 2);
    example3(a, 2);
    example4(a, 2);
    example5(a, 2);
    example6(a, 2);
    //example6_(a, 2);
    //example7(a, 2);
    //example8(a, 2);
}