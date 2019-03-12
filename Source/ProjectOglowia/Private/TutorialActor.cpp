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

#include "TutorialActor.h"

ATutorialActor::ATutorialActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ATutorialActor::StartNextLesson()
{
    if(this->CanStartNextLesson())
    {
        this->DoneCurrentLesson = true;
    }
}

bool ATutorialActor::CanStartNextLesson()
{
    if(this->CurrentLesson.LessonObjective)
        return this->CurrentLesson.LessonObjective->IsObjectiveCompleted();
    return true; 
}

void ATutorialActor::Tick(float InDeltaTime)
{
    if(this->DoneCurrentLesson)
    {
        this->LessonIndex++;
        if(this->LessonIndex >= this->TutorialAsset->Lessons.Num())
        {
            this->Destroy();
            return;
        }

        this->CurrentLesson = this->TutorialAsset->Lessons[this->LessonIndex];

        if(this->CurrentLesson.LessonObjective)
        {
            this->CurrentLesson.LessonObjective->StartObjective(this->UserContext);
        }
    }
    else
    {
        if(this->CurrentLesson.LessonObjective)
        {
            this->CurrentLesson.LessonObjective->Tick(InDeltaTime);
        }
    }
}