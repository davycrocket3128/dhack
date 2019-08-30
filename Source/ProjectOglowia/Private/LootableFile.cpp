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

#include "PeacegateFileSystem.h"
#include "LootableFile.h"

void ULootableFile::Spawn(UPeacegateFileSystem* TargetFileSystem, FRandomStream& Rng) {
    // This method simply wraps the static version that does the actual work.
    ULootableFile::StaticSpawn(TargetFileSystem, this->LootableSpawnInfo, Rng);
}

FString ResolvePath(FString InPath) {
    TArray<FString> SplitParts;
    TArray<FString> PartStack;

    FString CurrentPart = "";
    for(int i = 0; i <= InPath.Len(); i++) {
        if(i == InPath.Len() || InPath[i] == '/') {
            if(CurrentPart.Len()) {
                SplitParts.Add(CurrentPart);
                CurrentPart = "";
            }
            continue;
        }
        CurrentPart += InPath[i];
    }

    for(auto Part : SplitParts) {
        if(Part == ".") {
            continue;
        }
        if(Part == "..") {
            if(PartStack.Num()) {
                PartStack.Pop();
                continue;
            }
        }
        PartStack.Push(Part);
    }

    FString AbsolutePath = "";
    while(PartStack.Num()) {
        AbsolutePath = "/" + PartStack.Pop() + AbsolutePath;
    }
    return AbsolutePath;
}

void ULootableFile::StaticSpawn(UPeacegateFileSystem* TargetFileSystem, FLootableSpawnInfo SpawnInfo, FRandomStream& Rng) {
    // Check FS validity.
    check(TargetFileSystem);
    
    // Check that the directory path is not blank, and that it either starts with / or %home (random home directory)
    FString TrimmedDirectoryPath = SpawnInfo.TargetDirectory.TrimStartAndEnd();
    check(TrimmedDirectoryPath.Len());
    check(TrimmedDirectoryPath.StartsWith("/") || TrimmedDirectoryPath.StartsWith("%home/"));
    
    // The absolute directory path we'll use.
    FString AbsolutePath = "";

    // If the directory path starts with "%home" this means we need to pick a random home directory.
    // We also need to "jail" the path so the lootable doesn't spawn outside of this home directory.
    if(TrimmedDirectoryPath.StartsWith("%home/")) {
        // Get the path without the prefix.
        FString Relative = TrimmedDirectoryPath;
        Relative.RemoveAt(0, FString("%home").Len());
        
        // Resolve things like "..", ".", etc treating the relative path above as if it is relative to the fs root.
        FString Resolved = ResolvePath(Relative);

        // Now we need to pick a random Peacegate user...
        FComputer& Computer = TargetFileSystem->GetComputer();
        FUser& User = Computer.Users[Rng.RandRange(0, Computer.Users.Num() - 1)];

        // If it's root then we spawn in '/root'
        if(User.ID == 0) {
            AbsolutePath = "/root" + Resolved;
        } else {
            AbsolutePath = "/home/" + User.Username + Resolved;
        }
    } else {
        // Simply write relative to /
        AbsolutePath = ResolvePath(TrimmedDirectoryPath);
    }

    // Check that the file contents we'll write are valid, gracefully stop if not.
    check(SpawnInfo.Content);
    if(!SpawnInfo.Content) {
        return;
    }

    // Create the directory if it does not yet exist.
    if(!TargetFileSystem->DirectoryExists(AbsolutePath)) {
        EFilesystemStatusCode Status;
        TargetFileSystem->CreateDirectory(AbsolutePath, Status);
    }

    // If there is a file in the directory with the same name as stated in the
    // lootablee info and said name isn't blank, proceed to generate a new file name.
    bool GenerateNewFileName = true;
    if(SpawnInfo.FileName.Len()) {
        if(TargetFileSystem->FileExists(AbsolutePath + "/" + SpawnInfo.FileName)) {
            if(SpawnInfo.Content->FileMatches(AbsolutePath + "/" + SpawnInfo.FileName, TargetFileSystem)) {
                return;
            }
            GenerateNewFileName = true;
        } else {
            GenerateNewFileName = false;
        }
    }

    // Generate a new file name until the file doesn't exist in the directory.
    if(GenerateNewFileName) {
        FString ExistingName = SpawnInfo.FileName;
        if(!ExistingName.Len()) {
            ExistingName = "Untitled";
        }
        do {
            SpawnInfo.FileName = ExistingName + "_" + FString::FromInt(Rng.RandRange(0, 4096));
        } while(TargetFileSystem->FileExists(AbsolutePath + "/" + SpawnInfo.FileName));
    }

    // Construct the full path to the file and pass it to the file contents object so
    // the content can be written.
    SpawnInfo.Content->SpawnFile(AbsolutePath + "/" + SpawnInfo.FileName, TargetFileSystem);
}