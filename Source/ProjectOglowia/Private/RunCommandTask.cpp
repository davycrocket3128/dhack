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

#include "RunCommandTask.h"
#include "CommandShell.h"
#include "UserContext.h"

void URunCommandTask::NativeStart()
{
    FString CommandText = this->Command->ID.ToString();

    if(ExpectedArguments.Len())
    {
        CommandText += " " + ExpectedArguments;
    }

    this->SetObjectiveText(FText::Format(NSLOCTEXT("Objectives", "RunCommand", "Run {0}."), FText::FromString(CommandText)));
}

void URunCommandTask::NativeEvent(FString EventName, TMap<FString, FString> InEventArgs)
{
    // Only handle "Command Complete" events.
    if(EventName != "CommandComplete") return;

    // Check if we've gotten arguments and a command name AT LEAST.
    check(InEventArgs.Contains("Command"));
    check(InEventArgs.Contains("Arguments"));
    
    // Check that our command isn't null.
    check(this->Command);

    // If the argument "Command" matches our command's name, we're finished.
    if(InEventArgs["Command"] == this->Command->ID.ToString())
    {
        // If the expected args are empty we're complete.
        if(!this->ExpectedArguments.Len())
        {
            this->Complete();
            return;
        }

        // If not, then we need to check if the arguments match.
        // First we need a temp arguments variable because we're going
        // to need to check for "~" appearing in the expected args.
        //
        // This fixes an issue where we can't get the player to change
        // to their home directory with "cd ~" and trigger an objective complete.
        FString expected = this->ExpectedArguments;

        // If the expected args contains "~" AT ALL, we need to use
        // the bash lexer to replace it with the home directory of the
        // mission's User Context.
        if(expected.Contains("~"))
        {
            // Get the player home directory.
            FString home = this->GetPlayerUser()->GetHomeDirectory();

            // Use bash to tokenize the command into a list of arguments. Each token
            // will have "~" replaced with the home directory if said token starts ith "~".
            FText error;
            TArray<FString> tokens = ACommandShell::Tokenize(expected, home, error);

            // Check that there was no error.
            check(error.IsEmpty());

            // Now we combine the tokens back into an argument string.
            FString newExpected = "";
            for(auto arg : tokens)
            {
                newExpected += arg + " ";
            }
            
            // Trum excess space off of the new expected args.
            newExpected = newExpected.TrimEnd();
        
            // Copy it over.
            expected = newExpected;
        }

        // NOW we can check to see if the arguments match.
        if(InEventArgs["Arguments"] == expected)
        {
            this->Complete();
            return;
        }
    }
}