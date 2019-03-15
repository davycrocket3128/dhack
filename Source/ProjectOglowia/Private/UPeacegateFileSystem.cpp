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


#include "UPeacegateFileSystem.h"
#include "Base64.h"
#include "FileUtilities.h"
#include "USystemContext.h"

TArray<FFileRecord> UPeacegateFileSystem::GetFileRecords(FFolder& Folder)
{
	TArray<FFileRecord> Records;
	for(auto RecordID : Folder.FileRecords)
	{
		for(auto& Record : this->SystemContext->GetComputer().FileRecords)
		{
			if(Record.ID == RecordID)
			{
				Records.Add(Record);
			}
		}
	}
	return Records;
}

bool UPeacegateFileSystem::GetFile(FFolder Parent, FString FileName, int & Index, FFileRecord & File)
{
	for (int i = 0; i < Parent.FileRecords.Num(); i++)
	{
		int fileID = Parent.FileRecords[i];

		for(auto& record : this->SystemContext->GetComputer().FileRecords)
		{
			if(record.ID == fileID && record.Name == FileName)
			{
				File = record;
				Index = i;
				return true;
			}
		}
	}

	return false;
}

void UPeacegateFileSystem::RecursiveDelete(FFolder& InFolder)
{

	for (auto& Subfolder : InFolder.SubFolders)
	{
		FFolder SubRecord = GetFolderByID(Subfolder);
		RecursiveDelete(SubRecord);
	}
	
	TArray<FFolder> FolderTree;

	SystemContext->GetFolderTree(FolderTree);

	for (int i = 0; i < FolderTree.Num(); i++)
	{
		if (FolderTree[i].FolderID == InFolder.FolderID)
		{
			FolderTree.RemoveAt(i);
			return;
		}
	}

	SystemContext->PushFolderTree(FolderTree);
}

FFolder UPeacegateFileSystem::GetFolderByID(int FolderID)
{
	TArray<FFolder> FolderTree;
	SystemContext->GetFolderTree(FolderTree);

	for (auto& Folder : FolderTree)
	{
		if (Folder.FolderID == FolderID)
			return Folder;
	}

	return FFolder();
}

void UPeacegateFileSystem::SetFolderByID(int FolderID, FFolder Folder)
{
	TArray<FFolder> FolderTree;
	SystemContext->GetFolderTree(FolderTree);

	for (int i = 0; i < FolderTree.Num(); i++)
	{
		FFolder ExFolder = FolderTree[i];

		if (ExFolder.FolderID == FolderID)
		{
			FolderTree[i] = Folder;
			SystemContext->PushFolderTree(FolderTree);
			return;
		}
	}
}

int UPeacegateFileSystem::GetNewFolderID()
{
	TArray<FFolder> FolderTree;
	SystemContext->GetFolderTree(FolderTree);
	int ID = 0;

	for (auto& Folder : FolderTree)
	{
		if (Folder.FolderID > ID)
		{
			ID = Folder.FolderID;
		}
	}

	return ID+1;
}

bool UPeacegateFileSystem::TraversePath(const TArray<FString>& PathParts, UFolderNavigator *& OutNavigator)
{
	return TraversePath(PathParts, PathParts.Num(), OutNavigator);
}

bool UPeacegateFileSystem::TraversePath(const TArray<FString>& PathParts, const int Count, UFolderNavigator* &OutNavigator)
{
	UFolderNavigator* Navigator = Root;

	if (!Navigator)
	{
		// The filesystem is not ready.
		OutNavigator = nullptr;
		return false;
	}

	// Go through each part of the path that we want to.
	for (int i = 0; i < PathParts.Num() && i < Count; i++)
	{
		// Get the current path part.
		FString FolderName = PathParts[i];

		if (Navigator->SubFolders.Contains(FolderName))
		{
			// Traverse down the Navigation Graph.
			Navigator = Navigator->SubFolders[FolderName];
		}
		else
		{
			// Directory not found.
			OutNavigator = nullptr;
			return false;
		}
	}

	// Let the caller know where we ended up
	OutNavigator = Navigator;
	return true;
}

TArray<FString> UPeacegateFileSystem::GetPathParts(FString InPath, FString& ResolvedPath)
{
	ResolvedPath = ResolveToAbsolute(InPath);
	TArray<FString> Parts;
	ResolvedPath.ParseIntoArray(Parts, TEXT("/"), true);
	return Parts;
}

