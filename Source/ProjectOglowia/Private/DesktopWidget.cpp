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


#include "DesktopWidget.h"
#include "PeacenetWorldStateActor.h"
#include "SystemContext.h"
#include "Computer.h"
#include "UserContext.h"
#include "GameTypeAsset.h"
#include "PeacenetIdentity.h"
#include "CommonUtils.h"
#include "WallpaperAsset.h"
#include "Program.h"
#include "TutorialPromptState.h"
#include "MissionActor.h"
#include "ConsoleContext.h"
#include "SystemUpgrade.h"
#include "MissionAsset.h"
#include "Process.h"
#include "CommandInfo.h"
#include "TerminalCommand.h"

UUserWidget* UDesktopWidget::CreateWidgetOwnedByDesktop(TSubclassOf<UUserWidget> InWidgetClass) {
	return CreateWidget<UUserWidget, APlayerController>(this->GetOwningPlayer(), InWidgetClass);
}

void UDesktopWidget::StartMissionIfAvailable(UMissionAsset* InMission) {
	if(this->IsMissionActive()) {
		return;
	}

	if(!InMission) {
		return;
	}

	if(this->GetPeacenet()->IsMissionCompleted(InMission)) {
		return;
	}

	this->GetPeacenet()->StartMission(InMission);
}

bool UDesktopWidget::IsUpgradeInstalled(USystemUpgrade* InUpgrade) {
	if(!this->IsSessionActive()) {
		return false;
	}
	return this->GetUserContext()->IsUpgradeInstalled(InUpgrade);
}

float UDesktopWidget::GetStealthiness() {
	return this->GetUserContext()->GetStealthiness();
}

void UDesktopWidget::ResetSession(UUserContext* InNewSession) {
	this->SessionActive = false;
	this->ActivateSession(InNewSession);
}

void UDesktopWidget::ShowProgramOnWorkspace(UProgram* InProgram) {
	// Make sure nothing is invalid.
	check(InProgram);
	check(this->GetWorkspace());
	check(InProgram->Window);

	// Add the program's window to our workspace.
	this->GetWorkspace()->AddWindow(InProgram->Window);

	// Let the mission system know that a program has been added to the desktop.
	this->GetPeacenet()->SendGameEvent("ProgramOpened", {
		{ "WindowTitle", InProgram->GetWindowTitle().ToString() },
		{ "ProcessID", FString::FromInt(InProgram->GetProcessID())}
	});
}

APeacenetWorldStateActor* UDesktopWidget::GetPeacenet() {
	return this->GetUserContext()->GetPeacenet();
}

UUserContext* UDesktopWidget::GetUserContext() {
	return this->SystemContext->GetUserContext(this->UserID);
}

void UDesktopWidget::CloseActiveProgram() {
	this->EventActiveProgramClose.Broadcast();
}

USystemContext* UDesktopWidget::GetSystemContext() {
	return this->SystemContext;
}

ATerminalCommand* UDesktopWidget::ForkCommand(UCommandInfo* InCommandInfo, UConsoleContext* InConsoleContext) {
	UProcess* Child = InConsoleContext->GetUserContext()->Fork(InCommandInfo->ID.ToString());

	return ATerminalCommand::CreateCommandFromAsset(InConsoleContext->GetUserContext(), InCommandInfo, Child);
}

UProgram * UDesktopWidget::SpawnProgramFromClass(TSubclassOf<UProgram> InClass, const FText& InTitle, UTexture2D* InIcon, bool InEnableMinimizeMaximize) {
	UWindow* OutputWindow = nullptr;

	UProgram* Program = UProgram::CreateProgram(this->SystemContext->GetPeacenet()->WindowClass, InClass, this->SystemContext->GetUserContext(this->UserID), OutputWindow, InTitle.ToString(), nullptr);

	OutputWindow->WindowTitle = InTitle;
	OutputWindow->Icon = InIcon;
	OutputWindow->EnableMinimizeAndMaximize = InEnableMinimizeMaximize;

	return Program;
}

