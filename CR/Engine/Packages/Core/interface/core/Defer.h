#pragma once

import CR.Engine.Core.Defer;

// Don't use lambda syntax. for convienence, just write a code block. i.e.
// defer({code here});
// TODO: c++26 use _ for var name
#define defer(codeBlock) CR::Engine::Core::Defer defer_var##__LINE__{[&]() codeBlock};