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

void UProgram::HandleProcessEnded(const FPeacegateProcess& InProcess)
{
	if(InProcess.PID == this->ProcessID && !this->IsClosing)
	{
		this->Window->Close();
	}
}

bool UProgram::LoadPeacenetSite(FString InURL, UPeacenetSiteWidget*& OutWidget, EConnectionError& OutConnectionError)
{
	FString IPAddress;

	if(this->GetUserContext()->GetPeacenet()->SaveGame->DomainNameMap.Contains(InURL))
	{
		IPAddress = this->GetUserContext()->GetPeacenet()->SaveGame->DomainNameMap[InURL];
	}
	else if(this->GetUserContext()->GetPeacenet()->SaveGame->ComputerIPMap.Contains(InURL))
	{
		IPAddress = InURL;
	}
	else
	{
		OutConnectionError = EConnectionError::ConnectionTimedOut;
		return false;
	}

	int ComputerEntity = this->GetUserContext()->GetPeacenet()->SaveGame->ComputerIPMap[IPAddress];

	FComputer Computer;
	int ComputerIndex;
	bool result=  this->GetUserContext()->GetPeacenet()->SaveGame->GetComputerByID(ComputerEntity, Computer, ComputerIndex);
	check(result);

	if(Computer.ComputerType != EComputerType::PeacenetSite)
	{
		OutConnectionError = EConnectionError::ConnectionRefused;
		return false;
	}

	if(!Computer.PeacenetSite)
	{
		OutConnectionError = EConnectionError::ConnectionRefused;
		return false;
	}

	if(!Computer.PeacenetSite->PeacenetSite)
	{
		OutConnectionError = EConnectionError::ConnectionRefused;
		return false;
	}

	OutWidget = CreateWidget<UPeacenetSiteWidget, APlayerController>(this->GetOwningPlayer(), Computer.PeacenetSite->PeacenetSite);
	OutWidget->Setup(Computer.PeacenetSite, this);
	OutConnectionError = EConnectionError::None;
	return true;
}

UUserContext* UProgram::GetUserContext()
{
	return this->Window->GetUserContext();
}

void UProgram::PushNotification(const FText & InNotificationMessage)
{
	this->GetUserContext()->GetDesktop()->EnqueueNotification(this->Window->WindowTitle, InNotificationMessage, this->Window->Icon);
}

void UProgram::RequestPlayerAttention(bool PlaySound)
{
	this->PlayerAttentionNeeded.Broadcast(PlaySound);
}

UProgram* UProgram::CreateProgram(const TSubclassOf<UWindow> InWindow, const TSubclassOf<UProgram> InProgramClass, UUserContext* InUserContext, UWindow*& OutWindow, FString InProcessName, bool DoContextSetup)
{
	// Preventative: make sure the system context isn't null.
	check(InUserContext);

	// TODO: Take in a user context instead of a system context and user ID.
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
	ProgramInstance->ProcessID = InUserContext->StartProcess(InProcessName, InProcessName);

	// Make sure we get notified when the window closes.
	TScriptDelegate<> CloseDelegate;
	CloseDelegate.BindUFunction(ProgramInstance, "OwningWindowClosed");

	Window->NativeWindowClosed.Add(CloseDelegate);

	// Set up the program's contexts if we're told to.
	if (DoContextSetup)
	{
		ProgramInstance->SetupContexts();
		ProgramInstance->GetUserContext()->ShowProgramOnWorkspace(ProgramInstance);
	}

	// Let the program handle itself being killed...
	TScriptDelegate<> ProcessEndedDelegate;
	ProcessEndedDelegate.BindUFunction(ProgramInstance, "HandleProcessEnded");
	InUserContext->GetOwningSystem()->ProcessEnded.Add(ProcessEndedDelegate);

	// Return the window and program.
	OutWindow = Window;

	return ProgramInstance;
}

void UProgram::OwningWindowClosed()
{
	// If we haven't closed already...
	if(!this->IsClosing)
	{
		this->IsClosing = true;

	    // Finish up our process.
    	this->GetUserContext()->GetOwningSystem()->FinishProcess(this->ProcessID);
	}
}


void UProgram::ActiveProgramCloseEvent()
{
	if (this->Window->HasAnyUserFocus() || this->Window->HasFocusedDescendants() || this->Window->HasKeyboardFocus())
	{
		this->Window->Close();
	}
}

void UProgram::ShowInfoWithCallbacks(const FText & InTitle, const FText & InMessage, const EInfoboxIcon InIcon, const EInfoboxButtonLayout ButtonLayout, const bool ShowTextInput, const FInfoboxDismissedEvent & OnDismissed, const FInfoboxInputValidator & ValidatorFunction)
{
	Window->ShowInfoWithCallbacks(InTitle, InMessage, InIcon, ButtonLayout, ShowTextInput, OnDismissed, ValidatorFunction);
}

void UProgram::ShowInfo(const FText & InTitle, const FText & InMessage, const EInfoboxIcon InIcon)
{
	Window->ShowInfo(InTitle, InMessage, InIcon);
}

void UProgram::AskForFile(const FString InBaseDirectory, const FString InFilter, const EFileDialogType InDialogType, const FFileDialogDismissedEvent & OnDismissed)
{
	Window->AskForFile(InBaseDirectory, InFilter, InDialogType, OnDismissed);
}

void UProgram::SetupContexts()
{
	// Add ourself to the window's client slot.
	this->Window->AddWindowToClientSlot(this);

	this->NativeProgramLaunched();
}

void UProgram::SetWindowMinimumSize(FVector2D InSize)
{
	Window->SetClientMinimumSize(InSize);
}

void UProgram::NativeProgramLaunched() {}

