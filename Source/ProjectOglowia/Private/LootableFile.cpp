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

void ULootableFile::Spawn(UPeacegateFileSystem* TargetFileSystem) {
    // This method simply wraps the static version that does the actual work.
    ULootableFile::StaticSpawn(TargetFileSystem, this->LootableSpawnInfo);
}

void ULootableFile::StaticSpawn(UPeacegateFileSystem* TargetFileSystem, FLootableSpawnInfo SpawnInfo) {
    // Check that there is a directory path provided, that the path is valid, and that it doesn't
    // exist as a file.

    // Check that the file contents we'll write are valid, gracefully stop if not.

    // Create the directory if it does not yet exist.

    // If there is a file in the directory with the same name as stated in the
    // lootablee info and said name isn't blank, proceed to generate a new file name.

    // Generate a new file name until the file doesn't exist in the directory.

    // Construct the full path to the file and pass it to the file contents object so
    // the content can be written.
    
}