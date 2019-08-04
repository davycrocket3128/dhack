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


#include "TerminalCommand.h"
#include "CommandInfo.h"
#include "PeacenetWorldStateActor.h"
#include "UserContext.h"
#include "Process.h"

void ATerminalCommand::SendGameEvent(FString InEventName, TMap<FString, FString> InEventData)
{
	this->Console->GetUserContext()->GetPeacenet()->SendGameEvent(InEventName, InEventData);
}

bool ATerminalCommand::IsTutorialActive()
{
	return this->Console->GetUserContext()->GetPeacenet()->IsTutorialActive();
}

bool ATerminalCommand::IsSet(FString InSaveBoolean)
{
	return this->Console->GetUserContext()->GetPeacenet()->IsTrue(InSaveBoolean);
}

void ATerminalCommand::ShowTutorialIfNotSet(FString InSaveBoolean, const FText& InTutorialTitle, const FText& InTutorialText)
{
	if(!this->IsSet(InSaveBoolean) && !this->IsTutorialActive())
	{
		this->Console->GetUserContext()->GetPeacenet()->GetTutorialState()->ActivatePrompt(InTutorialTitle, InTutorialText);
		this->Console->GetUserContext()->GetPeacenet()->SetSaveValue(InSaveBoolean, true);
	}
}

int ATerminalCommand::GetProcessID()
{
	return this->MyProcess->GetProcessID();
}

UUserContext* ATerminalCommand::GetUserContext()
{
	check(this->GetConsole());
	return this->GetConsole()->GetUserContext();
}

UConsoleContext* ATerminalCommand::GetConsole()
{
	return this->Console;
}

void ATerminalCommand::RunCommand(UConsoleContext* InConsole, TArray<FString> Argv)
{
	// Make sure Peacegate has given us a process before we can run.
	check(this->MyProcess);

	// When the process says it's time to end, we've gotta complete.
	TScriptDelegate<> TimeToEnd;
	TimeToEnd.BindUFunction(this, "ProcessEnded");
	this->MyProcess->OnTimeToEnd.Add(TimeToEnd);

	this->CommandName = Argv[0];
	this->Console = InConsole;	

	Argv.RemoveAt(0);

	if(this->CommandInfo->UsageStrings.Num())
	{
		FString Usage = "usage: ";
		for(auto UsageString : this->CommandInfo->UsageStrings)
		{
			Usage += "\n    " + this->CommandName + " " + UsageString;
		}

		FString Error;
		bool HasError = false;

		this->ArgumentMap = UDocoptForUnrealBPLibrary::ParseArguments(Usage, Argv, false, "", true, HasError, Error);

		if(HasError)
		{
			InConsole->SetForegroundColor(EConsoleColor::Magenta);
			InConsole->WriteLine(FText::Format(NSLOCTEXT("TerminalCommand", "DocoptError", "{0}: {1}"), FText::FromString(CommandName), FText::FromString(Error)));
			InConsole->ResetForegroundColor();
			InConsole->WriteLine(FText::GetEmpty());
			InConsole->WriteLine(NSLOCTEXT("TerminalCommand", "Usage", "Usage: "));
			
			bool PrintAll = true;
			
			for(auto UsageStr : this->CommandInfo->UsageStrings)
			{
				bool Print = true;
				for(FString arg : Argv)
				{
					if(!UsageStr.Contains(arg))
					{
						Print = false;
						break;
					}
				}

				if(!Print) continue;

				PrintAll = false;

				InConsole->SetForegroundColor(EConsoleColor::Green);
				InConsole->Write(FText::FromString(" "));
				InConsole->Write(FText::FromString(this->CommandName));
				InConsole->ResetForegroundColor();
				InConsole->Write(FText::FromString(" "));
				InConsole->WriteLine(FText::FromString(UsageStr));
			}

			if(PrintAll)
			{
				for(auto UsageStr : this->CommandInfo->UsageStrings)
				{
					InConsole->SetForegroundColor(EConsoleColor::Green);
					InConsole->Write(FText::FromString(" "));
					InConsole->Write(FText::FromString(this->CommandName));
					InConsole->ResetForegroundColor();
					InConsole->Write(FText::FromString(" "));
					InConsole->WriteLine(FText::FromString(UsageStr));
				}
			}

			InConsole->ResetFormatting();
			InConsole->WriteLine(FText::GetEmpty());

			InConsole->WriteLine(FText::Format(NSLOCTEXT("TerminalCommand", "MoreInfo", "See 'man {0}' for more information about this command."), FText::FromString(this->CommandName)));

			this->Complete();
			return;
		}


	}

	this->ArgumentList = Argv;

	NativeRunCommand(InConsole, Argv);
}

void ATerminalCommand::NativeRunCommand(UConsoleContext * InConsole, TArray<FString> InArguments)
{
	// Call into BP to do the rest.
	this->OnRunCommand(InConsole, InArguments);
}

void ATerminalCommand::Complete()
{
	this->CompleteInternal(true);
}

void ATerminalCommand::ProcessEnded()
{
	this->CompleteInternal(false);
}

void ATerminalCommand::CompleteInternal(bool KillProcess)
{
	// If we end up killing our process, this stops this function
	// from triggering by the "process ended" handler.
	this->IsCompleting = true;

	// If we're killing the process, we need to tell the game that the command
	// has finished running.  This gets sent to both Peacegate OS (resulting in
	// the process being killed) and the mission system that we've finished.
	if(KillProcess)
	{
		// Join the argument list together.
		FString ArgListText = "";
		for(auto Arg : this->ArgumentList)
		{
			ArgListText += Arg + " ";
		}
		ArgListText = ArgListText.TrimStartAndEnd();

		// Create event data for the mission system.
		TMap<FString, FString> MissionEventData;
		MissionEventData.Add("Command", this->CommandName);
		MissionEventData.Add("Arguments", ArgListText);

		// Add docopt arguments to the event data.
		for(auto DocoptArg : this->ArgumentMap)
		{
			if(DocoptArg.Value->IsEmpty()) continue;
			if(!MissionEventData.Contains(DocoptArg.Key))
			{
				MissionEventData.Add(DocoptArg.Key, DocoptArg.Value->AsString());
			}
		}

		// Broadcast to the mission system that we have just run.
		this->Console->GetUserContext()->GetPeacenet()->SendGameEvent("CommandComplete", MissionEventData);

		// Tell Peacegate OS that the process has ended.
		this->MyProcess->Kill();
	}

	this->Completed.Broadcast();
	
	// Despawn the actor and clean up memory used by the
	// terminal command. If we were a UObject we wouldn't
	// have to manually destroy ourselves, but then we
	// also wouldn't have access to "Delay" in Blueprint.
	this->Destroy();
}

ATerminalCommand* ATerminalCommand::CreateCommandFromAsset(UUserContext* InUserContext, UCommandInfo* InCommandInfo)
{
	// Return nullptr if the command asset or user context are invalid.
	if(!InUserContext) return nullptr;
	if(!InCommandInfo) return nullptr;

	FVector Location(0.0f, 0.0f, 0.0f);
	FRotator Rotation(0.0f, 0.0f, 0.0f);
 	FActorSpawnParameters SpawnInfo;

	ATerminalCommand* Command = InUserContext->GetPeacenet()->GetWorld()->SpawnActor<ATerminalCommand>(InCommandInfo->CommandClass, Location, Rotation, SpawnInfo);

	Command->CommandInfo = InCommandInfo;

	return Command;

	
}