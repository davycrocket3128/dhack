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


#include "PeacenetSaveGame.h"
#include "Computer.h"
#include "MissionAsset.h"

void UPeacenetSaveGame::ClearNonPlayerEntities()
{
	TArray<int> ComputersToRemove;
    TArray<int> CharactersToRemove;
    
    // Collect all computers that are NPC-owned.
    for(int i = 0; i < this->Computers.Num(); i++)
    {
        FComputer& Computer = this->Computers[i];
        if(Computer.OwnerType != EComputerOwnerType::Player)
        {
            ComputersToRemove.Add(i);
        }
    }

    // Collect all characters to remove.
    for(int i = 0; i < this->Characters.Num(); i++)
    {
        FPeacenetIdentity& Character = this->Characters[i];
        if(Character.CharacterType != EIdentityType::Player)
        {
            CharactersToRemove.Add(i);
        }
    }

    // Remove all characters..
    while(CharactersToRemove.Num())
    {
        this->Characters.RemoveAt(CharactersToRemove[0]);
        CharactersToRemove.RemoveAt(0);
        for(int i = 0; i < CharactersToRemove.Num(); i++)
            CharactersToRemove[i]--;
    }

    // Remove all computers.
    while(ComputersToRemove.Num())
    {
        this->Computers.RemoveAt(ComputersToRemove[0]);
        ComputersToRemove.RemoveAt(0);
        for(int i = 0; i < ComputersToRemove.Num(); i++)
            ComputersToRemove[i]--;
    }

    // Fix up entity IDs.
    this->FixEntityIDs();
}

int UPeacenetSaveGame::GetGameStat(FName InStatName)
{
	if(this->GameStats.Contains(InStatName))
		return this->GameStats[InStatName];
	return 0;
}

void UPeacenetSaveGame::SetGameStat(FName InStatName, int InValue)
{
	if(this->GameStats.Contains(InStatName))
		this->GameStats[InStatName] = InValue;
	else
		this->GameStats.Add(InStatName, InValue);
}

bool UPeacenetSaveGame::CharacterNameExists(FString CharacterName)
{
	for (auto& Character : Characters)
	{
		if (Character.CharacterName == CharacterName)
		{
			return true;
		}
	}
	return false;
}

int UPeacenetSaveGame::GetSkillOf(FComputer& InComputer)
{
	for(auto& Character : this->Characters)
	{
		if(Character.ComputerID == InComputer.ID)
		{
			// If the computer is owned by a personal character, the skill is equal to that of the character.
			return Character.Skill;
		}
	}

	return 0; // If the computer doesn't have an owning entity, its skill is 0.
}

bool UPeacenetSaveGame::DomainNameExists(FString InDomainName)
{
	return DomainNameMap.Contains(InDomainName);
}

bool UPeacenetSaveGame::IPAddressAllocated(FString InIPAddress)
{
	for(auto IP : this->ComputerIPMap)
	{
		if(IP.Key == InIPAddress)
			return true;
	}
	return false;
}

bool UPeacenetSaveGame::PlayerHasComputer()
{
	return this->PlayerComputerID != -1;
}

bool UPeacenetSaveGame::PlayerHasIdentity()
{
	if(!this->PlayerHasComputer())
		return false;

	FComputer pc;
	int i = -1;
	this->GetComputerByID(this->PlayerComputerID, pc, i);

	return pc.SystemIdentity != -1;
}

int UPeacenetSaveGame::GetPlayerIdentity()
{
	if(this->PlayerHasComputer())
	{
		FComputer pc;
		int i = -1;
		this->GetComputerByID(this->PlayerComputerID, pc, i);

		return pc.SystemIdentity;
	}
	return -1;
}

TArray<int> UPeacenetSaveGame::GetLinkedSystems(FComputer& InOrigin)
{
	TArray<int> Ret;
	for(auto& Link : this->ComputerLinks)
	{
		if(Link.ComputerA == InOrigin.ID)
		{
			Ret.Add(Link.ComputerB);
			continue;
		}
		if(Link.ComputerB == InOrigin.ID)
		{
			Ret.Add(Link.ComputerA);
			continue;
		}
	}
	for(int ID : Ret)
	{
		if(!PlayerKnownPCs.Contains(ID))
			PlayerKnownPCs.Add(ID);
	}
	return Ret;
}

