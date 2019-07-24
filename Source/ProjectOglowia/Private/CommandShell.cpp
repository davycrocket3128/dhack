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
#include "CommonUtils.h"


ACommandShell::ACommandShell()
{
    PrimaryActorTick.bCanEverTick = true;
}

FPeacegateCommandInstruction ACommandShell::ParseCommand(const FString& InCommand, FString InHome, FText& OutputError)
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
						OutputError = NSLOCTEXT("Shell", "UnexpectedPipe", "unexpected token '|' (pipe)");
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
						OutputError = NSLOCTEXT("Shell", "UnexpectedWhitespace", "unexpected whitespace in filename.");
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
						OutputError = NSLOCTEXT("Shell", "UnexpectedRedirect", "unexpected token '>' (redirect) in filename");
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
		OutputError = NSLOCTEXT("Shell", "ExpectedQuote", "expected closing quotation mark, got end of command.");
		return FPeacegateCommandInstruction::Empty();
	}
	if (escaping)
	{
		OutputError = NSLOCTEXT("Shell", "UnfinishedEscape", "expected escape sequence, got end of command.");
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

void ACommandShell::FinishSpecialCommand()
{
    this->CommandCompleted();
}

void ACommandShell::WriteToOutputFile(FString FileText)
{
	FString path = this->GetConsole()->CombineWithWorkingDirectory(this->FilePath);
	bool overwrite = this->FileOverwrite;
	UPeacegateFileSystem* fs = this->GetUserContext()->GetFilesystem();
	EFilesystemStatusCode Status = EFilesystemStatusCode::OK;

	if(fs->DirectoryExists(path))
	{
		this->GetConsole()->WriteLine(NSLOCTEXT("Bash", "DirectoryExists", "bash: redirect: error: Directory exists."));
		return;
	}

	if(overwrite || !fs->FileExists(path))
	{
		fs->WriteText(path, FileText);
	}
	else
	{
		// Read the contents of the file.
		FString content;
		if(!fs->ReadText(path, content, Status))
		{
			this->GetConsole()->WriteLine(FText::Format(NSLOCTEXT("Bash", "FileReadError", "bash: redirect: error: {0}"), UCommonUtils::GetFriendlyFilesystemStatusCode(Status)));
			return;
		}
		content += FileText;
		fs->WriteText(path, content);
	}
	this->FilePath = "";
	this->FileBuffer = nullptr;
	this->FileOverwrite = false;
}

void ACommandShell::Tick(float InDeltaTime)
{
    Super::Tick(InDeltaTime);

    if(this->IsWaitingForCommand) return;

    if(this->IsWaitingForInput)
    {
        // Try to read the next line of text from the console.
        FString Input;
        if(this->GetConsole()->UpdateAdvancedGetLine(Input))
        {
            // We're not waiting for input anymore.
            this->IsWaitingForInput = false;

            // If the trimmed input is empty, then we're going to skip this line of text.
            if(!Input.TrimStartAndEnd().Len())
                return;

            // Execute the line of input as a command.
            this->ExecuteLine(Input);

            if(this->Instructions.Num())
            {
                this->ExecuteNextCommand();
                return;
            }
        }
    }
    else
    {
        // Are we not waiting for a command, and, is there an instruction available?
        if(!this->IsWaitingForCommand && this->Instructions.Num())
        {
            this->ExecuteNextCommand();
            return;
        }
        else
        {
            if(this->CurrentConsole)
            {
                // Update the working directory of the master console so commands like cd work in pipes.
                this->GetConsole()->SetWorkingDirectory(this->CurrentConsole->GetWorkingDirectory());

                // If we have a file buffer then we will dump the contents of it to the output file path.
                if(this->FileBuffer)
                {
					FString BufContent = this->FileBuffer->DumpToString();
                    this->WriteToOutputFile(BufContent);
					this->FileBuffer = nullptr;
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

void ACommandShell::ExecuteNextCommand()
{
    // Wait for the command.     
    this->IsWaitingForCommand = true;

    // Set the current console to the command's intended console.
    this->CurrentConsole = this->Instructions[0].IntendedContext;

    // Update working directory of next console.
    if(this->LastConsole)
        this->CurrentConsole->SetWorkingDirectory(this->LastConsole->GetWorkingDirectory());
    else
        this->CurrentConsole->SetWorkingDirectory(this->GetConsole()->GetWorkingDirectory());

    // For special commands, make a copy of the arguments and remove the command name from the copy.
    // The special commands already get their command name.
    TArray<FString> SpecialArgs = this->Instructions[0].Arguments;
    SpecialArgs.RemoveAt(0);

    // Try to run the command as a special command.  If this is successful then the game will
    // wait for that command to complete.
    if(this->RunSpecialCommand(this->CurrentConsole, this->Instructions[0].Command, SpecialArgs))
    {
        // If we're supposed to auto-complete after the command's done, then we'll do that now.
        if(this->AutoCompleteSpecials())
        {
            this->FinishSpecialCommand();
        }
        return;
    }

    // If the above ends up failing then we'll try to spawn in a terminal command actor.
    this->CurrentCommand = this->GetCommand(this->Instructions[0].Command);

    if(this->CurrentCommand)
    {
        // Ensure that we advance to the next command when the current one completes.
        TScriptDelegate<> CompletedDelegate;
        CompletedDelegate.BindUFunction(this, "CommandCompleted");
        this->CurrentCommand->Completed.Add(CompletedDelegate);

        // Run the command.
        this->CurrentCommand->RunCommand(this->CurrentConsole, this->Instructions[0].Arguments);
        return;
    }
    
    // Command not found.
    this->CurrentConsole->WriteLine(FText::Format(NSLOCTEXT("Shell", "CommandNotFound", "{0}: Command not found."), FText::FromString(this->Instructions[0].Command)));
    this->CommandCompleted();
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
    this->GetConsole()->InitAdvancedGetLine(this->GetShellPrompt().ToString());
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
    FText Error = FText::GetEmpty();
    FPeacegateCommandInstruction InstructionData = this->ParseCommand(Input, this->GetUserContext()->GetHomeDirectory(), Error);

    // Output the error if there is any.
    if(!Error.IsEmpty())
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
			this->GetConsole()->WriteLine(FText::Format(NSLOCTEXT("Shell", "OutputFileIsDirectory", "error: {0}: Directory exists."), FText::FromString(InstructionData.OutputFile)));
			return;
		}
	}

    // The last piper console context used by a command.
    UConsoleContext* LastPiper = nullptr;

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
        TArray<FString> Tokens = this->Tokenize(Command, Home, Error);

        // If there's an error, fail.
        if(!Error.IsEmpty())
        {
            this->GetConsole()->WriteLine(Error);
            return;
        }

        // Get the name of the command.
        FString CommandNameToken = Tokens[0];

        // Create an instruction object for the command and place it in our instructions list.
        FCommandRunInstruction Instruction;
        Instruction.Arguments = Tokens;
        Instruction.Command = CommandNameToken; 

        // Are we piping?
        if(IsPiping)
        {
            // Create a piper context.
            UConsoleContext* Piper = nullptr;

            // Set the piper up with a valid user context.
            if(LastPiper)
				Piper = LastPiper->Pipe();
			else
				Piper = this->GetConsole()->Clone()->Pipe();
            
            // Use this piper as the instruction's intended console.
            // Also set it as the last piper.
            LastPiper = Piper;
            Instruction.IntendedContext = Piper;
        }
        else
        {
            // The console that will be used by the command.
            UConsoleContext* Piper = nullptr;
            
            // If we're not redirecting output to a file, then output console
            // becomes the same as our shell console and the command console is
            // is just a generic piper.
            if(InstructionData.OutputFile.IsEmpty())
            {
                if(LastPiper)
					Piper = LastPiper->PipeOut(this->GetConsole());
				else
					Piper = this->GetConsole();
            }
            else
            {
				// Create a new fifo bugffer to hold the file output.
				this->FileBuffer = NewObject<UPtyFifoBuffer>();

				// Store the file write instructions
				this->FilePath = InstructionData.OutputFile;
				this->FileOverwrite = InstructionData.Overwrites;

				// Redirect the piper to our fifo buffer
				if(LastPiper)
					Piper = LastPiper->Redirect(this->FileBuffer);
				else
					Piper = this->GetConsole()->Clone()->Redirect(this->FileBuffer);
            }

            // And use it for the command.
            Instruction.IntendedContext = Piper;
        }

        // Add the instruction to the list we're about to run.
        this->Instructions.Add(Instruction);

        // Annnd the loop index goes up!
        i++;
    }

    this->LastConsole = this->GetConsole();
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

TArray<FString> ACommandShell::Tokenize(const FString& InCommand, const FString& Home, FText& OutputError) 
{
	TArray<FString> tokens;
	FString current = TEXT("");
	bool escaping = false;
	bool inQuote = false;

	int cmdLength = InCommand.Len();

	TArray<TCHAR> cmd = InCommand.GetCharArray();

	for (int i = 0; i < cmdLength; i++)
	{
		TCHAR c = cmd[i];
		if (c == TEXT('\\'))
		{
			if (escaping == false)
				escaping = true;
			else
			{
				escaping = false;
				current.AppendChar(c);
			}
			continue;
		}
		if (escaping == true)
		{
			switch (c)
			{
			case TEXT(' '):
				current.AppendChar(TEXT(' '));
				break;
			case TEXT('~'):
				current.AppendChar(TEXT('~'));
				break;
			case TEXT('n'):
				current.AppendChar(TEXT('\n'));
				break;
			case TEXT('r'):
				current.AppendChar(TEXT('\r'));
				break;
			case TEXT('t'):
				current.AppendChar(TEXT('\t'));
				break;
			case TEXT('"'):
				current.AppendChar(TEXT('"'));
				break;
			default:
				OutputError = NSLOCTEXT("Shell", "UnrecognizedEscape", "unrecognized escape sequence.");
				return TArray<FString>();
			}
			escaping = false;
			continue;
		}
		if (c == TEXT('~'))
		{
			if (inQuote == false && current.IsEmpty())
			{
				current = current.Append(Home);
				continue;
			}
		}
		if (FChar::IsWhitespace(c))
		{
			if (inQuote)
			{
				current.AppendChar(c);
			}
			else
			{
				if (!current.IsEmpty())
				{
					tokens.Add(current);
					current = TEXT("");
				}
			}
			continue;
		}
		if (c == TEXT('"'))
		{
			inQuote = !inQuote;
			if (!inQuote)
			{
				if (i + 1 < cmdLength)
				{
					if (cmd[i + 1] == TEXT('"'))
					{
						OutputError = NSLOCTEXT("Shell", "StringSplice", "String splice detected. Did you mean to use a literal double-quote (\\\")?");
						return TArray<FString>();
					}
				}
			}
			continue;
		}
		current.AppendChar(c);
	}
	if (inQuote)
	{
		OutputError = NSLOCTEXT("Shell", "ExpectedDoubleQuote", "expected ending double-quote, got end of command instead.");
		return TArray<FString>();
	}
	if (escaping)
	{
		OutputError = NSLOCTEXT("Shell", "ExpectedEscapeSequence", "expected escape sequence, got end of command instead.");
		return TArray<FString>();
	}
	if (!current.IsEmpty())
	{
		tokens.Add(current);
		current = TEXT("");
	}
	return tokens;
}