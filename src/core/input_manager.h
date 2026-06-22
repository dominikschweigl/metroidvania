#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>

enum class GameAction {
	MoveLeft,
	MoveRight,
	Jump,
	Sprint,
	ThrowHat,
	ZoomIn,
	ZoomOut,
	AttackMelee,
	ToggleDebugHitboxes,
	ToggleMinimap,
	OpenInventory,
	UseItem,
	Interact,
	UseItemSlot1,
	UseItemSlot2,
	UseItemSlot3,
	UseItemSlot4,
	UseItemSlot5,
	UseItemSlot6,
	UseItemSlot7
};
enum class MenuAction { Back, Confirm, NavigateUp, NavigateDown, NavigateLeft, NavigateRight };

using InputBinding = std::variant<sf::Keyboard::Scancode, sf::Mouse::Button>;

struct Binding {
	InputBinding primary;
	std::optional<InputBinding> secondary = std::nullopt;
};

class InputManager {
  public:
	struct ActionMeta {
		GameAction action;
		std::string_view displayName;
	};

	[[nodiscard]] static InputManager &getInstance();

	~InputManager() = default;
	InputManager(const InputManager &) = delete;
	InputManager &operator=(const InputManager &) = delete;
	InputManager(InputManager &&) = delete;
	InputManager &operator=(InputManager &&) = delete;

	void handleEvent(const sf::Event &event);
	void clearFrameState();

	[[nodiscard]] bool isHeld(GameAction action) const;

	/// True if the action was triggered this frame; does not consume the event.
	[[nodiscard]] bool wasPressed(GameAction action) const;
	[[nodiscard]] bool wasPressed(MenuAction action) const;

	/// True if the action was triggered this frame; clears the flag so later callers this frame see false.
	[[nodiscard]] bool consume(GameAction action);
	[[nodiscard]] bool consume(MenuAction action);

	/// Silences the action for the rest of this frame: isHeld() and wasPressed() both return false for it.
	void suppress(GameAction action);
	void suppress(MenuAction action);
	void suppressPlayerActions();

	void rebind(GameAction action, InputBinding newBinding);

	// Returns the action (other than `except`) that already has `binding` as its
	// primary, or std::nullopt if none. Used to warn the player before stealing.
	[[nodiscard]] std::optional<GameAction> findConflict(InputBinding binding, GameAction except) const;

	void resetToDefaults();

	[[nodiscard]] std::string inputName(GameAction action) const;
	[[nodiscard]] InputBinding getPrimaryBinding(GameAction action) const;
	[[nodiscard]] static std::span<const ActionMeta> gameActions() noexcept;
	[[nodiscard]] static std::span<const GameAction> hotbarSlotActions() noexcept;
	[[nodiscard]] static std::span<const GameAction> playerActions() noexcept;

  private:
	InputManager();

	static constexpr auto gameActionsMeta = std::to_array<ActionMeta>({
	    {GameAction::MoveLeft, "Move Left"},
	    {GameAction::MoveRight, "Move Right"},
	    {GameAction::Jump, "Jump"},
	    {GameAction::Sprint, "Sprint"},
	    {GameAction::ThrowHat, "Throw Hat"},
	    {GameAction::ZoomIn, "Zoom In"},
	    {GameAction::ZoomOut, "Zoom Out"},
	    {GameAction::AttackMelee, "Attack (Melee)"},
	    {GameAction::ToggleDebugHitboxes, "Toggle Debug Hitboxes"},
	    {GameAction::ToggleMinimap, "Toggle Minimap"},
	    {GameAction::OpenInventory, "Open Inventory"},
	    {GameAction::UseItem, "Use Item"},
	    {GameAction::Interact, "Interact"},
	    {GameAction::UseItemSlot1, "Use Item Slot 1"},
	    {GameAction::UseItemSlot2, "Use Item Slot 2"},
	    {GameAction::UseItemSlot3, "Use Item Slot 3"},
	    {GameAction::UseItemSlot4, "Use Item Slot 4"},
	    {GameAction::UseItemSlot5, "Use Item Slot 5"},
	    {GameAction::UseItemSlot6, "Use Item Slot 6"},
	    {GameAction::UseItemSlot7, "Use Item Slot 7"},
	});
	static constexpr auto menuBindings = std::to_array<Binding>({
	    Binding{sf::Keyboard::Scancode::Escape},
	    Binding{sf::Keyboard::Scancode::Enter, sf::Keyboard::Scancode::Space},
	    Binding{sf::Keyboard::Scancode::Up, sf::Keyboard::Scancode::W},
	    Binding{sf::Keyboard::Scancode::Down, sf::Keyboard::Scancode::S},
	    Binding{sf::Keyboard::Scancode::Left, sf::Keyboard::Scancode::A},
	    Binding{sf::Keyboard::Scancode::Right, sf::Keyboard::Scancode::D},
	});

	static constexpr auto actionCount = gameActionsMeta.size();
	static constexpr auto menuActionCount = menuBindings.size();

	std::array<Binding, actionCount> bindings_;
	std::array<bool, actionCount> wasPressed_ = {};
	std::array<bool, menuActionCount> wasMenuPressed_ = {};
	std::array<bool, actionCount> suppressed_ = {};
	std::array<bool, menuActionCount> menuSuppressed_ = {};

	static const std::array<Binding, actionCount> defaultBindings;

	static constexpr std::size_t idx(GameAction action) noexcept { return static_cast<std::size_t>(action); }
	static constexpr std::size_t idx(MenuAction action) noexcept { return static_cast<std::size_t>(action); }

	static bool isHeldBinding(const InputBinding &binding);
	static std::string bindingDisplayName(const InputBinding &binding);
	static bool matchesKey(const InputBinding &binding, sf::Keyboard::Scancode scancode);
	static bool matchesKey(const Binding &binding, sf::Keyboard::Scancode scancode);
	static bool matchesMouse(const InputBinding &binding, sf::Mouse::Button button);
	static bool matchesMouse(const Binding &binding, sf::Mouse::Button button);

	static constexpr std::array<std::string_view, sf::Mouse::ButtonCount> MOUSE_BUTTON_NAMES = {
	    "Mouse Left", "Mouse Right", "Mouse Middle", "Mouse 4", "Mouse 5"};

	friend struct InputManagerTestAccess;
};
