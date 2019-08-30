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
#include "GameFramework/CheatManager.h"
#include "PeacenetCheatManager.generated.h"

class APeacenetWorldStateActor;

UCLASS(BlueprintType)
class PROJECTOGLOWIA_API UPeacenetCheatManager : public UCheatManager {
    GENERATED_BODY()

protected:
    UFUNCTION()
    void PrintMessage(FString Message);

    UFUNCTION()
    APeacenetWorldStateActor* GetPeacenet();

    UFUNCTION()
    UUserContext* GetPlayerUser();

public:
    /**
     * Prints whether or not The Peacenet is actively running gameplay simulations.
     */
    UFUNCTION(Exec)
    void PeacenetStat();

    UFUNCTION(Exec)
    void DnsTable();

    UFUNCTION(Exec)
    void Missions();

    UFUNCTION(Exec)
    void UnlockMission(FString MissionName);

    UFUNCTION(Exec)
    void Upgrades();

    UFUNCTION(Exec)
    void UnlockUpgrade(FString Upgrade);

    UFUNCTION(Exec)
    void ComputerInfo(int EntityID);

    UFUNCTION(Exec)
    void IdentityInfo(int EntityID);

    UFUNCTION(Exec)
    void ResetFirewall(int EntityID);

    UFUNCTION(Exec)
    void OpenPort(int EntityID, int Port);

    UFUNCTION(Exec)
    void CompleteMission();

    UFUNCTION(Exec)
    void EndMission();

    UFUNCTION(Exec)
    void Panic();

    UFUNCTION(Exec)
    void ForceTextMode();

    UFUNCTION(Exec)
    void UnforceTextMode();

    UFUNCTION(Exec)
    void SetCover(float Percentage);

    UFUNCTION(Exec)
    void Uncrash(int EntityID);

    UFUNCTION(Exec)
    void UncrashService(int EntityID, int Port);

    UFUNCTION(Exec)
    void LockUpgrade(FString Upgrade);

    UFUNCTION(Exec)
    void SetSkill(int Skill);

    UFUNCTION(Exec)
    void AddSkill(int Skill);
    
    UFUNCTION(Exec)
    void RemoveSkill(int Skill);

    UFUNCTION(Exec)
    void Lootables();

    UFUNCTION(Exec)
    void DropLootable(int EntityID, FString Lootable);

    UFUNCTION(Exec)
    void DropLootablePlayer(FString Lootable);

    UFUNCTION(Exec)
    void ForceRegenerateRandomLootables(int EntityID);

    UFUNCTION(Exec)
    void DnsResolve(FString Host);
};