void UPeacenetSaveGame::FixEntityIDs()
{
	TMap<int, int> ComputerIDMap;
	TMap<int, int> CharacterIDMap;

	// Set the IDs of every computer to match their array index.
	for(int i = 0; i < Computers.Num(); i++)
	{
		ComputerIDMap.Add(Computers[i].ID, i);
		Computers[i].ID = i;
	}

	// Fix up character IDs and reassign their computer IDs
	for(int i = 0; i < Characters.Num(); i++)
	{
		CharacterIDMap.Add(Characters[i].ID, i);
		Characters[i].ID = i;
		if(Characters[i].ComputerID != -1)
			Characters[i].ComputerID = ComputerIDMap[Characters[i].ComputerID];
	}

	// Fix up player computer ID
	if(ComputerIDMap.Contains(PlayerComputerID))
		PlayerComputerID = ComputerIDMap[PlayerComputerID];

	// Fix up IP addresses.
	TArray<FString> IPsToRemove;
	for(auto& IPAddress : ComputerIPMap)
	{
		if(ComputerIDMap.Contains(IPAddress.Value))
		{
			IPAddress.Value = ComputerIDMap[IPAddress.Value];
		}
		else
		{
			IPsToRemove.Add(IPAddress.Key);
		}
	}

	while(IPsToRemove.Num())
	{
		ComputerIPMap.Remove(IPsToRemove[0]);
		IPsToRemove.RemoveAt(0);
	}

	// A bug has been found where the game doesn't check
	// to see if a computer has an IP address before generating
	// a new one.  This cleanup will fix save files created
	// while this bug existed.
	//
	// A list of duplicate IP addresses to remove:
	TArray<FString> DuplicateIPs;

	// A list of entity IDs to track when finding duped IPs.
	TArray<int> DupeComputers;

	// Go through the IP address map to find duplicates.
	for(auto& IP : this->ComputerIPMap)
	{
		if(DupeComputers.Contains(IP.Value))
			DuplicateIPs.Add(IP.Key);
		else
			DupeComputers.Add(IP.Value);
	}

	// Remove all duplicate IPs.
	while(DuplicateIPs.Num())
	{
		this->ComputerIPMap.Remove(DuplicateIPs[0]);
		DuplicateIPs.RemoveAt(0);
	}

	// The above IP duplication cleanup will inherently fix
	// an issue where the game generates excessive amounts of
	// domain names.

	// What domain names shall we expire later?
	TArray<FString> DeadDomains;
	
	// Keeps track of domain name IPs so we can remove
	// any duplicates.
	//
	// Two domains shouldn't point to the same IP.
	TArray<FString> DomainIPs;

	// Loop through all domain names and find ones that
	// point to invalid IP addresses.
	for(auto& Domain : this->DomainNameMap)
	{
		if(!ComputerIPMap.Contains(Domain.Value))
			DeadDomains.Add(Domain.Key);
		
		if(DomainIPs.Contains(Domain.Value) && !DeadDomains.Contains(Domain.Key))
			DeadDomains.Add(Domain.Key);

		if(!DomainIPs.Contains(Domain.Value))
			DomainIPs.Add(Domain.Value);
	}

	// Removes all dead domain names from the game.
	while(DeadDomains.Num())
	{
		int Last = DeadDomains.Num() - 1;
		DomainNameMap.Remove(DeadDomains[Last]);
		DeadDomains.RemoveAt(Last);
	}

	// With the new identity system in Peacenet 0.2.x, the save file no longer stores
	// a direct link to the player identity - this information is now stored in the player's
	// computer data.
	//
	// This is also the case for NPC computers, their owning identities are stored within their
	// save data.  That way, an identity no longer requires a Peacegate computer - allowing us
	// to generate things like companies and story characters much easier.
	//
	// However this means we need to re-assign these identity IDs as the above cleanup code
	// throws everything out of whack.  Fucking hell.  This is the one part of the codebase
	// I fucking despise.  I just wanna be with Kaylin.  Fucking kill me.  - Michael
	for(int i = 0; i < this->Computers.Num(); i++)
	{
		// Skip over computers that don't have an owning identity - some computers are like this,
		// namely those that are part of company networks but not exposed to the public.
		if(this->Computers[i].SystemIdentity == -1) continue;

		// If our character ID map does not contain a record for the computer's System Identity,
		// the computer becomes orphaned.  This shouldn't happen unless I fucked something up like
		// usual at 2 AM with no caffeine but....
		//
		// ...it also prevents potential crashes.
		if(!CharacterIDMap.Contains(this->Computers[i].SystemIdentity))
		{
			this->Computers[i].SystemIdentity = -1;
			continue;
		}

		// And now we actually fucking reassign the motherfucking piece of shit entity ID.
		this->Computers[i].SystemIdentity = CharacterIDMap[this->Computers[i].SystemIdentity];
	}

	// Maybe THAT will stop the game from fucking wiping the player identity.

	// COMPUTER LINKS
	// ==============
	//
	// I'm too lazy to code a full LAN system for the game like I want to
	// so, for 0.3.0, this is how the player's able to scan for nearby devices
	// to hack.
	// 
	// Essentially, right now, Peacenet is one massive network.
	//
	// We're going to make sure all compuyter links are valid and,
	// if one has an invalid entity ID we can't map, then we will
	// remove the link.
	TArray<int> InvalidLinks;

	for(int i = 0; i < this->ComputerLinks.Num(); i++)
	{
		FComputerLink& Link = this->ComputerLinks[i];
		if(ComputerIDMap.Contains(Link.ComputerA) && ComputerIDMap.Contains(Link.ComputerB))
		{
			Link.ComputerA = ComputerIDMap[Link.ComputerA];
			Link.ComputerB = ComputerIDMap[Link.ComputerB];
		}
		else
		{
			InvalidLinks.Add(i);
		}
	}

	// Now we remove any invalid links.
	//
	// Gonna try something different in 0.3.x, invalid entities should
	// be removed in reverse.  That should fix bugs.
	// If it works then all the above entity removals in this function will do it too.
	while(InvalidLinks.Num())
	{
		int Last = InvalidLinks.Num() - 1;
		this->ComputerLinks.RemoveAt(InvalidLinks[Last]);
		InvalidLinks.RemoveAt(Last);
	}

	// Also do the same for known PCs.
	TArray<int> BadKnownPCs;
	for(int i = 0; i < this->PlayerKnownPCs.Num(); i++)
	{
		int id = PlayerKnownPCs[i];
		if(ComputerIDMap.Contains(id))
			PlayerKnownPCs[i] = ComputerIDMap[id];
		else
			BadKnownPCs.Add(i);
	}

	while(BadKnownPCs.Num())
	{
		int Last = BadKnownPCs.Num() - 1;
		this->PlayerKnownPCs.RemoveAt(BadKnownPCs[Last]);
		BadKnownPCs.RemoveAt(Last);
	}
}

