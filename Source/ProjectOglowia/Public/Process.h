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
#include "Process.generated.h"

class UProcessManager;
class ATerminalCommand;
class UProgram;

UCLASS()
class PROJECTOGLOWIA_API UProcess : public UObject
{
    friend UProcessManager;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FProcessKillEvent);

    GENERATED_BODY()

private:
    UPROPERTY()
    TArray<UProcess*> ChildProcesses;

    UPROPERTY()
    FString Path;

    UPROPERTY()
    FString Name;

    UPROPERTY()
    int ProcessID = 0;

    UPROPERTY()
    int UserID = 0;

    UPROPERTY()
    bool Dead = false;

    UPROPERTY()
    UProcessManager* ProcessManager;

private:
    UFUNCTION()
    void Initialize(UProcessManager* OwningProcessManager, int InUserID, FString InPath, FString InName);

public:
    UPROPERTY()
    FProcessKillEvent OnKilled;

    UPROPERTY()
    FProcessKillEvent OnTimeToEnd;

private:
    UFUNCTION()
    void KillInternal(bool NotifyProcessManager = true);

    UFUNCTION()
    void CullDeadChildren();

public:
    UFUNCTION()
    void Kill();

    UFUNCTION()
    bool Fork(FString FilePath, TArray<FString> Arguments);

    UFUNCTION()
    int GetProcessID();
};