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

#include "NonPlayerIntelligence.h"
#include "UProceduralGenerationEngine.h"

ANonPlayerIntelligence::ANonPlayerIntelligence()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ANonPlayerIntelligence::LinkToPeacenet(APeacenetWorldStateActor* InPeacenet)
{
    this->Peacenet = InPeacenet;
}

void ANonPlayerIntelligence::EndPlay(const EEndPlayReason::Type InReason)
{
    // If we're possessing an NPC, stop.
    this->PossessedUser = nullptr;

    // Unlink from The Peacenet.
    this->Peacenet = nullptr;
}
	
void ANonPlayerIntelligence::Tick(float DeltaTime)
{
    // Do not tick if The Peacenet isn't active yet.
    if(!this->Peacenet) return;

    // If we're not possessing an NPC, we're going to look for one to possess.
    if(!this->PossessedUser)
    {
        int Index = 0;
        int Counter = this->Peacenet->SaveGame->Characters.Num() * 10;
        while(Counter > 0 && this->Peacenet->SaveGame->Characters[Index].CharacterType != EIdentityType::NonPlayer)
        {
            FPeacenetIdentity& Identity = this->Peacenet->SaveGame->Characters[Index];
            if(Identity.CharacterType == EIdentityType::NonPlayer)
            {
                Counter -= this->Peacenet->GetProcgen()->GetRNG().RandRange(1, 10);
            }
            
            Index++;
            if(this->Peacenet->SaveGame->Characters.Num() <= Index)
            {
                Index = 0;
            }            
        }

        // Grab the chosen identity from the loop above.
        FPeacenetIdentity& TheChosenOne = this->Peacenet->SaveGame->Characters[Index];

        // This gets (or creates) a system context for that identity.  As soon as this happens,
        // that NPC's gonna have a log entry of their computer turning on....
        USystemContext* TheChosenSystemContext = this->Peacenet->GetSystemContext(TheChosenOne.ID);

        // And this allows us to essentially play the game as that NPC, programmatically.
        // ARTIFICIAL INTELLIGENCE DO EVERYTHING NOW.
        // NOBODY IS REALLY HERE.
        this->PossessedUser = TheChosenSystemContext->GetUserContext(1);
    }
}
