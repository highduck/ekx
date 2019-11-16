#include <iostream>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <entt/entity/registry.hpp>
#include "common.hpp"

namespace entt_b {

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
    entt::registry registry;

    for (std::uint64_t i = 0; i < 500'000L; i++) {
        const auto entity = registry.create();
        registry.assign<position>(entity);
        registry.assign<velocity>(entity);
        registry.assign<comp<0>>(entity);
    }

    for (auto i = 0; i < 10; ++i) {
        registry.each([i = 0, &registry](const auto entity) mutable {
            if (!(++i % 7)) { registry.reset<position>(entity); }
            if (!(++i % 11)) { registry.reset<velocity>(entity); }
            if (!(++i % 13)) { registry.reset<comp<0>>(entity); }
            if (!(++i % 17)) { registry.destroy(entity); }
        });

        for (std::uint64_t j = 0; j < 50'000L; j++) {
            const auto entity = registry.create();
            registry.assign<position>(entity);
            registry.assign<velocity>(entity);
            registry.assign<comp<0>>(entity);
        }
    }

    func(registry, [](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void construct(run_context_t& ctx) {
    entt::registry registry;
    ctx.measure([&] {
        for (std::uint64_t i = 0; i < 1'000'000L; i++) {
            registry.create();
        }
    });
}

void construct_many(run_context_t& ctx) {
    entt::registry registry;
    std::vector<entt::entity> entities(1'000'000L);

    ctx.measure([&] {
        registry.create(entities.begin(), entities.end());
    });
}

void construct_many_and_assign_components(run_context_t& ctx) {
    entt::registry registry;
    std::vector<entt::entity> entities(1'000'000L);

    ctx.measure([&] {
        registry.create(entities.begin(), entities.end());

        for (const auto entity: entities) {
            registry.assign<position>(entity);
            registry.assign<velocity>(entity);
        }
    });
}

void construct_many_with_components(run_context_t& ctx) {
    entt::registry registry;
    std::vector<entt::entity> entities(1'000'000L);

    ctx.measure([&] {
        registry.create<position, velocity>(entities.begin(), entities.end());
    });
}

void destroy(run_context_t& ctx) {
    entt::registry registry;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        registry.create();
    }

    ctx.measure([&] {
        registry.each([&registry, &ctx](auto entity) {
            registry.destroy(entity);
        });
    });
}

void IterateSingleComponent1M(run_context_t& ctx) {
    entt::registry registry;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<position>(entity);
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&] {
            registry.view<position>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateSingleComponentRuntime1M(run_context_t& ctx) {
    entt::registry registry;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<position>(entity);
    }

    auto test = [&registry, &ctx](auto func) {
        entt::component types[] = {registry.type<position>()};

        ctx.measure([&] {
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });
    };

    test([&registry, &ctx](auto entity) {
        registry.get<position>(entity).x = {};
    });
}

void IterateTwoComponents1M(run_context_t& ctx) {
    entt::registry registry;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<position>(entity);
        registry.assign<velocity>(entity);
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
    entt::registry registry;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);

        if (i % 2) {
            registry.assign<position>(entity);
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
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);

        if (i == 500'000L) {
            registry.assign<position>(entity);
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

//void IterateTwoComponentsNonOwningGroup1M(run_context_t& ctx) {
//    entt::registry registry;
//    registry.group<>(entt::get<position, velocity>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity);
//        registry.assign<velocity>(entity);
//    }
//
//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&]{
//        registry.group<>(entt::get<position, velocity>).each(func);
//}
//);
//        
//    };
//
//    test([](auto &... comp) {
//        ((comp.x = {}), ...);
//    });
//}
//
//void IterateTwoComponentsFullOwningGroup1M(run_context_t& ctx) {
//    entt::registry registry;
//    registry.group<position, velocity>();
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity);
//        registry.assign<velocity>(entity);
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
//    entt::registry registry;
//    registry.group<position>(entt::get<velocity>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity);
//        registry.assign<velocity>(entity);
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
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<position>(entity);
        registry.assign<velocity>(entity);
    }

    auto test = [&registry, &ctx](auto func) {
        entt::component types[] = {registry.type<position>(), registry.type<velocity>()};

        ctx.measure([&] {
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });

    };

