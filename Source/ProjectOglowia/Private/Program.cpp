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


#include "Program.h"
#include "ConsoleContext.h"
#include "Window.h"
#include "SystemContext.h"
#include "UserContext.h"
#include "CommonUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "PeacenetWorldStateActor.h"
#include "PeacenetSiteAsset.h"
#include "PeacegateFileSystem.h"
#include "DesktopWidget.h"
#include "Process.h"
#include "CommandInfo.h"
#include "TerminalCommand.h"
#include "Process.h"
#include "FileRecordUtils.h"
#include "UserContext.h"
#include "LootableFile.h"
#include "CommonUtils.h"
#include "PeacenetSiteWidget.h"

ATerminalCommand* UProgram::ForkCommand(UCommandInfo* InCommandInfo, UConsoleContext* InConsole) {
	// Fork a Peacegate process for this command.
	UProcess* Child = MyProcess->Fork(InCommandInfo->ID.ToString());
	
	// Spawn the command actor that the process owns.
	ATerminalCommand* Command = ATerminalCommand::CreateCommandFromAsset(InConsole->GetUserContext(), InCommandInfo, Child);

	// And give it back to the caller.
	return Command;
}

bool UProgram::LoadPeacenetSite(FString InURL, UPeacenetSiteWidget*& OutWidget, EConnectionError& OutConnectionError) {
	// The computer that holds the Peacenet Site we want.
	FComputer Computer;
	
	// Try and resolve the host to a computer.
	bool result = this->GetUserContext()->DnsResolve(InURL, Computer, OutConnectionError);
	if(!result) {
		return false;
	}
	
	// If the computer doesn't have a Peacenet Site assigned to it we'll refuse.
	if(!Computer.PeacenetSite || !Computer.PeacenetSite->PeacenetSite) {
		OutConnectionError = EConnectionError::ConnectionRefused;
		return false;
	}

	// Construct the widget associated with the Peacenet site.
	OutWidget = CreateWidget<UPeacenetSiteWidget, APlayerController>(this->GetOwningPlayer(), Computer.PeacenetSite->PeacenetSite);
	OutWidget->Setup(Computer.PeacenetSite, this);
	OutConnectionError = EConnectionError::None;
	return true;
}

UUserContext* UProgram::GetUserContext() {
	return this->Window->GetUserContext();
}

void UProgram::PushNotification(const FText & InNotificationMessage) {
	this->GetUserContext()->GetDesktop()->EnqueueNotification(this->Window->WindowTitle, InNotificationMessage, this->Window->Icon);
}

void UProgram::RequestPlayerAttention(bool PlaySound) {
	this->PlayerAttentionNeeded.Broadcast(PlaySound);
}

bool UProgram::OpenFile(FString FilePath, bool Fork) {
	UPeacegateFileSystem* Filesystem = this->GetUserContext()->GetFilesystem();
	if(Filesystem->FileExists(FilePath)) {
		FFileRecord Record = Filesystem->GetFileRecord(FilePath);
		if(Record.RecordType == EFileRecordType::Program || Record.RecordType == EFileRecordType::Command) {
			UProcess* ChildProcess = nullptr;
			UFileRecordUtils::LaunchProcess(FilePath, TArray<FString> { FilePath }, this->Console, (Fork) ? this->MyProcess : nullptr, ChildProcess);
			return ChildProcess;
		} else {
			UProcess* ChildProcess = nullptr;
			bool result =  UFileRecordUtils::LaunchSuitableProgram(FilePath, this->Console, ChildProcess, (Fork) ? this->MyProcess : nullptr, nullptr);
			return ChildProcess && result;
		}
	} else {
		return false;
	}
}

void UProgram::Launch(UConsoleContext* InConsoleContext, UProcess* OwningProcess, UDesktopWidget* TargetDesktop) {
	// Don't allow launch if we already have a process.
	check(!this->MyProcess);
	if(this->MyProcess) {
		return;
	}

	// Assign user context and process.
	this->Window->SetUserContext(InConsoleContext->GetUserContext());
	this->MyProcess = OwningProcess;

	// Kill the process when the window is closed.
	TScriptDelegate<> CloseDelegate;
	CloseDelegate.BindUFunction(this, "OwningWindowClosed");
	this->Window->NativeWindowClosed.Add(CloseDelegate);

	// Show ourselves on the workspace.
	this->SetupContexts();
	if(TargetDesktop) {
		TargetDesktop->ShowProgramOnWorkspace(this);
	} else {
		this->GetUserContext()->ShowProgramOnWorkspace(this);
	}
}

