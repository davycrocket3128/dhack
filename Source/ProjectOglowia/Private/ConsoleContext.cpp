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


#include "ConsoleContext.h"
#include "CommonUtils.h"
#include "UserContext.h"
#include "SystemContext.h"

void UConsoleContext::Setup(UPtyStream* InPty, UUserContext* InUserContext)
{
	// Crash if we're not being given a valid user context.
	check(InUserContext);
	check(InPty);

	// Crash if we already have a user context.
	check(!this->GetUserContext());
	check(!this->Pty);

	// Set our user context.
	this->UserContext = InUserContext;
	this->Pty = InPty;

	// Set the working directory to the user's home.
	this->WorkingDirectory = this->UserContext->GetHomeDirectory();
}

FString UConsoleContext::GetWorkingDirectory()
{
	return this->WorkingDirectory;
}

UUserContext* UConsoleContext::GetUserContext()
{
	return this->UserContext;
}

bool UConsoleContext::GetLine(FString& OutLine)
{
	FString line;
	TCHAR c;
	bool res = false;
	while(this->Pty->ReadChar(c))
	{
		res = true;
		line += c;
	}
	OutLine = line;
	return res;
}

void UConsoleContext::SetWorkingDirectory(const FString & InPath)
{
	if (this->GetUserContext()->GetFilesystem()->DirectoryExists(InPath))
	{
		this->WorkingDirectory = InPath;
	}
}

FString UConsoleContext::CombineWithWorkingDirectory(const FString & InPath)
{
	if (InPath.StartsWith("/"))
		return this->GetUserContext()->GetFilesystem()->ResolveToAbsolute(InPath);
	return this->GetUserContext()->GetFilesystem()->ResolveToAbsolute(WorkingDirectory + TEXT("/") + InPath);
}

FString UConsoleContext::GetDisplayWorkingDirectory()
{
	if (WorkingDirectory.StartsWith(this->GetUserContext()->GetHomeDirectory()))
	{
		FString NewWorkingDirectory(WorkingDirectory);
		NewWorkingDirectory.RemoveFromStart(this->GetUserContext()->GetHomeDirectory());
		return TEXT("~") + NewWorkingDirectory;
	}
	return WorkingDirectory;
}

void UConsoleContext::WriteToPty(FString str)
{
	check(this->Pty);

	TArray<TCHAR> buf = str.GetCharArray();
	this->Pty->Write(buf, 0, buf.Num());
}

void UConsoleContext::MakeBold()
{
	this->WriteToPty("\x1B)");
}

void UConsoleContext::MakeBoldItalic()
{
	this->MakeBold();
	this->MakeItalic();
}

void UConsoleContext::MakeItalic()
{
	// TODO: Don't know how to do italic with ANSI escape sequences yet.  We may need to invent our own sequence.
}

void UConsoleContext::ResetFormatting()
{
	this->WriteToPty("\x1B[0m");
}

void UConsoleContext::InvertColors()
{
	this->WriteToPty("\x1B[7m");
}

void UConsoleContext::SetColor(ETerminalColor InColor)
{
	// TODO: Map terminal colors to ANSI escape codes, also support background colors.
}

void UConsoleContext::ReadLine(UObject* WorldContextObject, FLatentActionInfo LatentInfo, FString& OutText)
{
	// TODO: Blueprint Latent Action for reading a line from the terminal.

	// Unlike before with old terminal, we should just call GetLine every frame
	// until we get a line of text then report the action as done.

	// Making this thing work without multithreading man... Wow...
}

void UConsoleContext::Clear()
{
	this->WriteToPty("\x1B[2J");
}

void UConsoleContext::Write(const FText& InText, float InDelaySeconds)
{
	// TODO: Have the game wait InDelaySeconds before actually writing the text to the pty.
	// The pty doesn't have a concept of time, it's just a set of FIFO buffers, so
	// we'll ignore the delay for now. Blueprints can, of course, use the "Delay" node,
	// as Terminal Command Blueprints are Actors and thus have access to Blueprint Latent
	// Actions.  The Delay argument for this call is for BPs that don't support that, or
	// C++-driven terminal commands (i.e bash, gsfconsole, etc)

	// Convert the text to a string so we can write it to the pty.
	FString AsString = InText.ToString();

	// Write it.
	this->WriteToPty(AsString);
}

