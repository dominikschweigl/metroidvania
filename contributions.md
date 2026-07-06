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
- Player Abilities - `src/entities/player/abilities/*`
- Asset loading - `src/core/asset_manager.*`
- Game Action abstraction from key inputs - `src/core/input_manager.*`
- Key rebinding menu - `src/menus/key_bindings_menu.h`
- Inventory menu - `src/scenes/inventory_scene.*`
- Game item | health | effect hotbar - `src/ui/health_bar.*`, `src/ui/hotbar_hud.*`

### Additional:
Project Setup with vcpkg, clang-format, editorconfig, git, Cmake, Catch2.

## Lukas Schmölz

(2) enemies
attack the player character as it gets in range
variants with melee attacks
variants with ranged attacks
bosses are capable of using different attacks

(1) menus
main menu
new game
load game
exit
pause menu
shows player stats
inventory management
continue
go to main menu
game over
player dies
player defeats final boss