bool UPeacenetSaveGame::IsTrue(FString InKey)
{
	if(this->Booleans.Contains(InKey))
		return this->Booleans[InKey];
	return false;
}

void UPeacenetSaveGame::SetValue(FString InKey, bool InValue)
{
	if(this->Booleans.Contains(InKey))
		this->Booleans[InKey] = InValue;
	else
		this->Booleans.Add(InKey, InValue);
}

bool UPeacenetSaveGame::GetCharacterByID(int InEntityID, FPeacenetIdentity & OutCharacter, int& OutIndex)
{
	int min = 0;
	int max = this->Characters.Num() - 1;

	int average = (min + max) / 2;

	while((max - min) >= 0)
	{
		FPeacenetIdentity& Character = Characters[average];

		if(Character.ID == InEntityID)
		{
			OutCharacter = Character;
			OutIndex = average;
			return true;
		}
		else if(Character.ID < InEntityID)
		{
			min = average + 1;
			average = (min + max) / 2;
		}
		else
		{
			max = average - 1;
			average = (min + max) / 2;
		}
	}

	return false;
}

bool UPeacenetSaveGame::GetComputerByID(int InEntityID, FComputer& OutComputer, int& OutIndex)
{
	int min = 0;
	int max = this->Computers.Num() - 1;

	int average = (min + max) / 2;

	while((max - min) >= 0)
	{
		FComputer& Computer = Computers[average];

		if(Computer.ID == InEntityID)
		{
			OutComputer = Computer;
			OutIndex = average;
			return true;
		}
		else if(Computer.ID < InEntityID)
		{
			min = average + 1;
			average = (min + max) / 2;
		}
		else
		{
			max = average - 1;
			average = (min + max) / 2;
		}
	}

	return false;
}

TArray<int> UPeacenetSaveGame::GetAllEntities()
{
	TArray<int> Ret;
	for(auto& Identity : Characters)
	{
		Ret.Add(Identity.ID);
	}
	return Ret;
}

bool UPeacenetSaveGame::GetStoryCharacterID(UStoryCharacter* InStoryCharacter, int& OutIdentity)
{
	check(InStoryCharacter);

	for(auto& IDMap : this->StoryCharacterIDs)
	{
		if(IDMap.CharacterAsset == InStoryCharacter)
		{
			FPeacenetIdentity IdentityData;
			int Index = 0;
			if(this->GetCharacterByID(IDMap.Identity, IdentityData, Index))
			{
				OutIdentity = IDMap.Identity;
				return true;
			}
		}
	}

	return false;
}

void UPeacenetSaveGame::AssignStoryCharacterID(UStoryCharacter* InStoryCharacter, int InIdentity)
{
	check(InStoryCharacter);

	FPeacenetIdentity IdentityData;
	int Index = 0;
	bool result = this->GetCharacterByID(InIdentity, IdentityData, Index);
	check(result);

	for(auto& Map : this->StoryCharacterIDs)
	{
		if(Map.CharacterAsset == InStoryCharacter)
		{
			Map.Identity = InIdentity;
			return;
		}
	}

	FStoryCharacterIDMap NewMap;
	NewMap.Identity = InIdentity;
	NewMap.CharacterAsset = InStoryCharacter;
	StoryCharacterIDs.Add(NewMap);
}

bool UPeacenetSaveGame::ResolveEmailAddress(FString InEmailAddress, int& OutEntityID)
{
	for(auto& Entity : this->Characters)
	{
		if(Entity.EmailAddress == InEmailAddress)
		{
			OutEntityID = Entity.ID;
			return true;
		}
	}

	OutEntityID = -1;
	return false;
}

TArray<FEmail> UPeacenetSaveGame::GetEmailsForIdentity(FPeacenetIdentity& InIdentity)
{
	TArray<FEmail> Ret;

	for(auto EmailMessage : this->EmailMessages)
	{
		if(EmailMessage.FromEntity == InIdentity.ID || EmailMessage.ToEntities.Contains(InIdentity.ID))
		{
			Ret.Add(EmailMessage);
		}
	}

	return Ret;
}