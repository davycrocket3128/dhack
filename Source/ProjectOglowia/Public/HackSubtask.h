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
#include "UserContext.h"
#include "SystemContext.h"
#include "PeacenetSaveGame.h"
#include "PeacenetWorldStateActor.h"
#include "HackTask.h"
#include "HackSubtask.generated.h"

UCLASS(Blueprintable, BlueprintType, EditInlineNew, Abstract)
class PROJECTOGLOWIA_API UHackSubtask : public UObject
{
    GENERATED_BODY()

private:
    UPROPERTY()
    UHackTask* HackTask;

    UPROPERTY()
    bool IsFailed = false;

    UPROPERTY()
    bool IsFinished = false;

    UPROPERTY()
    FText FailMessage;

protected:
    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void SetObjectiveText(const FText& InObjectiveText); 

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void Fail(const FText& InFailMessage);

    UFUNCTION(BlueprintCallable, Category = "Hacking")
    void Finish();

    UFUNCTION(BlueprintImplementableEvent, Category = "Hacking")
    void OnTick(float InDeltaSeconds);

    UFUNCTION(BlueprintImplementableEvent, Category = "Hacking")
    void OnStart();

    UFUNCTION(BlueprintImplementableEvent, Category = "Hacking")
    void OnGameEvent(const FString& EventName, const TMap<FString, FString>& InEventData);

    virtual void NativeTick(float InDeltaSeconds) {}
    virtual void NativeStart() {}
    virtual void NativeGameEvent(FString EventName, TMap<FString, FString> InEventData) {}

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Hacking")
    UUserContext* GetPlayerUser();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Hacking")
    USystemContext* GetHackedSystem();

public:
    UFUNCTION()
    void Start(UHackTask* InHackTask);

    UFUNCTION()
    void Tick(float InDeltaSeconds);

    UFUNCTION()
    bool GetIsFinished();

    UFUNCTION()
    bool GetIsFailed();

    UFUNCTION()
    FText GetFailMessage();

    UFUNCTION()
    void GameEvent(FString EventName, TMap<FString, FString> InEventData);
};