UProgram* UProgram::CreateProgram(const TSubclassOf<UWindow> InWindow, const TSubclassOf<UProgram> InProgramClass, UUserContext* InUserContext, UWindow*& OutWindow, FString InProcessName, UProcess* OwnerProcess, bool DoContextSetup) {
	// Preventative: make sure the system context isn't null.
	check(InUserContext);
	check(InUserContext->GetPeacenet());

	APlayerController* MyPlayer = UGameplayStatics::GetPlayerController(InUserContext->GetPeacenet()->GetWorld(), 0);

	// The window is what contains the program's UI.
	UWindow* Window = CreateWidget<UWindow, APlayerController>(MyPlayer, InWindow);

	// Construct the actual program.
	UProgram* ProgramInstance = CreateWidget<UProgram, APlayerController>(MyPlayer, InProgramClass);

	// Program and window are friends with each other
	ProgramInstance->Window = Window;

	// Window gets our user context.
	Window->SetUserContext(InUserContext);

	// Start the process for the program.
	if(OwnerProcess) {
		ProgramInstance->MyProcess = OwnerProcess->Fork(InProcessName);
	} else {
		ProgramInstance->MyProcess = InUserContext->Fork(InProcessName);
	}
	

	// That above value will be -1 if the player doesn't have enough RAM for the program to run.

	// Make sure we get notified when the window closes.
	TScriptDelegate<> CloseDelegate;
	CloseDelegate.BindUFunction(ProgramInstance, "OwningWindowClosed");

	Window->NativeWindowClosed.Add(CloseDelegate);

	// Set up the program's contexts if we're told to.
	if (DoContextSetup) {
		ProgramInstance->SetupContexts();
		ProgramInstance->GetUserContext()->ShowProgramOnWorkspace(ProgramInstance);
	}

	// TODO: Hook into the process kill event for our owning Peacegate process.

	// Return the window and program.
	OutWindow = Window;

	return ProgramInstance;
}

void UProgram::OwningWindowClosed() {
	// If we haven't closed already...
	if(!this->IsClosing) {
		this->IsClosing = true;

	    // Finish up our process.
    	this->MyProcess->Kill();
	}
}


int UProgram::GetProcessID() {
	return this->MyProcess->GetProcessID();
}

FText UProgram::GetWindowTitle() {
	return this->Window->WindowTitle;
}

void UProgram::ActiveProgramCloseEvent() {
	if (this->Window->HasAnyUserFocus() || this->Window->HasFocusedDescendants() || this->Window->HasKeyboardFocus()) {
		this->Window->Close();
	}
}

void UProgram::ShowInfoWithCallbacks(const FText & InTitle, const FText & InMessage, const EInfoboxIcon InIcon, const EInfoboxButtonLayout ButtonLayout, const bool ShowTextInput, const FInfoboxDismissedEvent & OnDismissed, const FInfoboxInputValidator & ValidatorFunction) {
	Window->ShowInfoWithCallbacks(InTitle, InMessage, InIcon, ButtonLayout, ShowTextInput, OnDismissed, ValidatorFunction);
}

void UProgram::ShowInfo(const FText & InTitle, const FText & InMessage, const EInfoboxIcon InIcon) {
	Window->ShowInfo(InTitle, InMessage, InIcon);
}

void UProgram::AskForFile(const FString InBaseDirectory, const FString InFilter, const EFileDialogType InDialogType, const FFileDialogDismissedEvent & OnDismissed) {
	Window->AskForFile(InBaseDirectory, InFilter, InDialogType, OnDismissed);
}

void UProgram::SetupContexts() {
	// Add ourself to the window's client slot.
	this->Window->AddWindowToClientSlot(this);

	this->NativeProgramLaunched();
}

void UProgram::NativeTick(const FGeometry& MyGeometry, float InDeltaSeconds) {
	if(this->MyProcess->IsDead()) {
		this->Window->Close();
		return;
	}

	Super::NativeTick(MyGeometry, InDeltaSeconds);
}

void UProgram::SetWindowMinimumSize(FVector2D InSize) {
	Window->SetClientMinimumSize(InSize);
}

void UProgram::NativeProgramLaunched() {}