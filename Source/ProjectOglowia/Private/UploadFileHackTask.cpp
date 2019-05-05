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

#include "UploadFileHackTask.h"

void UUploadFileHackTask::NativeStart()
{
    check(this->FileRecordChecker);
    this->SetObjectiveText(this->ObjectiveText);
}

void UUploadFileHackTask::NativeGameEvent(FString Event, TMap<FString, FString> Arguments)
{
    // Listen to FileUpload events only.
    if(Event == "FileUpload")
    {
        // Check that the source and destination identities are ours.
        FString SourceIdentity = Arguments["SourceIdentity"];
        FString DestinationIdentity = Arguments["DestinationIdentity"];

        FString OurSource = FString::FromInt(this->GetPlayerUser()->GetOwningSystem()->GetCharacter().ID);
        FString OurDestination = FString::FromInt(this->GetHackedSystem()->GetCharacter().ID);

        // If they match then we can start grabbing filesystem contexts and reading the destination file to make sure it is the file
        // we want the user to upload.
        if(SourceIdentity == OurSource && DestinationIdentity == OurDestination)
        {
            // Use our file record checker to check if the file matches what we want.
            if(this->FileRecordChecker->FileRecordMatches(this->GetHackedSystem(), Arguments["DestinationPath"]))
            {
                // Complete the objective!
                this->Finish();
            }
        }       
    }
}