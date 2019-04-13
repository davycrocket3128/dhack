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
#include "Computer.h"
#include "PeacenetIdentity.h"
#include "MarkovTrainingDataAsset.h"
#include "RandomStream.h"
#include "ProceduralGenerationEngine.generated.h"

class ULootableFile;
class UPeacenetSiteAsset;
class UMarkovChain;
class UUserContext;
class UProtocolVersion;
class APeacenetWorldStateActor;
class UPeacenetSaveGame;
class UStoryCharacter;

UCLASS()
class PROJECTOGLOWIA_API UProceduralGenerationEngine : public UObject
{
    GENERATED_BODY()

private:
    UPROPERTY()
    APeacenetWorldStateActor* Peacenet;

    UPROPERTY()
    TArray<UPeacenetSiteAsset*> PeacenetSites;

    UPROPERTY()
    FRandomStream RNG;

    UPROPERTY()
    TArray<UProtocolVersion*> ProtocolVersions;

    UPROPERTY()
    TArray<ULootableFile*> LootableFiles;

    UPROPERTY()
    UMarkovChain* MaleNameGenerator;

    UPROPERTY()
    UMarkovChain* DomainGenerator;

    UPROPERTY()
    UMarkovChain* UsernameGenerator;

    UPROPERTY()
    UMarkovChain* FemaleNameGenerator;

    UPROPERTY()
    UMarkovChain* LastNameGenerator;

private:
    TArray<FString> GetMarkovData(EMarkovTrainingDataUsage InUsage);

    UFUNCTION()
    void GenerateEmailServers();

public:
    UFUNCTION()
    void SpawnPeacenetSites();

    UFUNCTION()
    void GenerateIdentityPosition(FPeacenetIdentity& Pivot, FPeacenetIdentity& Identity);

    UFUNCTION()
    void UpdateStoryCharacter(UStoryCharacter* InStoryCharacter);

    UFUNCTION()
    UProtocolVersion* GetProtocol(UComputerService* InService, int InSkill);

    UFUNCTION()
    void GenerateAdjacentNodes(FPeacenetIdentity& InIdentity);

    UFUNCTION()
    void GenerateFirewallRules(FComputer& InComputer);

    UFUNCTION()
    void GenerateNonPlayerCharacters();

    UFUNCTION()
    void GenerateCharacterRelationships();

    UFUNCTION()
    FPeacenetIdentity& GenerateNonPlayerCharacter();

    UFUNCTION()
    FString GeneratePassword(int InLength);

    UFUNCTION()
    void ClearNonPlayerEntities();

    UFUNCTION()
    FComputer& GenerateComputer(FString InHostname, EComputerType InComputerType, EComputerOwnerType InOwnerType);

    UFUNCTION()
    FString GenerateIPAddress();

    UFUNCTION()
    void UpdateStoryIdentities();

    UFUNCTION()
    void GenerateUniqueWalletAddress(FCryptoWallet& InWallet, TArray<FString>& InExistingWallets);

    UFUNCTION()
    void GenerateCryptoWallets();

    UFUNCTION()
    void PlaceLootableFiles(UUserContext* InUserContext);

    UFUNCTION()
    void Initialize(APeacenetWorldStateActor* InPeacenet);

    UFUNCTION()
    FRandomStream& GetRNG();

    UFUNCTION()
    FString ChooseEmailDomain();
};