#include "story_snippets.h"

namespace {
constexpr const char *PLAYER = "You";
constexpr const char *UNKNOWN = "???";
constexpr const char *SEGFAULT = "Segfault";
} // namespace

namespace StorySnippets {

std::vector<DialogueLine> newGameIntro()
{
	return {
	    {PLAYER, "Ugh... I must have dozed off on my keyboard again. That assignment really drained me..."},
	    {PLAYER, "Wait. This is not my desk. These walls... are those motherboard parts?"},
	    {PLAYER, "It seems I am inside my computer?! Okay. Stay calm. There has to be a way "
	             "out... I just have to find it."},
	};
}

std::vector<DialogueLine> beforeTransistorBoss()
{
	return {
	    {PLAYER, "There is a strange humming noice and it is getting louder."},
		{PLAYER, "Something in here must make... "},
	    {PLAYER, "That is the biggest transistor I have ever seen?! And it is glaring at me. No point in talking. "
	             "Seems like it is wanting to fight me"},
	};
}

std::vector<DialogueLine> afterTransistorBoss()
{
	return {
	    {PLAYER, "It just... discharged. All that raw power, gone in a single flash."},
	    {PLAYER, "It dropped some sort of.. USB Stick.. Maybe this will allow me to get out of this place?"},
	    {PLAYER, "I should look for a lock to stick it in and see what happens"},
	};
}

std::vector<DialogueLine> beforeSegfaultBoss()
{
	return {
	    {UNKNOWN, "A living process? In MY laboratory? How curious... you slipped past my firewall and terminated "
	              "my beautiful hardware. You will pay for this!"},
	    {PLAYER, "He is glitching apart and dragging the whole system down with him. "
	             "I get the feeling he is at the center of all those strange things happening to me."},
		{PLAYER, "Let's get this over with!"},
	};
}

std::vector<DialogueLine> epilogue()
{
	return {
	    {PLAYER, "...huh?"},
	    {PLAYER, "I'm back at my desk. It was a dream. Of course it was "},
	    {PLAYER, "Transistors don't glare at people and scientists don't glitch. Time to get back to studying..."},
	    {PLAYER, "...wait, why does it smell like burnt plastic?"},
	    {PLAYER, "My PC! It's fried! Smoke is curling out of the case. Maybe it wasn't just a dream after all."},
	};
}

std::vector<DialogueLine> gameOver()
{
	return {
	    {PLAYER, "My vision flickers..."},
	    {PLAYER, "Not like this. I'm not staying trapped in here. I have to try again."},
	};
}

} // namespace StorySnippets