    test([&registry, &ctx](auto entity) {
        registry.get<position>(entity).x = {};
        registry.get<velocity>(entity).x = {};
    });
}

void IterateTwoComponentsRuntime1MHalf(run_context_t& ctx) {
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);

        if (i % 2) {
            registry.assign<position>(entity);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        entt::component types[] = {registry.type<position>(), registry.type<velocity>()};

        ctx.measure([&] {
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });

    };

    test([&registry, &ctx](auto entity) {
        registry.get<position>(entity).x = {};
        registry.get<velocity>(entity).x = {};
    });
}

void IterateTwoComponentsRuntime1MOne(run_context_t& ctx) {
    entt::registry registry;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);

        if (i == 500'000L) {
            registry.assign<position>(entity);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        entt::component types[] = {registry.type<position>(), registry.type<velocity>()};

        ctx.measure([&] {
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });

    };

    test([&registry, &ctx](auto entity) {
        registry.get<position>(entity).x = {};
        registry.get<velocity>(entity).x = {};
    });
}

void IterateThreeComponents1M(run_context_t& ctx) {
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<position>(entity);
        registry.assign<velocity>(entity);
        registry.assign<comp<0 >>(entity);
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&] {
            registry.view<position, velocity, comp<0 >>().each(func);
        });

    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateThreeComponents1MHalf(run_context_t& ctx) {
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);
        registry.assign<comp<0 >>(entity);

        if (i % 2) {
            registry.assign<position>(entity);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&] {
            registry.view<position, velocity, comp<0 >>().each(func);
        });

    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateThreeComponents1MOne(run_context_t& ctx) {
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);
        registry.assign<comp<0 >>(entity);

        if (i == 500'000L) {
            registry.assign<position>(entity);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&] {
            registry.view<position, velocity, comp<0>>().each(func);
        });
    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

//void IterateThreeComponentsNonOwningGroup1M(run_context_t& ctx) {
//    entt::registry registry;
//    registry.group<>(entt::get<position, velocity, comp<0>>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity);
//        registry.assign<velocity>(entity);
//        registry.assign<comp<0>>(entity);
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
//    entt::registry registry;
//    registry.group<position, velocity, comp<0>>();
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity);
//        registry.assign<velocity>(entity);
//        registry.assign<comp<0>>(entity);
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
//    entt::registry registry;
//    registry.group<position, velocity>(entt::get<comp<0>>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity);
//        registry.assign<velocity>(entity);
//        registry.assign<comp<0>>(entity);
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
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<position>(entity);
        registry.assign<velocity>(entity);
        registry.assign<comp<0>>(entity);
    }

    auto test = [&registry, &ctx](auto func) {
        entt::component types[] = {registry.type<position>(), registry.type<velocity>(), registry.type<comp<0 >>()};

        ctx.measure([&] {
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });

    };

    test([&registry, &ctx](auto entity) {
        registry.get<position>(entity).x = {};
        registry.get<velocity>(entity).x = {};
        registry.get<comp<0 >>(entity).x = {};
    });
}

void IterateThreeComponentsRuntime1MHalf(run_context_t& ctx) {
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);
        registry.assign<comp<0 >>(entity);

        if (i % 2) {
            registry.assign<position>(entity);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        entt::component types[] = {registry.type<position>(), registry.type<velocity>(), registry.type<comp<0 >>()};

        ctx.measure([&] {
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });

    };

    test([&registry, &ctx](auto entity) {
        registry.get<position>(entity).x = {};
        registry.get<velocity>(entity).x = {};
        registry.get<comp<0 >>(entity).x = {};
    });
}

void IterateThreeComponentsRuntime1MOne(run_context_t& ctx) {
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);
        registry.assign<comp<0 >>(entity);

        if (i == 500'000L) {
            registry.assign<position>(entity);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        entt::component types[] = {registry.type<position>(), registry.type<velocity>(), registry.type<comp<0 >>()};

        ctx.measure([&] {
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });

    };

    test([&registry, &ctx](auto entity) {
        registry.get<position>(entity).x = {};
        registry.get<velocity>(entity).x = {};
        registry.get<comp<0 >>(entity).x = {};
    });
}

