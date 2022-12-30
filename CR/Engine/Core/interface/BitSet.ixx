module;

#include "core/Log.h"

export module CR.Engine.Core.BitSet;

namespace CR::Engine::Core {
	export class BitSet final {
	  public:
		BitSet()                         = default;
		BitSet(const BitSet&)            = default;
		BitSet(BitSet&&)                 = default;
		BitSet& operator=(const BitSet&) = default;
		BitSet& operator=(BitSet&&)      = default;

	  private:
	};
}    // namespace CR::Engine::Core

module :private;

namespace cecore = CR::Engine::Core;
