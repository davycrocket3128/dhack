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


#include "ProceduralGenerationEngine.h"
#include "PeacenetSaveGame.h"
#include "Base64.h"
#include "FirewallRule.h"
#include "ComputerService.h"
#include "UserContext.h"
#include "LootableFile.h"
#include "ProtocolVersion.h"
#include "CommonUtils.h"
#include "WallpaperAsset.h"
#include "PeacegateFileSystem.h"
#include "StoryCharacter.h"
#include "MarkovChain.h"
#include "PeacenetSiteAsset.h"
#include "StoryComputer.h"
#include "PeacenetWorldStateActor.h"

void UProceduralGenerationEngine::Setup(APeacenetWorldStateActor* InPeacenet) {
    check(InPeacenet);
    check(!this->Peacenet);

    this->Peacenet = InPeacenet;

    // Load story entity data
    this->Peacenet->LoadAssets<UStoryCharacter>("StoryCharacter", this->StoryCharacters);
    this->Peacenet->LoadAssets<UStoryComputer>("StoryComputer", this->StoryComputers);
    
}

int UProceduralGenerationEngine::CreateIdentity() {
    int EntityID = 0;
    TArray<int> Taken;

    // Collect all taken entity IDs.
    for(auto Identity : this->SaveGame->Characters) {
        Taken.Add(Identity.ID);
    }
    
    // Find the lowest entity ID that's not taken.
    while(Taken.Contains(EntityID)) {
        EntityID++;
    }

    // Create the new identity.
    FPeacenetIdentity NewIdentity = FPeacenetIdentity();
    NewIdentity.ID = EntityID;

    // Add to the save file; return the ID.
    this->SaveGame->Characters.Add(NewIdentity);
    return NewIdentity.ID;
}

FPeacenetIdentity& UProceduralGenerationEngine::GetIdentity(int EntityID) {
    for(FPeacenetIdentity& Identity : this->SaveGame->Characters) {
        if(Identity.ID == EntityID) {
            return Identity;
        }
    }
    return this->InvalidIdentity;
}

void UProceduralGenerationEngine::UpdateStoryCharacter(UStoryCharacter* InStoryCharacter) {
    // Find the entity IF of the story character.
    int EntityID = -1;
    for(auto Map : this->SaveGame->StoryCharacterIDs) {
        if(Map.CharacterAsset == InStoryCharacter) {
            EntityID = Map.Identity;
            break;
        }
    }

    // If the entity is not valid then create a new one.
    if(EntityID == -1) {
        EntityID = this->CreateIdentity();
    }

    // Get a reference to the entity.
    FPeacenetIdentity& Identity = this->GetIdentity(EntityID);

    // Update the entity's data based on what's in the story character asset.
    Identity.CharacterName = InStoryCharacter->Name.ToString();
    Identity.CharacterType = EIdentityType::Story;
    Identity.ComputerID = -1;
    Identity.PreferredAlias = (InStoryCharacter->UseNameForEmail || !InStoryCharacter->EmailAlias.Len()) ? Identity.CharacterName : InStoryCharacter->EmailAlias;
    Identity.EmailAddress = UCommonUtils::Aliasify(Identity.PreferredAlias);

    // Assign the entity to the story character asset.
    this->SaveGame->AssignStoryCharacterID(InStoryCharacter, EntityID);
}

void UProceduralGenerationEngine::UpdateStoryComputer(UStoryComputer* InStoryComputer) {

}

void UProceduralGenerationEngine::Update(float DeltaTime) {
    check(this->Peacenet);

    // Update all story characters, then all story computers, but only do one per frame.
    if(StoryCharactersToUpdate.Num()) {
        int Index = StoryCharactersToUpdate.Pop();
        this->UpdateStoryCharacter(this->StoryCharacters[Index]);
    } else if(StoryComputersToUpdate.Num()) {
        int Index = StoryComputersToUpdate.Pop();
        this->UpdateStoryComputer(this->StoryComputers[Index]);
    } else {
        // Send mission emails if we've just updated.
        if(this->JustUpdated) {
            this->Peacenet->SendAvailableMissions();
            this->JustUpdated = false;
        }
    }
}

void UProceduralGenerationEngine::ResetState() {
    // Define some hardcoded constants for the world.
    const int MAX_NPC_IDENTITIES = 1000;
    const int MAX_EMAIL_SERVERS = 20;

    // Determine the world's seed and re-initialize the random number stream.
    if(this->SaveGame->IsNewGame) {
        this->SaveGame->WorldSeed = (int)FDateTime::Now().ToUnixTimestamp();
        this->SaveGame->IsNewGame = false;
    }
    this->Rng = FRandomStream(this->SaveGame->WorldSeed);

    // Determine which story characters need to spawn.
    this->StoryCharactersToUpdate.Empty();
    for(int i = 0; i < this->StoryCharacters.Num(); i++) {
        this->StoryCharactersToUpdate.Push(i);
    }

    // Determine which story computers need to spawn.
    this->StoryComputersToUpdate.Empty();
    for(int i = 0; i < this->StoryComputers.Num(); i++) {
        this->StoryComputersToUpdate.Push(i);
    }

    // Determine which systems need a public IP address.

    // Determine which domain names need to be spawned.

    // Dtermine how many identities need to spawn.

    // Determine how many computers need to spawn.

}

void UProceduralGenerationEngine::GiveSaveGame(UPeacenetSaveGame* InSaveGame) {
    if(this->SaveGame != InSaveGame) {
        this->SaveGame = InSaveGame;
        this->ResetState();
        this->JustUpdated = true;
    }
}

bool UProceduralGenerationEngine::IsDoneGeneratingStoryCharacters() {
    return !this->StoryCharactersToUpdate.Num();
}