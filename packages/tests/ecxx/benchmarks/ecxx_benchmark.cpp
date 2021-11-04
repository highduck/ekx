#include <iostream>
#include <cstddef>
#include <cstdint>
#include <chrono>
#include <iterator>
#include <ecxx/ecxx.hpp>
#include "common.hpp"

using ecs::World;
using ecs::EntityApi;

namespace ecxx_b {

struct position {
    std::uint64_t x;
    std::uint64_t y;
};

struct velocity {
    std::uint64_t x;
    std::uint64_t y;
};

template<std::size_t>
struct comp {
    int x;
};

template<typename Func>
void pathological(Func func) {
    World registry;

    for (std::uint64_t i = 0; i < 500'000L; i++) {
        const auto e = registry.create();
        registry.assign<position>(e);
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);
    }

    for (auto i = 0; i < 10; ++i) {
        registry.each([i = 0, &registry](const auto e) mutable {
            // note: reset is better?
            if (!(++i % 7)) { registry.remove<position>(e); }
            if (!(++i % 11)) { registry.remove<velocity>(e); }
            if (!(++i % 13)) { registry.remove<comp<0>>(e); }
            if (!(++i % 17)) { registry.destroy(e); }
        });

        for (std::uint64_t j = 0; j < 50'000L; j++) {
            const auto e = registry.create();
            registry.assign<position>(e);
            registry.assign<velocity>(e);
            registry.assign<comp<0>>(e);
        }
    }

