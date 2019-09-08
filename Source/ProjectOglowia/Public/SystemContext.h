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
#include "Computer.h"
#include "PeacegateFileSystem.h"
#include "UserInfo.h"
#include "PeacenetIdentity.h"
#include "MailProvider.h"
#include "ConnectionError.h"
#include "ProcessManager.h"
#include "DaemonManager.h"
#include "ProgramFile.h"
#include "SystemContext.generated.h"

class UHackable;
class UDesktopWidget;
class UPayloadAsset;
class USystemUpgrade;
class UProgram;
class UExploit;
class APeacenetWorldStateActor;

/**
 * Represents the state and allows access/modification of an NPC or player computer in Peacenet.
 */
UCLASS(BlueprintType)
class PROJECTOGLOWIA_API USystemContext : public UObject
{
	friend UProcessManager;

	GENERATED_BODY()

private:
	UPROPERTY()
	UDaemonManager* DaemonManager;

	UPROPERTY()
	UProcessManager* ProcessManager;

	UPROPERTY()
	FPeacenetIdentity InternalIdentity;

	UPROPERTY()
	UMailProvider* MailProvider;

	UFUNCTION()
	void UpdateInternalIdentity();

protected:
	UFUNCTION()
	void InitDaemonManager();

	UFUNCTION()
	void RestartDaemonManager();

	UPROPERTY()
	FString CurrentHostname;

	UPROPERTY()
	TMap<int, UPeacegateFileSystem*> RegisteredFilesystems;

	UPROPERTY()
	APeacenetWorldStateActor * Peacenet;

	UPROPERTY()
	int ComputerID = 0;

	UPROPERTY()
	TArray<UUserContext*> Hackers;
	
	UPROPERTY()
	int CharacterID = 0;

	UPROPERTY()
	UDesktopWidget* Desktop;

	UPROPERTY()
	TMap<int, UUserContext*> Users;

protected:
	UFUNCTION()
	void HandleFileSystemEvent(EFilesystemEventType InType, FString InPath);

	UFUNCTION()
	void PopulateInstalledPrograms();

	UFUNCTION()
	void Crash();

public: // Property getters
	UFUNCTION()
	bool IsDaemonRunning(FName InDaemonName);

	UFUNCTION()
	bool GetDaemonManager(UUserContext* InUserContext, UDaemonManager*& OutDaemonManager);

	UFUNCTION()
	UUserContext* GetHackerContext(int InUserID, UUserContext* HackingUser);

	UFUNCTION()
	void Destroy();

	UFUNCTION()
	bool HasIdentity();

	UFUNCTION()
	bool IsEnvironmentVariableSet(FString InVariable);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Upgrades")
	bool IsUpgradeInstalled(USystemUpgrade* InUpgrade);

	UFUNCTION()
	UMailProvider* GetMailProvider();

	UFUNCTION()
	TArray<UPayloadAsset*> GetPayloads();

	UFUNCTION()
	bool GetEnvironmentVariable(FString InVariable, FString& OutValue);

	UFUNCTION()
	void SetEnvironmentVariable(FString InVariable, FString InValue);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Context")
	bool IsSet(FString InSaveBoolean);

	UFUNCTION(BlueprintCallable, Category = "System Context")
	void SetSaveBoolean(FString InSaveBoolean, bool InValue);

	UFUNCTION()
	TArray<int> GetRunningProcesses();

	UFUNCTION()
	bool GetProcess(int ProcessID, UUserContext* InUserContext, UProcess*& OutProcess, EProcessResult& OutProcessResult);

	UFUNCTION()
	bool KillProcess(int ProcessID, UUserContext* UserContext, EProcessResult& OutKillResult);

	UFUNCTION()
	void UnsetEnvironmentVariable(FString InVariable);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Context")
	APeacenetWorldStateActor* GetPeacenet();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Context")
	TArray<UWallpaperAsset*> GetAvailableWallpapers();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Email")
	FString GetEmailAddress();

	UFUNCTION(BlueprintCallable, Category = "System Context")
	void SetCurrentWallpaper(UWallpaperAsset* InWallpaperAsset);

	UFUNCTION(BlueprintCallable, Category = "System Context")
	void DisableWallpaper();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game Stats")
	int GetGameStat(FName InStatName);

	UFUNCTION(BlueprintCallable, Category = "Game Stats")
	void SetGameStat(FName InStatName, int InValue);

	UFUNCTION(BlueprintCallable, Category = "Game Stats")
	void IncreaseGameStat(FName InStatName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Context")
	UDesktopWidget* GetDesktop();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Context")
	bool IsNewGame();

	UFUNCTION()
	FPeacenetIdentity& GetCharacter();

	UFUNCTION()
	FComputer& GetComputer();

	UFUNCTION()
	bool DnsResolve(FString InHost, FComputer& OutComputer, EConnectionError& OutConnectionError);

	UFUNCTION(BlueprintCallable, Category = "System Context")
	FString GetHostname();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Context")
	TArray<FProgramFile> GetInstalledPrograms();

public:
	UFUNCTION()
	void RebuildFilesystemNavigators();

	UFUNCTION()
	void AppendLog(FString InLogText);

	UFUNCTION()
	TArray<FString> GetNearbyHosts();

public:
	UFUNCTION()
	int GetOpenConnectionCount();

	UFUNCTION()
	bool UsernameExists(FString InUsername);

	UFUNCTION()
	bool IsIPAddress(FString InIPAddress);

	UFUNCTION()
	void SetupDesktop(int InUserID);

	UFUNCTION()
	TArray<UCommandInfo*> GetInstalledCommands();

	UFUNCTION()
	TArray<UExploit*> GetExploits();

	UFUNCTION()
	void Setup(int InComputerID, int InCharacterID, APeacenetWorldStateActor* InPeacenet);

	UFUNCTION()
	void ExecuteCommand(FString InCommand);

	UFUNCTION()
	void UpdateSystemFiles();

	UFUNCTION()
	bool OpenProgram(FName InExecutableName, UProgram*& OutProgram, bool InCheckForExistingWindow = true);

	UFUNCTION()
	UUserContext* GetUserContext(int InUserID);

	UFUNCTION()
	UPeacegateFileSystem* GetFilesystem(const int UserID);

	UFUNCTION()
	FString GetIPAddress();

	UFUNCTION()
	int GetUserIDFromUsername(FString InUsername);

	UFUNCTION()
	FUserInfo GetUserInfo(const int InUserID);

	UFUNCTION()
	void ShowWindowOnWorkspace(UProgram* InProgram);

	UFUNCTION()
	EUserDomain GetUserDomain(int InUserID);

	UFUNCTION()
	FString GetUsername(int InUserID);

	UFUNCTION()
	FString GetUserHomeDirectory(int UserID);

	UFUNCTION()
	bool Authenticate(const FString& Username, const FString& Password, int& UserID);

	UFUNCTION()
	bool GetSuitableProgramForFileExtension(const FString& InExtension, class UPeacegateProgramAsset*& OutProgram);

	UFUNCTION()
	void GetFolderTree(TArray<FFolder>& OutFolderTree);

	UFUNCTION()
	void PushFolderTree(const TArray<FFolder>& InFolderTree);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Peacegate")
	FText GetTimeOfDay();
};
