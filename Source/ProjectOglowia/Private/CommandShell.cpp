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

FPeacegateCommandInstruction ACommandShell::ParseCommand(const FString& InCommand, FString InHome, FString& OutputError)
{
	//This is the list of commands to run in series
	TArray<FString> commands;

	//Parser state
	bool escaping = false;
	bool inQuote = false;
	FString current = TEXT("");
	bool isFileName = false;

	//output file name
	FString fileName = TEXT("");

	//Will the game overwrite the specified file with output?
	bool shouldOverwriteOnFileRedirect = false;

	//Length of the input command.
	int cmdLength = InCommand.Len();

	//Iterate through each character in the command.
	for (int i = 0; i < cmdLength; i++)
	{
		//Get the character at the current index.
		TCHAR c = InCommand[i];

		//If we're a backslash, parse an escape sequence.
		if (c == TEXT('\\'))
		{
			//Ignore escape if we're not in a quote or file.
			if (!(inQuote || isFileName))
			{
				current.AppendChar(c);
				continue;
			}
			//If we're not currently escaping...
			if (!escaping)
			{
				escaping = true;
				//If we're a filename, append to the filename string.
				if (isFileName)
				{
					fileName.AppendChar(c);
				}
				else
				{
					current.AppendChar(c);
				}
				continue;
			}
			else
			{
				escaping = false;
			}
		}
		else if (c == TEXT('"'))
		{
			if (!isFileName)
			{
				if (!escaping)
				{
					inQuote = !inQuote;
				}
			}
		}
		if (c == TEXT('|') && this->AllowPipes())
		{
			if (!isFileName)
			{
				if (!inQuote)
				{
					if (current.TrimStartAndEnd().IsEmpty())
					{
						OutputError = TEXT("unexpected token '|' (pipe)");
						return FPeacegateCommandInstruction::Empty();
					}
					commands.Add(current.TrimStartAndEnd());
					current = TEXT("");
					continue;
				}
			}
		}
		else if (FChar::IsWhitespace(c))
		{
			if (isFileName)
			{
				if (!escaping)
				{
					if (fileName.IsEmpty())
					{
						continue;
					}
					else
					{
						OutputError = TEXT("unexpected whitespace in filename.");
						return FPeacegateCommandInstruction::Empty();
					}
				}
			}
		}
		else if (c == TEXT('>') && this->AllowRedirection())
		{
			if (!isFileName)
			{
				isFileName = true;
				shouldOverwriteOnFileRedirect = true;
				continue;
			}
			else
			{
				if (InCommand[i - 1] == TEXT('>'))
				{
					if (!shouldOverwriteOnFileRedirect)
					{
						shouldOverwriteOnFileRedirect = false;
					}
					else {
						OutputError = TEXT("unexpected token '>' (redirect) in filename");
						return FPeacegateCommandInstruction::Empty();
					}
					continue;
				}
			}
		}
		if (isFileName)
			fileName.AppendChar(c);
		else
			current.AppendChar(c);
		if (escaping)
			escaping = false;

	}
	if (inQuote)
	{
		OutputError = TEXT("expected closing quotation mark, got end of command.");
		return FPeacegateCommandInstruction::Empty();
	}
	if (escaping)
	{
		OutputError = TEXT("expected escape sequence, got end of command.");
		return FPeacegateCommandInstruction::Empty();
	}
	if (!current.IsEmpty())
	{
		commands.Add(current.TrimStartAndEnd());
		current = TEXT("");
	}
	if (!fileName.IsEmpty())
	{
		if (fileName.StartsWith("~"))
		{
			fileName.RemoveAt(0, 1, true);
			while (fileName.StartsWith("/"))
			{
				fileName.RemoveAt(0, 1);
			}
			if (InHome.EndsWith("/"))
			{
				fileName = InHome + fileName;
			}
			else
			{
				fileName = InHome + "/" + fileName;
			}
		}
	}
	return FPeacegateCommandInstruction(commands, fileName, shouldOverwriteOnFileRedirect);
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
            if(this->LastConsole)
                 this->CurrentConsole->SetWorkingDirectory(this->LastConsole->GetWorkingDirectory());
            else
                this->CurrentConsole->SetWorkingDirectory(this->GetConsole()->GetWorkingDirectory());

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

    // Parse the command into a list of instructions.
    FString Error = "";
    FPeacegateCommandInstruction InstructionData = this->ParseCommand(Input, this->GetUserContext()->GetHomeDirectory(), Error);

    // Output the error if there is any.
    if(Error.Len())
    {
        this->GetConsole()->WriteLine(Error);
        return;
    }

    // Clear the current list of instructions.
    this->Instructions.Empty();

    // Are there any commands to actually run?
    if(!InstructionData.Commands.Num())
    {
        return;
    }

    // If we are to redirect output to a file...
    if (!InstructionData.OutputFile.IsEmpty())
	{
        // Then make DAMN sure the file path doesn't point to a directory.
		if (this->GetUserContext()->GetFilesystem()->DirectoryExists(InstructionData.OutputFile))
		{
			this->GetConsole()->WriteLine("`3`*error: " + InstructionData.OutputFile + ": Directory exists.`1`r");
			return;
		}
	}

    // The last piper console context used by a command.
    UPiperContext* LastPiper = nullptr;

    // Keep track of the current instruction index.  I'm doing it this way because I don't
    // like C for loops as much as I do C++ ones.  But I also need to keep track of the loop
    // index.
    int i = 0;

    // Iterate through every command and get each command parsed and located.
    for(auto& Command : InstructionData.Commands)
    {
        // Determine if we're piping.
        bool IsPiping = (i < InstructionData.Commands.Num() - 1);

        // The home directory of the user - so the parser knows what to replace "~" with at the
        // beginning of tokens.
        FString Home = this->GetUserContext()->GetHomeDirectory();

        // If the previous command created a piper, we use that piper's home directory instead.
        if(LastPiper)
            Home = LastPiper->GetUserContext()->GetHomeDirectory();

        // Parse the command into a list of tokens which will be passed as the argument list.
        // This also lets us know the name of the command which is used to locate the command object.
        TArray<FString> Tokens = UTerminalCommandParserLibrary::Tokenize(Command, Home, Error);

        // If there's an error, fail.
        if(Error.Len())
        {
            this->GetConsole()->WriteLine(Error);
            return;
        }

        // Get the name of the command.
        FString CommandNameToken = Tokens[0];

        // Try to get a terminal command object.  If we get nullptr, the command wasn't found.
        ATerminalCommand* CommandObject = this->GetCommand(CommandNameToken);
        if(!CommandObject)
        {
            this->GetConsole()->WriteLine(CommandNameToken + ": Command not found.");
            return;
        }

        // Create an instruction object for the command and place it in our instructions list.
        FCommandRunInstruction Instruction;
        Instruction.Arguments = Tokens;
        Instruction.Command = CommandObject;        

        // Are we piping?
        if(IsPiping)
        {
            // Create a piper context.
            UPiperContext* Piper = NewObject<UPiperContext>();

            // Set the piper up with a valid user context.
            if(LastPiper)
            {
                Piper->Setup(LastPiper->GetUserContext());
            }
            else
            {
                Piper->Setup(this->GetUserContext());
            }

            // Perform further setup.
            Piper->SetupPiper(LastPiper, nullptr);

            // Use this piper as the instruction's intended console.
            // Also set it as the last piper.
            LastPiper = Piper;
            Instruction.IntendedContext = Piper;
        }
        else
        {
            // The console that will be used by the command.
            UPiperContext* Piper = nullptr;
            
            // The console which will be used for output.
            UConsoleContext* StdOut = nullptr;

            // The console used for input.
            UPiperContext* StdIn = LastPiper;

            // If we're not redirecting output to a file, then output console
            // becomes the same as our shell console and the command console is
            // is just a generic piper.
            if(InstructionData.OutputFile.IsEmpty())
            {
                Piper = NewObject<UPiperContext>();
                StdOut = this->GetConsole();
            }
            else
            {
                // Otherwise we need to create a redirector console.
                Piper = NewObject<UPiperContext>(this->GetConsole(), URedirectedConsoleContext::StaticClass());
                auto Redirected = Cast<URedirectedConsoleContext>(Piper);

                // Set up the redirector so it can output to the file.
                Redirected->OutputFilePath = InstructionData.OutputFile;
                Redirected->Overwrite = InstructionData.Overwrites;
            }

            // Set up the piper's user context.
            if(LastPiper)
            {
                Piper->Setup(LastPiper->GetUserContext());
            }
            else
            {
                Piper->Setup(this->GetUserContext());
            }

            // And now we set up the piper's input and output!
            Piper->SetupPiper(StdIn, StdOut);

            // And use it for the command.
            Instruction.IntendedContext = Piper;
        }

        // Add the instruction to the list we're about to run.
        this->Instructions.Add(Instruction);

        // Annnd the loop index goes up!
        i++;
    }
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