void IterateFiveComponents1M_IMPL(entt::registry& registry, run_context_t& ctx) {
    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&] {
            registry.view<position, velocity, comp<0>, comp<1>, comp<2 >>().each(func);
        });

    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateFiveComponents1M(run_context_t& ctx) {
    entt::registry registry;

    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<position>(entity);
        registry.assign<velocity>(entity);
        registry.assign<comp<0 >>(entity);
        registry.assign<comp<1 >>(entity);
        registry.assign<comp<2 >>(entity);
    }

    IterateFiveComponents1M_IMPL(registry, ctx);

//    auto test = [&registry, &ctx](auto func) {
//        ctx.measure([&] {
//            registry.view<position, velocity, comp<0>, comp<1>, comp<2 >>().each(func);
//        });
//
//    };
//
//    test([](auto& ... comp) {
//        ((comp.x = {}), ...);
//    });
}

void IterateFiveComponents1MHalf(run_context_t& ctx) {
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);
        registry.assign<comp<0 >>(entity);
        registry.assign<comp<1 >>(entity);
        registry.assign<comp<2 >>(entity);

        if (i % 2) {
            registry.assign<position>(entity);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&] {
            registry.view<position, velocity, comp<0>, comp<1>, comp<2 >>().each(func);
        });

    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}

void IterateFiveComponents1MOne(run_context_t& ctx) {
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);
        registry.assign<comp<0 >>(entity);
        registry.assign<comp<1 >>(entity);
        registry.assign<comp<2 >>(entity);

        if (i == 500'000L) {
            registry.assign<position>(entity);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        ctx.measure([&] {
            registry.view<position, velocity, comp<0>, comp<1>, comp<2 >>().each(func);
        });

    };

    test([](auto& ... comp) {
        ((comp.x = {}), ...);
    });
}
//
//void IterateFiveComponentsNonOwningGroup1M(run_context_t& ctx) {
//    entt::registry registry;
//    registry.group<>(entt::get<position, velocity, comp<0>, comp<1>, comp<2>>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity);
//        registry.assign<velocity>(entity);
//        registry.assign<comp<0>>(entity);
//        registry.assign<comp<1>>(entity);
//        registry.assign<comp<2>>(entity);
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
//    entt::registry registry;
//    registry.group<position, velocity, comp<0>, comp<1>, comp<2>>();
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity);
//        registry.assign<velocity>(entity);
//        registry.assign<comp<0>>(entity);
//        registry.assign<comp<1>>(entity);
//        registry.assign<comp<2>>(entity);
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
//    entt::registry registry;
//    registry.group<position, velocity, comp<0>, comp<1>>(entt::get<comp<2>>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity);
//        registry.assign<velocity>(entity);
//        registry.assign<comp<0>>(entity);
//        registry.assign<comp<1>>(entity);
//        registry.assign<comp<2>>(entity);
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
//    entt::registry registry;
//    registry.group<position, velocity, comp<0>>(entt::get<comp<1>, comp<2>>);
//
//    
//
//    for(std::uint64_t i = 0; i < 1'000'000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity);
//        registry.assign<velocity>(entity);
//        registry.assign<comp<0>>(entity);
//        registry.assign<comp<1>>(entity);
//        registry.assign<comp<2>>(entity);
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
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<position>(entity);
        registry.assign<velocity>(entity);
        registry.assign<comp<0 >>(entity);
        registry.assign<comp<1 >>(entity);
        registry.assign<comp<2 >>(entity);
    }

    auto test = [&registry, &ctx](auto func) {
        entt::component types[] = {
                registry.type<position>(),
                registry.type<velocity>(),
                registry.type<comp<0 >>(),
                registry.type<comp<1 >>(),
                registry.type<comp<2 >>()
        };

        ctx.measure([&] {
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });

    };

    test([&registry, &ctx](auto entity) {
        registry.get<position>(entity).x = {};
        registry.get<velocity>(entity).x = {};
        registry.get<comp<0 >>(entity).x = {};
        registry.get<comp<1 >>(entity).x = {};
        registry.get<comp<2 >>(entity).x = {};
    });
}

