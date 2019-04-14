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
#include "Text.h"
#include "Dialog.h"
#include "PeacenetSiteWidget.h"
#include "Blueprint/UserWidget.h"
#include "RAMUsage.h"
#include "Program.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerAttentionNeededEvent, bool, PlaySound);

class UUserContext;
class USystemContext;
class UAddressBookContext;
class UConsoleContext;
class UWindow;

UCLASS(Blueprintable, BlueprintType)
class PROJECTOGLOWIA_API UProgram : public UUserWidget
{
	GENERATED_BODY()

private:
	// Learned this hack from the ShiftOS Visual Basic days.
	bool JustOpened = true;

	UPROPERTY()
	int ProcessID = 0;

	UPROPERTY()
	bool IsClosing = false;

public:
	UFUNCTION()
	void ActiveProgramCloseEvent();

protected:
	UFUNCTION()
	void HandleProcessEnded(const FPeacegateProcess& InProcess);

	UFUNCTION(BlueprintCallable, Category = "Desktop")
	void PushNotification(const FText& InNotificationMessage);

	UFUNCTION(BlueprintCallable, Category = "Program")
	void RequestPlayerAttention(bool PlaySound);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Program")
	UUserContext* GetUserContext();

	UPROPERTY(BlueprintAssignable, Category = "Program")
	FPlayerAttentionNeededEvent PlayerAttentionNeeded;

	UFUNCTION(BlueprintCallable, Category = "Peacegate")
	static UProgram* CreateProgram(const TSubclassOf<UWindow> InWindowClass, const TSubclassOf<UProgram> InProgramClass, UUserContext* InUserContext, UWindow*& OutWindow, FString InProcessName, ERAMUsage InRAMUsage, bool DoContextSetup = true);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = "true"))
	UWindow* Window;

	UFUNCTION(BlueprintCallable, Category = "Infobox")
	void ShowInfoWithCallbacks(const FText& InTitle, const FText& InMessage, const EInfoboxIcon InIcon, const EInfoboxButtonLayout ButtonLayout, const bool ShowTextInput, const FInfoboxDismissedEvent& OnDismissed, const FInfoboxInputValidator& ValidatorFunction);

	UFUNCTION(BlueprintCallable, Category = "Peacenet Sites")
	bool LoadPeacenetSite(FString InURL, UPeacenetSiteWidget*& OutWidget, EConnectionError& OutConnectionError);

	UFUNCTION(BlueprintCallable, Category = "Infobox")
	void ShowInfo(const FText& InTitle, const FText& InMessage, const EInfoboxIcon InIcon);

	UFUNCTION()
	void OwningWindowClosed();

	UFUNCTION(BlueprintCallable, Category = "File Management")
	void AskForFile(const FString InBaseDirectory, const FString InFilter, const EFileDialogType InDialogType, const FFileDialogDismissedEvent& OnDismissed);
	
	void SetupContexts();

	UFUNCTION(BlueprintCallable)
	void SetWindowMinimumSize(FVector2D InSize);

	UFUNCTION(BlueprintImplementableEvent, Category = "Program")
	void FileOpened(const FString& InPath);
protected:
	UFUNCTION()
	virtual void NativeProgramLaunched();

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaSeconds) override;

	// The console allows the program to output to a Terminal, or run Terminal Commands as its user.
	UPROPERTY(BlueprintReadOnly, Category = "Program")
	UConsoleContext* Console;
};
