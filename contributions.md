## Emanuel Schöpf

### Allocated Tasks:
(2) two consecutive areas, each:
with its own tone (architecture, sprites, music, enemies etc)
with a handful of rooms each
a boss guarding an item / ability

(1) save / load with dedicated save points (rooms)

(1) map
shows discovered rooms and their doorways
shows where the player is currently located

### Highlights:

- Asynchronous Room loading - world.cpp:108-176
- Entitiy creation - world.cpp:43-102
- Entity Collision detection - entity_physics.cpp:49-179
- Save and load - serialize and deserialize functions

### Other implementations:
- rest of world.*
- room.*
- minimap.*
- ItemFactory.*
- EnemyFactory.*
- EnemyStateFactory.*

### Additional:
- Tileset, Room and Map designs to ensure a fun gaming experience.

## Dominik Schweigl

### Allocated Tasks:
(2) basic player movement
running left / right
jumping
interacting with game entities (doors, items etc)

(1) one or more advanced movement mechanics
wall jump, wall sliding
needs to be unlocked

(1) main combat
melee and/or ranged
hitting enemies
getting hit by enemies
(Shared with Lukas - combat_system.cpp)

### Highlights:

Player State System and 3-part Sprite
- `src/entities/player/player.cpp:263-272`
- `src/entities/player/states/player_state.h`
- Other state files in `src/entities/player/states/`

Player Inventory
- `src/entities/player/inventory.[h/cpp]`
- `src/items/slot_ref.h`

Items
- `src/items/item.h`
- `src/items/world_item.[h/cpp]`
- Other item files in `src/items`

### Other Implementations:
- Player Effects - `src/effects/*`
- Player Attack Abilities - `src/entities/player/abilities/*`
- Asset loading - `src/core/asset_manager.[h/cpp]`
- Game Action abstraction from key inputs - `src/core/input_manager.[h/cpp]`
- Key rebinding menu - `src/menus/key_bindings_menu.h`
- Inventory menu - `src/scenes/inventory_scene.[h/cpp]`
- Game item | health | effect hotbar - `src/ui/health_bar.[h/cpp]`, `src/ui/hotbar_hud.[h/cpp]`

### Additional:
Project Setup with vcpkg, clang-format, editorconfig, git, Cmake, Catch2.

## Lukas Schmölz

(2) enemies
attack the player character as it gets in range
variants with melee attacks
variants with ranged attacks
bosses are capable of using different attacks

Done in `src/entities/enemies/*`

(1) menus
- Main Menu
- Settings
- New Game
- Pause Menu
- Game Over
- Victory Menu
- Dialogue Menus

Done in `src/scenes/*` (except `inventory_scene` done by Dominik) and
in `src/ui/*`

### Highlights:

Central Combat System with per-attack hit deduplication
- `src/combat/combat_system.cpp:5-31`
- `src/combat/hitbox.h`

Enemy State Machines
- `src/entities/enemies/enemy_state.h`
- `src/entities/enemies/base_enemy.cpp:6-35`
- `src/entities/enemies/bosses/transistor_boss/*` for enemy implementation highlight

Scenes and Scene-Stack System
- `src/core/scene_actions.h`
- `src/core/scene_stack.cpp:23-89`

### Other Implementations:
- Central audio manager with volume settings - `src/core/audio_manager.*`
- Story dialogue scene - `src/scenes/dialogue_scene.*`, `src/scenes/story_snippets.*`

### Organisational:
CI pipeline in Github Repository
