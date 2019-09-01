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

#include "FileRecordUtils.h"
#include "PeacegateFileSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Window.h"
#include "Program.h"
#include "TerminalCommand.h"
#include "DesktopWidget.h"
#include "PeacenetWorldStateActor.h"

#define ERR_FILENOTFOUND NSLOCTEXT("FileSystem", "FileNotFound", "File not found.")
#define ERR_NOEXEC NSLOCTEXT("FileSystem", "NoExec", "File not executable.")
#define ERR_WHATTHEFUCK NSLOCTEXT("FileSystem", "WhatTheFuck", "What the fuck? Somehow, in the process launching routine, The Peacenet has allowed the code to flow past the point of checking whether the file is a program or a command; yet the file is not a program or command.  What the FUCK did you do?")

bool UFileRecordUtils::GetExecutable(APeacenetWorldStateActor* Peacenet, FFileRecord Record, UPeacegateProgramAsset*& OutProgram, UCommandInfo*& OutCommandInfo) {
    // FIXME: Better content ID system.  At least this one doesn't copyright-strike bullshit, it just finds the right
    // program or command for the file.
    if(Record.RecordType == EFileRecordType::Command) {
        TArray<FName> CommandNames;
        Peacenet->CommandInfo.GetKeys(CommandNames);
        if(Record.ContentID >= 0 && Record.ContentID < CommandNames.Num()) {
            OutCommandInfo = Peacenet->CommandInfo[CommandNames[Record.ContentID]];
            return OutCommandInfo;
        }
    } else if(Record.RecordType == EFileRecordType::Program) {
        if(Record.ContentID >= 0 && Record.ContentID < Peacenet->Programs.Num()) {
            OutProgram = Peacenet->Programs[Record.ContentID];
            return OutProgram;
        }
    }
    return false;
}

bool UFileRecordUtils::LaunchProcess(FString InFilePath, TArray<FString> Arguments, UConsoleContext* InConsoleContext, UProcess* OwningProcess, UProcess*& OutProcess, UDesktopWidget* TargetDesktop) {
    // Get a filesystem context.
    UPeacegateFileSystem* FileSystem = InConsoleContext->GetUserContext()->GetFilesystem();

    // Only continue if the file exists.
    if(FileSystem->FileExists(InFilePath)) {
        // Get the file record so we know the underlying metadata in the save game for the file
        FFileRecord Record = FileSystem->GetFileRecord(InFilePath);
        
        // Determine whether the file is a program or command, retrieve the corresponding asset
        UPeacegateProgramAsset* ProgramAsset = nullptr;
        UCommandInfo* CommandInfo = nullptr;
        if(!UFileRecordUtils::GetExecutable(InConsoleContext->GetUserContext()->GetPeacenet(), Record, ProgramAsset, CommandInfo)) {
            InConsoleContext->WriteLine(ERR_NOEXEC);
            return false;
        }

        // Spawn a Peacegate process for the user, hooking it into the program/command so it can signal
        UProcess* Forked = nullptr;
        if(OwningProcess) {
            Forked = OwningProcess->Fork(InFilePath);
        } else {
            Forked = InConsoleContext->GetUserContext()->Fork(InFilePath);
        }

        // Instantiate whatever class the asset tells us to if valid
        APlayerController* MyPlayer = UGameplayStatics::GetPlayerController(InConsoleContext->GetUserContext()->GetPeacenet()->GetWorld(), 0);
        if(ProgramAsset) {
            UWindow* Window = CreateWidget<UWindow, APlayerController>(MyPlayer, InConsoleContext->GetUserContext()->GetPeacenet()->WindowClass);
            UProgram* Program = CreateWidget<UProgram, APlayerController>(MyPlayer, ProgramAsset->ProgramClass);
            Program->Window = Window;
            Program->Launch(InConsoleContext, Forked, TargetDesktop);
        } else if(CommandInfo) {
            FVector Location(0.0f, 0.0f, 0.0f);
	        FRotator Rotation(0.0f, 0.0f, 0.0f);
 	        FActorSpawnParameters SpawnInfo;

	        ATerminalCommand* Command = InConsoleContext->GetUserContext()->GetPeacenet()->GetWorld()->SpawnActor<ATerminalCommand>(CommandInfo->CommandClass, Location, Rotation, SpawnInfo);
	        Command->CommandInfo = CommandInfo;
            Command->Launch(InConsoleContext, Forked, Arguments);
        } else {
            Forked->Kill();
            InConsoleContext->WriteLine(ERR_WHATTHEFUCK);
            return false;
        }

        // Give the process to our caller.
        OutProcess = Forked;
        return true;
    } else {
        // Otherwise we've failed.
        InConsoleContext->WriteLine(ERR_FILENOTFOUND);
        return false;
    }
}

#undef ERR_FILENOTFOUND
#undef ERR_WHATTHEFUCK
#undef ERR_NOEXEC