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
#include "ManualPageAssetBase.h"
#include "ManualPageBuilder.h"
#include "SystemUpgrade.generated.h"

class USystemContext;

UCLASS(Blueprintable)
class PROJECTOGLOWIA_API USystemUpgrade : public UManualPageAssetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Upgrade")
    int RequiredSkillPoints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Upgrade")
    TArray<USystemUpgrade*> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Upgrade")
    bool CanUserUnlock = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "System Upgrade")
    bool UnlockedByDefault = false;

public:
    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool UpgradeIsUnlockable(UUserContext* InPeacegateUser);

    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool ShouldShowUpgradeInUserInterface(UUserContext* InPeacegateUser);

    UFUNCTION()
    bool DependenciesFulfilled(USystemContext* InSystemContext);

    UFUNCTION()
    void TriggerUnlock(USystemContext* InSystemContext);

    UFUNCTION()
    bool IsUnlocked(USystemContext* InSystemContext);

protected:
    virtual void BuildManualPage(UManualPageBuilder* InBuilder) override;

};