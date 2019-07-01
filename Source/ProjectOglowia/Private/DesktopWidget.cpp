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
#include "PTerminalWidget.h"
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

void UDesktopWidget::StartMissionIfAvailable(UMissionAsset* InMission)
{
	if(this->IsMissionActive()) return;

	if(!InMission) return;

	if(this->GetPeacenet()->SaveGame->CompletedMissions.Contains(InMission))
		return;

	this->GetPeacenet()->StartMission(InMission);
}

bool UDesktopWidget::IsUpgradeInstalled(USystemUpgrade* InUpgrade)
{
	if(!this->IsSessionActive()) return false;
	return this->GetUserContext()->IsUpgradeInstalled(InUpgrade);
}

float UDesktopWidget::GetStealthiness()
{
	return this->GetUserContext()->GetStealthiness();
}

void UDesktopWidget::ShowProgramOnWorkspace(UProgram* InProgram)
{
	// Make sure nothing is invalid.
	check(InProgram);
	check(this->GetWorkspace());
	check(InProgram->Window);

	// Add the program's window to our workspace.
	this->GetWorkspace()->AddWindow(InProgram->Window);
}

UTutorialPromptState* UDesktopWidget::GetTutorialPrompt()
{
	return this->GetUserContext()->GetPeacenet()->GetTutorialState();
}

bool UDesktopWidget::IsTutorialActive()
{
	return this->GetTutorialPrompt()->IsPromptActive();
}

APeacenetWorldStateActor* UDesktopWidget::GetPeacenet()
{
	return this->GetUserContext()->GetPeacenet();
}

UUserContext* UDesktopWidget::GetUserContext()
{
	return this->SystemContext->GetUserContext(this->UserID);
}

void UDesktopWidget::CloseActiveProgram()
{
	this->EventActiveProgramClose.Broadcast();
}

USystemContext* UDesktopWidget::GetSystemContext()
{
	return this->SystemContext;
}

UProgram * UDesktopWidget::SpawnProgramFromClass(TSubclassOf<UProgram> InClass, const FText& InTitle, UTexture2D* InIcon, bool InEnableMinimizeMaximize, ERAMUsage InRAMUsage)
{
	UWindow* OutputWindow = nullptr;

	UProgram* Program = UProgram::CreateProgram(this->SystemContext->GetPeacenet()->WindowClass, InClass, this->SystemContext->GetUserContext(this->UserID), OutputWindow, InTitle.ToString(), InRAMUsage);

	OutputWindow->WindowTitle = InTitle;
	OutputWindow->Icon = InIcon;
	OutputWindow->EnableMinimizeAndMaximize = InEnableMinimizeMaximize;

	return Program;
}

void UDesktopWidget::NativeConstruct()
{
	// Bind events for Peacegate processes.
	TScriptDelegate<> ProcessStartedDelegate;
	TScriptDelegate<> ProcessEndedDelegate;
	ProcessStartedDelegate.BindUFunction(this, "ProcessStarted");
	ProcessEndedDelegate.BindUFunction(this, "ProcessEnded");
	this->SystemContext->ProcessStarted.Add(ProcessStartedDelegate);
	this->SystemContext->ProcessEnded.Add(ProcessEndedDelegate);

	// Notify Blueprint when a mission is completed.
	TScriptDelegate<> MissionCompleteDelegate;
	MissionCompleteDelegate.BindUFunction(this, "OnMissionComplete");
	this->SystemContext->GetPeacenet()->MissionCompleteEvent.Add(MissionCompleteDelegate);

	// Let Blueprint know when a mission is failed.
	TScriptDelegate<> MissionFailDelegate;
	MissionFailDelegate.BindUFunction(this, "OnMissionFailed");
	this->SystemContext->GetPeacenet()->MissionFailed.Add(MissionFailDelegate);

	// Set the default wallpaper if our computer doesn't have one.
	if(!this->GetSystemContext()->GetComputer().CurrentWallpaper && !this->GetSystemContext()->GetComputer().HasWallpaperBeenSet)
	{
		// Go through all wallpaper assets.
		for(auto Wallpaper : this->GetSystemContext()->GetPeacenet()->Wallpapers)
		{
			// Is it the default wallpaper?
			if(Wallpaper->IsDefault)
			{
				// Assign the texture to the computer.
				this->GetSystemContext()->GetComputer().CurrentWallpaper = Wallpaper->WallpaperTexture;
			}
		}

		this->GetSystemContext()->GetComputer().HasWallpaperBeenSet = true;
	}

	TScriptDelegate<> MapUpdateDelegate;
	MapUpdateDelegate.BindUFunction(this, "UpdateMap");
	this->SystemContext->GetPeacenet()->MapsUpdated.Add(MapUpdateDelegate);

	if(this->SystemContext->GetComputer().Users.Num() > 1)
	{
		this->SessionActive = false;
	}
	else 
	{
		this->ActivateSession(this->GetUserContext());
	}

	Super::NativeConstruct();
}

FText UDesktopWidget::GetObjectiveText()
{
	return this->ObjectiveText;
}

void UDesktopWidget::SetObjectiveText(const FText& InObjectiveText)
{
	this->ObjectiveText = InObjectiveText;
}

