#pragma once
#include "scene.h"
#include "scene_actions.h"
#include <memory>
#include <vector>

// Owns the current stack of scenes and mediates transitions.
class SceneStack : public SceneActions {
  public:
	void push(Factory factory) override;
	void pop() override;
	void replace(Factory factory) override; // pop all + push one
	void clear();

	bool empty() const { return scenes_.empty(); }

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
