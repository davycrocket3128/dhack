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
#include "UserWidget.h"
#include "Text.h"
#include "TextProperty.h"
#include "PeacegateProgramAsset.h" 
#include "PeacegateFileSystem.h"
#include "GameRules.h"
#include "Workspace.h"
#include "DesktopWidget.generated.h"

class AMissionActor;
class USystemContext;
class UConsoleContext;
class APeacenetWorldStateActor;
class UUserContext;
class USystemUpgrade;
class UTutorialPromptState;
class UMissionAsset;
class UCommandInfo;
class ATerminalCommand;
class UProcess;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FActiveProgramCloseEvent);

USTRUCT()
struct PROJECTOGLOWIA_API FDesktopNotification
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FText Title;

	UPROPERTY()
	FText Message;

	UPROPERTY()
	UTexture2D* Icon;
};

/**
 * Base class of a Desktop environment.
 */
UCLASS(Blueprintable, Abstract)
class PROJECTOGLOWIA_API UDesktopWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY()
	bool SessionActive = false;

	UPROPERTY()
	FText LastTutorialText;

	UPROPERTY()
	FText ObjectiveText;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	bool SupportsTextMode();

	UFUNCTION(BlueprintImplementableEvent)
	bool DetermineIsTextMode();

	UFUNCTION(BlueprintImplementableEvent)
	void TextModeActivated();

	UFUNCTION(BlueprintImplementableEvent)
	void TextModeDeactivated();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Upgrades")
	bool IsUpgradeInstalled(USystemUpgrade* InUpgrade);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mission")
	bool IsMissionActive();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void StartMissionIfAvailable(UMissionAsset* InMissionAsset);

	UFUNCTION(BlueprintCallable)
	void ActivateSession(UUserContext* UserContext);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<UUserContext*> GetAvailableSessions();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mission")
	FText GetObjectiveText();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mission")
	FText GetMissionName();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mission")
	FText GetMissionAcquisition();

	UFUNCTION(BlueprintCallable, Category = "Mission")
	void AbandonMission();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System")
	USystemContext* GetSystemContext();

	UFUNCTION(BlueprintImplementableEvent)
	void OnMissionComplete(UMissionAsset* InMission);

	UFUNCTION(BlueprintImplementableEvent)
	void OnMissionFailed(AMissionActor* InMissionActor, const FText& InFailReason);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnKernelPanic();

public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTutorial(const FText& InTitle, const FText& InNewText, UTutorialPromptState* InTutorialPromptState);

	UFUNCTION()
	void KernelPanic();

	UFUNCTION(BlueprintCallable, Category = "Desktop")
	ATerminalCommand* ForkCommand(UCommandInfo* InCommandInfo, UConsoleContext* InConsoleContext);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UUserContext* GetUserContext();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsSessionActive();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Desktop")
	bool IsInTextMode();

	UFUNCTION(BlueprintCallable, Category = "Desktop")
	void ActivateTextMode();

	UFUNCTION(BlueprintCallable, Category = "Desktop")
	void DeactivateTextMode();

	UFUNCTION()
	void SetObjectiveText(const FText& InObjectiveText);

	UFUNCTION(BlueprintImplementableEvent, Category = "System")
	void ExecuteCommand(const FString& InCommand);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "System")
	void SwitchWorkspace(int InWorkspaceNumber);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stealthiness")
	float GetStealthiness();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "System")
	void MoveActiveProgramToWorkspace(int InWorkspaceNumber);

	UFUNCTION(BlueprintCallable, Category = "System")
	void CloseActiveProgram();

	FActiveProgramCloseEvent EventActiveProgramClose;

protected:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Workspaces")
	UWorkspace* GetWorkspace();

public:
	UFUNCTION(BlueprintCallable, Category = "Desktop")
	UProgram* SpawnProgramFromClass(TSubclassOf<UProgram> InClass, const FText& InTitle, UTexture2D* InIcon, bool InEnableMinimizeMaximize);

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	APeacenetWorldStateActor* GetPeacenet();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateMap();

	UPROPERTY(BlueprintReadOnly, Category = "Desktop")
	FPeacenetIdentity MyCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "Desktop")
	FComputer MyComputer;

private:
	bool bIsWaitingForNotification = false;

public:
	UPROPERTY()
	USystemContext * SystemContext;

	UFUNCTION()
	void SetWallpaper(UTexture2D* InTexture);

	UFUNCTION()
	void OnFilesystemOperation(EFilesystemEventType InType, FString InPath);

	UPROPERTY()
	int UserID = 0;

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY()
	FString UserHomeDirectory;

	UPROPERTY()
	UPeacegateFileSystem* Filesystem;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Desktop")
	FText TimeOfDay;

	UPROPERTY(BlueprintReadOnly, Category = "Desktop")
	FText NotificationTitle;

	UPROPERTY(BlueprintReadOnly, Category = "Desktop")
	FText NotificationMessage;
	
	UPROPERTY(BlueprintReadOnly, Category = "Desktop")
	UTexture2D* NotificationIcon;

	UPROPERTY(BlueprintReadOnly, Category = "Desktop")
	FString CurrentUsername;

	UPROPERTY(BlueprintReadOnly, Category = "Desktop")
	FString CurrentHostname;

	UPROPERTY(BlueprintReadOnly, Category = "Desktop")
	FString CurrentPeacenetName;

	UPROPERTY(BlueprintReadOnly, Category = "Desktop")
	UTexture2D* WallpaperTexture;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Game Rules")
	FGameRules GameRules;

public:
	UFUNCTION()
	void ShowProgramOnWorkspace(UProgram* InProgram);

	UFUNCTION(BlueprintImplementableEvent, Category = "Desktop")
	void ClearAppLauncherMainMenu();

	UFUNCTION(BlueprintImplementableEvent, Category = "Desktop")
	void ClearAppLauncherSubMenu();

	UFUNCTION(BlueprintImplementableEvent, Category = "Desktop")
	void AddAppLauncherMainMenuItem(const FText& ItemName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Desktop")
	void AddAppLauncherSubMenuItem(const FText& ItemName, const FText& ItemDescription, const FString& ExecutableName, const UTexture2D* Icon, const FText& InCategory);

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetOpenConnectionCount();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Desktop")
	FString GetIPAddress();

	UPROPERTY()
	TArray<FDesktopNotification> NotificationQueue;

	UFUNCTION(BlueprintCallable, Category = "Desktop")
	void FinishShowingNotification();

	UFUNCTION(BlueprintImplementableEvent, Category = "Desktop")
	void OnShowNotification();

	UFUNCTION(BlueprintCallable, Category = "Desktop")
	void ShowAppLauncherCategory(const FString& InCategory);

	UFUNCTION(BlueprintCallable, Category = "Desktop")
	bool OpenProgram(const FName InExecutableName, UProgram*& OutProgram);

public:
	UFUNCTION()
	void ResetAppLauncher();

	UFUNCTION(BlueprintCallable)
	void EnqueueNotification(const FText& InTitle, const FText& InMessage, UTexture2D* InIcon);

	UFUNCTION()
	void ResetDesktopIcons();


};