void UPeacegateFileSystem::BuildChildNavigators(UFolderNavigator * RootNav)
{
	FFolder Folder = GetFolderByID(RootNav->FolderIndex);

	for (auto SubfolderIndex : Folder.SubFolders)
	{
		FFolder SubFolder = GetFolderByID(SubfolderIndex);
		UFolderNavigator* ChildNav = NewObject<UFolderNavigator>();
		ChildNav->FolderIndex = SubFolder.FolderID;
		RootNav->SubFolders.Add(SubFolder.FolderName, ChildNav);
		BuildChildNavigators(ChildNav);
	}
}

FString UPeacegateFileSystem::ResolveToAbsolute(const FString Path)
{
	TArray<FString> PartStack;
	TArray<FString> Split;

	Path.ParseIntoArray(Split, TEXT("/"), true);

	for (auto& Part : Split)
	{
		if (Part == TEXT("."))
			continue;
		if (Part == TEXT(".."))
		{
			if (PartStack.Num() > 0)
				PartStack.Pop();
			continue;
		}
		PartStack.Push(Part);
	}

	FString Absolute;
	if (PartStack.Num() == 0)
	{
		Absolute = TEXT("/");
	}
	else
	{
		while (PartStack.Num() > 0)
		{
			Absolute = Absolute.Append(TEXT("/") + PartStack[0]);
			PartStack.RemoveAt(0);
		}
	}

	return Absolute;
}

void UPeacegateFileSystem::BuildFolderNavigator()
{
	TArray<FFolder> FolderTree;
	SystemContext->GetFolderTree(FolderTree);

	if (FolderTree.Num()==0)
	{
		UFileUtilities::FormatFilesystem(FolderTree);
		SystemContext->PushFolderTree(FolderTree);
	}

	Root = NewObject<UFolderNavigator>();

	BuildChildNavigators(Root);

}

void UPeacegateFileSystem::Initialize(int InUserID)
{
	UserID = InUserID;
	BuildFolderNavigator();
}

