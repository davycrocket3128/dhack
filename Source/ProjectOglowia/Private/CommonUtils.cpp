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

#include "CommonUtils.h"
#include "Engine/Font.h"
#include "PeacenetSaveGame.h"
#include "Parse.h"
#include "PlatformApplicationMisc.h"
#include "SystemContext.h"

FLinearColor UCommonUtils::GetForegroundColor(FLinearColor InColor)
{
	float r = InColor.R;
	float g = InColor.G;
	float b = InColor.B;

	if((r + g + b) / 3 <= 0.6f)
	{
		return FLinearColor(1.f, 1.f, 1.f, 1.f);
	}
	else {
		return FLinearColor(0.f, 0.f, 0.f, 1.f);
	}
}

FText UCommonUtils::GetRichTextSegment(const FText& InSourceText, int InEndIndex, bool& FoundIncompleteTag, int& TrueEndIndex)
{
	// Convert the source text to a string, that way we can more easily perform parsing
	// on it.
	FString SourceString = InSourceText.ToString();

	// Are we currently in a rich tag?
	bool InRichTag = false;

	// The current tag text.
	FString TagIdentifier = "";

	// Keep track of the loop index outside of the loop.
	int i = 0;

	// Go through every character in the source string UNTIL THE END INDEX.
	for(i = 0; i < SourceString.Len() && i < InEndIndex; i++)
	{
		// Get the current character.
		TCHAR c = SourceString[i];

		// Are we outside of a tag?
		if(!InRichTag)
		{
			// Check if the current character is the start of a tag.
			if(c == '<')
			{
				// We're now in a tag.
				InRichTag = true;
				TagIdentifier = FString::Chr(c);
				continue;
			}
		}
		else
		{
			// If we are in a tag, add the current character to the identifier...
			TagIdentifier = TagIdentifier.AppendChar(c);

			// And check if the current character is the ending of a tag.
			if(c == '>')
			{
				// We're no longer in the tag.
				InRichTag = false;
			}
		}
	}

	// If we're still in a tag then continue until we hit the end tag.
	if(InRichTag)
	{
		FoundIncompleteTag = true;
		for(i = i; i < SourceString.Len(); i++)
		{
			TCHAR c = SourceString[i];
			TagIdentifier = TagIdentifier.AppendChar(c);
			if(c == '>')
			{
				InRichTag = false;
				break;
			}
		}	
	}
	else
	{
		FoundIncompleteTag = false;
	}

	// Wherever i is now, we can grab a substring starting from index 0 to i and that's
	// the text we're going to return.
	FString ReturnedText = SourceString.Left(i);

	// Unless the tag identifier isn't empty and is not equal to "</>".
	if(TagIdentifier.Len() && TagIdentifier != "</>")
	{
		// Then we append "</>" to the end of returned text.
		ReturnedText = ReturnedText.Append("</>");
	}

	// Output where we left off in the string.
	TrueEndIndex = i;

	// And then we return it.
	return FText::FromString(ReturnedText);
}

int UCommonUtils::GetRAMAmount(ERAMAmount InRAMAmount)
{
	switch(InRAMAmount)
	{
		case ERAMAmount::Level0:
			return 256;
		case ERAMAmount::Level1:
			return 512;
		case ERAMAmount::Level2:
			return 1024;
		case ERAMAmount::Level3:
			return 2048;
		case ERAMAmount::Level4:
			return 4096;
		case ERAMAmount::Level5:
			return 8192;
		case ERAMAmount::Level6:
		default:
			return 16384;
		
	}
}

int UCommonUtils::GetRAMUsage(ERAMUsage InRAMUsage)
{
	switch(InRAMUsage)
	{
		case ERAMUsage::None:
			return 0;
		case ERAMUsage::Low:
			return 64;
		case ERAMUsage::Medium:
			return 128;
		case ERAMUsage::High:
		default:
			return 256;
	}
}

