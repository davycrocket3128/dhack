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

#include "StartHackStoryCharacterTask.h"
#include "PeacenetWorldStateActor.h"

void UStartHackStoryCharacterTask::NativeTick(float InDeltaSeconds)
{
    if(this->IsInHack)
    {
        if(!this->IsTutorialActive())
        {
            if(this->IsSet("gigasploit.firstScan"))
            {
                this->ShowTutorialIfNotSet("tuts.gigasploit.analyze", 
                    NSLOCTEXT("Tutorials", "GigasploitAnalyzeTitle", "Analyzing a port"),
                    NSLOCTEXT("Tutorials", "GigasploitAnalyze", "Gigasploit Analyze Tutorial")
                );
                this->ShowTutorialIfNotSet("tuts.gigasploit.exploits", 
                    NSLOCTEXT("Tutorials", "GigasploitExploitsTitle", "Exploits"),
                    NSLOCTEXT("Tutorials", "GigasploitExploits", "Gigasploit Exploits Tutorial")
                );
                this->ShowTutorialIfNotSet("tuts.gigasploit.payloads", 
                    NSLOCTEXT("Tutorials", "GigasploitPayloadsTitle", "Payloads"),
                    NSLOCTEXT("Tutorials", "GigasploitPayloads", "Gigasploit Payloads Tutorial")
                );
                this->ShowTutorialIfNotSet("tuts.gigasploit.use", 
                    NSLOCTEXT("Tutorials", "GigasploitUseTitle", "Using"),
                    NSLOCTEXT("Tutorials", "GigasploitUse", "Gigasploit Use Tutorial")
                );
                this->ShowTutorialIfNotSet("tuts.gigasploit.attack", 
                    NSLOCTEXT("Tutorials", "GigasploitAttackTitle", "Attack"),
                    NSLOCTEXT("Tutorials", "GigasploitAttack", "Gigasploit Attack Tutorial")
                );
            }
        }
    }
}

void UStartHackStoryCharacterTask::NativeStart()
{
    // Check that we have a story character to hack.
    check(this->StoryCharacter);

    // Get the target entity.
    bool result = this->GetPlayerUser()->GetPeacenet()->SaveGame->GetStoryCharacterID(this->StoryCharacter, this->TargetEntity);
    check(result);
}

void UStartHackStoryCharacterTask::NativeEvent(FString EventName, TMap<FString, FString> InEventArgs)
{
    if(EventName == "HackStart" && !IsInHack)
    {
        if(InEventArgs["Identity"] == FString::FromInt(this->TargetEntity))
        {
            this->IsInHack = true;
            this->ShowTutorialIfNotSet("tuts.gigasploit.welcome",
                    NSLOCTEXT("CommandNames", "Gigasploit", "Gigasploit Framework Console"),
                    NSLOCTEXT("Tutorials", "GigasploitWelcome", "Gigasploit Welcome Tutorial Text")
                );
        }
    }
}