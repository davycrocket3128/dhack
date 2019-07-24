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

#pragma once

#include "CoreMinimal.h"
#include "ConsoleContext.h"
#include "UserContext.h"
#include "NetworkedConsoleContext.generated.h"

/**
 * Allows the ability for a Terminal Command to output to a console context that is
 * owned by a different user on a different system than the terminal command itself.
 * 
 * This is used for both hackable connections and for the sudo command.
 */
UCLASS(BlueprintType)
class PROJECTOGLOWIA_API UNetworkedConsoleContext : public UConsoleContext
{
    GENERATED_BODY()

private:
    UPROPERTY()
    UConsoleContext* OutputConsole;

    UPROPERTY()
    UUserContext* OwningUser;

protected:
    UFUNCTION()
    void GetOutputConsoleSize(int& OutRows, int& OutColumns);

public:
    UFUNCTION()
    void SetupNetworkedConsole(UConsoleContext* InOutputConsole, UUserContext* InOwningUser);
};