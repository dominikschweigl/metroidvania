#pragma once
#include "scene.h"
#include <functional>
#include <memory>
#include <vector>

// Owns the current stack of scenes and mediates transitions.
// Push/pop/replace are queued and applied between frames so scenes can request
// transitions from inside their own handlers without invalidating iteration.
class SceneStack {
  public:
	using Factory = std::function<std::unique_ptr<Scene>()>;

	void push(Factory factory);
	void pop();
	void replace(Factory factory); // pop all + push one (transition to a new top-level scene)
	void clear();

	bool empty() const
	{
		return scenes_.empty();
	}

	void handleEvent(const sf::Event &event, sf::RenderWindow &window);
	void update(float deltaTime);
	void draw(sf::RenderWindow &window);

	// Apply any queued push/pop/replace. Called once per frame by the driver.
	void applyPending();

  private:
	enum class Op { Push, Pop, Replace, Clear };
	struct PendingOp {
		Op op;
		Factory factory;
	};

	std::vector<std::unique_ptr<Scene>> scenes_;
	std::vector<PendingOp> pending_;
};
