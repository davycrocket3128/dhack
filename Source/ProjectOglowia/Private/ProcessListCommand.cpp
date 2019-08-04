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

#include "ProcessListCommand.h"
#include "UserContext.h"

// processes - Lists all running processes on the system.
//
// Usage:
//   processes
void AProcessListCommand::NativeRunCommand(UConsoleContext* InConsole, TArray<FString> InArguments)
{
    // Retrieve all running processes:
    TArray<int> RunningProcesses = this->GetUserContext()->GetRunningProcesses();

    // Localized strings for the table header.
    FText PidHeader = NSLOCTEXT("Processes", "Pid", "PID");
    FText UidHeader = NSLOCTEXT("Processes", "Uid", "UID");
    FText NameHeader = NSLOCTEXT("Processes", "Name", "Name");

    // Lengths for the headers.
    int PidHeaderLen = PidHeader.ToString().Len();
    int UidHeaderLen = UidHeader.ToString().Len();
    int NameHeaderLen = NameHeader.ToString().Len();

    // Longest values for each table cell.
    int LongestPid = PidHeaderLen;
    int LongestUid = UidHeaderLen;
    int LongestName = NameHeaderLen;

    // List of FText values for each column.
    TArray<FText> Names;
    TArray<FText> Pids;
    TArray<FText> Uids;

    // Go through all of the running processes:
    for(int ProcessID : RunningProcesses)
    {
        // Get the process information.
        EProcessResult Result;
        UProcess* Process;
        if(this->GetUserContext()->GetProcess(ProcessID, Process, Result))
        {
            // Get the process name, username and UID for the process.
            FString ProcessName = Process->GetProcessName();
            FString Username = Process->GetUsername();
            int UserID = Process->GetUserID();
            
            // Create localized text out of it.
            FText Name = FText::FromString(ProcessName);
            FText Uid = FText::FromString(FString::FromInt(UserID) + " (" + Username + ")");
            FText Pid = FText::FromString(FString::FromInt(ProcessID));

            // Calculate longest length.
            if(Name.ToString().Len() > LongestName)
                LongestName = Name.ToString().Len();

            if(Uid.ToString().Len() > LongestUid)
                LongestUid = Uid.ToString().Len();

            if(Pid.ToString().Len() > LongestPid)
                LongestPid = Pid.ToString().Len();

            // Add the text to the right columns.
            Names.Add(Name);
            Uids.Add(Uid);
            Pids.Add(Pid);
        }
    }

    // Print the table cell headers.
    InConsole->SetBold(true);
    InConsole->Write(PidHeader);
    for(int i = 0; i <= LongestPid - PidHeaderLen; i++)
        InConsole->Write(FText::FromString(" "));
    InConsole->Write(FText::FromString(" | "));
    InConsole->Write(UidHeader);
    for(int i = 0; i <= LongestUid - UidHeaderLen; i++)
        InConsole->Write(FText::FromString(" "));
    InConsole->Write(FText::FromString(" | "));
    InConsole->WriteLine(NameHeader);
    InConsole->SetBold(false);

    // Separator between table header and cells.
    InConsole->WriteEmptyLine();

    // Now we write all the cells!
    for(int i = 0; i < Names.Num(); i++)
    {
        // Get the text for each cell.
        FText Name = Names[i];
        FText Uid = Uids[i];
        FText Pid = Pids[i];

        // Get the padding lengths.
        int NamePad = LongestName - Name.ToString().Len();
        int UidPad = LongestUid - Uid.ToString().Len();
        int PidPad = LongestPid - Pid.ToString().Len();

        // Write everything correctly!
        InConsole->Write(Pid);
        for(int j = 0; j <= PidPad; j++)
            InConsole->Write(FText::FromString(" "));
        InConsole->Write(FText::FromString(" | "));
        InConsole->Write(Uid);
        for(int j = 0; j <= UidPad; j++)
            InConsole->Write(FText::FromString(" "));
        InConsole->Write(FText::FromString(" | "));
        InConsole->WriteLine(Name);

    }

    // Separator between table and bash prompt.
    InConsole->WriteEmptyLine();

    // All done.
    this->Complete();
}