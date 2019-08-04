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
#include "DocoptForUnrealBPLibrary.h"
#include "ConsoleContext.h"
#include "GameFramework/Actor.h"
#include "SystemContext.h"
#include "TerminalCommand.generated.h"

class UUserContext;
class UAddressBookContext;
class UCommandInfo;
class UProcess;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCommandCompletedEvent);

/**
 * A simple Unreal object containing functions needed to run a Peacegate OS Terminal command.
 */
UCLASS(Blueprintable)
class PROJECTOGLOWIA_API ATerminalCommand : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY()
	bool IsCompleting = false;

	UPROPERTY()
	UConsoleContext* Console;

	UPROPERTY()
	TArray<FString> ArgumentList;

	UPROPERTY()
	UProcess* MyProcess;

protected:
	UFUNCTION()
	void ProcessEnded();

	UFUNCTION()
	void SendGameEvent(FString InEventName, TMap<FString, FString> InEventData);

	UFUNCTION()
	void CompleteInternal(bool KillProcess = true);

public:
	UPROPERTY(BlueprintReadOnly, Category = "Terminal Command")
	UCommandInfo* CommandInfo;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FCommandCompletedEvent Completed;

	UFUNCTION(BlueprintCallable, Category = "Terminal Command")
	void RunCommand(UConsoleContext* InConsole, TArray<FString> Argv);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Terminal Command")
	UConsoleContext* GetConsole();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tutorial")
	bool IsTutorialActive();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void ShowTutorialIfNotSet(FString InSaveBoolean, const FText& InTutorialTitle, const FText& InTutorialText);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Save System")
	bool IsSet(FString InSaveBoolean);

protected:
	template<typename T>
	inline ATerminalCommand* SpawnCommand();

	UFUNCTION()
	int GetProcessID();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Arguments")
	TMap<FString, UDocoptValue*> ArgumentMap;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Arguments")
	FString CommandName;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Terminal Command")
	UUserContext* GetUserContext();

	virtual void NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments);

	UFUNCTION(BlueprintImplementableEvent, Category = "Terminal Command")
	void OnRunCommand(const UConsoleContext* InConsole, const TArray<FString>& InArguments);

public:
	UFUNCTION(BlueprintCallable, Category = "Terminal Command")
	void Complete();

public:
	UFUNCTION(BlueprintCallable, Category = "Terminal")
	static ATerminalCommand* CreateCommandFromAsset(UUserContext* InUserContext, UCommandInfo* InCommandInfo);
};

template<typename T>
inline ATerminalCommand* ATerminalCommand::SpawnCommand()
{
	FVector Location(0.0f, 0.0f, 0.0f);
	FRotator Rotation(0.0f, 0.0f, 0.0f);
 	FActorSpawnParameters SpawnInfo;

	return Cast<ATerminalCommand>(this->GetWorld()->SpawnActor<T>(Location, Rotation, SpawnInfo));
}