void UDesktopWidget::NativeConstruct() {
	// Notify Blueprint when a mission is completed.
	TScriptDelegate<> MissionCompleteDelegate;
	MissionCompleteDelegate.BindUFunction(this, "OnMissionComplete");
	this->SystemContext->GetPeacenet()->MissionCompleteEvent.Add(MissionCompleteDelegate);

	// Let Blueprint know when a mission is failed.
	TScriptDelegate<> MissionFailDelegate;
	MissionFailDelegate.BindUFunction(this, "OnMissionFailed");
	this->SystemContext->GetPeacenet()->MissionFailed.Add(MissionFailDelegate);

	// Set the default wallpaper if our computer doesn't have one.
	if(!this->GetSystemContext()->GetComputer().CurrentWallpaper && !this->GetSystemContext()->GetComputer().HasWallpaperBeenSet) {
		// Go through all wallpaper assets.
		for(auto Wallpaper : this->GetSystemContext()->GetPeacenet()->Wallpapers) {
			// Is it the default wallpaper?
			if(Wallpaper->IsDefault) {
				// Assign the texture to the computer.
				this->GetSystemContext()->GetComputer().CurrentWallpaper = Wallpaper->WallpaperTexture;
			}
		}

		this->GetSystemContext()->GetComputer().HasWallpaperBeenSet = true;
	}

	TScriptDelegate<> MapUpdateDelegate;
	MapUpdateDelegate.BindUFunction(this, "UpdateMap");
	this->SystemContext->GetPeacenet()->MapsUpdated.Add(MapUpdateDelegate);

	if(this->SystemContext->GetComputer().Users.Num() > 1) {
		// We have more than a root user.
		// If we only have two users then we'll possess the first user that isn't root
		if(this->SystemContext->GetComputer().Users.Num() == 2) {
			this->ActivateSession(this->SystemContext->GetUserContext(this->SystemContext->GetComputer().Users[1].ID));
		} else {
			// We don't know what the fuck to do, it's up to the player
			this->SessionActive = false;
		}
	} else {
		this->ActivateSession(this->GetUserContext());
	}

	Super::NativeConstruct();
}

FText UDesktopWidget::GetObjectiveText() {
	return this->ObjectiveText;
}

void UDesktopWidget::SetObjectiveText(const FText& InObjectiveText) {
	this->ObjectiveText = InObjectiveText;
}

FText UDesktopWidget::GetMissionAcquisition() {
	if(!this->IsMissionActive()) {
		return FText::GetEmpty();
	}
	return this->GetPeacenet()->GetMissionActor()->GetMissionAsset()->Description;
}

FText UDesktopWidget::GetMissionName() {
	if(!this->IsMissionActive()) {
		return FText::GetEmpty();
	}
	return this->GetPeacenet()->GetMissionActor()->GetMissionName();
}

bool UDesktopWidget::IsMissionActive() {
	return this->GetPeacenet()->IsInMission();
}

void UDesktopWidget::AbandonMission() {
	if(this->IsMissionActive()) {
		this->GetPeacenet()->GetMissionActor()->FailCurrentTask(NSLOCTEXT("Mission", "Abandoned", "The mission was abandoned."));
	}
}

void UDesktopWidget::SetWallpaper(UTexture2D* InTexture) {
	this->GetSystemContext()->GetComputer().CurrentWallpaper = InTexture;
}

void UDesktopWidget::OnFilesystemOperation(EFilesystemEventType InType, FString InPath)
{
}

void UDesktopWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) {
	check(this->SystemContext);
	check(this->SystemContext->GetPeacenet());

	// If a session is active, and we can get a user context from it...
	if(this->IsSessionActive()) {
		UUserContext* User = this->GetUserContext();
		if(User) {
			// If the user context is invalid, kernel panic for now.
			// TODO: Graceful user log-out.
			if(!User->IsUserContextValid()) {
				this->KernelPanic();
			}
		}
	}

	this->MyCharacter = this->SystemContext->GetCharacter();
	this->MyComputer = this->SystemContext->GetComputer();

	// If a notification isn't currently active...
	if (!bIsWaitingForNotification) {
		// Do we have notifications left in the queue?
		if (this->NotificationQueue.Num()) {
			// Grab the first notification.
			FDesktopNotification Note = this->NotificationQueue[0];

			// Remove it from the queue - we have a copy.
			this->NotificationQueue.RemoveAt(0);

			// We're waiting again.
			bIsWaitingForNotification = true;

			// Set the notification values.
			this->NotificationTitle = Note.Title;
			this->NotificationMessage = Note.Message;
			this->NotificationIcon = Note.Icon;

			// Fire the event.
			this->OnShowNotification();
		}
	}

	// update username.
	this->CurrentUsername = this->SystemContext->GetUsername(this->UserID);

	// update hostname
	this->CurrentHostname = this->SystemContext->GetHostname();

	if(this->GetUserContext()->HasIdentity()) {
		// And now the Peacenet name.
		this->CurrentPeacenetName = this->MyCharacter.CharacterName;
	}

	this->TimeOfDay = this->SystemContext->GetPeacenet()->GetTimeOfDay();

	// Set our wallpaper.
	this->WallpaperTexture = this->GetSystemContext()->GetComputer().CurrentWallpaper;

	Super::NativeTick(MyGeometry, InDeltaTime);
}

