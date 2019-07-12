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

bool UPeacenetSaveGame::CryptoWalletExists(FString InAddress)
{
	for(auto& Identity : this->Characters)
	{
		for(auto& Wallet : Identity.CryptoWallets)
		{
			if(Wallet.Address == InAddress) return true;
		}
	}
	return false;
}

bool UPeacenetSaveGame::RemoveFromWallet(FString InWallet, int InAmount)
{
	for(auto& Identity : this->Characters)
	{
		for(auto& Wallet : Identity.CryptoWallets)
		{
			if(Wallet.Address == InWallet)
			{
				if(Wallet.Amount < InAmount) return false;

				Wallet.Amount -= InAmount;
				return true;
			}
		}
	}

	return false;
}

bool UPeacenetSaveGame::AddToWallet(FString InWallet, int InAmount)
{
	for(auto& Identity : this->Characters)
	{
		for(auto& Wallet : Identity.CryptoWallets)
		{
			if(Wallet.Address == InWallet)
			{
				Wallet.Amount += InAmount;
				return true;
			}
		}
	}

	return false;
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

	TArray<int> RelationshipsToRemove;

	// Now we fix up character relationships.
	for(int i = 0; i < CharacterRelationships.Num(); i++)
	{
		FCharacterRelationship& Relationship = this->CharacterRelationships[i];
		if(!(CharacterIDMap.Contains(Relationship.FirstEntityID) && CharacterIDMap.Contains(Relationship.SecondEntityID)))
		{
			RelationshipsToRemove.Add(i);
			continue;
		}

		Relationship.FirstEntityID = CharacterIDMap[Relationship.FirstEntityID];
		Relationship.SecondEntityID = CharacterIDMap[Relationship.SecondEntityID];
	}

	while(RelationshipsToRemove.Num())
	{
		CharacterRelationships.RemoveAt(RelationshipsToRemove[0]);
		RelationshipsToRemove.RemoveAt(0);
		for(int i = 0; i < RelationshipsToRemove.Num(); i++)
		{
			RelationshipsToRemove[i]--;
		}
	}

	// Fix up adjacent nodes list.
	TArray<int> AdjacentsToRemove;
	for(int i = 0; i < AdjacentNodes.Num(); i++)
	{
		FAdjacentNode& Adjacent = this->AdjacentNodes[i];
		if(CharacterIDMap.Contains(Adjacent.NodeA) && CharacterIDMap.Contains(Adjacent.NodeB))
		{
			Adjacent.NodeA = CharacterIDMap[Adjacent.NodeA];
			Adjacent.NodeB = CharacterIDMap[Adjacent.NodeB];
		}
		else
		{
			AdjacentsToRemove.Add(i);
		}
	}

	int AdjacentsRemoved = 0;
	while(AdjacentsToRemove.Num())
	{
		AdjacentNodes.RemoveAt(AdjacentsToRemove[0] - AdjacentsRemoved);
		AdjacentsToRemove.RemoveAt(0);
		AdjacentsRemoved++;
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
		DomainNameMap.Remove(DeadDomains[0]);
		DeadDomains.RemoveAt(0);
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

TArray<int> UPeacenetSaveGame::GetAdjacents(int Node, EAdjacentLinkType LinkType)
{
	TArray<int> Ret;
	for(auto& Adjacent : this->AdjacentNodes)
	{
		if(Adjacent.NodeA == Node && (LinkType == EAdjacentLinkType::Bidirectional || LinkType == EAdjacentLinkType::AToB))
			Ret.Add(Adjacent.NodeB);
		else if(Adjacent.NodeB == Node && (LinkType == EAdjacentLinkType::Bidirectional || LinkType == EAdjacentLinkType::BToA))
			Ret.Add(Adjacent.NodeA);
	}
	return Ret;
}

bool UPeacenetSaveGame::RelatesWith(int InFirstEntity, int InSecondEntity)
{
	for(auto& Relationship : this->CharacterRelationships)
	{
		if(Relationship.FirstEntityID == InFirstEntity && Relationship.SecondEntityID == InSecondEntity)
			return true;
		if(Relationship.SecondEntityID == InFirstEntity && Relationship.FirstEntityID == InSecondEntity)
			return true;	
	}
	return false;
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

void UPeacenetSaveGame::AddAdjacent(int NodeA, int NodeB)
{
	check(!this->AreAdjacent(NodeA, NodeB));

	int CharAIndex, CharBIndex;
	FPeacenetIdentity CharA, CharB;
	bool ResultA = GetCharacterByID(NodeA, CharA, CharAIndex);
	bool ResultB = GetCharacterByID(NodeB, CharB, CharBIndex);
	
	check(ResultA && ResultB);

	FAdjacentNode Adjacent;
	Adjacent.NodeA = NodeA;
	Adjacent.NodeB = NodeB;
	AdjacentNodes.Add(Adjacent);
}

void UPeacenetSaveGame::RemoveAdjacent(int NodeA, int NodeB)
{
	check(this->AreAdjacent(NodeA, NodeB));

	for(int i = 0; i < this->AdjacentNodes.Num(); i++)
	{
		FAdjacentNode& Adjacent = AdjacentNodes[i];

		if((Adjacent.NodeA == NodeA && Adjacent.NodeB == NodeB) || (Adjacent.NodeA == NodeB && Adjacent.NodeB == NodeA))
		{
			AdjacentNodes.RemoveAt(i);
			return;
		}
	}
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

bool UPeacenetSaveGame::AreAdjacent(int NodeA, int NodeB)
{
	for(int i = 0; i < AdjacentNodes.Num(); i++)
	{
		FAdjacentNode& Adjacent = AdjacentNodes[i];
		if((Adjacent.NodeA == NodeA && Adjacent.NodeB == NodeB) || (Adjacent.NodeA == NodeB && Adjacent.NodeB == NodeA))
			return true;
	}
	return false;
}

void UPeacenetSaveGame::SetEntityPosition(int EntityID, FVector2D Position)
{
	for(auto& EntityPosition : EntityPositions)
	{
		if(EntityPosition.EntityID == EntityID)
		{
			EntityPosition.Position = Position;
			return;
		}
	}

	FPeacenetIdentity Identity;
	int IdentityIndex;
	bool result = this->GetCharacterByID(EntityID, Identity, IdentityIndex);
	check(result);

	FEntityPosition NewPos;
	NewPos.EntityID = EntityID;
	NewPos.Position = Position;
	EntityPositions.Add(NewPos);
}

bool UPeacenetSaveGame::GetPosition(int EntityID, FVector2D& OutPosition)
{
	for(auto& EntityPos : EntityPositions)
	{
		if(EntityPos.EntityID == EntityID)
		{
			OutPosition = EntityPos.Position;
			return true;
		}
	}
	return false;
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

bool UPeacenetSaveGame::LocationTooCloseToEntity(FVector2D InLocation, float InMinimumDistance)
{
	for(auto& Position : EntityPositions)
	{
		float dist = FMath::Abs(FVector2D::Distance(InLocation, Position.Position));
		if(dist <= InMinimumDistance)
			return true;
	}
	return false;
}