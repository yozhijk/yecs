#pragma once

#include "yecs/common.h"

namespace yecs
{
/**
 * @brief Helper class for easy type list building.
 *
 * Access to components requires type list specification.
 * This class allows for easy type list building.
 **/
template <typename... Args>
class ComponentTypesBuilder
{
public:
    ComponentTypesBuilder() : types{typeid(Args)...} {}
    // Return component types.
    ComponentTypes Build() const& { return types; }
    // Return component types with R-value optimization.
    ComponentTypes&& Build() && { return std::move(types); }

private:
    ComponentTypes types;
};
}  // namespace yecs