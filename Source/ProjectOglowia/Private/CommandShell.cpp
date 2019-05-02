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

#include "CommandShell.h"
#include "UserContext.h"
#include "PeacegateFileSystem.h"
#include "RedirectedConsoleContext.h"

ACommandShell::ACommandShell()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ACommandShell::Tick(float InDeltaTime)
{
    Super::Tick(InDeltaTime);

    if(this->IsWaitingForInput)
    {
        // Try to read the next line of text from the console.
        FString Input;
        if(this->GetConsole()->GetLine(Input))
        {
            // We're not waiting for input anymore.
            this->IsWaitingForInput = false;

            // If the trimmed input is empty, then we're going to skip this line of text.
            if(!Input.TrimStartAndEnd().Len())
                return;

            // Execute the line of input as a command.
            this->ExecuteLine(Input);
        }
    }
    else
    {
        // Are we not waiting for a command, and, is there an instruction available?
        if(!this->IsWaitingForCommand && this->Instructions.Num())
        {
            // Wait for the command.
            this->IsWaitingForCommand = true;

            // Set the current command that we're waiting for and its intended console.
            this->CurrentCommand = this->Instructions[0].Command;
            this->CurrentConsole = this->Instructions[0].IntendedContext;

            // Update working directory of next console.
            this->CurrentConsole->SetWorkingDirectory(this->LastConsole->GetWorkingDirectory());

            // Ensure that we advance to the next command when the current one completes.
            TScriptDelegate<> CompletedDelegate;
            CompletedDelegate.BindUFunction(this, "CommandCompleted");
            this->CurrentCommand->Completed.Add(CompletedDelegate);

            // Run the command.
            this->CurrentCommand->RunCommand(this->CurrentConsole, this->Instructions[0].Arguments);

            return;
        }
        else
        {
            if(this->CurrentConsole)
            {
                // Update the working directory of the master console so commands like cd work in pipes.
                this->GetConsole()->SetWorkingDirectory(this->CurrentConsole->GetWorkingDirectory());

                // If the current console is a redirector then we must dump the console's output to the file.
                if(this->CurrentConsole->IsA<URedirectedConsoleContext>())
                {
                    URedirectedConsoleContext* RedirectedConsole = Cast<URedirectedConsoleContext>(this->CurrentConsole);
                    RedirectedConsole->DumpToFile(this->GetConsole());
                }

                // Remove the crap.
                this->CurrentConsole = nullptr;
                this->LastConsole = nullptr;
            }
        }

        // Write the shell prompt.
        this->WriteShellPrompt();

        // Wait for input.
        this->IsWaitingForInput = true;
    }
}

void ACommandShell::CommandCompleted()
{
    // Current console becomes last console.
    this->LastConsole = this->CurrentConsole;

    // Current Command becomes nullptr.
    this->CurrentCommand = nullptr;

    // Remove the current command from the queue.
    this->Instructions.RemoveAt(0);

    // And stop waiting for it to complete.
    this->IsWaitingForCommand = false;
}

void ACommandShell::WriteShellPrompt()
{
    // Get the username, hostname and current working directory.
    FString Username = this->GetUserContext()->GetUsername();
    FString Hostname = this->GetUserContext()->GetHostname();
    FString WorkingDirectory = this->GetConsole()->GetDisplayWorkingDirectory();

    // Output them to the screen.
    this->GetConsole()->Write(Username + "@" + Hostname + ":" + WorkingDirectory);

    // Are we root?
    if(this->GetUserContext()->IsAdministrator())
    {
        this->GetConsole()->Write("#");
    }
    else
    {
        this->GetConsole()->Write("$");
    }

    // Write a space.
    this->GetConsole()->Write(" ");
}

void ACommandShell::ExecuteLine(FString Input)
{
    // Trim any excess preceding/trailing whitespace.
    Input = Input.TrimStartAndEnd();

    // If the command starts with "exit" we immediately stop.
    if(Input.ToLower().StartsWith("exit"))
    {
        this->Complete();
        return;
    }

    // Use the command processor methods to get a list of instructions to perform.
    this->Instructions = UCommandProcessor::ProcessCommand(this->GetConsole(), Input);

    // Skip if there are no instructions.
    if(!Instructions.Num()) return;

    // Keep track of the last console context used by a command.
    this->LastConsole = this->GetConsole();

    // Log the command to .bash_history.
    this->WriteToHistory(Input);


}

void ACommandShell::WriteToHistory(FString Input)
{
    // History file is always in ~/.bash_history.
    FString HistoryPath = this->GetUserContext()->GetHomeDirectory() + "/.bash_history";

    // Get a pointer to the user's filesystem.
    UPeacegateFileSystem* FileSystem = this->GetUserContext()->GetFilesystem();

    // Where existing history is loaded.
    FString History;

    // If the file exists, load in existing history.
    if(FileSystem->FileExists(HistoryPath))
    {
        EFilesystemStatusCode Status;
        FileSystem->ReadText(HistoryPath, History, Status);
        History = History.TrimStartAndEnd();
    }

    // Append the new command to the history.
    History += "\r\n" + Input;

    // Write it to disk.
    FileSystem->WriteText(HistoryPath, History);
}