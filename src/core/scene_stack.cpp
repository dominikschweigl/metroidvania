#include "scene_stack.h"

void SceneStack::push(Factory factory)
{
	pending_.push_back({Op::Push, std::move(factory)});
}

void SceneStack::pop()
{
	pending_.push_back({Op::Pop, {}});
}

void SceneStack::replace(Factory factory)
{
	pending_.push_back({Op::Replace, std::move(factory)});
}

void SceneStack::clear()
{
	pending_.push_back({Op::Clear, {}});
}

void SceneStack::applyPending()
{
	for (auto &p : pending_) {
		switch (p.op) {
		case Op::Push:
			if (p.factory)
				scenes_.push_back(p.factory());
			break;
		case Op::Pop:
			if (!scenes_.empty())
				scenes_.pop_back();
			break;
		case Op::Replace: {
			auto next = p.factory ? p.factory() : nullptr;
			scenes_.clear();
			if (next)
				scenes_.push_back(std::move(next));
			break;
		}
		case Op::Clear:
			scenes_.clear();
			break;
		}
	}
	pending_.clear();
}

void SceneStack::handleEvent(const sf::Event &event, sf::RenderWindow &window)
{
	if (scenes_.empty())
		return;
	// Resize must reach every scene so layered scenes (e.g. a pause overlay
	// over gameplay) all update their views. Other events only go to the top.
	if (event.is<sf::Event::Resized>()) {
		for (auto &s : scenes_)
			s->handleEvent(event, window);
		return;
	}
	scenes_.back()->handleEvent(event, window);
}

void SceneStack::update(float deltaTime)
{
	if (scenes_.empty())
		return;
	// Walk from top down until a scene blocks updates below it.
	std::size_t start = scenes_.size();
	while (start > 0) {
		--start;
		if (!scenes_[start]->updateBelow())
			break;
	}
	for (std::size_t i = scenes_.size(); i-- > start;)
		scenes_[i]->update(deltaTime);
}

void SceneStack::draw(sf::RenderWindow &window)
{
	if (scenes_.empty())
		return;
	// Find the lowest transparent run from the top.
	std::size_t start = scenes_.size() - 1;
	while (start > 0 && scenes_[start]->isTransparent())
		--start;
	for (std::size_t i = start; i < scenes_.size(); ++i)
		scenes_[i]->draw(window);
}
