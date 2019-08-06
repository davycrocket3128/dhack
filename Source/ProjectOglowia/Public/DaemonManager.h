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
#include "PeacegateDaemon.h"
#include "DaemonInfoAsset.h"
#include "PeacenetGameInstance.h"
#include "Process.h"
#include "DaemonManager.generated.h"

class USystemContext;
class APeacenetWorldStateActor;

UCLASS()
class PROJECTOGLOWIA_API UDaemonManager : public UObject
{
    GENERATED_BODY()

private:
    UPROPERTY()
    TArray<FDaemonInfo> AvailableDaemons;

    UPROPERTY()
    USystemContext* SystemContext;

    UPROPERTY()
    UProcess* SystemProcess;

    UPROPERTY()
    UPeacenetGameInstance* GameInstance;

    UPROPERTY()
    TArray<UPeacegateDaemon*> ActiveDaemons;

protected:
    UFUNCTION()
    bool IsDaemonEnabled(FName InName);

    UFUNCTION()
    void StartEnabledDaemons();

    UFUNCTION()
    UPeacegateDaemon* CreateDaemon(FName InName);

    UFUNCTION()
    void StartDaemon(UPeacegateDaemon* InDaemon);

public:
    UFUNCTION()
    void Initialize(USystemContext* InSystem, UProcess* InSystemProcess);

    UFUNCTION()
    void DaemonStoppedGracefully(UPeacegateDaemon* InDaemon);

    UFUNCTION()
    APeacenetWorldStateActor* GetPeacenet();

    UFUNCTION()
    USystemContext* GetSystemContext();

    UFUNCTION()
    bool IsActive();

    UFUNCTION()
    UProcess* Fork(FString InProcessName);

    UFUNCTION(BlueprintCallable)
    void DisableDaemon(FName InName);

    UFUNCTION(BlueprintCallable)
    void EnableDaemon(FName InName);

    UFUNCTION(BlueprintCallable)
    void StartDaemonByName(FName InName);

    UFUNCTION(BlueprintCallable)
    void StopDaemonByName(FName InName);

    UFUNCTION(BlueprintCallable)
    void RestartDaemonByName(FName InName);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsDaemonRunning(FName InDaemonName);
};