void UCommonUtils::ParseURL(FString InURL, FString& OutUsername, FString& OutHost, int& OutPort, FString& OutPath, bool& HasPath, bool& HasUser, bool& HasPort)
{
	HasPort=false;
	HasPath=false;
	HasUser=false;

	FString DefaultUser = "root";
	FString UserDelim = "@";
	FString PortDelim = ":";
	FString PathDelim = "/";

	if(InURL.Contains(PathDelim))
	{
		int PathStart = InURL.Find(PathDelim);
		OutPath = InURL.RightChop(PathStart);
		InURL.RemoveFromEnd(OutPath);
		HasPath = true;
	}

	if(InURL.Contains(PortDelim))
	{
		int PortStart = InURL.Find(PortDelim, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		FString PortString = InURL.RightChop(PortStart+1);
		if(PortString.Len())
		{
			HasPort = true;
			OutPort = FCString::Atoi(*PortString);
		}
		InURL = InURL.Left(PortStart);
	}

	if(InURL.Contains(UserDelim))
	{
		int UserEnd = InURL.Find(UserDelim);
		OutUsername = InURL.Left(UserEnd);
		InURL = InURL.RightChop(UserEnd+1);
		HasUser = true;
	}

	OutHost = InURL;
}

void UCommonUtils::GetFriendlyFileOpenText(EFileOpenResult InResult, FString& OutTitle, FString& OutDescription)
{
	OutTitle = "";
	OutDescription = "";

	switch(InResult)
	{
		case EFileOpenResult::FileNotFound:
			OutTitle = "File not found";
			OutDescription = "The system could not find the file specified.";
			break;
		case EFileOpenResult::PermissionDenied:
			OutTitle = "Access denied";
			OutDescription = "You don't have permission to open this file.";
			break;
		case EFileOpenResult::NoSuitableProgram:
			OutTitle = "Can't open file";
			OutDescription = "There are no programs installed that can open this file.";
			break;
	}
}

bool UCommonUtils::GetClipboardText(FString& OutText)
{
	FPlatformApplicationMisc::ClipboardPaste(OutText);
	return OutText.Len();
}

void UCommonUtils::PutClipboardText(FString InText)
{
	FPlatformApplicationMisc::ClipboardCopy(*InText);
}

FText UCommonUtils::GetFriendlyFilesystemStatusCode(const EFilesystemStatusCode InStatusCode)
{
	switch (InStatusCode)
	{
	case EFilesystemStatusCode::OK:
		return FText();
	case EFilesystemStatusCode::DirectoryNotEmpty:
		return NSLOCTEXT("Peacegate", "DirectoryNotEmpty", "Directory not empty.");
	case EFilesystemStatusCode::FileOrDirectoryExists:
		return NSLOCTEXT("Peacegate", "FileOrDirectoryExists", "File or directory exists.");
	case EFilesystemStatusCode::FileOrDirectoryNotFound:
		return NSLOCTEXT("Peacegate", "FileOrDirectoryNotFound", "File or directory not found.");
	case EFilesystemStatusCode::PermissionDenied:
		return NSLOCTEXT("Peacegate", "PermissionDenied", "Permission denied.");


	default:
		return NSLOCTEXT("Peacegate", "UnknownError", "An unknown error has occurred.");
	}
}

UPeacegateFileSystem * UCommonUtils::CreateFilesystem(USystemContext* InSystemContext, int InUserID)
{
	UPeacegateFileSystem* FS = NewObject<UPeacegateFileSystem>();
	FS->SystemContext = InSystemContext;
	FS->Initialize(InUserID);
	return FS;
}

FLinearColor UCommonUtils::GetTerminalColor(ETerminalColor InColor)
{
	switch (InColor)
	{
	case ETerminalColor::Black:
	default:
		return FLinearColor::Black;
	case ETerminalColor::Blue:
		return FLinearColor(0.f, 0.f, 0.5f, 1.f);
	case ETerminalColor::Red:
		return FLinearColor(0.5f, 0.f, 0.f, 1.f);
	case ETerminalColor::Green:
		return FLinearColor(0.f, 0.5f, 0.f, 1.f);
	case ETerminalColor::Aqua:
		return FLinearColor(0.f, 0.5f, 0.5f, 1.f);
	case ETerminalColor::Purple:
		return FLinearColor(0.5f, 0.f, 0.5f, 1.f);
	case ETerminalColor::Yellow:
		return FLinearColor(0.5f, 0.5f, 0.f, 1.f);
	case ETerminalColor::Gray:
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
	case ETerminalColor::White:
		return FLinearColor(0.75F, 0.75f, 0.75f, 1.f);
	case ETerminalColor::LightBlue:
		return FLinearColor(0.f, 0.f, 1.f, 1.f);
	case ETerminalColor::LightGreen:
		return FLinearColor(0.f, 1.f, 0.f, 1.f);
	case ETerminalColor::LightRed:
		return FLinearColor(1.f, 0.f, 0.f, 1.f);
	case ETerminalColor::LightAqua:
		return FLinearColor(0.f, 1.f, 1.f, 1.f);
	case ETerminalColor::LightPurple:
		return FLinearColor(1.f, 0.f, 1.f, 1.f);
	case ETerminalColor::LightYellow:
		return FLinearColor(1.f, 1.f, 0.f, 1.f);
	case ETerminalColor::BrightWhite:
		return FLinearColor(1.f, 1.f, 1.f, 1.f);
	}
}

FString UCommonUtils::GetTerminalColorCode(ETerminalColor InColor)
{
	switch (InColor)
	{
	case ETerminalColor::Black:
	default:
		return "&0";
	case ETerminalColor::Blue:
		return "&1";
	case ETerminalColor::Red:
		return "&4";
	case ETerminalColor::Green:
		return "&2";
	case ETerminalColor::Aqua:
		return "&3";
	case ETerminalColor::Purple:
		return "&5";
	case ETerminalColor::Yellow:
		return "&6";
	case ETerminalColor::Gray:
		return "&8";
	case ETerminalColor::White:
		return "&7";
	case ETerminalColor::LightBlue:
		return "&9";
	case ETerminalColor::LightGreen:
		return "&A";
	case ETerminalColor::LightRed:
		return "&C";
	case ETerminalColor::LightAqua:
		return "&B";
	case ETerminalColor::LightPurple:
		return "&D";
	case ETerminalColor::LightYellow:
		return "&E";
	case ETerminalColor::BrightWhite:
		return "&F";
	}
}

bool UCommonUtils::IsColorCode(FString InControlCode, ETerminalColor& OutColor)
{
	if (!InControlCode.StartsWith("&"))
		return false;

	if (InControlCode.Len() != 2)
		return false;

	// Get rid of the "&" at the start so we can parse as hex
	InControlCode.RemoveAt(0, 1);

	int Code = FParse::HexNumber(InControlCode.GetCharArray().GetData());
	if (Code == 0 && InControlCode != "0")
		return false; //HexNumber returns 0 if the code is invalid.

	// Enums are awesome because we can do this.
	OutColor = (ETerminalColor)Code;
	return true;
}

void UCommonUtils::MeasureChar(const TCHAR InChar, const FSlateFontInfo & InSlateFont, float & OutWidth, float & OutHeight)
{
	float x, y = 0;

	const UFont* RawFont = Cast<UFont>(InSlateFont.FontObject);

	float MeasureSize = RawFont->LegacyFontSize;
	float RealSize = InSlateFont.Size;
	float Scale = RealSize / MeasureSize;

	RawFont->GetCharSize(InChar, x, y);

	OutWidth = x * Scale;
	OutHeight = y * Scale;
}

void UCommonUtils::SetEnableBloom(UCameraComponent * InCamera, bool InEnableBloom)
{
	auto PostProcessSettings = InCamera->PostProcessSettings;
	PostProcessSettings.bOverride_BloomIntensity = InEnableBloom;
	InCamera->PostProcessSettings = PostProcessSettings;
}

void UCommonUtils::ParseCharacterName(const FString InCharacterName, FString & OutUsername, FString & OutHostname)
{
	// No sense doing this if there's only whitespace
	if (InCharacterName.IsEmpty())
		return;

	// Unix usernames can only be lower-case.
	FString NameString = InCharacterName.ToLower();

	// this will be the username.
	FString FirstName;
	FString Rem;

	// These characters are valid as name chars.
	const FString ValidUnixUsernameChars = TEXT("abcdefghijklmnopqrstuvwxyz0123456789_-");

	// the first char that isn't valid.
	TCHAR InvalidChar = TEXT('\0');

	// the chars in the name string
	TArray<TCHAR> NameChars = NameString.GetCharArray();

	for (auto Char : NameChars)
	{
		if (!ValidUnixUsernameChars.Contains(FString(1, &Char)))
		{
			InvalidChar = Char;
			break;
		}
	}

	// Did that for loop above change us?
	if (InvalidChar != TEXT('\0'))
	{
		NameString.Split(FString(1, &InvalidChar), &FirstName, &Rem);
	}
	else
	{
		FirstName = NameString;
	}

	OutUsername = FirstName;
	OutHostname = FirstName + TEXT("-pc");
}

float UCommonUtils::GetRotation(FVector2D InA, FVector2D InB)
{
	float adj = InA.X - InB.X;
    float opp = InA.Y - InB.Y;
    return FMath::RadiansToDegrees<float>(FMath::Atan2(opp, adj)/* - FMath::PI*/);
}