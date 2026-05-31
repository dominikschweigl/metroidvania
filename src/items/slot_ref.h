#pragma once

enum class SlotKind { Hat, Gum, Grid, Hotbar };

struct SlotRef {
	SlotKind kind;
	int index = 0;
};
