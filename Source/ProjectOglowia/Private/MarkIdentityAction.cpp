/********************************************************************************
 * The Peacenet - bit::phoenix("software");
 * 
 * MIT License
 *
 * Copyright (c) 2018-2019 Michael VanOverbeek, Declan Hoare, Ian Clary, 
 * Trey Smith, Richard Moch, Victor Tran and Warren Harris
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Contributors:
 *  - Michael VanOverbeek <alkaline@bitphoenixsoftware.com>
 *
 ********************************************************************************/

#include "MarkIdentityAction.h"
#include "UUserContext.h"
#include "USystemContext.h"
#include "PeacenetWorldStateActor.h"
#include "UPeacenetSaveGame.h"
#include "FPeacenetIdentity.h"
#include "UProceduralGenerationEngine.h"

void UMarkIdentityAction::NativeStart()
{
    // Crash if the identity is invalid.
    check(this->Identity);

    // Get the entity ID of the story character.
    int StoryIdentityID = 0;
    bool result = this->GetPlayerUser()->GetPeacenet()->SaveGame->GetStoryCharacterID(this->Identity, StoryIdentityID);
    check(result);

    // Get the player's entity ID.
    int PlayerEntity = this->GetPlayerUser()->GetOwningSystem()->GetCharacter().ID;

    // Get the story character's identity data.
    int IdentityIndex;
    FPeacenetIdentity IdentityData;
    result = this->GetPlayerUser()->GetPeacenet()->SaveGame->GetCharacterByID(StoryIdentityID, IdentityData, IdentityIndex);
    check(result);

    // If the player and the story character are not adjacent, make them adjacent.
    if(!this->GetPlayerUser()->GetPeacenet()->SaveGame->AreAdjacent(StoryIdentityID, PlayerEntity))
    {
        // Add the adjacent node.
        this->GetPlayerUser()->GetPeacenet()->SaveGame->AddAdjacent(StoryIdentityID, PlayerEntity);
    
        // If the story character doesn't have a position on the map, we need to use procgen
        // to assign one using the player as a pivot.
        FVector2D pos;
        if(!this->GetPlayerUser()->GetPeacenet()->SaveGame->GetPosition(StoryIdentityID, pos))
        {
            this->GetPlayerUser()->GetPeacenet()->GetProcgen()->GenerateIdentityPosition(this->GetPlayerUser()->GetOwningSystem()->GetCharacter(), this->GetPlayerUser()->GetPeacenet()->SaveGame->Characters[IdentityIndex]);
        }

        // Discover the node.
        this->GetPlayerUser()->GetPeacenet()->SaveGame->PlayerDiscoveredNodes.Add(StoryIdentityID);

        // Update the player's desktop map.
        this->GetPlayerUser()->GetPeacenet()->UpdateMaps();
    }

    // Mark the identity as story-important.
    IdentityData.IsMissionImportant = true;

    // Update the save file.
    this->GetPlayerUser()->GetPeacenet()->SaveGame->Characters[IdentityIndex] = IdentityData;

    // Complete the objective.
    this->Complete();
}