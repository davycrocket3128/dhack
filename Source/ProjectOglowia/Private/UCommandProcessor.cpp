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


#include "UCommandProcessor.h"
#include "USystemContext.h"
#include "CommonUtils.h"
#include "URedirectedConsoleContext.h"
#include "CommandInfo.h"
#include "UPiperContext.h"
#include "UGraphicalTerminalCommand.h"
#include "UUserContext.h"
#include "TerminalCommand.h"
#include "TerminalCommandParserLibrary.h"


TArray<FCommandRunInstruction> UCommandProcessor::ProcessCommand(UConsoleContext* InConsole, const FString& InCommand)
{
	check(InConsole);

	TArray<FCommandRunInstruction> CommandsToRun;

	FString TrimmedCommand = InCommand.TrimStartAndEnd();

	if (TrimmedCommand.IsEmpty())
		return CommandsToRun;

	FString OutError;

	FPeacegateCommandInstruction Instruction = UTerminalCommandParserLibrary::GetCommandList(InCommand, InConsole->GetUserContext()->GetHomeDirectory(), OutError);

	

	if (!OutError.IsEmpty())
	{
		InConsole->WriteLine(OutError);
		return CommandsToRun;
	}

	int CommandNum = Instruction.Commands.Num();

	if (CommandNum == 0)
		return CommandsToRun;

	if (!Instruction.OutputFile.IsEmpty())
	{
		if (InConsole->GetUserContext()->GetFilesystem()->DirectoryExists(Instruction.OutputFile))
		{
			InConsole->WriteLine("`3`*error: " + Instruction.OutputFile + ": Directory exists.`1`r");
			return CommandsToRun;
		}
	}

	UPiperContext* LastPiper = nullptr;

	for (int i = 0; i < CommandNum; i++)
	{
		bool IsPiping = (i < CommandNum - 1);
	
		FString CommandText = Instruction.Commands[i];

		FString Home;
		if (LastPiper)
		{
			Home = LastPiper->GetUserContext()->GetHomeDirectory();
		}
		else 
		{
			Home = InConsole->GetUserContext()->GetHomeDirectory();
		}

		FString TokenError;

		TArray<FString> Tokens = UTerminalCommandParserLibrary::Tokenize(CommandText, Home, TokenError);

		if (!TokenError.IsEmpty())
		{
			InConsole->WriteLine(TokenError);
			return TArray<FCommandRunInstruction>();
		}

		FString Name = Tokens[0];
		FString InternalUsage;
		FString FriendlyUsage;



		ATerminalCommand* CommandImpl = nullptr;
		
		UPeacegateFileSystem* UserFilesystem = InConsole->GetUserContext()->GetFilesystem();

		if(UserFilesystem->FileExists(Name))
		{
			FFileRecord Record = UserFilesystem->GetFileRecord(Name);
			if(Record.RecordType == EFileRecordType::Command)
			{
				TArray<FName> CommandKeys;
				InConsole->GetUserContext()->GetPeacenet()->CommandInfo.GetKeys(CommandKeys);

				if(Record.ContentID < CommandKeys.Num())
				{
					UCommandInfo* Info = InConsole->GetUserContext()->GetPeacenet()->CommandInfo[CommandKeys[Record.ContentID]];
 					FVector Location(0.0f, 0.0f, 0.0f);
					FRotator Rotation(0.0f, 0.0f, 0.0f);
 					FActorSpawnParameters SpawnInfo;

					CommandImpl = InConsole->GetUserContext()->GetPeacenet()->GetWorld()->SpawnActor<ATerminalCommand>(Info->CommandClass, Location, Rotation, SpawnInfo);

					CommandImpl->CommandInfo = Info;
				}
			}
			else if(Record.RecordType == EFileRecordType::Program)
			{
				if(Record.ContentID < InConsole->GetUserContext()->GetPeacenet()->Programs.Num())
				{
					UPeacegateProgramAsset* ProgramAsset = InConsole->GetUserContext()->GetPeacenet()->Programs[Record.ContentID];
					FVector Location(0.0f, 0.0f, 0.0f);
		 			FRotator Rotation(0.0f, 0.0f, 0.0f);
 					FActorSpawnParameters SpawnInfo;

					AGraphicalTerminalCommand* GraphicalCommand = InConsole->GetUserContext()->GetPeacenet()->GetWorld()->SpawnActor<AGraphicalTerminalCommand>(Location, Rotation, SpawnInfo);
					GraphicalCommand->ProgramAsset = ProgramAsset;
					GraphicalCommand->CommandInfo = InConsole->GetUserContext()->GetPeacenet()->CommandInfo[ProgramAsset->ID];
					CommandImpl = GraphicalCommand;
				}
			}
		}
		else if (InConsole->GetUserContext()->GetOwningSystem()->TryGetTerminalCommand(FName(*Name), CommandImpl, InternalUsage, FriendlyUsage))
		{
			// Stub.
		}

		if(CommandImpl)
		{
			FCommandRunInstruction NewInst;
			NewInst.Arguments = Tokens;
			NewInst.Command = CommandImpl;

			if (IsPiping)
			{
				UPiperContext* Ctx = NewObject<UPiperContext>();
				if(LastPiper)
				{
					Ctx->Setup(LastPiper->GetUserContext());
				}
				else
				{
					Ctx->Setup(InConsole->GetUserContext());
				}
				Ctx->SetupPiper(LastPiper, nullptr);
				NewInst.IntendedContext = Ctx;
				LastPiper = Ctx;
			}
			else
			{
				UPiperContext* Ctx = nullptr;
				UConsoleContext* Output = nullptr;
				UPiperContext* Input = LastPiper;
				if (Instruction.OutputFile.IsEmpty())
				{
					Ctx = NewObject<UPiperContext>();
					Output = InConsole;
				}
				else
				{
					Ctx = NewObject<UPiperContext>(InConsole, URedirectedConsoleContext::StaticClass());
					auto Redirected = Cast<URedirectedConsoleContext>(Ctx);
					Redirected->OutputFilePath = Instruction.OutputFile;
					Redirected->Overwrite = Instruction.Overwrites;
				}

				if (LastPiper)
				{
					Ctx->Setup(LastPiper->GetUserContext());
				}
				else
				{
					Ctx->Setup(InConsole->GetUserContext());
				}

				Ctx->SetupPiper(Input, Output);

				NewInst.IntendedContext = Ctx;
			}

			CommandsToRun.Add(NewInst);
		}
		else 
		{
			InConsole->WriteLine(Name + TEXT(": Command not found."));
			return TArray<FCommandRunInstruction>();
		}
	}

	return CommandsToRun;
}