void UConsoleContext::WriteLine(const FText& InText, float InDelaySeconds)
{
	// Write the text as usual...
	this->Write(InText, InDelaySeconds);

	// with a CRLF at the end.
	this->WriteToPty("\r\n");
}

void UConsoleContext::OverwriteLine(const FText& InText, float InDelaySeconds)
{
	// Forces the game to wait before writing the escape codes.
	this->Write(FText::GetEmpty(), InDelaySeconds);

	// Return the cursor to the start of the line.
	this->WriteToPty("\r");

	// Erase the current line.
	this->WriteToPty("\x1B[2K");

	// Write the new text.
	this->Write(InText, 0);
}

UConsoleContext* UConsoleContext::Pipe()
{
	// TODO
	return this;
}

UConsoleContext* UConsoleContext::Redirect(UPtyFifoBuffer* InBuffer)
{
	// TODO
	return this;
}

UConsoleContext* UConsoleContext::PipeOut(UConsoleContext* InConsole)
{
	// TODO
	return this;
}

UPtyStream* UConsoleContext::GetPty()
{
	return this->Pty;
}

FIntPoint UConsoleContext::GetCursorPosition()
{
	this->WriteToPty("\x1B[6n");
	FString Line;
	while(!this->GetLine(Line))
	{
		// force the terminal to respond.
		this->UpdateTty.ExecuteIfBound();
	}

	check(Line.StartsWith("\x1B[") && Line.Contains(";") && Line.EndsWith("R"));
	Line = Line.Replace(TEXT("\x1B["), TEXT("")).Replace(TEXT("R"), TEXT(""));
	FString Row = Line.Left(Line.Find(TEXT(";")));
	Line.RemoveAt(0, Line.Find(";") + 1);
	FString Col = Line;

	int RowNum = FCString::Atoi(*Row);
	int ColNum = FCString::Atoi(*Col);

	return FIntPoint(ColNum, RowNum);
}

int UConsoleContext::GetCursorColumn()
{
	return this->GetCursorPosition().X;
}

int UConsoleContext::GetCursorRow()
{
	return this->GetCursorPosition().Y;
}

void UConsoleContext::OnTtyUpdate(UObject* Callee, FName FunctionName)
{
	check(!this->UpdateTty.IsBound());
	this->UpdateTty.BindUFunction(Callee, FunctionName);
}

void UConsoleContext::OnGetSize(UObject* Callee, FName FunctionName)
{
    check(!this->GetSizeDelegate.IsBound());
    this->GetSizeDelegate.BindUFunction(Callee, FunctionName);
}

FIntPoint UConsoleContext::GetTerminalSize()
{
    int w = 0;
    int h = 0;

    this->GetSizeDelegate.ExecuteIfBound(h, w);

    return FIntPoint(w, h);
}

int UConsoleContext::GetRows()
{
    return this->GetTerminalSize().Y;
}

int UConsoleContext::GetColumns()
{
    return this->GetTerminalSize().X;
}

void UConsoleContext::Beep()
{
	this->WriteToPty("\x7");
}

void UConsoleContext::InitAdvancedGetLine(FString Prompt)
{
	check(!this->LineNoise);

	this->LineNoise = NewObject<ULineNoise>();
	this->LineNoise->SetConsole(this, Prompt);
}

bool UConsoleContext::UpdateAdvancedGetLine(FString& Line)
{
	check(this->LineNoise);
	bool result = this->LineNoise->GetLine(Line);
	if(result) {
		this->LineNoise = nullptr;
		this->GetPty()->RawMode(false);
		this->WriteToPty("\r\n");
	}
	return result;
}