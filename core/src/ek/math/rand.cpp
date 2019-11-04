#include "rand.hpp"

namespace ek {

Lcg32 lcg_32_default{};
Random<Lcg32> rand_default{lcg_32_default};

Lcg32 lcg_32_game{};
Random<Lcg32> rand_game{lcg_32_game};

Lcg32 lcg_32_fx{};
Random<Lcg32> rand_fx{lcg_32_fx};

}