bool UDesktopWidget::IsInTextMode() {
	if(!this->IsSessionActive()) {
		return false;
	}
	if(!this->SupportsTextMode()) {
		return false;
	}

	return this->DetermineIsTextMode();
}

void UDesktopWidget::ActivateTextMode() {
	if(!this->IsSessionActive()) {
		return;
	}
	if(this->IsInTextMode()) {
		return;
	}
	if(!this->SupportsTextMode()) {
		return;
	}

	this->TextModeActivated();
}

void UDesktopWidget::DeactivateTextMode() {
	if(!this->IsSessionActive()) {
		return;
	}
	if(!this->IsInTextMode()) {
		return;
	}
	if(!this->SupportsTextMode()) {
		return;
	}

	this->TextModeDeactivated();
}

void UDesktopWidget::ResetAppLauncher() {
	if(!IsSessionActive()) {
		return;
	}

	// Clear the main menu.
	this->ClearAppLauncherMainMenu();

	// Collect app launcher categories.
	TArray<FString> CategoryNames;
	for (auto Program : this->SystemContext->GetInstalledPrograms()) {
		if (!CategoryNames.Contains(Program->AppLauncherItem.Category.ToString())) {
			CategoryNames.Add(Program->AppLauncherItem.Category.ToString());
			this->AddAppLauncherMainMenuItem(Program->AppLauncherItem.Category);
		}
	}
}

bool UDesktopWidget::IsSessionActive() {
	return this->SessionActive;
}

TArray<UUserContext*> UDesktopWidget::GetAvailableSessions() {
	TArray<UUserContext*> Ret;

	if(this->IsSessionActive()) {
		return Ret;
	}

	for(auto user : this->SystemContext->GetComputer().Users) {
		if(user.ID != 0) {
			Ret.Add(SystemContext->GetUserContext(user.ID));
		}
	}

	return Ret;
}

void UDesktopWidget::ActivateSession(UUserContext* user) {
	if(this->SessionActive) {
		return;
	}

	this->UserID = user->GetUserID();
	this->SessionActive = true;

	this->ResetDesktopIcons();
	this->ResetAppLauncher();

	// Grab the user's home directory.
	this->UserHomeDirectory = this->SystemContext->GetUserHomeDirectory(this->UserID);

	// Get the filesystem context for this user.
	this->Filesystem = this->SystemContext->GetFilesystem(this->UserID);

	// Make sure that we intercept filesystem write operations that pertain to us.
	TScriptDelegate<> FSOperation;
	FSOperation.BindUFunction(this, "OnFilesystemOperation");

	if(!this->Filesystem->FilesystemOperation.Contains(FSOperation)) {
		this->Filesystem->FilesystemOperation.Add(FSOperation);
	}
}

int UDesktopWidget::GetOpenConnectionCount() {
	return this->SystemContext->GetOpenConnectionCount();
}

void UDesktopWidget::KernelPanic() {
	// Call the visual aspect of the kernel panic:
	this->OnKernelPanic();

	// If a session is active, deactivate it.
	if(this->IsSessionActive()) {
		this->UserID = -1;
		this->SessionActive = false;
	}
}

FString UDesktopWidget::GetIPAddress() {
	return this->GetSystemContext()->GetIPAddress();
}

void UDesktopWidget::EnqueueNotification(const FText & InTitle, const FText & InMessage, UTexture2D * InIcon) {
	FDesktopNotification note;
	note.Title = InTitle;
	note.Message = InMessage;
	note.Icon = InIcon;
	this->NotificationQueue.Add(note);
}

void UDesktopWidget::ResetDesktopIcons() {
	// stub
}

void UDesktopWidget::ShowAppLauncherCategory(const FString& InCategoryName) {
	if(!IsSessionActive()) {
		return;
	}

	// Clear the sub-menu.
	this->ClearAppLauncherSubMenu();

	// Add all the programs.
	for (auto Program : this->SystemContext->GetInstalledPrograms()) {
		if (Program->AppLauncherItem.Category.ToString() != InCategoryName) {
			continue;
		}

		// because Blueprints hate strings being passed by-value.
		FString ProgramName = Program->ID.ToString();

		// Add the item. Woohoo.
		this->AddAppLauncherSubMenuItem(Program->FullName, Program->Summary, ProgramName, Program->AppLauncherItem.Icon, Program->AppLauncherItem.Category);
	}
}

bool UDesktopWidget::OpenProgram(const FName InExecutableName, UProgram*& OutProgram) {
	if(!IsSessionActive()) {
		return false;
	}

	return this->SystemContext->OpenProgram(InExecutableName, OutProgram);
}

void UDesktopWidget::FinishShowingNotification() {
	bIsWaitingForNotification = false;
}