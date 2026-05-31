#pragma once
#include "item.h"
#include <functional>
#include <span>

[[nodiscard]] std::span<const std::reference_wrapper<const Item>> registeredItems();
