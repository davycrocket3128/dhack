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


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PeacenetIdentity.h"
#include "DesktopWidget.h"
#include "Computer.h"
#include "Window.h"
#include "StoryCharacterIDMap.h"
#include "ComputerLink.h"
#include "Email.h"
#include "StoryComputerMap.h"
#include "PeacenetSaveGame.generated.h"

class UDesktopWidget;
class UMissionAsset;

/**
 * Represents a world state within Peacenet
 */
UCLASS()
class PROJECTOGLOWIA_API UPeacenetSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Missions")
	TArray<UMissionAsset*> CompletedMissions;

	UPROPERTY(VisibleAnywhere, Category = "Missions")
	TMap<FName, int> GameStats;

	UPROPERTY(VisibleAnywhere, Category = "Peacegate")
	bool IsNewGame = true;

	UPROPERTY(VisibleAnywhere, Category = "Peacegate")
	int PlayerComputerID = -1;

	UPROPERTY(VisibleAnywhere, Category = "Save Game")
	TArray<int> PlayerKnownPCs;

	UPROPERTY(VisibleAnywhere, Category = "Peacegate")
	int PlayerUserID = 0;

	UFUNCTION()
	int GetPlayerIdentity();

	UFUNCTION()
	bool PlayerHasComputer();

	UFUNCTION()
	bool PlayerHasIdentity();

	UPROPERTY(VisibleAnywhere)
	TArray<FStoryCharacterIDMap> StoryCharacterIDs;

	UPROPERTY(VisibleAnywhere, Category = "Peacegate")
	TSubclassOf<UWindow> WindowClass;

	UPROPERTY(VisibleAnywhere, Category = "Peacegate")
	FString GameTypeName;

	UPROPERTY(VisibleAnywhere, Category = "Entities")
	TArray<FComputer> Computers;

	UPROPERTY(VisibleAnywhere, Category = "Entities")
	TArray<FPeacenetIdentity> Characters;

	UPROPERTY(VisibleAnywhere, Category = "Save Game")
	TArray<FComputerLink> ComputerLinks;

	UPROPERTY(VisibleAnywhere, Category = "World")
	float EpochTime = 43200.f;

	UPROPERTY(VisibleAnywhere, Category = "Unlocks and Game State")
	TMap<FString, bool> Booleans;

	UPROPERTY(VisibleAnywhere, Category = "Networking")
	TMap<FString, FString> DomainNameMap;

	UPROPERTY(VisibleAnywhere, Category = "Missions")
	TArray<FName> Missions;

	UPROPERTY(VisibleAnywhere, Category = "Procgen")
	TArray<FStoryComputerMap> StoryComputerIDs;

	UPROPERTY(VisibleAnywhere, Category = "Procgen")
	TMap<FString, int> ComputerIPMap;
	
	UPROPERTY(VisibleAnywhere, Category = "Email")
	TArray<FEmail> EmailMessages;

	UPROPERTY(VisibleAnywhere, Category = "Master Password Table")
	int WorldSeed = -1;

	const float SECONDS_DAY_LENGTH = 86400.f;

	UFUNCTION()
	bool GetStoryCharacterID(UStoryCharacter* InStoryCharacter, int& OutIdentity);

	UFUNCTION()
	void ClearNonPlayerEntities();

	UFUNCTION()
	void AssignStoryCharacterID(UStoryCharacter* InStoryCharacter, int InIdentity);

	UFUNCTION()
	bool IsTrue(FString InKey);

	UFUNCTION()
	bool ResolveEmailAddress(FString InEmailAddress, int& OutEntityID);

	UFUNCTION()
	TArray<FEmail> GetEmailsForIdentity(FPeacenetIdentity& InIdentity);

	UFUNCTION()
	void SetValue(FString InKey, bool InValue);

	UFUNCTION()
	void FixEntityIDs();

	UFUNCTION()
	TArray<int> GetAllEntities();

	UFUNCTION()
	TArray<int> GetLinkedSystems(FComputer& InOrigin);

	UFUNCTION()
	int GetGameStat(FName InStatName);

	UFUNCTION()
	void SetGameStat(FName InStatName, int InValue);

	UFUNCTION()
	bool CharacterNameExists(FString CharacterName);

	UFUNCTION()
	bool DomainNameExists(FString InDomainName);

	UFUNCTION()
	bool IPAddressAllocated(FString InIPAddress);

	UFUNCTION()
		bool GetCharacterByID(int InEntityID, FPeacenetIdentity& OutCharacter, int& OutIndex);

	UFUNCTION()
	bool GetComputerByID(int InEntityID, FComputer& OutComputer, int& OutIndex);

	UFUNCTION()
	int GetSkillOf(FComputer& InComputer);
};