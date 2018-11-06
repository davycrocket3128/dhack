// Copyright (c) 2018 The Peacenet & Alkaline Thunder.

#include "UPeacenetSaveGame.h"

bool UPeacenetSaveGame::CharacterNameExists(FText CharacterName)
{
	for (auto& Character : Characters)
	{
		if (Character.CharacterName.EqualTo(CharacterName))
		{
			return true;
		}
	}
	return false;
}