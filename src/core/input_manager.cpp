#include "input_manager.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

// clang-format off
const std::array<Binding, InputManager::actionCount> InputManager::defaultBindings = {{
    Binding{sf::Keyboard::Scancode::A},
    Binding{sf::Keyboard::Scancode::D},
    Binding{sf::Keyboard::Scancode::Space},
    Binding{sf::Keyboard::Scancode::LShift},
    Binding{sf::Mouse::Button::Right},
    Binding{sf::Keyboard::Scancode::PageUp},
    Binding{sf::Keyboard::Scancode::PageDown},
    Binding{sf::Mouse::Button::Left},
    Binding{sf::Keyboard::Scancode::F3},
    Binding{sf::Keyboard::Scancode::M},
    Binding{sf::Keyboard::Scancode::E},
    Binding{sf::Keyboard::Scancode::Q},
    Binding{sf::Keyboard::Scancode::F},
    Binding{sf::Keyboard::Scancode::Num1},
    Binding{sf::Keyboard::Scancode::Num2},
    Binding{sf::Keyboard::Scancode::Num3},
    Binding{sf::Keyboard::Scancode::Num4},
    Binding{sf::Keyboard::Scancode::Num5},
    Binding{sf::Keyboard::Scancode::Num6},
    Binding{sf::Keyboard::Scancode::Num7},
}};
// clang-format on

InputManager::InputManager() : bindings_(defaultBindings) {}

InputManager &InputManager::getInstance()
{
	static InputManager instance;
	return instance;
}

std::span<const InputManager::ActionMeta> InputManager::gameActions() noexcept
{
	return gameActionsMeta;
}

namespace {
constexpr std::array<GameAction, 7> hotbarSlotActionsArray = {
    GameAction::UseItemSlot1, GameAction::UseItemSlot2, GameAction::UseItemSlot3, GameAction::UseItemSlot4,
    GameAction::UseItemSlot5, GameAction::UseItemSlot6, GameAction::UseItemSlot7,
};
}

std::span<const GameAction> InputManager::hotbarSlotActions() noexcept
{
	return hotbarSlotActionsArray;
}

bool InputManager::matchesKey(const InputBinding &binding, const sf::Keyboard::Scancode scancode)
{
	return std::holds_alternative<sf::Keyboard::Scancode>(binding)
	       && std::get<sf::Keyboard::Scancode>(binding) == scancode;
}

bool InputManager::matchesKey(const Binding &binding, const sf::Keyboard::Scancode scancode)
{
	return matchesKey(binding.primary, scancode) || (binding.secondary && matchesKey(*binding.secondary, scancode));
}

bool InputManager::matchesMouse(const InputBinding &binding, const sf::Mouse::Button button)
{
	return std::holds_alternative<sf::Mouse::Button>(binding) && std::get<sf::Mouse::Button>(binding) == button;
}

bool InputManager::matchesMouse(const Binding &binding, const sf::Mouse::Button button)
{
	return matchesMouse(binding.primary, button) || (binding.secondary && matchesMouse(*binding.secondary, button));
}

void InputManager::handleEvent(const sf::Event &event)
{
	if (const auto *key = event.getIf<sf::Event::KeyPressed>()) {
		for (std::size_t i = 0; i < actionCount; ++i) {
			if (matchesKey(bindings_[i], key->scancode))
				wasPressed_[i] = true;
		}
		for (std::size_t i = 0; i < menuBindings.size(); ++i) {
			if (matchesKey(menuBindings[i], key->scancode))
				wasMenuPressed_[i] = true;
		}
	}
	if (const auto *mouse = event.getIf<sf::Event::MouseButtonPressed>()) {
		for (std::size_t i = 0; i < actionCount; ++i) {
			if (matchesMouse(bindings_[i], mouse->button))
				wasPressed_[i] = true;
		}
	}
}

void InputManager::clearFrameState()
{
	wasPressed_.fill(false);
	wasMenuPressed_.fill(false);
}

bool InputManager::isHeldBinding(const InputBinding &binding)
{
	if (std::holds_alternative<sf::Keyboard::Scancode>(binding))
		return sf::Keyboard::isKeyPressed(std::get<sf::Keyboard::Scancode>(binding));
	return sf::Mouse::isButtonPressed(std::get<sf::Mouse::Button>(binding));
}

bool InputManager::isHeld(const GameAction action) const
{
	return isHeldBinding(bindings_[idx(action)].primary);
}

bool InputManager::wasPressed(const GameAction action) const
{
	return wasPressed_[idx(action)];
}

bool InputManager::wasPressed(const MenuAction action) const
{
	return wasMenuPressed_[idx(action)];
}

bool InputManager::consume(const GameAction action)
{
	bool &flag = wasPressed_[idx(action)];
	const bool result = flag;
	flag = false;
	return result;
}

bool InputManager::consume(const MenuAction action)
{
	bool &flag = wasMenuPressed_[idx(action)];
	const bool result = flag;
	flag = false;
	return result;
}

void InputManager::rebind(const GameAction action, InputBinding newBinding)
{
	const std::size_t targetIdx = idx(action);
	for (std::size_t i = 0; i < actionCount; ++i) {
		if (i != targetIdx && bindings_[i].primary == newBinding)
			bindings_[i].primary = sf::Keyboard::Scancode::Unknown;
	}
	bindings_[targetIdx].primary = newBinding;
}

std::optional<GameAction> InputManager::findConflict(const InputBinding binding, const GameAction except) const
{
	const std::size_t exceptIdx = idx(except);
	for (std::size_t i = 0; i < actionCount; ++i) {
		if (i == exceptIdx)
			continue;
		if (bindings_[i].primary == binding) {
			return gameActionsMeta[i].action;
		}
	}
	return std::nullopt;
}

void InputManager::resetToDefaults()
{
	bindings_ = defaultBindings;
	wasPressed_.fill(false);
	wasMenuPressed_.fill(false);
}

std::string InputManager::bindingDisplayName(const InputBinding &binding)
{
	if (std::holds_alternative<sf::Mouse::Button>(binding)) {
		const auto index = static_cast<std::size_t>(std::get<sf::Mouse::Button>(binding));
		return std::string(index < MOUSE_BUTTON_NAMES.size() ? MOUSE_BUTTON_NAMES[index] : "Mouse?");
	}

	const auto scancode = std::get<sf::Keyboard::Scancode>(binding);
	if (scancode == sf::Keyboard::Scancode::Unknown) {
		return "(unbound)";
	}
	const sf::String description = sf::Keyboard::getDescription(scancode);
	return description.isEmpty() ? "(unbound)" : description.toAnsiString();
}

std::string InputManager::inputName(const GameAction action) const
{
	return bindingDisplayName(bindings_[idx(action)].primary);
}