void IterateFiveComponentsRuntime1MHalf(run_context_t& ctx) {
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);
        registry.assign<comp<0 >>(entity);
        registry.assign<comp<1 >>(entity);
        registry.assign<comp<2 >>(entity);

        if (i % 2) {
            registry.assign<position>(entity);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        entt::component types[] = {
                registry.type<position>(),
                registry.type<velocity>(),
                registry.type<comp<0 >>(),
                registry.type<comp<1 >>(),
                registry.type<comp<2 >>()
        };

        ctx.measure([&] {
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });

    };

    test([&registry, &ctx](auto entity) {
        registry.get<position>(entity).x = {};
        registry.get<velocity>(entity).x = {};
        registry.get<comp<0 >>(entity).x = {};
        registry.get<comp<1 >>(entity).x = {};
        registry.get<comp<2 >>(entity).x = {};
    });
}

void IterateFiveComponentsRuntime1MOne(run_context_t& ctx) {
    entt::registry registry;


    for (std::uint64_t i = 0; i < 1'000'000L; i++) {
        const auto entity = registry.create();
        registry.assign<velocity>(entity);
        registry.assign<comp<0 >>(entity);
        registry.assign<comp<1 >>(entity);
        registry.assign<comp<2 >>(entity);

        if (i == 500'000L) {
            registry.assign<position>(entity);
        }
    }

    auto test = [&registry, &ctx](auto func) {
        entt::component types[] = {
                registry.type<position>(),
                registry.type<velocity>(),
                registry.type<comp<0 >>(),
                registry.type<comp<1 >>(),
                registry.type<comp<2 >>()
        };

        ctx.measure([&] {
            registry.runtime_view(std::begin(types), std::end(types)).each(func);
        });

    };

    test([&registry, &ctx](auto entity) {
        registry.get<position>(entity).x = {};
        registry.get<velocity>(entity).x = {};
        registry.get<comp<0 >>(entity).x = {};
        registry.get<comp<1 >>(entity).x = {};
        registry.get<comp<2 >>(entity).x = {};
    });
}

void IteratePathological(run_context_t& ctx) {
    pathological([&ctx](auto& registry, auto func) {
        ctx.measure([&] {
            registry.template view<position, velocity, comp<0>>
                    ().each(func);
        });
    });
}
//
//void IteratePathologicalNonOwningGroup(run_context_t& ctx) {
//    
//
//    pathological([](auto &registry, auto func) {
//        auto group = registry.template group<>(entt::get<position, velocity, comp<0>>);
//
//        ctx.measure([&]{
//        group.each(func);
//}
//);
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
//
//void SortSingle(run_context_t& ctx) {
//    entt::registry registry;
//
//    
//
//    for(std::uint64_t i = 0; i < 150000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity, i, i);
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
//
//void SortMulti(run_context_t& ctx) {
//    entt::registry registry;
//
//    
//
//    for(std::uint64_t i = 0; i < 150000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity, i, i);
//        registry.assign<velocity>(entity, i, i);
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
//
//void AlmostSortedStdSort(run_context_t& ctx) {
//    entt::registry registry;
//    entt::entity entities[3];
//
//    
//
//    for(std::uint64_t i = 0; i < 150000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity, i, i);
//
//        if(!(i % 50000)) {
//            entities[i / 50000] = entity;
//        }
//    }
//
//    for(std::uint64_t i = 0; i < 3; ++i) {
//        registry.destroy(entities[i]);
//        const auto entity = registry.create();
//        registry.assign<position>(entity, 50000 * i, 50000 * i);
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
//
//void AlmostSortedInsertionSort(run_context_t& ctx) {
//    entt::registry registry;
//    entt::entity entities[3];
//
//    
//
//    for(std::uint64_t i = 0; i < 150000L; i++) {
//        const auto entity = registry.create();
//        registry.assign<position>(entity, i, i);
//
//        if(!(i % 50000)) {
//            entities[i / 50000] = entity;
//        }
//    }
//
//    for(std::uint64_t i = 0; i < 3; ++i) {
//        registry.destroy(entities[i]);
//        const auto entity = registry.create();
//        registry.assign<position>(entity, 50000 * i, 50000 * i);
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

void run_entt(run_context_t& ctx) {
    using namespace entt_b;
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