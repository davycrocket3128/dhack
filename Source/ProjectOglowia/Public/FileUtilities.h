#pragma once

#include "CoreMinimal.h"
#include "Folder.h"
#include "FileUtilities.generated.h"

UCLASS(Blueprintable)
class PROJECTOGLOWIA_API UFileUtilities : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Filesystem")
	static FString GetNameFromPath(const FString& InPath)
	{
		FString NewPath(InPath);
		int LastSlash = -1;
		if (!NewPath.FindLastChar(TEXT('/'), LastSlash))
			return NewPath;
		NewPath.RemoveAt(0, LastSlash + 1);
		return NewPath;
	}

	UFUNCTION(BlueprintCallable, Category="Filesystem")
	static void FormatFilesystem(UPARAM(Ref) TArray<FFolder>& InFilesystem)
	{
		InFilesystem.Empty();

		FFolder Root;

		Root.FolderID = 0;
		Root.IsReadOnly = true;
		Root.FolderName = TEXT("");
		Root.SubFolders = TArray<int>();
		Root.FileRecords = TArray<int>();

        FFolder Binaries;
        Binaries.FolderID = 1;
        Binaries.FolderName = "bin";
        Root.SubFolders.Add(Binaries.FolderID);

		InFilesystem.Add(Root);
        InFilesystem.Add(Binaries);
	}
};