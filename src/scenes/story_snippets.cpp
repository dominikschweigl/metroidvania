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
	             "Seems like it is ready to fight."},
	};
}

std::vector<DialogueLine> afterTransistorBoss()
{
	return {
	    {PLAYER, "It just... discharged. All that raw power, gone in a single flash."},
	    {PLAYER, "It dropped some sort of.. USB Stick.. Maybe this will allow me to get out of this place?"},
	    {PLAYER, "I should look for a lock to stick it in and see what happens."},
	};
}

std::vector<DialogueLine> beforeSegfaultBoss()
{
	return {
	    {UNKNOWN, "A living process? In MY laboratory? How curious... you slipped past my firewall and destroyed "
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
	    {PLAYER, "I'm back at my desk. It was a dream. Of course it was. "},
	    {PLAYER, "Transistors don't glare at people and scientists don't glitch. Time to get back to studying..."},
	    {PLAYER, "...wait, why does it smell like something burned?"},
	    {PLAYER, "My PC! It's fried! Smoke is coming out of the case. Maybe it wasn't just a dream after all..."},
	};
}

std::vector<DialogueLine> gameOver()
{
	return {
	    {PLAYER, "My vision flickers..."},
	    {PLAYER, "Not like this. I'm not staying trapped in here. I have to try again."},
	};
}

std::vector<DialogueLine> lockedDoorNoKey()
{
	return {
	    {PLAYER, "This door won't move at all. It's locked. I'll need to find something to open it."},
		{PLAYER, "Weird. The lock somehow looks like a USB Port."},
	};
}

std::vector<DialogueLine> lockedDoorWithKey()
{
	return {
	    {PLAYER, "I've got that USB stick on me. Let's see if plugging it in here does anything."},
	};
}

std::vector<DialogueLine> roomEnemiesRemain()
{
	return {
	    {PLAYER, "I can't just walk out with these things still after me. I have to deal with them first."},
	};
}

std::vector<DialogueLine> pickedUpHat()
{
	return {
	    {PLAYER, "A hat? Doesn't look like much, but... wait is he talking to me??."},
		{PLAYER, "It tells me to throw him around. I guess if that's what he wants..."},
	};
}

std::vector<DialogueLine> pickedUpGum()
{
	return {
	    {PLAYER, "Sticky chewing gum. Great, maybe I can use that to reach new heights. "
	             "This place keeps getting weirder and weirder."},
	};
}

} // namespace StorySnippets
