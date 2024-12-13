# Updates

<ins>Update (2024-12-12)</ins>: The bug has been fixed in Xcode 16.2, as can be seen in [CI](https://github.com/ychin/xcode16-clang-codegen-bug/actions/workflows/ci.yaml). This means you would not see this issue as long as you don't use Xcode 16.0 / 16.1. This repo is now archived as a result.

<ins>Update (2024-10-23)</ins>: It was [identified](https://github.com/Homebrew/homebrew-core/issues/195325#issuecomment-2433476723) that using the compilation flags `-mllvm -enable-constraint-elimination=0` would fix this issue, indicating that the bug is in the "ConstraintEliminationPass" step.

# Introduction

See:
- https://github.com/vim/vim/pull/15764#issuecomment-2406661051
- https://github.com/Homebrew/homebrew-core/issues/195325
- https://developer.apple.com/forums/thread/766030

This is a quick test to demonstrate Xcode 16's clang compiler codegen bug. When using `-O2`/`-O3`, this version of clang seems to generate wrong optimized inline code, as demonstrated in this sample.

The bug is also tested in [CI](https://github.com/ychin/xcode16-clang-codegen-bug/actions/workflows/ci.yaml). A failing run means the bug occurs.

[![Test Status](https://github.com/ychin/xcode16-clang-codegen-bug/actions/workflows/ci.yaml/badge.svg)](https://github.com/ychin/xcode16-clang-codegen-bug/actions/workflows/ci.yaml)


# Running

Simply run `make test` to run the test. It will return cleanly if the compiler works properly.

Correct results should look like the following:

```
./program_dbg
Output: 6. Expected: 6
./program_opt
Output: 6. Expected: 6
```

Incorrect results (when using Xcode 16) looks like this:

```
./program_dbg
Output: 6. Expected: 6
./program_opt
Output: 7. Expected: 6
make: *** [test] Error 1
```

# Analysis

The test was done on an Apple M1 Max MacBook Pro, running macOS 14.6.1 (also tested on macOS 15.0 in a VM). Compiler was Xcode clang-1600.0.26.3. This issue does not seem to occur in Xcode 15, or open source versions of clang (obtained from Homebrew).

The code was extracted from Vim. It tries to find the first invalid byte sequence in a UTF-8 string. The generated code seems to fail and does not break out of the loop at the first invalid byte (`\xE2`), which is invalid because it's supposed to be a 3-byte sequence but the string in the test code only has 2 bytes.

From analysis, seems like the error comes from this following snippet in `utf_find_illegal`:

```c
int len = utf_ptr2len(p);
if (*p >= 0x80u && (len == 1
            || utf_char2len(utf_ptr2char(p)) != len))
```

Both `utf_ptr2len` and `utf_ptr2char` contains somewhat similar code. The inliner seems to inline both functions and then combine the logic as it's looping through the bytes but it doesn't seem to have the correct logic for breaking out of the loop as a result (in this case, `utf_ptr2len` is supposed to return 1 in line 126, which would have resulted in `len` being 1 and causing the if statement to be true, and therefore returning from `utf_find_illegal`). If the `utf_char2len(utf_ptr2char(p)) != len` condition is commented out in the snippet, then the code would work properly.

Also, turning on UBSAN using `-fsanitize=undefined` (which is commented out in the Makefile) would for some reason fix the issue.
