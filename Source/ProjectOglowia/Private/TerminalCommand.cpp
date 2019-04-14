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

int ATerminalCommand::GetProcessID()
{
	return this->ProcessID;
}

void ATerminalCommand::HandleProcessEnded(const FPeacegateProcess& InProcess)
{
	if(InProcess.PID == this->ProcessID && !this->IsCompleting)
	{
		this->CompleteInternal(false);
	}
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
	// Bind our "process ended" handler to Peacegate's "process ended"
	// event so that we can see when we get killed by the player.
	TScriptDelegate<> ProcessEndedDelegate;
	ProcessEndedDelegate.BindUFunction(this, "HandleProcessEnded");
	InConsole->GetUserContext()->GetOwningSystem()->ProcessEnded.Add(ProcessEndedDelegate);

	this->CommandName = Argv[0];
	this->Console = InConsole;	
	this->ProcessID = this->Console->GetUserContext()->StartProcess(this->CommandInfo->ID.ToString(), this->CommandInfo->FullName.ToString());

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
			InConsole->WriteLine(CommandName + ": " + Error);
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
			if(!MissionEventData.Contains(DocoptArg.Key))
			{
				MissionEventData.Add(DocoptArg.Key, DocoptArg.Value->AsString());
			}
		}

		// Broadcast to the mission system that we have just run.
		this->Console->GetUserContext()->GetPeacenet()->SendGameEvent("CommandComplete", MissionEventData);

		// Tell Peacegate OS that the process has ended.
		this->Console->GetUserContext()->GetOwningSystem()->FinishProcess(this->ProcessID);
	}

	this->Completed.Broadcast();
	
	// Despawn the actor and clean up memory used by the
	// terminal command. If we were a UObject we wouldn't
	// have to manually destroy ourselves, but then we
	// also wouldn't have access to "Delay" in Blueprint.
	this->Destroy();
}