bool UPeacegateFileSystem::CreateDirectory(const FString InPath, EFilesystemStatusCode& OutStatusCode)
{
	// Used for logging.
	FString ResolvedPath;

	// Resolve path and split into parts.
	TArray<FString> Parts = GetPathParts(InPath, ResolvedPath);

	// Don't try to create /.
	if (Parts.Num() == 0)
	{
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryExists;
		return false;
	}

	// The navigator which points to the parent directory of the one we want to create.
	UFolderNavigator* ParentNav = nullptr;

	// If we can't traverse to the parent, return FileOrDirectoryNotFound.
	if (!TraversePath(Parts, Parts.Num() - 1, ParentNav))
	{
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	FString FolderName = Parts[Parts.Num() - 1]; // New folder name
	
	// Does a folder with this name exist here?
	if (ParentNav->SubFolders.Contains(FolderName))
	{
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryExists;
		return false;
	}

	TArray<FFolder> FolderTree;
	SystemContext->GetFolderTree(FolderTree);

	// Allocate a new Folder structure in memory
	FFolder NewFolder;

	// Name it.
	NewFolder.FolderName = FolderName;

	// Give it a new Peacegate Folder ID.
	NewFolder.FolderID = GetNewFolderID();

	// The Parent ID becomes the ID of ParentNav, for when we rebuild the NavigationGraph again.
	NewFolder.ParentID = ParentNav->FolderIndex;

	// Add the new folder to the filesystem
	FolderTree.Add(NewFolder);

	// Alert the save system over in Blueprint land of the change too, so the game saves
	SystemContext->PushFolderTree(FolderTree);

	// Retrieve the filesystem entry for the parent folder
	FFolder CurrFolder = GetFolderByID(ParentNav->FolderIndex);

	// Add the ID of the new folder as a subfolder of its parent
	CurrFolder.SubFolders.Add(NewFolder.FolderID);;

	// Update the data in the filesystem
	SetFolderByID(ParentNav->FolderIndex, CurrFolder);

	// Create a new folder navigator so we can update the Navigation Graph
	UFolderNavigator* NewNav = NewObject<UFolderNavigator>();

	// Folder index matches the new folder ID
	NewNav->FolderIndex = NewFolder.FolderID;

	// Add it to the parent navigator to link it all up.
	ParentNav->SubFolders.Add(NewFolder.FolderName, NewNav);

	// Tell Peacegate that we created a directory - the mission system can consume this event.
	FilesystemOperation.Broadcast(EFilesystemEventType::CreateDirectory, ResolvedPath);

	// Success.
	return true;
}



bool UPeacegateFileSystem::DirectoryExists(const FString InPath)
{
	FString ResolvedPath = ResolveToAbsolute(InPath);
	TArray<FString> Parts;
	ResolvedPath.ParseIntoArray(Parts, TEXT("/"), true);
	UFolderNavigator* Navigator = Root;

	if (Parts.Num() == 0)
		return true;

	for (auto& Part : Parts)
	{
		if (Navigator->SubFolders.Contains(Part))
		{
			Navigator = Navigator->SubFolders[Part];
		}
		else {
			return false;
		}
	}

	return true;
}

bool UPeacegateFileSystem::FileExists(const FString InPath)
{
	FString ResolvedPath = ResolveToAbsolute(InPath);
	TArray<FString> Parts;
	ResolvedPath.ParseIntoArray(Parts, TEXT("/"), true);
	UFolderNavigator* Navigator = Root;

	if (Parts.Num() == 0)
		return false;

	for (int i = 0; i < Parts.Num() - 1; i++)
	{
		auto& Part = Parts[i];

		if (Navigator->SubFolders.Contains(Part))
		{
			Navigator = Navigator->SubFolders[Part];
		}
		else {
			return false;
		}
	}

	FString Filename = Parts[Parts.Num() - 1];

	FFolder FileParent = GetFolderByID(Navigator->FolderIndex);

	for (auto& FileRecord : this->GetFileRecords(FileParent))
	{
		if (FileRecord.Name == Filename)
			return true;

	}

	return false;
}

bool UPeacegateFileSystem::Delete(const FString InPath, const bool InRecursive, EFilesystemStatusCode& OutStatusCode)
{
	// Used for logging.
	FString ResolvedPath;

	EFilesystemEventType EventType = EFilesystemEventType::DeleteDirectory;

	TArray<FString> Parts = GetPathParts(InPath, ResolvedPath);

	// If the path part list is empty, user tried to delete root. Not allowed.
	if (Parts.Num() == 0)
	{
		OutStatusCode = EFilesystemStatusCode::PermissionDenied;
		return false;
	}

	// Parent directory to look inside.
	UFolderNavigator* ParentNav = nullptr;

	// Do we even have an existing parent?
	if (!TraversePath(Parts, Parts.Num() - 1, ParentNav))
	{
		// There's nothing to delete.
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	// File name could also be a folder name, we'll check in a second.
	FString Filename = Parts[Parts.Num() - 1];

	// If the filename is a folder...
	if (ParentNav->SubFolders.Contains(Filename))
	{
		// Get the folder's navigator so we know what to delete and can check it.
		UFolderNavigator* NavToDelete = ParentNav->SubFolders[Filename];

		// Get the folder data from the filesystem
		FFolder FolderToDelete = GetFolderByID(NavToDelete->FolderIndex);

		// If we have subfolders...
		if (NavToDelete->SubFolders.Num() > 0)
		{
			// Make sure the delete operation is recursive
			if (!InRecursive)
			{
				OutStatusCode = EFilesystemStatusCode::DirectoryNotEmpty;
				return false;
			}
		}

		// Recursively delete everything inside the folder.
		RecursiveDelete(FolderToDelete);

		// Get the parent folder data
		FFolder ParentFolder = GetFolderByID(ParentNav->FolderIndex);

		// Remove the just-deleted folder from the parent.
		ParentFolder.SubFolders.Remove(NavToDelete->FolderIndex);

		// Update the FS.
		SetFolderByID(ParentNav->FolderIndex, ParentFolder);

		// Remove the just deleted navigator from its parent
		ParentNav->SubFolders.Remove(Filename);
	}
	else
	{
		// Get the parent folder data.
		FFolder ParentFolder = GetFolderByID(ParentNav->FolderIndex);

		// Info about the file
		int FileIndex;
		FFileRecord FileToDelete;

		// Try to find the file.
		if (!GetFile(ParentFolder, Filename, FileIndex, FileToDelete))
		{
			// File or directory not found.
			OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
			return false;
		}

		// If the file record is a text file we must remove the text file.
		if(FileToDelete.RecordType == EFileRecordType::Text)
		{
			for(int i = 0; i < this->SystemContext->GetComputer().TextFiles.Num(); i++)
			{
				if(this->SystemContext->GetComputer().TextFiles[i].ID == FileToDelete.ContentID)
				{
					this->SystemContext->GetComputer().TextFiles.RemoveAt(i);
					break;
				}
			}
		}

		// Now we remove the file record itself from the save file.
		for(int i = 0; i < this->SystemContext->GetComputer().FileRecords.Num(); i++)
		{
			if(this->SystemContext->GetComputer().FileRecords[i].ID == FileToDelete.ID)
			{
				this->SystemContext->GetComputer().FileRecords.RemoveAt(i);
				break;
			}
		}

		// Remove the file at the found index. I'm not wasting CPU cycles comparing file structures.
		ParentFolder.FileRecords.RemoveAt(FileIndex);

		// Update the FS.
		SetFolderByID(ParentNav->FolderIndex, ParentFolder);
	
		// Change event type
		EventType = EFilesystemEventType::DeleteFile;
	}


	// Alert the Blueprint land.
	FilesystemOperation.Broadcast(EventType, ResolvedPath);

	return true;
}

FFileRecord UPeacegateFileSystem::GetFileRecord(FString InPath)
{
	// used for logging.
	FString ResolvedPath;

	// retrieve path parts.
	TArray<FString> Parts = GetPathParts(InPath, ResolvedPath);

	// If path part list is empty, don't read.
	if (Parts.Num() == 0)
	{
		return FFileRecord();
	}

	// Folder navigator to look up when finding the file.
	UFolderNavigator* ParentNav = nullptr;

	if (!TraversePath(Parts, Parts.Num() - 1, ParentNav))
	{
		return FFileRecord();
	}

	// File name is always last part in path.
	FString FileName = Parts[Parts.Num() - 1];

	// Grab the folder data for the parent.
	FFolder Parent = GetFolderByID(ParentNav->FolderIndex);

	// Places to store found file info
	FFileRecord FoundFile;
	int FoundIndex;

	if (!GetFile(Parent, FileName, FoundIndex, FoundFile))
	{
		return FFileRecord();
	}

	return FoundFile;
}

void UPeacegateFileSystem::SetFileRecord(FString InPath, EFileRecordType RecordType, int ContentID)
{
	if (InPath.EndsWith(TEXT("/")))
		return;

	if (DirectoryExists(InPath))
		return;
	
	FString FolderPath;
	FString FileName;

	if (!InPath.Split(TEXT("/"), &FolderPath, &FileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		return;

	if (!DirectoryExists(FolderPath))
		return;

	FString ResolvedPath = ResolveToAbsolute(FolderPath);
	TArray<FString> Parts;
	ResolvedPath.ParseIntoArray(Parts, TEXT("/"), true);
	UFolderNavigator* Navigator = this->Root;

	for (auto& Part : Parts)
	{
		if (Navigator->SubFolders.Contains(Part))
		{
			Navigator = Navigator->SubFolders[Part];
		}
		else {
			return;
		}
	}

	FFolder Folder = GetFolderByID(Navigator->FolderIndex);

	bool FoundFile = false;

	for (int RecordID : Folder.FileRecords)
	{
		for(auto& File : this->SystemContext->GetComputer().FileRecords)
		{
			if(File.ID == RecordID && File.Name == FileName)
			{
				// Set the record type and content ID of the file.
				File.RecordType = RecordType;
				File.ContentID = ContentID;

				FoundFile = true;
				break;
			}
		}
	}

	if (!FoundFile)
	{
		FFileRecord NewFile;
		NewFile.ID = this->GetNextFileRecordID();
		NewFile.Name = FileName;
		NewFile.RecordType = RecordType;
		NewFile.ContentID = ContentID;

		this->SystemContext->GetComputer().FileRecords.Add(NewFile);

		Folder.FileRecords.Add(NewFile.ID);
	}

	SetFolderByID(Navigator->FolderIndex, Folder);

	FilesystemOperation.Broadcast(EFilesystemEventType::WriteFile, ResolvedPath + TEXT("/") + FileName);
}

bool UPeacegateFileSystem::GetDirectories(const FString & InPath, TArray<FString>& OutDirectories, EFilesystemStatusCode& OutStatusCode)
{
	// used for logging
	FString ResolvedPath;

	// Split path into parts.
	TArray<FString> Parts = GetPathParts(InPath, ResolvedPath);

	// Navigator that we're going to read.
	UFolderNavigator* Navigator = nullptr;

	if (!TraversePath(Parts, Navigator))
	{
		// Directory doesn't exist.
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	TArray<FString> Keys;
	Navigator->SubFolders.GetKeys(Keys);

	for (auto Key : Keys)
	{
		OutDirectories.Add(ResolvedPath + TEXT("/") + Key);
	}

	return true;
}

bool UPeacegateFileSystem::GetFiles(const FString & InPath, TArray<FString>& OutFiles, EFilesystemStatusCode& OutStatusCode)
{
	// used for logging
	FString ResolvedPath;

	// split path into parts.
	TArray<FString> Parts = GetPathParts(InPath, ResolvedPath);

	// We need this in order to get the folder data
	UFolderNavigator* Navigator;

	if (!TraversePath(Parts, Navigator))
	{
		// Directory not found.
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	// read folder data
	FFolder Folder = GetFolderByID(Navigator->FolderIndex);

	// loop through each file
	for (FFileRecord File : this->GetFileRecords(Folder))
	{
		OutFiles.Add(ResolvedPath + TEXT("/") + File.Name);
	}

	return true;
}

int UPeacegateFileSystem::GetNextFileRecordID()
{
	int ID = 0;
	for(auto& Record : this->SystemContext->GetComputer().FileRecords)
	{
		if(Record.ID >= ID)
		{
			ID = Record.ID + 1;
		}
	}
	return ID;
}

int UPeacegateFileSystem::GetNextTextFileID()
{
	int ID = 0;
	for(auto& Record : this->SystemContext->GetComputer().TextFiles)
	{
		if(Record.ID >= ID)
		{
			ID = Record.ID + 1;
		}
	}
	return ID;
}


void UPeacegateFileSystem::WriteText(const FString & InPath, const FString & InText)
{
	if (InPath.EndsWith(TEXT("/")))
		return;

	if (DirectoryExists(InPath))
		return;
	
	FString FolderPath;
	FString FileName;

	if (!InPath.Split(TEXT("/"), &FolderPath, &FileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		return;

	if (!DirectoryExists(FolderPath))
		return;

	FString ResolvedPath = ResolveToAbsolute(FolderPath);
	TArray<FString> Parts;
	ResolvedPath.ParseIntoArray(Parts, TEXT("/"), true);
	UFolderNavigator* Navigator = this->Root;

	for (auto& Part : Parts)
	{
		if (Navigator->SubFolders.Contains(Part))
		{
			Navigator = Navigator->SubFolders[Part];
		}
		else {
			return;
		}
	}

	FFolder Folder = GetFolderByID(Navigator->FolderIndex);

	bool FoundFile = false;

	for (int RecordID : Folder.FileRecords)
	{
		for(auto& File : this->SystemContext->GetComputer().FileRecords)
		{
			if(File.ID == RecordID && File.Name == FileName)
			{
				if(File.RecordType != EFileRecordType::Text)
				{
					File.RecordType = EFileRecordType::Text;
				
					FTextFile NewTextFile;
					NewTextFile.ID = this->GetNextTextFileID();

					File.ContentID = NewTextFile.ID;
					NewTextFile.Content = InText;
					
					this->SystemContext->GetComputer().TextFiles.Add(NewTextFile);
				}
				else
				{
					for(auto& TextFile : this->SystemContext->GetComputer().TextFiles)
					{
						if(TextFile.ID == File.ContentID)
						{
							TextFile.Content = InText;
						}
					}
				}
				FoundFile = true;
				break;
			}
		}
	}

	if (!FoundFile)
	{
		FFileRecord NewFile;
		NewFile.ID = this->GetNextFileRecordID();
		NewFile.Name = FileName;
		NewFile.RecordType = EFileRecordType::Text;

		FTextFile TextFile;
		TextFile.ID = this->GetNextTextFileID();
		TextFile.Content = InText;

		this->SystemContext->GetComputer().TextFiles.Add(TextFile);

		NewFile.ContentID = TextFile.ID;

		this->SystemContext->GetComputer().FileRecords.Add(NewFile);

		Folder.FileRecords.Add(NewFile.ID);
	}

	SetFolderByID(Navigator->FolderIndex, Folder);

	FilesystemOperation.Broadcast(EFilesystemEventType::WriteFile, ResolvedPath + TEXT("/") + FileName);
}

bool UPeacegateFileSystem::ReadText(const FString & InPath, FString& OutText, EFilesystemStatusCode& OutStatusCode)
{
	// used for logging.
	FString ResolvedPath;

	// retrieve path parts.
	TArray<FString> Parts = GetPathParts(InPath, ResolvedPath);

	// If path part list is empty, don't read.
	if (Parts.Num() == 0)
	{
		OutStatusCode = EFilesystemStatusCode::PermissionDenied;
		return false;
	}

	// Folder navigator to look up when finding the file.
	UFolderNavigator* ParentNav = nullptr;

	if (!TraversePath(Parts, Parts.Num() - 1, ParentNav))
	{
		// File's parent wasn't found.
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	// File name is always last part in path.
	FString FileName = Parts[Parts.Num() - 1];

	// Grab the folder data for the parent.
	FFolder Parent = GetFolderByID(ParentNav->FolderIndex);

	// Places to store found file info
	FFileRecord FoundFile;
	int FoundIndex;

	if (!GetFile(Parent, FileName, FoundIndex, FoundFile))
	{
		// File not found.
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	OutStatusCode = EFilesystemStatusCode::OK;
	// File contents are in base 64.
	if(FoundFile.RecordType == EFileRecordType::Text)
	{
		for(auto& TextFile : this->SystemContext->GetComputer().TextFiles)
		{
			if(TextFile.ID == FoundFile.ContentID)
			{
				OutText = TextFile.Content;
			}
		}
	}
	return true;
}

bool UPeacegateFileSystem::MoveFile(const FString & Source, const FString & Destination, const bool InOverwrite, EFilesystemStatusCode & OutStatusCode)
{
	FString SourceResolved;
	FString DestResolved;

	TArray<FString> SourceParts = GetPathParts(Source, SourceResolved);
	TArray<FString> DestParts = GetPathParts(Destination, DestResolved);

	if (SourceParts.Num() == 0 || DestParts.Num() == 0)
	{
		// don't. fucking. touch. ROOT.
		OutStatusCode = EFilesystemStatusCode::PermissionDenied;
		return false;
	}

	UFolderNavigator* SourceNav = nullptr;
	UFolderNavigator* DestNav = nullptr;

	if (!TraversePath(SourceParts, SourceParts.Num() - 1, SourceNav) || !TraversePath(DestParts, DestParts.Num() - 1, DestNav))
	{
		// source/dest parent not found
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	FFolder SourceFolder = GetFolderByID(SourceNav->FolderIndex);
	FFolder DestFolder = GetFolderByID(DestNav->FolderIndex);

	int SourceFileIndex;
	int DestFileIndex;
	FFileRecord SourceFile;
	FFileRecord DestFile;

	FString SourceName = SourceParts[SourceParts.Num() - 1];
	FString DestName = DestParts[DestParts.Num() - 1];

	if (!GetFile(SourceFolder, SourceName, SourceFileIndex, SourceFile))
	{
		// Source file not found.
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	if (GetFile(DestFolder, DestName, DestFileIndex, DestFile))
	{
		if (!InOverwrite)
		{
			OutStatusCode = EFilesystemStatusCode::FileOrDirectoryExists;
			return false;
		}

		// Overwrite the destination file.
		DestFile.RecordType = SourceFile.RecordType;
		DestFile.ContentID = SourceFile.ContentID;

		this->UpdateFileRecord(DestFile);
	}
	else 
	{
		// Allocate a new destination file
		DestFile = FFileRecord();
		DestFile.ID = this->GetNextFileRecordID();
		DestFile.Name = DestName;
		DestFile.RecordType = SourceFile.RecordType;
		DestFile.ContentID = SourceFile.ContentID;

		this->SystemContext->GetComputer().FileRecords.Add(DestFile);

		// add to destination folder
		DestFolder.FileRecords.Add(DestFile.ID);
	}

	// We need to remove the file record of the original file from the save game.
	for(int i = 0; i < this->SystemContext->GetComputer().FileRecords.Num(); i++)
	{
		if(this->SystemContext->GetComputer().FileRecords[i].ID == SourceFile.ID)
		{
			this->SystemContext->GetComputer().FileRecords.RemoveAt(i);
			break;
		}
	}

	// remove source file from source folder
	SourceFolder.FileRecords.RemoveAt(SourceFileIndex);

	// update FS
	SetFolderByID(SourceNav->FolderIndex, SourceFolder);
	SetFolderByID(DestNav->FolderIndex, DestFolder);

	return true;
}

bool UPeacegateFileSystem::MoveFolder(const FString & Source, const FString & Destination, const bool InOverwrite, EFilesystemStatusCode & OutStatusCode)
{
	FString SourceResolved;
	FString DestResolved;

	TArray<FString> SourceParts = GetPathParts(Source, SourceResolved);
	TArray<FString> DestParts = GetPathParts(Destination, DestResolved);

	if (SourceParts.Num() == 0 || DestParts.Num() == 0)
	{
		// don't. fucking. touch. ROOT.
		OutStatusCode = EFilesystemStatusCode::PermissionDenied;
		return false;
	}

	UFolderNavigator* SourceNav = nullptr;
	UFolderNavigator* DestNav = nullptr;

	if (!TraversePath(SourceParts, SourceParts.Num() - 1, SourceNav) || !TraversePath(DestParts, DestParts.Num() - 1, DestNav))
	{
		// source/dest parent not found
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	FFolder SourceParent = GetFolderByID(SourceNav->FolderIndex);
	FFolder DestParent = GetFolderByID(DestNav->FolderIndex);

	FString SourceName = SourceParts[SourceParts.Num() - 1];
	FString DestName = DestParts[DestParts.Num() - 1];

	if (!SourceNav->SubFolders.Contains(SourceName))
	{
		// source directory not found.
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	UFolderNavigator* SourceChild = SourceNav->SubFolders[SourceName];

	if (DestNav->SubFolders.Contains(DestName))
	{
		if (!InOverwrite)
		{
			// I NEED A FUCKING ADVIL.
			OutStatusCode = EFilesystemStatusCode::FileOrDirectoryExists;
			return false;
		}

		UFolderNavigator* DestChild = DestNav->SubFolders[DestName];

		FFolder FolderToDelete = GetFolderByID(DestChild->FolderIndex);

		// Recursively delete this folder.
		RecursiveDelete(FolderToDelete);

		// Remove the child from the parent
		DestParent.SubFolders.Remove(DestChild->FolderIndex);

		// Remove the old navigator from its parent.
		DestNav->SubFolders.Remove(DestName);
	}

	// Remove the source child from its parent.
	SourceNav->SubFolders.Remove(SourceName);

	// And add it to the destination.
	DestNav->SubFolders.Add(DestName, SourceChild);

	// Remove SourceChild's folder index from the source parent
	SourceParent.SubFolders.Remove(SourceChild->FolderIndex);

	// And add it to the destination
	DestParent.SubFolders.Add(SourceChild->FolderIndex);

	// Update the FS.
	SetFolderByID(DestParent.FolderID, DestParent);
	SetFolderByID(SourceParent.FolderID, SourceParent);

	// Update source child folder's parent ID.
	FFolder SourceChildData = GetFolderByID(SourceChild->FolderIndex);

	SourceChildData.ParentID = DestParent.FolderID;

	// Rename the folder.
	SourceChildData.FolderName = DestName;

	//Update FS
	SetFolderByID(SourceChildData.FolderID, SourceChildData);

	return true;
}

void UPeacegateFileSystem::UpdateFileRecord(FFileRecord InRecord)
{
	// This ensures that a file record is successfully updated in the save file.
	for(int i = 0; i < this->SystemContext->GetComputer().FileRecords.Num(); i++)
	{
		if(this->SystemContext->GetComputer().FileRecords[i].ID == InRecord.ID)
		{
			this->SystemContext->GetComputer().FileRecords[i] = InRecord;
			return;
		}
	}
}

bool UPeacegateFileSystem::CopyFile(const FString & Source, const FString & Destination, const bool InOverwrite, EFilesystemStatusCode & OutStatusCode)
{
	FString SourceResolved;
	FString DestResolved;

	TArray<FString> SourceParts = GetPathParts(Source, SourceResolved);
	TArray<FString> DestParts = GetPathParts(Destination, DestResolved);

	if (SourceParts.Num() == 0 || DestParts.Num() == 0)
	{
		// don't. fucking. touch. ROOT.
		OutStatusCode = EFilesystemStatusCode::PermissionDenied;
		return false;
	}

	UFolderNavigator* SourceNav = nullptr;
	UFolderNavigator* DestNav = nullptr;

	if (!TraversePath(SourceParts, SourceParts.Num() - 1, SourceNav) || !TraversePath(DestParts, DestParts.Num() - 1, DestNav))
	{
		// source/dest parent not found
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	FFolder SourceFolder = GetFolderByID(SourceNav->FolderIndex);
	FFolder DestFolder = GetFolderByID(DestNav->FolderIndex);

	int SourceFileIndex;
	int DestFileIndex;
	FFileRecord SourceFile;
	FFileRecord DestFile;

	FString SourceName = SourceParts[SourceParts.Num() - 1];
	FString DestName = DestParts[DestParts.Num() - 1];

	if (!GetFile(SourceFolder, SourceName, SourceFileIndex, SourceFile))
	{
		// Source file not found.
		OutStatusCode = EFilesystemStatusCode::FileOrDirectoryNotFound;
		return false;
	}

	if (GetFile(DestFolder, DestName, DestFileIndex, DestFile))
	{
		if (!InOverwrite)
		{
			OutStatusCode = EFilesystemStatusCode::FileOrDirectoryExists;
			return false;
		}

		// If the destination file is a text file then we need to remove that
		// text file entry.
		if(DestFile.RecordType == EFileRecordType::Text)
		{
			for(int i = 0; i < this->SystemContext->GetComputer().TextFiles.Num(); i++)
			{
				if(this->SystemContext->GetComputer().TextFiles[i].ID == DestFile.ContentID)
				{
					this->SystemContext->GetComputer().TextFiles.RemoveAt(i);
				}
			}
		}

		// Copy over the record types.
		DestFile.RecordType = SourceFile.RecordType;

		// If the source file is a text file then we need to create a new text file
		// entry and use that as the destination content ID.  Otherwise, we'll get
		// an issue where overwriting the destination content overwrites the source
		// content.  If it's not a text file, we can just copy the content ID over
		// because anything that isn't text is a UE4 asset (which the player can't write to).
		if(SourceFile.RecordType == EFileRecordType::Text)
		{
			// This creates the new entry ID.
			FTextFile TextFile;
			TextFile.ID = this->GetNextTextFileID();
			
			// Now we need to read the text of the source file to copy it over.
			for(auto SourceText : this->SystemContext->GetComputer().TextFiles)
			{
				if(SourceText.ID == SourceFile.ContentID)
				{
					// Copy over the text.
					TextFile.Content =  SourceText.Content;
				}
			}

			// This actually adds the content to the save file.
			this->SystemContext->GetComputer().TextFiles.Add(TextFile);

			// And this re-assigns the file content ID!
			DestFile.ContentID = TextFile.ID;

			this->UpdateFileRecord(DestFile);
		}
		else
		{
			// Just copy over the content ID.
			DestFile.ContentID = SourceFile.ContentID;
		}
	}
	else
	{
		// Allocate a new destination file
		DestFile = FFileRecord();
		DestFile.ID = this->GetNextFileRecordID();
		DestFile.Name = DestName;
		DestFile.RecordType = SourceFile.RecordType;

		// Just like above, we need to handle copying over text differently
		// than other content types.
		if(SourceFile.RecordType == EFileRecordType::Text)
		{
			// This creates the new entry ID.
			FTextFile TextFile;
			TextFile.ID = this->GetNextTextFileID();
			
			// Now we need to read the text of the source file to copy it over.
			for(auto SourceText : this->SystemContext->GetComputer().TextFiles)
			{
				if(SourceText.ID == SourceFile.ContentID)
				{
					// Copy over the text.
					TextFile.Content =  SourceText.Content;
				}
			}

			// This actually adds the content to the save file.
			this->SystemContext->GetComputer().TextFiles.Add(TextFile);

			// And this re-assigns the file content ID!
			DestFile.ContentID = TextFile.ID;
		}
		else
		{
			// Just copy over the content ID.
			DestFile.ContentID = SourceFile.ContentID;
		}

		// Writes the destination file record to the save file.
		this->SystemContext->GetComputer().FileRecords.Add(DestFile);

		// add to destination folder
		DestFolder.FileRecords.Add(DestFile.ID);
	}

	// update FS
	SetFolderByID(SourceNav->FolderIndex, SourceFolder);
	SetFolderByID(DestNav->FolderIndex, DestFolder);

	return true;
}


bool UPeacegateFileSystem::IsValidAsFileName(const FString & InFileName)
{
	if (InFileName.IsEmpty())
		return false;

	if(InFileName.StartsWith("."))
		return false;

	TArray<TCHAR> CharsInString = InFileName.GetCharArray();

	FString AllowedCharString = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_- .");

	int index = 0;

	for (auto& Character : CharsInString)
	{
		if (!AllowedCharString.FindChar(Character, index))
			return false;
	}

	return true;
}

bool UPeacegateFileSystem::IsValidAsUserName(const FString & InUserName)
{
	if (InUserName.IsEmpty())
		return false;

	TArray<TCHAR> CharsInString = InUserName.GetCharArray();

	FString AllowedCharString = TEXT("abcdefghijklmnopqrstuvwxyz0123456789_-");

	int index = 0;

	for (auto& Character : CharsInString)
	{
		if (!AllowedCharString.FindChar(Character, index))
			return false;
	}

	return true;
}
