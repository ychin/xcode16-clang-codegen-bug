See:
- https://github.com/vim/vim/pull/15764#issuecomment-2406164629
- https://developer.apple.com/forums/thread/766030

This is a quick test to demonstrate Xcode 16's clang compiler codegen bug. When using `-O2`/`-O3`, this version of clang seems to generate wrong optimized inline code, as demonstrated in this sample. This issue does not seem to occur in Xcode 15, or open source versions of clang.

Simply run `make test` to run the test. It will return cleanly if the compiler works properly.
