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

#include "ReverseShell.h"
#include "ReverseShellCommandHandler.h"
#include "UserContext.h"
#include "PeacegateFileSystem.h"
#include "FileUtilities.h"

bool AReverseShell::RunSpecialCommand(UConsoleContext* InConsole, FString InCommand, TArray<FString> Arguments) 
{
    // Commands should work no matter what the case is.
    InCommand = InCommand.ToLower();

    if(InCommand == "upload")
    {
        // First argument is source. Second argument is destination.
        if(Arguments.Num() < 3)
        {
            InConsole->WriteLine(NSLOCTEXT("ReverseShell", "UploadUsage", "Usage: upload <source> <destination>"));
            this->FinishSpecialCommand();
            return true;
        }

        // Get the source and destination from the argument list.  Docopt is no bueno for these commands.
        FString Source = Arguments[1];
        FString Destination = InConsole->CombineWithWorkingDirectory(Arguments[2]);

        // Get the filesystem of the source computer.
        UPeacegateFileSystem* SourceFileSystem = this->GetUserContext()->GetHacker()->GetFilesystem();

        // And for the destination computer.
        UPeacegateFileSystem* DestinationFileSystem = this->GetUserContext()->GetFilesystem();

        // Check if the source file actually exists.
        if(!SourceFileSystem->FileExists(Source))
        {
            InConsole->WriteLine(FText::Format(NSLOCTEXT("ReverseShell", "FileNotFound", "Error: File not found: {0}"), FText::FromString(Source)));
            this->FinishSpecialCommand();
            return true;
        }

        // If the destination path is a directory, then the destination becomes <destination>/<sourceFileName>.
        if(DestinationFileSystem->DirectoryExists(Destination))
        {
            FString SourceName = UFileUtilities::GetNameFromPath(Source);

            if(Destination.EndsWith("/"))
                Destination = Destination + SourceName;
            else
                Destination = Destination + "/" + SourceName;
        }

        // Read the file record info at the source.
        FFileRecord SourceRecord = SourceFileSystem->GetFileRecord(Source);

        // If it's a text file then we'll use the ReadText and WriteText methods.
        if(SourceRecord.RecordType == EFileRecordType::Text)
        {
            EFilesystemStatusCode StatusCode;
            FString SourceText;

            // Read the text of the source.
            if(SourceFileSystem->ReadText(Source, SourceText, StatusCode))
            {
                // Write the source text to the destination file.
                DestinationFileSystem->WriteText(Destination, SourceText);
            }
            else
            {
                InConsole->WriteLine(NSLOCTEXT("ReverseShell", "SourceFileError", "Error occured while reading source file text."));
                this->FinishSpecialCommand();
                return true;
            }
        }
        else
        {
            // Write the low-level record directly to the destination filesystem.
            DestinationFileSystem->SetFileRecord(Destination, SourceRecord.RecordType, SourceRecord.ContentID);
        }

        InConsole->WriteLine(FText::Format(NSLOCTEXT("ReverseShell", "UploadSuccess", "Uploaded {0} to {1} successfully."), FText::FromString(Source), FText::FromString(Destination)));
        this->FinishSpecialCommand();

        // Let the game event system know we've just uploaded a file.
        this->SendGameEvent("FileUpload", {
            { "SourceIdentity", FString::FromInt(this->GetUserContext()->GetHacker()->GetOwningSystem()->GetCharacter().ID) },
            { "SourcePath", Source },
            { "DestinationIdentity", FString::FromInt(this->GetUserContext()->GetOwningSystem()->GetCharacter().ID) },
            { "DestinationPath", Destination }
        });

        return true;
    }
    if(InCommand == "download")
    {

        // First argument is source. Second argument is destination.
        if(Arguments.Num() < 3)
        {
            InConsole->WriteLine(NSLOCTEXT("ReverseShell", "DownloadUsage", "Usage: download <source> <destination>"));
            this->FinishSpecialCommand();
            return true;
        }

        // Get the source and destination from the argument list.  Docopt is no bueno for these commands.
        FString Source = InConsole->CombineWithWorkingDirectory(Arguments[1]);
        FString Destination = Arguments[2];

        // Get the filesystem of the source computer.
        UPeacegateFileSystem* SourceFileSystem = this->GetUserContext()->GetFilesystem();

        // And for the destination computer.
        UPeacegateFileSystem* DestinationFileSystem = this->GetUserContext()->GetHacker()->GetFilesystem();

        // Check if the source file actually exists.
        if(!SourceFileSystem->FileExists(Source))
        {
            InConsole->WriteLine(FText::Format(NSLOCTEXT("ReverseShell", "FileNotFound", "Error: File not found: {0}"), FText::FromString(Source)));
            this->FinishSpecialCommand();
            return true;
        }

        // If the destination path is a directory, then the destination becomes <destination>/<sourceFileName>.
        if(DestinationFileSystem->DirectoryExists(Destination))
        {
            FString SourceName = UFileUtilities::GetNameFromPath(Source);

            if(Destination.EndsWith("/"))
                Destination = Destination + SourceName;
            else
                Destination = Destination + "/" + SourceName;
        }

        // Read the file record info at the source.
        FFileRecord SourceRecord = SourceFileSystem->GetFileRecord(Source);

        // If it's a text file then we'll use the ReadText and WriteText methods.
        if(SourceRecord.RecordType == EFileRecordType::Text)
        {
            EFilesystemStatusCode StatusCode;
            FString SourceText;

            // Read the text of the source.
            if(SourceFileSystem->ReadText(Source, SourceText, StatusCode))
            {
                // Write the source text to the destination file.
                DestinationFileSystem->WriteText(Destination, SourceText);
            }
            else
            {
                InConsole->WriteLine(NSLOCTEXT("ReverseShell", "SourceFileError", "Error occured while reading source file text."));
                this->FinishSpecialCommand();
                return true;
            }
        }
        else
        {
            // Write the low-level record directly to the destination filesystem.
            DestinationFileSystem->SetFileRecord(Destination, SourceRecord.RecordType, SourceRecord.ContentID);
        }

        InConsole->WriteLine(FText::Format(NSLOCTEXT("ReverseShell", "DownloadSuccess", "Downloaded {0} to {1} successfully."), FText::FromString(Source), FText::FromString(Destination)));
        this->FinishSpecialCommand();

        // Let the game event system know we've just downloaded a file.
        this->SendGameEvent("FileDownload", {
            { "SourceIdentity", FString::FromInt(this->GetUserContext()->GetOwningSystem()->GetCharacter().ID) },
            { "SourcePath", Source },
            { "DestinationIdentity", FString::FromInt(this->GetUserContext()->GetHacker()->GetOwningSystem()->GetCharacter().ID) },
            { "DestinationPath", Destination }
        });

        return true;
    }
    
    return false;
}