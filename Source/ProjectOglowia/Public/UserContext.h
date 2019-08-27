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
#include "FileOpenResult.h"
#include "UserInfo.h"
#include "MailProvider.h"
#include "UserColor.h"
#include "Computer.h"
#include "PeacenetIdentity.h"
#include "UserContext.generated.h"

class USystemContext;
class UExploit;
class UPeacegateFileSystem;
class APeacenetWorldStateActor;
class UAddressBookContext;
class UComputerService;
class USystemUpgrade;
class UDesktopWidget;
class UVulnerability;
class UProgram;
class UDaemonManager;
class UConsoleContext;
class UPayloadAsset;
class UProcess;


/**
 * A System Context that acts as a specific user.
 */
UCLASS(BlueprintType)
class PROJECTOGLOWIA_API UUserContext : public UObject
{
    friend USystemContext;

    GENERATED_BODY()

private:
    UPROPERTY()
    UUserContext* HackingUser;

    // The owning system context.
    UPROPERTY()
    USystemContext* OwningSystem = nullptr;

    // The user ID of the user who owns this system context.
    UPROPERTY()
    int UserID = 0;

    UPROPERTY()
    UProcess* UserSession;

protected:
    UFUNCTION()
    USystemContext* GetOwningSystem();

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Daemons")
    bool IsDaemonRunning(FName InDaemonName);

    UFUNCTION(BlueprintPure, BlueprintCallable, Category = "User Context")
    FText GetFirstName();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Peacegate")
    bool GetDaemonManager(UDaemonManager*& OutDaemonManager);

    UFUNCTION(BlueprintCallable, Category = "User Context")
    bool KillProcess(int ProcessID, EProcessResult& OutKillResult);

    UFUNCTION()
    UProcess* Fork(FString InName);

    UFUNCTION()
    TArray<FString> GetNearbyHosts();

    UFUNCTION()
    FComputer& GetComputer();

    UFUNCTION()
    bool DnsResolve(FString InHost, FComputer& OutComputer, EConnectionError& OutError);

    UFUNCTION()
    FPeacenetIdentity& GetPeacenetIdentity();

    UFUNCTION()
    bool TryGetTerminalCommand(FName CommandName, UProcess* OwningProcess, ATerminalCommand*& Command, FString& InternalUsage, FString& FriendlyUsage);

    UFUNCTION()
    UUserContext* GetHacker();

    UFUNCTION()
    void SetHacker(UUserContext* InHacker);

    UFUNCTION()
    void Destroy();

    UFUNCTION()
    bool IsPowerUser();

    UFUNCTION()
    TArray<UPeacegateProgramAsset*> GetInstalledPrograms();

    UFUNCTION()
    TArray<UCommandInfo*> GetInstalledCommands();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Networking")
    FString GetIPAddress();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "URL parsing")
    void ParseURL(FString InURL, int InDefaultPort, FString& OutUsername, FString& OutHost, int& OutPort, FString& OutPath);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Email")
    UMailProvider* GetMailProvider();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Identity")
    bool HasIdentity();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    int GetSkill();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    TArray<FString> GetKnownHosts();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Upgrades")
    bool IsUpgradeInstalled(USystemUpgrade* InUpgrade);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    TArray<UWallpaperAsset*> GetAvailableWallpapers();

    UFUNCTION()
    TArray<UPayloadAsset*> GetPayloads();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Exploits")
    TArray<UExploit*> GetExploits();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    UTexture2D* GetCurrentWallpaper();

    UFUNCTION(BlueprintCallable, Category = "User Context")
    void SetCurrentWallpaper(UWallpaperAsset* InWallpaperAsset);

    UFUNCTION(BlueprintCallable, Category = "User Context")
    void DisableWallpaper();

    UFUNCTION()
    FUserInfo GetUserInfo();

    UFUNCTION()
    int GetUserID();

    UFUNCTION()
    float GetStealthiness();

    UFUNCTION()
    void SetStealthiness(float InValue);

    UFUNCTION()
    void Setup(USystemContext* InOwningSystem, int InUserID, UProcess* InUserSessionProcess);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    FString GetUsername();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    FString GetHostname();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    FString GetHomeDirectory();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    FString GetCharacterName();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    UPeacegateFileSystem* GetFilesystem();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    APeacenetWorldStateActor* GetPeacenet();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    UDesktopWidget* GetDesktop();

    UFUNCTION(BlueprintCallable, Category = "User Context")
    bool OpenProgram(FName InExecutableName, UProgram*& OutProgram, bool InCheckForExistingWindow = true);

    UFUNCTION()
    void ShowProgramOnWorkspace(UProgram* InProgram);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    FString GetUserTypeDisplay();

	UFUNCTION(BlueprintCallable, Category = "Program")
	bool OpenFile(const FString& InPath, EFileOpenResult& OutResult);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    FString GetEmailAddress();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    bool IsAdministrator();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    bool GetProcess(int ProcessID, UProcess*& OutProcess, EProcessResult& OutProcessResult);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    TArray<int> GetRunningProcesses();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    bool IsUserContextValid();

    UFUNCTION()
    UUserContext* RequestValidUser();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "User Context")
    EUserColor GetUserColor();

    UFUNCTION(BlueprintCallable, Category = "User Context")
    void SetUserColor(EUserColor InColor);
};