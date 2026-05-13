#pragma once
#include <functional>
#include <memory>

class Scene;

// Actions that scenes use to request stack transitions.
class SceneActions {
  public:
	using Factory = std::function<std::unique_ptr<Scene>()>;

	virtual ~SceneActions() = default;

	virtual void push(Factory factory) = 0;
	virtual void pop() = 0;
	virtual void replace(Factory factory) = 0;
};
