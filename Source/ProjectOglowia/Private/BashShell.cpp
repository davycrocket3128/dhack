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

#include "BashShell.h"
#include "UserContext.h"
#include "CommandInfo.h"
#include "GraphicalTerminalCommand.h"

ATerminalCommand* ABashShell::GetCommand(FString Command)
{
    ATerminalCommand* CommandObject = nullptr;
    FString InternalUsage;
    FString FriendlyUsage;

    if(this->GetUserContext()->GetFilesystem()->FileExists(this->GetConsole()->CombineWithWorkingDirectory(Command)))
	{
		FFileRecord Record = this->GetUserContext()->GetFilesystem()->GetFileRecord(this->GetConsole()->CombineWithWorkingDirectory(Command));
		if(Record.RecordType == EFileRecordType::Command)
		{
			TArray<FName> CommandKeys;
			this->GetUserContext()->GetPeacenet()->CommandInfo.GetKeys(CommandKeys);

			if(Record.ContentID < CommandKeys.Num())
			{
				UCommandInfo* Info = this->GetUserContext()->GetPeacenet()->CommandInfo[CommandKeys[Record.ContentID]];
 				FVector Location(0.0f, 0.0f, 0.0f);
				FRotator Rotation(0.0f, 0.0f, 0.0f);
 				FActorSpawnParameters SpawnInfo;

				ATerminalCommand* CommandImpl = this->GetUserContext()->GetPeacenet()->GetWorld()->SpawnActor<ATerminalCommand>(Info->CommandClass, Location, Rotation, SpawnInfo);

				CommandImpl->CommandInfo = Info;

                return CommandImpl;
			}
		}
		else if(Record.RecordType == EFileRecordType::Program)
		{
			if(Record.ContentID < this->GetUserContext()->GetPeacenet()->Programs.Num())
			{
				UPeacegateProgramAsset* ProgramAsset = this->GetUserContext()->GetPeacenet()->Programs[Record.ContentID];
				FVector Location(0.0f, 0.0f, 0.0f);
	 			FRotator Rotation(0.0f, 0.0f, 0.0f);
					FActorSpawnParameters SpawnInfo;

				AGraphicalTerminalCommand* GraphicalCommand = this->GetUserContext()->GetPeacenet()->GetWorld()->SpawnActor<AGraphicalTerminalCommand>(Location, Rotation, SpawnInfo);
				GraphicalCommand->ProgramAsset = ProgramAsset;
				GraphicalCommand->CommandInfo = this->GetUserContext()->GetPeacenet()->CommandInfo[ProgramAsset->ID];
				return GraphicalCommand;
			}
		}
	}
	else if (this->GetUserContext()->GetOwningSystem()->TryGetTerminalCommand(FName(*Command), CommandObject, InternalUsage, FriendlyUsage))
	{
		return CommandObject;
	}
    return nullptr;
}

FString ABashShell::GetShellPrompt()
{
	    // Get the username, hostname and current working directory.
    FString Username = this->GetUserContext()->GetUsername();
    FString Hostname = this->GetUserContext()->GetHostname();
    FString WorkingDirectory = this->GetConsole()->GetDisplayWorkingDirectory();

	// The little thingy that tells whether we're root or not.
	FString UserStatus = "$";

    // Are we root?
    if(this->GetUserContext()->IsAdministrator())
    {
        UserStatus = "#";
    }

	return "[" + Username + "@" + Hostname + " " + WorkingDirectory + "]" + UserStatus + " ";
}