    func(registry, [](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void construct(run_context_t& ctx) {
    world w;

    ctx.measure([&]{
        for (std::uint64_t i = 0; i < 1'000'000L; i++) {
            w.create();
        }
    });
}

void construct_many(run_context_t& ctx) {
    world w;
    std::vector<EntityApi> entities(1'000'000L);

    ctx.measure([&]{
        w.create(entities.begin(), entities.end());
    });
}

void construct_many_and_assign_components(run_context_t& ctx) {
    world w;
    std::vector<EntityApi> entities(1'000'000L);

    ctx.measure([&]{
        w.create(entities.begin(), entities.end());
        for (const auto e: entities) {
            w.assign<position>(e);
            w.assign<velocity>(e);
        }
    });
}

void construct_many_with_components(run_context_t& ctx) {
    world w;
    std::vector<EntityApi> entities(1'000'000L);

    ctx.measure([&]{
        w.create<position, velocity>(entities.begin(), entities.end());
    });
}

void destroy(run_context_t& ctx) {
    world w;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        w.create();
    }

    ctx.measure([&] {
        w.each([&w, &ctx](auto e) {
            w.destroy(e);
        });
    });
}

void IterateSingleComponent1M(run_context_t& ctx) {
    world w;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = w.create();
        w.assign<position>(e);
    }

    auto test = [&w, &ctx](auto func) {
        ctx.measure([&] {
            w.view<position>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateSingleComponentRuntime1M(run_context_t& ctx) {
    world w;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = w.create();
        w.assign<position>(e);
    }

    auto test = [&w, &ctx](auto func) {
        using component_type = world::component_typeid;
        component_type types[] = {w.type<position>()};

        ctx.measure([&] {
            w.runtime_view(std::begin(types), std::end(types)).each(func);
        });
    };

    test([&w, &ctx](auto e) {
        w.get<position>(e).x = {};
    });
}

void IterateTwoComponents1M(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<position>(e);
        registry.assign<velocity>(e);
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&] {
            registry.view<position, velocity>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateTwoComponents1MHalf(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<velocity>(e);

        if (i % 2) {
            registry.assign<position>(e);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&] {
            registry.view<position, velocity>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateTwoComponents1MOne(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<velocity>(e);

        if (i == 500'000L) {
            registry.assign<position>(e);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&]{
            registry.view<position, velocity>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

//void IterateTwoComponentsNonOwningGroup1M(run_context_t& ctx) {
//    World registry;
//    registry.group<>(entt::get<position, velocity>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e);
//        registry.assign<velocity>(e);
//    }
//
//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//        registry.group<>(entt::get<position, velocity>).each(func);
//});
//        
//    };
//
//    test([](auto &... comp) {
//        ((comp.x = {}), ...);
//    });
//}
//
//void IterateTwoComponentsFullOwningGroup1M(run_context_t& ctx) {
//    World registry;
//    registry.group<position, velocity>();
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e);
//        registry.assign<velocity>(e);
//    }
//
//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//        registry.group<position, velocity>().each(func);
//});
//        
//    };
//
//    test([](auto &... comp) {
//        ((comp.x = {}), ...);
//    });
//}
//
//void IterateTwoComponentsPartialOwningGroup1M(run_context_t& ctx) {
//    World registry;
//    registry.group<position>(entt::get<velocity>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e);
//        registry.assign<velocity>(e);
//    }
//
//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//        registry.group<position>(entt::get<velocity>).each(func);
//});
//        
//    };
//
//    test([](auto &... comp) {
//        ((comp.x = {}), ...);
//    });
//}

void IterateTwoComponentsRuntime1M(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<position>(e);
        registry.assign<velocity>(e);
    }

    auto test = [&registry, &ctx](auto func) {
        using component_type = typename world::component_typeid;
        component_type types[] = {registry.type<position>(), registry.type<velocity>()};

        ctx.measure([&]{
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });
    };

    test([&registry, &ctx](auto e) {
        registry.get<position>(e).x = {};
        registry.get<velocity>(e).x = {};
    });
}

void IterateTwoComponentsRuntime1MHalf(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<velocity>(e);

        if (i % 2) {
            registry.assign<position>(e);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        using component_type = typename world::component_typeid;
        component_type types[] = {registry.type<position>(), registry.type<velocity>()};

        ctx.measure([&]{
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });
    };

    test([&registry, &ctx](auto e) {
        registry.get<position>(e).x = {};
        registry.get<velocity>(e).x = {};
    });
}

void IterateTwoComponentsRuntime1MOne(run_context_t& ctx) {
    World registry;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<velocity>(e);

        if (i == 500'000L) {
            registry.assign<position>(e);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        using component_type = typename world::component_typeid;
        component_type types[] = {registry.type<position>(), registry.type<velocity>()};

        ctx.measure([&]{
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });
    };

    test([&registry, &ctx](auto e) {
        registry.get<position>(e).x = {};
        registry.get<velocity>(e).x = {};
    });
}

void IterateThreeComponents1M(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<position>(e);
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&]{
            registry.view<position, velocity, comp<0>>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateThreeComponents1MHalf(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);

        if (i % 2) {
            registry.assign<position>(e);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&]{
            registry.view<position, velocity, comp<0>>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateThreeComponents1MOne(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);
        registry.assign<comp<0>>(entity);

        if (i == 500'000L) {
            registry.assign<position>(entity);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&]{
            registry.view<position, velocity, comp<0>>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

//void IterateThreeComponentsNonOwningGroup1M(run_context_t& ctx) {
//    World registry;
//    registry.group<>(entt::get<position, velocity, comp<0>>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e);
//        registry.assign<velocity>(e);
//        registry.assign<comp<0>>(e);
//    }
//
//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//        registry.group<>(entt::get<position, velocity, comp<0>>).each(func);
//});
//        
//    };
//
//    test([](auto &... comp) {
//        ((comp.x = {}), ...);
//    });
//}
//
//void IterateThreeComponentsFullOwningGroup1M(run_context_t& ctx) {
//    World registry;
//    registry.group<position, velocity, comp<0>>();
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e);
//        registry.assign<velocity>(e);
//        registry.assign<comp<0>>(e);
//    }
//
//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//        registry.group<position, velocity, comp<0>>().each(func);
//});
//        
//    };
//
//    test([](auto &... comp) {
//        ((comp.x = {}), ...);
//    });
//}
//
//void IterateThreeComponentsPartialOwningGroup1M(run_context_t& ctx) {
//    World registry;
//    registry.group<position, velocity>(entt::get<comp<0>>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e);
//        registry.assign<velocity>(e);
//        registry.assign<comp<0>>(e);
//    }
//
//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//        registry.group<position, velocity>(entt::get<comp<0>>).each(func);
//});
//        
//    };
//
//    test([](auto &... comp) {
//        ((comp.x = {}), ...);
//    });
//}

void IterateThreeComponentsRuntime1M(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<position>(e);
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);
    }

    auto test = [&registry, &ctx](auto func) {
        using component_type = typename world::component_typeid;
        component_type types[] = {registry.type<position>(), registry.type<velocity>(), registry.type<comp<0>>()};

        ctx.measure([&]{
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });
    };

    test([&registry, &ctx](auto e) {
        registry.get<position>(e).x = {};
        registry.get<velocity>(e).x = {};
        registry.get<comp<0>>(e).x = {};
    });
}

void IterateThreeComponentsRuntime1MHalf(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);

        if (i % 2) {
            registry.assign<position>(e);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        using component_type = typename world::component_typeid;
        component_type types[] = {registry.type<position>(), registry.type<velocity>(), registry.type<comp<0>>()};

        ctx.measure([&]{
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });
    };

    test([&registry, &ctx](auto e) {
        registry.get<position>(e).x = {};
        registry.get<velocity>(e).x = {};
        registry.get<comp<0>>(e).x = {};
    });
}

void IterateThreeComponentsRuntime1MOne(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);

        if (i == 500'000L) {
            registry.assign<position>(e);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        using component_type = typename world::component_typeid;
        component_type types[] = {registry.type<position>(), registry.type<velocity>(), registry.type<comp<0>>()};

        ctx.measure([&]{
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });
    };

    test([&registry, &ctx](auto e) {
        registry.get<position>(e).x = {};
        registry.get<velocity>(e).x = {};
        registry.get<comp<0>>(e).x = {};
    });
}

void IterateFiveComponents1M_IMPL(world& registry, run_context_t& ctx) {
    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&]{
            registry.view<position, velocity, comp<0>, comp<1>, comp<2>>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateFiveComponents1M(run_context_t& ctx) {
    World registry;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<position>(e);
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);
        registry.assign<comp<1>>(e);
        registry.assign<comp<2>>(e);
    }

    IterateFiveComponents1M_IMPL(registry, ctx);

//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//            registry.view<position, velocity, comp<0>, comp<1>, comp<2>>().each(func);
//        });
//    };
//
//    test([](auto& ... comp) {
//        ((comp.x = {}), ...);
//    });
}

void IterateFiveComponents1MHalf(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);
        registry.assign<comp<1>>(e);
        registry.assign<comp<2>>(e);

        if (i % 2) {
            registry.assign<position>(e);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&]{
            for(auto e : registry.view<position, velocity, comp<0>, comp<1>, comp<2>>()) {}
//            registry.view<position, velocity, comp<0>, comp<1>, comp<2>>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateFiveComponents1MOne(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);
        registry.assign<comp<1>>(e);
        registry.assign<comp<2>>(e);

        if (i == 500'000L) {
            registry.assign<position>(e);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&]{
            registry.view<position, velocity, comp<0>, comp<1>, comp<2>>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

//void IterateFiveComponentsNonOwningGroup1M(run_context_t& ctx) {
//    World registry;
//    registry.group<>(entt::get<position, velocity, comp<0>, comp<1>, comp<2>>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e);
//        registry.assign<velocity>(e);
//        registry.assign<comp<0>>(e);
//        registry.assign<comp<1>>(e);
//        registry.assign<comp<2>>(e);
//    }
//
//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//        registry.group<>(entt::get<position, velocity, comp<0>, comp<1>, comp<2>>).each(func);
//});
//        
//    };
//
//    test([](auto &... comp) {
//        ((comp.x = {}), ...);
//    });
//}
//
//void IterateFiveComponentsFullOwningGroup1M(run_context_t& ctx) {
//    World registry;
//    registry.group<position, velocity, comp<0>, comp<1>, comp<2>>();
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e);
//        registry.assign<velocity>(e);
//        registry.assign<comp<0>>(e);
//        registry.assign<comp<1>>(e);
//        registry.assign<comp<2>>(e);
//    }
//
//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//        registry.group<position, velocity, comp<0>, comp<1>, comp<2>>().each(func);
//});
//        
//    };
//
//    test([](auto &... comp) {
//        ((comp.x = {}), ...);
//    });
//}
//
//void IterateFiveComponentsPartialFourOfFiveOwningGroup1M(run_context_t& ctx) {
//    World registry;
//    registry.group<position, velocity, comp<0>, comp<1>>(entt::get<comp<2>>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e);
//        registry.assign<velocity>(e);
//        registry.assign<comp<0>>(e);
//        registry.assign<comp<1>>(e);
//        registry.assign<comp<2>>(e);
//    }
//
//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//        registry.group<position, velocity, comp<0>, comp<1>>(entt::get<comp<2>>).each(func);
//});
//        
//    };
//
//    test([](auto &... comp) {
//        ((comp.x = {}), ...);
//    });
//}
//
//void IterateFiveComponentsPartialThreeOfFiveOwningGroup1M(run_context_t& ctx) {
//    World registry;
//    registry.group<position, velocity, comp<0>>(entt::get<comp<1>, comp<2>>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e);
//        registry.assign<velocity>(e);
//        registry.assign<comp<0>>(e);
//        registry.assign<comp<1>>(e);
//        registry.assign<comp<2>>(e);
//    }
//
//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//        registry.group<position, velocity, comp<0>>(entt::get<comp<1>, comp<2>>).each(func);
//});
//        
//    };
//
//    test([](auto &... comp) {
//        ((comp.x = {}), ...);
//    });
//}

void IterateFiveComponentsRuntime1M(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<position>(e);
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);
        registry.assign<comp<1>>(e);
        registry.assign<comp<2>>(e);
    }

    auto test = [&registry, &ctx](auto func) {
        using component_type = typename world::component_typeid;
        component_type types[] = {
                registry.type<position>(),
                registry.type<velocity>(),
                registry.type<comp<0>>(),
                registry.type<comp<1>>(),
                registry.type<comp<2>>()
        };

        ctx.measure([&]{
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });
    };

    test([&registry, &ctx](auto e) {
        registry.get<position>(e).x = {};
        registry.get<velocity>(e).x = {};
        registry.get<comp<0>>(e).x = {};
        registry.get<comp<1>>(e).x = {};
        registry.get<comp<2>>(e).x = {};
    });
}

void IterateFiveComponentsRuntime1MHalf(run_context_t& ctx) {
    World registry;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);
        registry.assign<comp<1>>(e);
        registry.assign<comp<2>>(e);

        if (i % 2) {
            registry.assign<position>(e);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        using component_type = typename world::component_typeid;
        component_type types[] = {
                registry.type<position>(),
                registry.type<velocity>(),
                registry.type<comp<0>>(),
                registry.type<comp<1>>(),
                registry.type<comp<2>>()
        };

        ctx.measure([&]{
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });
    };

    test([&registry, &ctx](auto e) {
        registry.get<position>(e).x = {};
        registry.get<velocity>(e).x = {};
        registry.get<comp<0>>(e).x = {};
        registry.get<comp<1>>(e).x = {};
        registry.get<comp<2>>(e).x = {};
    });
}

void IterateFiveComponentsRuntime1MOne(run_context_t& ctx) {
    World registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto e = registry.create();
        registry.assign<velocity>(e);
        registry.assign<comp<0>>(e);
        registry.assign<comp<1>>(e);
        registry.assign<comp<2>>(e);

        if (i == 500'000L) {
            registry.assign<position>(e);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        using component_type = typename world::component_typeid;
        component_type types[] = {
                registry.type<position>(),
                registry.type<velocity>(),
                registry.type<comp<0>>(),
                registry.type<comp<1>>(),
                registry.type<comp<2>>()
        };

        ctx.measure([&]{
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });
    };

    test([&registry, &ctx](auto e) {
        registry.get<position>(e).x = {};
        registry.get<velocity>(e).x = {};
        registry.get<comp<0>>(e).x = {};
        registry.get<comp<1>>(e).x = {};
        registry.get<comp<2>>(e).x = {};
    });
}

void IteratePathological(run_context_t& ctx) {
    pathological([&ctx](auto& registry, auto func) {
        ctx.measure([&]{
            registry.template view<position, velocity, comp<0>>().each(func);
        });
    });
}

//void IteratePathologicalNonOwningGroup(run_context_t& ctx) {
//    
//
//    pathological([](auto &registry, auto func) {
//        auto group = registry.template group<>(entt::get<position, velocity, comp<0>>);
//
//        ctx.measure([&]{
//        group.each(func);
//});
//        
//    });
//}
//
//void IteratePathologicalFullOwningGroup(run_context_t& ctx) {
//    
//
//    pathological([](auto &registry, auto func) {
//        auto group = registry.template group<position, velocity, comp<0>>();
//
//        ctx.measure([&]{
//        group.each(func);
//});
//        
//    });
//}
//
//void IteratePathologicalPartialOwningGroup(run_context_t& ctx) {
//    
//
//    pathological([](auto &registry, auto func) {
//        auto group = registry.template group<position, velocity>(entt::get<comp<0>>);
//
//        ctx.measure([&]{
//        group.each(func);
//});
//        
//    });
//}

//void SortSingle(run_context_t& ctx) {
//    World registry;
//
//    
//
//    for(std::uint64_t i = 0; i < 150000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e, i, i);
//    }
//
//    ctx.measure([&]{
//
//});
//    registry.sort<position>([](const auto &lhs, const auto &rhs) {
//        return lhs.x < rhs.x && lhs.y < rhs.y;
//    });
//
//    
//}

//void SortMulti(run_context_t& ctx) {
//    World registry;
//
//    
//
//    for(std::uint64_t i = 0; i < 150000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e, i, i);
//        registry.assign<velocity>(e, i, i);
//    }
//
//    registry.sort<position>([](const auto &lhs, const auto &rhs) {
//        return lhs.x < rhs.x && lhs.y < rhs.y;
//    });
//
//    ctx.measure([&]{
//
//});
//    registry.sort<velocity, position>();
//
//    
//}

//void AlmostSortedStdSort(run_context_t& ctx) {
//    World registry;
//    entt::e entities[3];
//
//    
//
//    for(std::uint64_t i = 0; i < 150000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e, i, i);
//
//        if(!(i % 50000)) {
//            entities[i / 50000] = e;
//        }
//    }
//
//    for(std::uint64_t i = 0; i < 3; ++i) {
//        registry.destroy(entities[i]);
//        const auto e = registry.create();
//        registry.assign<position>(e, 50000 * i, 50000 * i);
//    }
//
//    ctx.measure([&]{
//
//});
//    registry.sort<position>([](const auto &lhs, const auto &rhs) {
//        return lhs.x > rhs.x && lhs.y > rhs.y;
//    });
//
//    
//}

//void AlmostSortedInsertionSort(run_context_t& ctx) {
//    World registry;
//    entt::e entities[3];
//
//    
//
//    for(std::uint64_t i = 0; i < 150000L; i++) {
//        const auto e = registry.create();
//        registry.assign<position>(e, i, i);
//
//        if(!(i % 50000)) {
//            entities[i / 50000] = e;
//        }
//    }
//
//    for(std::uint64_t i = 0; i < 3; ++i) {
//        registry.destroy(entities[i]);
//        const auto e = registry.create();
//        registry.assign<position>(e, 50000 * i, 50000 * i);
//    }
//
//    ctx.measure([&]{
//
//});
//    registry.sort<position>([](const auto &lhs, const auto &rhs) {
//        return lhs.x > rhs.x && lhs.y > rhs.y;
//    }, entt::insertion_sort{});
//
//    
//}
}

#define RUN(x) ctx.run(#x, x)

void run_ecxx(run_context_t& ctx) {
    using namespace ecxx_b;
    RUN(construct);
    RUN(construct_many);
    RUN(construct_many_and_assign_components);
    RUN(construct_many_with_components);
    RUN(destroy);

    RUN(IterateSingleComponent1M);
    RUN(IterateSingleComponentRuntime1M);

    RUN(IterateTwoComponents1M);
    RUN(IterateTwoComponents1MHalf);
    RUN(IterateTwoComponents1MOne);
    RUN(IterateTwoComponentsRuntime1M);
    RUN(IterateTwoComponentsRuntime1MHalf);
    RUN(IterateTwoComponentsRuntime1MOne);

    RUN(IterateThreeComponents1M);
    RUN(IterateThreeComponents1MHalf);
    RUN(IterateThreeComponents1MOne);
    RUN(IterateThreeComponentsRuntime1M);
    RUN(IterateThreeComponentsRuntime1MHalf);
    RUN(IterateThreeComponentsRuntime1MOne);

    RUN(IterateFiveComponents1M);
    RUN(IterateFiveComponents1MHalf);
    RUN(IterateFiveComponents1MOne);
    RUN(IterateFiveComponentsRuntime1M);
    RUN(IterateFiveComponentsRuntime1MHalf);
    RUN(IterateFiveComponentsRuntime1MOne);

    RUN(IteratePathological);
}