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

#include "ProfilerCommand.h"

AProfilerCommand::AProfilerCommand()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AProfilerCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    InConsole->WriteLine(FText::Format(NSLOCTEXT("Profiler", "Discovering", "Discovering new people and hubs based off identity of &6&*{0}&7&r..."), FText::FromString(this->GetUserContext()->GetCharacterName())));
    this->NodesLeft = InConsole->GetUserContext()->ScanForAdjacentNodes();
    InConsole->WriteLine(FText::GetEmpty());
}

void AProfilerCommand::Tick(float InDeltaSeconds)
{
    if(TimeToScan > 0)
    {
        TimeToScan -= InDeltaSeconds;
        TimeToNextDiscover = 0.f;
    }
    else
    {
        if(TimeToNextDiscover > 0)
        {
            TimeToNextDiscover -= InDeltaSeconds;
        }
        else
        {
            if(NodesLeft.Num())
            {
                TimeToNextDiscover = MaxDiscoverDelay;

                FAdjacentNodeInfo Node = NodesLeft[0];
                this->NodesLeft.RemoveAt(0);

                this->GetConsole()->WriteLine(FText::Format(NSLOCTEXT("Profiler", "DiscoveredPerson", "Discovered person &3&*{0}&7&r."), FText::FromString(Node.NodeName)));

                this->GetConsole()->GetUserContext()->GetPeacenet()->UpdateMaps();
            }
            else
            {
                // Send a game event that we've scanned for nodes.
                this->SendGameEvent("NodeScanComplete", {
                    { "Identity", FString::FromInt(this->GetUserContext()->GetOwningSystem()->GetCharacter().ID)}
                });

                this->Complete();
                return;
            }
        }
    }
}