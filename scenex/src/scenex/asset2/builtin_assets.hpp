#pragma once

#include "asset_object.hpp"

namespace scenex {

class builtin_asset_resolver_t : public asset_type_resolver_t {
public:

    [[nodiscard]]
    asset_object_t* create_from_file(const std::string& path) const override;

    [[nodiscard]]
    asset_object_t* create(const std::string& path) const override;

    [[nodiscard]]
    asset_object_t* create_for_type(const std::string& type, const std::string& path) const override;
};

}

