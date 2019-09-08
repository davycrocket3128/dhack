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

#include "BashShell.h"
#include "UserContext.h"
#include "CommandInfo.h"
#include "Path.h"
#include "FileRecordUtils.h"

UProcess* ABashShell::LaunchProcess(UConsoleContext* TargetConsole, FString InCommandName, TArray<FString> Arguments) {
	// TODO: Proper path env var.
	// These are the folders that bash will look in for an executable.
	TArray<FString> Paths = {
		"/bin",
		"/usr/bin",
		"/sbin",
		"/usr/sbin",
		TargetConsole->GetWorkingDirectory()
	};

	// Try to find an executable in these paths.
	for(FString Path : Paths) {
		// Combine the paths.
		FString FullPath = UPath::Combine(TArray<FString> { Path, InCommandName });

		// Try to launch a process.
		UProcess* ChildProcess = nullptr;
		TArray<FString> Args = { FullPath };
		Args.Append(Arguments);
		UFileRecordUtils::LaunchProcess(FullPath, Args, TargetConsole, this->GetProcess(), ChildProcess, TargetConsole->GetUserContext()->GetDesktop());
		if(ChildProcess) {
			return ChildProcess;
		}
	}
	return nullptr;
}

FText ABashShell::GetShellPrompt() {
	    // Get the username, hostname and current working directory.
    FString Username = this->GetUserContext()->GetUsername();
    FString Hostname = this->GetUserContext()->GetHostname();
    FString WorkingDirectory = this->GetConsole()->GetDisplayWorkingDirectory();

	// The little thingy that tells whether we're root or not.
	FString UserStatus = "$";

    // Are we root?
    if(this->GetUserContext()->IsAdministrator()) {
        UserStatus = "#";
    }

	return FText::Format(NSLOCTEXT("Bash", "Prompt", "[{0}@{1} {2}]{3} "), FText::FromString(Username), FText::FromString(Hostname), FText::FromString(WorkingDirectory), FText::FromString(UserStatus));
}