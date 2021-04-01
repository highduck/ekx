#include "rand.hpp"

namespace ek {

Random<Lcg32> rand_default{};
Random<Lcg32> rand_game{};
Random<Lcg32> rand_fx{};

}
