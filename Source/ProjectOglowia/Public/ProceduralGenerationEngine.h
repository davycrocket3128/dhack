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
#include "MarkovTrainingDataAsset.h"
#include "MarkovChain.h"
#include "Sex.h"
#include "ProceduralGenerationEngine.generated.h"

class ULootableFile;
class UPeacenetSiteAsset;
class UMarkovChain;
class UUserContext;
class UProtocolVersion;
class APeacenetWorldStateActor;
class UPeacenetSaveGame;
class UStoryCharacter;
class UStoryComputer;

UCLASS()
class PROJECTOGLOWIA_API UProceduralGenerationEngine : public UObject
{
    GENERATED_BODY()

private:
    UPROPERTY()
    bool JustUpdated = false;

    UPROPERTY()
    FComputer InvalidPC;

    UPROPERTY()
    FPeacenetIdentity InvalidIdentity;

    UPROPERTY()
    APeacenetWorldStateActor* Peacenet = nullptr;

    UPROPERTY()
    UPeacenetSaveGame* SaveGame = nullptr;

    UPROPERTY()
    UMarkovChain* MaleNameGenerator = nullptr;

    UPROPERTY()
    UMarkovChain* FemaleNameGenerator = nullptr;

    UPROPERTY()
    UMarkovChain* LastNameGenerator = nullptr;

    UPROPERTY()
    FRandomStream Rng;

    UPROPERTY()
    TArray<UStoryCharacter*> StoryCharacters;

    UPROPERTY()
    TArray<UStoryComputer*> StoryComputers;

    UPROPERTY()
    TArray<int> StoryCharactersToUpdate;

    UPROPERTY()
    TArray<int> StoryComputersToUpdate;

    UPROPERTY()
    int NonPlayerIdentitiesToGenerate = 0;

    UPROPERTY()
    TArray<UMarkovTrainingDataAsset*> MarkovTrainingData;

    UPROPERTY()
    TArray<FString> UsedHumanNames;

protected:
    UFUNCTION()
    void GenerateHumanName(FPeacenetIdentity& Identity);

    UFUNCTION()
    ESex DetermineSex();

    UFUNCTION()
    void ResetState();

    UFUNCTION()
    int CreateIdentity();

    UFUNCTION()
    void GenerateNPC();

    UFUNCTION()
    int CreateComputer();

    UFUNCTION()
    FPeacenetIdentity& GetIdentity(int Identity);

    UFUNCTION()
    FComputer& GetComputer(int EntityID);

    UFUNCTION()
    void CreateUser(FComputer& InComputer, FString InUsername, bool Sudoer);

    UFUNCTION()
    FString GenerateIPAddress();

    UFUNCTION()
    void UpdateStoryCharacter(UStoryCharacter* InStoryCharacter);

    UFUNCTION()
    void UpdateStoryComputer(UStoryComputer* InStoryComputer);

public:
    UFUNCTION()
    void SpawnServices(int ComputerID);

    UFUNCTION()
    bool IsDoneGeneratingStoryCharacters();

    UFUNCTION()
    void Setup(APeacenetWorldStateActor* InPeacenet);

    UFUNCTION()
    void Update(float DeltaTime);

    UFUNCTION()
    void GiveSaveGame(UPeacenetSaveGame* InSaveGame);
};