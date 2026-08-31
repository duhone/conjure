#pragma once

// Don't use lambda syntax. for convenience, just write a code block. i.e.
// defer({code here});
// TODO: c++26 use _ for var name
#define defer_var_concat(a, b) a##b
#define defer_var(line) defer_var_concat(defer_var_, line)
#define defer(codeBlock) CR::Engine::Core::Defer defer_var(__LINE__){[&]() codeBlock};