FText UDesktopWidget::GetMissionAcquisition()
{
	return this->GetPeacenet()->GetMissionActor()->GetMissionAsset()->Description;
}

FText UDesktopWidget::GetMissionName()
{
	return this->GetPeacenet()->GetMissionActor()->GetMissionName();
}

bool UDesktopWidget::IsMissionActive()
{
	return this->GetPeacenet()->IsInMission();
}

void UDesktopWidget::AbandonMission()
{
	if(this->IsMissionActive())
	{
		this->GetPeacenet()->GetMissionActor()->FailCurrentTask(NSLOCTEXT("Mission", "Abandoned", "The mission was abandoned."));
	}
}

UConsoleContext * UDesktopWidget::CreateConsole(UPTerminalWidget* InTerminal)
{
	// DEPRECATED IN FAVOUR OF UUserContext::CreateConsole().
	return this->SystemContext->GetUserContext(this->UserID)->CreateConsole(InTerminal);
}

void UDesktopWidget::SetWallpaper(UTexture2D* InTexture)
{
	this->GetSystemContext()->GetComputer().CurrentWallpaper = InTexture;
}

void UDesktopWidget::OnFilesystemOperation(EFilesystemEventType InType, FString InPath)
{
}

void UDesktopWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	check(this->SystemContext);
	check(this->SystemContext->GetPeacenet());

	this->MyCharacter = this->SystemContext->GetCharacter();
	this->MyComputer = this->SystemContext->GetComputer();

	// If a notification isn't currently active...
	if (!bIsWaitingForNotification)
	{
		// Do we have notifications left in the queue?
		if (this->NotificationQueue.Num())
		{
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

	if(this->GetUserContext()->HasIdentity())
	{
		// And now the Peacenet name.
		this->CurrentPeacenetName = this->MyCharacter.CharacterName;
	}

	this->TimeOfDay = this->SystemContext->GetPeacenet()->GetTimeOfDay();

	// Set our wallpaper.
	this->WallpaperTexture = this->GetSystemContext()->GetComputer().CurrentWallpaper;

	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UDesktopWidget::ResetAppLauncher()
{
	if(!IsSessionActive())
		return;

	// Clear the main menu.
	this->ClearAppLauncherMainMenu();

	// Collect app launcher categories.
	TArray<FString> CategoryNames;
	for (auto Program : this->SystemContext->GetInstalledPrograms())
	{
		if (!CategoryNames.Contains(Program->AppLauncherItem.Category.ToString()))
		{
			CategoryNames.Add(Program->AppLauncherItem.Category.ToString());
			this->AddAppLauncherMainMenuItem(Program->AppLauncherItem.Category);
		}
	}
}

bool UDesktopWidget::IsSessionActive()
{
	return this->SessionActive;
}

TArray<UUserContext*> UDesktopWidget::GetAvailableSessions()
{
	TArray<UUserContext*> Ret;

	if(this->IsSessionActive())
		return Ret;

	for(auto user : this->SystemContext->GetComputer().Users)
	{
		if(user.ID != 0)
		{
			Ret.Add(SystemContext->GetUserContext(user.ID));
		}
	}

	return Ret;
}

void UDesktopWidget::ActivateSession(UUserContext* user)
{
	if(this->SessionActive) return;

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

	if(!this->Filesystem->FilesystemOperation.Contains(FSOperation))
		this->Filesystem->FilesystemOperation.Add(FSOperation);
}

int UDesktopWidget::GetOpenConnectionCount()
{
	return this->SystemContext->GetOpenConnectionCount();
}

FString UDesktopWidget::GetIPAddress()
{
	return this->GetSystemContext()->GetIPAddress();
}

void UDesktopWidget::EnqueueNotification(const FText & InTitle, const FText & InMessage, UTexture2D * InIcon)
{
	FDesktopNotification note;
	note.Title = InTitle;
	note.Message = InMessage;
	note.Icon = InIcon;
	this->NotificationQueue.Add(note);
}

void UDesktopWidget::ResetDesktopIcons()
{
}

void UDesktopWidget::ShowAppLauncherCategory(const FString& InCategoryName)
{
	if(!IsSessionActive())
		return;

	// Clear the sub-menu.
	this->ClearAppLauncherSubMenu();

	// Add all the programs.
	for (auto Program : this->SystemContext->GetInstalledPrograms())
	{
		if (Program->AppLauncherItem.Category.ToString() != InCategoryName)
			continue;

		// because Blueprints hate strings being passed by-value.
		FString ProgramName = Program->ID.ToString();

		// Add the item. Woohoo.
		this->AddAppLauncherSubMenuItem(Program->FullName, Program->Summary, ProgramName, Program->AppLauncherItem.Icon, Program->AppLauncherItem.Category);
	}
}

bool UDesktopWidget::OpenProgram(const FName InExecutableName, UProgram*& OutProgram)
{
	if(!IsSessionActive())
		return false;

	return this->SystemContext->OpenProgram(InExecutableName, OutProgram);
}

void UDesktopWidget::FinishShowingNotification()
{
	bIsWaitingForNotification = false;
}