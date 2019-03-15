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
#include "Folder.h"
#include "FileRecord.h"
#include "Computer.h"
#include "UPeacegateFileSystem.generated.h"

class USystemContext;

UENUM(BlueprintType)
enum class EFilesystemEventType : uint8
{
	CreateDirectory,
	WriteFile,
	DeleteFile,
	DeleteDirectory,
	MoveDirectory,
	MoveFile,
	CopyDirectory,
	CopyFile
};

UCLASS(Blueprintable)
class PROJECTOGLOWIA_API UFolderNavigator : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<FString, UFolderNavigator*> SubFolders;
	
	UPROPERTY()
	int FolderIndex = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFilesystemOperationEvent, EFilesystemEventType, InType, FString, InPath);

UENUM(BlueprintType)
enum class EFilesystemStatusCode : uint8
{
	OK,
	FileOrDirectoryNotFound,
	FileOrDirectoryExists,
	DirectoryNotEmpty,
	PermissionDenied,
	UnknownError
};

/**
 * Encapsulates a filesystem of a Peacenet computer.
 */
UCLASS(Blueprintable)
class PROJECTOGLOWIA_API UPeacegateFileSystem : public UObject
{
	GENERATED_BODY()

	
private:
	void BuildChildNavigators(UFolderNavigator* RootNav);

public:
	int UserID = 0;
	
public:
	UFUNCTION()
	TArray<FFileRecord> GetFileRecords(FFolder& InFolder);

	UFUNCTION()
	int GetNextFileRecordID();

	UFUNCTION()
	int GetNextTextFileID();

	void BuildFolderNavigator();

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	void Initialize(int InUserID);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(ExposeOnSpawn="true"))
	USystemContext* SystemContext;

	UPROPERTY()
	UFolderNavigator* Root;

	
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FFilesystemOperationEvent FilesystemOperation;

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	bool CreateDirectory(const FString InPath, EFilesystemStatusCode& OutStatusCode);

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	bool DirectoryExists(const FString InPath);
	
	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	bool FileExists(const FString InPath);

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	bool Delete(const FString InPath, const bool InRecursive, EFilesystemStatusCode& OutStatusCode);

	UFUNCTION(BlueprintCallable, Category= "Filesystem")
	bool GetDirectories(const FString& InPath, TArray<FString>& OutDirectories, EFilesystemStatusCode& OutStatusCode);

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	bool GetFiles(const FString& InPath, TArray<FString>& OutFiles, EFilesystemStatusCode& OutStatusCode);

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	void WriteText(const FString& InPath, const FString& InText);

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	bool ReadText(const FString& InPath, FString& OutText, EFilesystemStatusCode& OutStatusCode);

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	bool MoveFile(const FString& Source, const FString& Destination, const bool InOverwrite, EFilesystemStatusCode& OutStatusCode);

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	bool MoveFolder(const FString& Source, const FString& Destination, const bool InOverwrite, EFilesystemStatusCode& OutStatusCode);
	
	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	bool CopyFile(const FString& Source, const FString& Destination, const bool InOverwrite, EFilesystemStatusCode& OutStatusCode);

	UFUNCTION()
	void UpdateFileRecord(FFileRecord InRecord);

public:
	UFUNCTION(BlueprintCallable, Category="Filesystem")
	static bool IsValidAsFileName(const FString& InFileName);

	UFUNCTION(BlueprintCallable, Category = "Filesystem")
	static bool IsValidAsUserName(const FString& InUserName);

private:
	bool GetFile(FFolder Parent, FString FileName, int& Index, FFileRecord& File);

	void RecursiveDelete(FFolder& InFolder);

	FFolder GetFolderByID(int FolderID);
	void SetFolderByID(int FolderID, FFolder Folder);
	int GetNewFolderID();

	bool TraversePath(const TArray<FString>& PathParts, UFolderNavigator*& OutNavigator);
	bool TraversePath(const TArray<FString>& PathParts, const int Count, UFolderNavigator*& OutNavigator);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Filesystem")
	static FString ResolveToAbsolute(const FString Path);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Filesystem")
	static TArray<FString> GetPathParts(FString InPath, FString& ResolvedPath);
};
