#pragma once

enum class SlotKind { Hat, Gum, Backup, Grid, Hotbar };

struct SlotRef {
	SlotKind kind;
	int index = 0;
};
