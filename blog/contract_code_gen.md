[Gudmundur F. Adalsteinsson](https://github.com/gummif) -- 2019-03-08 -- [Overview](http://gummif.github.io/blog)

# Code Generation with C++ Contracts

*Caveat: Note that contracts has not been standardized yet and this post will likely need to be updated when C++20 lands.*

[Contracts](https://en.cppreference.com/w/cpp/language/attributes/contract) (runtime pre/post-conditions and assertions) have been approved for C++20. There is a branch of gcc that has implemented parts of the specs (see [this](https://gitlab.com/lock3/gcc-new/wikis/contract-assertions) for more details on the semantics and flags).  With the power of [Compiler Explorer](https://godbolt.org/) I have put together various cases using the `[[assert]]` attribute (pre/post-condition seem not to have been implemented yet it seems) and other traditional forms of checking, and analyzed the generated assembly. 

The following contrived functions contain a branch when `x` is negative, and different ways verify that the input is non-negative. A smart compiler should optimize away the branch when checked (with noreturn semantics). The code was compiled with `gcc -std=c++2a -O3`.

#### Reference 
```c++
int f1(int x)
{
    if (x < 0)
        return 0;
    return 2 * x;
}
```
```nasm
f1(int):
  lea eax, [rdi+rdi]
  test edi, edi
  mov edx, 0
  cmovs eax, edx
  ret
```

#### C-style assert 
```c++
int f2(int x)
{
    assert(x >= 0);
    if (x < 0)
        return 0;
    return 2 * x;
}
```
```nasm
.LC0:
  .string "int f2(int)"
.LC1:
  .string "/tmp/compiler-explorer-compiler11921-62-1fj1k4v.sdg2/example.cpp"
.LC2:
  .string "x >= 0"
f2(int):
  test edi, edi
  js .L7
  lea eax, [rdi+rdi]
  ret
.L7:
  push rax
  mov ecx, OFFSET FLAT:.LC0
  mov edx, 7
  mov esi, OFFSET FLAT:.LC1
  mov edi, OFFSET FLAT:.LC2
  call __assert_fail
```

#### C-style assert with `-DNDEBUG`
```c++
int f3(int x)
{
    assert(x >= 0);
    if (x < 0)
        return 0;
    return 2 * x;
}
```
```nasm
f3(int):
  lea eax, [rdi+rdi]
  test edi, edi
  mov edx, 0
  cmovs eax, edx
  ret
```

#### Explicit `std::terminate()`
```c++
int f4(int x)
{
    if (x < 0)
        std::terminate();
    if (x < 0)
        return 0;
    return 2 * x;
}
```
```nasm
f4(int):
  test edi, edi
  js .L7
  lea eax, [rdi+rdi]
  ret
.L7:
  push rax
  call std::terminate()
```

#### `[[assert]]`
```c++
int f5(int x)
{
    [[assert: x >= 0]]
    if (x < 0)
        return 0;
    return 2 * x;
}
```
```nasm
.LC0:
  .string "default"
.LC1:
  .string "f5::x >= 0"
.LC2:
  .string "f5"
.LC3:
  .string "/tmp/compiler-explorer-compiler11921-62-1l40bvj.fbji/example.cpp"
f5(int):
  push rbp
  mov rbp, rsp
  push rbx
  sub rsp, 8
  test edi, edi
  js .L9
  lea eax, [rdi+rdi]
.L1:
  mov rbx, QWORD PTR [rbp-8]
  leave
  ret

f5(int) [clone .cold]:
.L9:
  push 0
  mov r9d, OFFSET FLAT:.LC0
  mov r8d, OFFSET FLAT:.LC1
  xor edi, edi
  push OFFSET FLAT:.LC0
  mov ecx, OFFSET FLAT:.LC2
  mov edx, OFFSET FLAT:.LC3
  mov esi, 6
  call __on_contract_violation(bool, int, char const*, char const*, char const*, char const*, char const*, int)
  pop rax
  xor eax, eax
  pop rdx
  jmp .L1
  mov rdi, rax
  call __cxa_begin_catch
  call __cxa_rethrow
  mov rbx, rax
  call __cxa_end_catch
  mov rdi, rbx
  call _Unwind_Resume
```

#### `[[assert audit]]`
```c++
int f6(int x)
{
    [[assert audit: x >= 0]]
    if (x < 0)
        return 0;
    return 2 * x;
}
```
```nasm
f6(int):
  lea eax, [rdi+rdi]
  test edi, edi
  mov edx, 0
  cmovs eax, edx
  ret
```

#### `[[assert axiom]]`
```c++
int f7(int x)
{
    [[assert axiom: x >= 0]]
    if (x < 0)
        return 0;
    return 2 * x;
}
```
```nasm
f7(int):
  lea eax, [rdi+rdi]
  ret
```

#### `[[assert]]` with `-fcontract-semantic=default:assume`
```c++
int f8(int x)
{
    [[assert: x >= 0]]
    if (x < 0)
        return 0;
    return 2 * x;
}
```
```nasm
f8(int):
  lea eax, [rdi+rdi]
  ret
```

#### `[[assert check_never_continue]]` with `-fcontract-semantic=default:assume`
```c++
int f9(int x)
{
    [[assert check_never_continue: x >= 0]]
    if (x < 0)
        return 0;
    return 2 * x;
}
```
```nasm
.LC0:
  .string ""
.LC1:
  .string "f9::x >= 0"
.LC2:
  .string "f9"
.LC3:
  .string "/tmp/compiler-explorer-compiler11928-61-10dpzje.0vni/example.cpp"
f9(int):
  push rbp
  mov rbp, rsp
  push rbx
  sub rsp, 8
  test edi, edi
  js .L9
  lea eax, [rdi+rdi]
.L1:
  mov rbx, QWORD PTR [rbp-8]
  leave
  ret

f9(int) [clone .cold]:
.L9:
  push 0
  mov r9d, OFFSET FLAT:.LC0
  mov r8d, OFFSET FLAT:.LC1
  xor edi, edi
  push 0
  mov ecx, OFFSET FLAT:.LC2
  mov edx, OFFSET FLAT:.LC3
  mov esi, 6
  call __on_contract_violation(bool, int, char const*, char const*, char const*, char const*, char const*, int)
  pop rax
  xor eax, eax
  pop rdx
  jmp .L1
  mov rdi, rax
  call __cxa_begin_catch
  call __cxa_rethrow
  mov rbx, rax
  call __cxa_end_catch
  mov rdi, rbx
  call _Unwind_Resume
```

## Summary of results

The results are summarized in the table below, where the default is unchecked and not optimized. Optimized means that the compiler has removed the branched, based a previous check or assumption:

| Case      | Checked | Optimized | Lines of assembly |
|-----------|---------|-----------|:---------------:|
| Reference |  |  | 6 |
| C-style assert        | checked | optimized | 18 |
| C-style assert with `-DNDEBUG` |   |  | 6 |
| Explicit `std::terminate()` | checked | optimized | 8 |
| `[[assert]]` | checked |  | 43 |
| `[[assert audit]]` |   |  | 6 |
| `[[assert axiom]]` |  | optimized | 3 |
| `[[assert]]` with `default:assume` | | optimized | 3 |
| `[[assert check_never_continue]]` with `default:assume` | checked |  | 43 |

What is surprising is that the `[[assert]]` contract does not produce optimized code even though the default semantics is `check_never_continue` (see [the wiki](https://gitlab.com/lock3/gcc-new/wikis/contract-assertions) for more details). I assume this is a feature they are working on and that in the future the contract violation handler will have noreturn semantics when `check_never_continue` semantics are enabled.

Currently there is no standard way to get the unchecked optimized code without resorting to compiler specific intrinsics. But contracts with the `assume` semantics are a very sharp knife. A program compiled with `-fcontract-semantic=default:assume` can be more heavily optimized, but any contract violations will result in (*really*) undefined behaviour.

The results can be summarized in form of a few guidelines for where, when and how to use contracts:

- Contracts are a good replacement for the `assert` macro and compiler intrinsics
- Do not sprinkle contracts on functions without a careful thought
- For each contract, consider whether the `assume` semantics are acceptable, and consider using `check_never_continue` when assuming or ignoring a contract is not acceptable
- Do not rely on contracts to verify user input

And finally, have fun!





