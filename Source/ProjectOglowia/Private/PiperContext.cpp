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


#include "PiperContext.h"
#include "PTerminalWidget.h"

UPTerminalWidget* UPiperContext::GetTerminal()
{
	if(this->Output)
		return this->Output->GetTerminal();
	return Super::GetTerminal();
}

void UPiperContext::SetupPiper(UPiperContext* InInput, UConsoleContext* InOutput)
{
    check(!this->Input);
    check(!this->Output);

    this->Input = InInput;
    this->Output = InOutput;
}

bool UPiperContext::GetLine(FString& OutLine)
{
	if (Input)
	{
		if (Input->Log.IsEmpty())
			return false;

		int NewlineIndex = -1;
		if (Input->Log.ToString().FindChar(TEXT('\n'), NewlineIndex))
		{
			OutLine = Input->Log.ToString().Left(NewlineIndex);
			
			FString InputLogStr = Input->Log.ToString();
			InputLogStr.RemoveAt(0, NewlineIndex + 1);
			Input->Log = FText::FromString(InputLogStr);
		}
		else {
			OutLine = Input->Log.ToString();
			Input->Log = FText::GetEmpty();
		}
		return true;
	}
	else
	{
		return Super::GetLine(OutLine);
	}
}


void UPiperContext::Write(const FText& InText, float DeltaSeconds)
{
	if (Output)
	{
		Output->Write(InText, DeltaSeconds);
	}
	else {
		Log = FText::Format(NSLOCTEXT("Terminal", "Line", "{0}{1}"), Log, InText);
	}
}

FString UPiperContext::GetInputBuffer()
{
	if(this->Input)
	{
		return this->Input->GetLog().ToString();
	}
	else
	{
		return FString();
	}
}


void UPiperContext::Clear()
{
	if (Output)
		Output->Clear();
	else
		Log = FText::GetEmpty();
}

void UPiperContext::WriteLine(const FText& InText, float DeltaSeconds)
{
    Write(FText::Format(NSLOCTEXT("Terminal", "Line", "{0}{1}"), InText, UPTerminalWidget::NewLine()), DeltaSeconds);
}

UConsoleContext* UPiperContext::CreateChildContext(USystemContext* InSystemContext, int InUserID)
{
	if (Output)
	{
		return Output->CreateChildContext(InSystemContext, InUserID);
	}
	else if (Input)
	{
		return Input->CreateChildContext(InSystemContext, InUserID);
	}

	return this;
}


void UPiperContext::OverwriteLine(const FText& InText, float DeltaSeconds)
{
	if (Output)
	{
		Output->OverwriteLine(InText, DeltaSeconds);
	}
	else 
    {
		WriteLine(InText, DeltaSeconds);
	}
}


FText UPiperContext::GetLog()
{
    return this->Log;
}

void UPiperContext::ReadLine(UObject* WorldContextObject, struct FLatentActionInfo LatentInfo, FString& OutText)
{
	if (Input)
	{
		this->GetLine(OutText);
		UWorld* world = WorldContextObject->GetWorld();
		if (world)
		{
			FLatentActionManager& LatentActionManager = world->GetLatentActionManager();
			if (LatentActionManager.FindExistingAction<FPlaceboLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == NULL)
			{
				//Here in a second, once I confirm the project loads, we need to see whats wrong with this
				LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FPlaceboLatentAction(LatentInfo));
			}
		}
	}
	else 
	{
		this->GetTerminal()->ReadLine(WorldContextObject, LatentInfo, OutText);
	}
}
