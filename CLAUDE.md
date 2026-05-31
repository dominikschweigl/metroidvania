# Metroidvania — Project Guide

## Overview
2D side-scrolling metroidvania in C++20 using SFML 3. Enemies and the player use state-machine architectures. The world is tile-based and loaded from Tiled JSON (.tmj) files.

## Build
```powershell
# Configure (first time only)
cmake --preset default-release   # or default-release

# Build
cmake --build ./build/release

# Tests
./build/release/metroidvania_tests.exe
```

Dependencies are managed via **vcpkg** (sfml, fmt, nlohmann-json, tinyxml2, catch2). Compiler: MinGW/GCC x64.

## Project Structure
```
src/
  core/           # Cross-cutting singletons (AssetManager)
  entities/
    base/         # BaseEnemy, EntityPhysics, EnemyState
    player/       # Player + 8 states + AttackLayer
    race_condition_slime/  # RaceConditionSlime + 5 states
  world/          # World, Room, Tile, JSON map loader
assets/
  images/         # Sprite sheets (32x32 px frames)
  audio/
data/
  maps/           # .tmj Tiled map files
  animations/
  entities/
tests/unit/       # Catch2 unit tests
```

## Architecture

### Entity pattern
Every entity runs a **state machine**. States are owned by value inside the entity as a `States` struct (no heap allocation during gameplay). The active state is a pointer into that pool.

```cpp
struct States { IdleState idle; ChaseState chase; ... };
States states;
EnemyState* currentState = &states.idle;
```

State transitions call `onExit()` on the old state and `onEnter()` on the new one.

### BaseEnemy template method
`BaseEnemy::update()` calls `onPreUpdate()` (timer tick hook) → state update → `EntityPhysics::simulateMovement()`. New enemies override `onPreUpdate()` and `draw()`.

### Physics
`EntityPhysics` (namespace of free functions in `src/entities/base/entity_physics.h`) handles gravity, horizontal/vertical collision resolution. All entities use the same functions.

### World / rooms
`World` holds a map of named rooms (`std::unordered_map<std::string, Room>`). Each room is a 2D grid of `Tile` structs with position, size, `isSolid`, and `textureId`. Maps are loaded from Tiled JSON (.tmj). Only one room is current at a time.

### AssetManager
Singleton in `src/core/asset_manager.h` / `src/core/asset_manager.cpp`. All texture loading goes through it — never load textures directly with `sf::Texture::loadFromFile`.

```cpp
const sf::Texture& tex = AssetManager::getInstance().getTexture(PLAYER_IDLE_HAT);
```

Add new assets: (1) add an entry to `TextureAsset` enum in `asset_manager.h`, (2) add the path to the `switch` in `texturePath()` in `asset_manager.cpp`.

## Coding Rules

### const
Use `const` wherever possible:
- Method parameters: `void foo(const TextureAsset asset)`
- Local variables that are not mutated: `const auto [it, inserted] = ...`
- Member functions that don't mutate state: `bool isActive() const noexcept`
- Reference members: `const sf::Texture& texture` (not pointer, not owned value)

### Header + implementation split
Every class must have a `.h` (interface only) and a `.cpp` (implementation). No method bodies in headers except single-line `noexcept` getters. No path strings, no asset loading logic, and no `#include` of implementation headers inside class headers.

### References over pointers
Prefer `const T&` over `const T*` wherever the value is guaranteed to be present:
- Member textures: `const sf::Texture& idle_texture;` (initialized from AssetManager in ctor init list)
- Function parameters that must be valid: `void draw(sf::RenderWindow& window)`
- To store references in containers use `std::reference_wrapper<const T>`

### Rule of Five
Any class managing a resource (or acting as a singleton) must explicitly declare all five special members:
```cpp
~MyClass()                         = default;  // or custom
MyClass(const MyClass&)            = delete;
MyClass& operator=(const MyClass&) = delete;
MyClass(MyClass&&)                 = delete;
MyClass& operator=(MyClass&&)      = delete;
```

### [[nodiscard]]
Apply to any function whose return value must be used: factory methods, getters, query functions.

### No raw texture loading in entities
Entity constructors initialize texture reference members via AssetManager in the ctor init list. No `loadFromFile` calls in entity code.

### Init list order matches declaration order
Member initialization order follows **declaration order** in the class, not the order in the init list. Always write the init list in declaration order to avoid `-Wreorder` warnings.

### Naming
Use descriptive names — short or cryptic names (`t`, `tmp`, `x2`, `mgr`) are not acceptable. Names should clearly express what the variable represents without needing a comment (`idleTexture`, `attackCooldown`, `isOnGround`).

### Error handling at boundaries only
Only validate at system boundaries (file loading, external input). Don't add null checks or fallbacks for internal invariants that can't be violated.

## Testing
Framework: **Catch2**. Tests live in `tests/unit/`. Run against the full `GAME_SOURCES` set (same sources as the main executable).

When adding a new enemy, add a corresponding `tests/unit/<enemy>_tests.cpp` and register it in `CMakeLists.txt`.
