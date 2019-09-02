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
#include "FileRecord.h"
#include "UserContext.h"
#include "Process.h"
#include "ConsoleContext.h"
#include "CommandInfo.h"
#include "PeacegateProgramAsset.h"
#include "FileRecordUtils.generated.h"

class UDesktopWidget;

UCLASS(BlueprintType)
class PROJECTOGLOWIA_API UFileRecordUtils : public UObject {
    GENERATED_BODY()

protected:
    UFUNCTION()
    static bool GetExecutable(APeacenetWorldStateActor* Peacenet, FFileRecord Record, UPeacegateProgramAsset*& OutProgram, UCommandInfo*& OutCommandInfo);

    UFUNCTION()
    static void UpgradeFileRecord(const FComputer& InComputer, FFileRecord& FileRecord);

public:
    UFUNCTION()
    static FString GetTextContent(const FComputer& InComputer, FFileRecord& InFileRecord);

    UFUNCTION(BlueprintCallable, Category = "Peacegate")
    static bool LaunchProcess(FString InFilePath, TArray<FString> Arguments, UConsoleContext* InConsoleContext, UProcess* OwningProcess, UProcess*& OutProcess, UDesktopWidget* TargetDesktop = nullptr);
};