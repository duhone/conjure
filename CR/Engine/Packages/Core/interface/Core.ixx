export module CR.Engine.Core;

// Note that Log and Defer are not included here, as those should be used through Core.h
export import CR.Engine.Core.Algorithm;
export import CR.Engine.Core.BinaryStream;
export import CR.Engine.Core.BitSet;
export import CR.Engine.Core.Buffer;
export import CR.Engine.Core.Defer;
export import CR.Engine.Core.EightCC;
export import CR.Engine.Core.Embedded;
export import CR.Engine.Core.FileHandle;
export import CR.Engine.Core.Function;
export import CR.Engine.Core.Guid;
export import CR.Engine.Core.Handle;
export import CR.Engine.Core.Hash;
export import CR.Engine.Core.Literals;
export import CR.Engine.Core.Locked;
export import CR.Engine.Core.Log;
export import CR.Engine.Core.Random;
export import CR.Engine.Core.Rect;
export import CR.Engine.Core.TypeTraits;

export namespace CR::Engine::Core {
}    // namespace CR::Engine::Core

module :private;

namespace cecore = CR::Engine::Core;

// core doesn't have any systems. nothing to do
