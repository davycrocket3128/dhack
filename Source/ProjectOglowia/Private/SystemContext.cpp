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


#include "SystemContext.h"
#include "Kismet/GameplayStatics.h"
#include "PeacenetWorldStateActor.h"
#include "DesktopWidget.h"
#include "ProceduralGenerationEngine.h"
#include "Exploit.h"
#include "PeacegateFileSystem.h"
#include "CommonUtils.h"
#include "PeacegateProgramAsset.h"
#include "UserContext.h"
#include "Program.h"
#include "WallpaperAsset.h"
#include "GraphicalTerminalCommand.h"
#include "CommandInfo.h"
#include "PayloadAsset.h"
#include "SystemUpgrade.h"
#include "TerminalCommand.h"
#include "Process.h"

void USystemContext::RebuildFilesystemNavigators() {
	for(auto FS : this->RegisteredFilesystems) {
		FS.Value->BuildFolderNavigator();
	}
}

bool USystemContext::IsDaemonRunning(FName InDaemonName) {
	return this->DaemonManager && this->DaemonManager->IsDaemonRunning(InDaemonName);
}

TArray<FString> USystemContext::GetNearbyHosts() {
	return this->GetPeacenet()->GetLinkedHosts(this);
}

bool USystemContext::IsUpgradeInstalled(USystemUpgrade* InUpgrade) {
	if(!InUpgrade) {
		return false;
	}

	return InUpgrade->IsUnlocked(this->GetUserContext(0));
}

void USystemContext::Destroy() {
	// Remove the desktop from the screen, thus killing any UI related to this system context.
	if(this->Desktop) {
		this->Desktop->RemoveFromParent();
	}

	// Unlink the context from the Peacenet world.
	this->Peacenet = nullptr;
	this->Desktop = nullptr;
	
	// Destroy all filesystems.
	TArray<int> FilesystemKeys;
	this->RegisteredFilesystems.GetKeys(FilesystemKeys);

	while(this->RegisteredFilesystems.Num()) {
		int key = FilesystemKeys[0];
		FilesystemKeys.RemoveAt(0);

		this->RegisteredFilesystems.Remove(key);
	}

	// Kill all hacker contexts.
	while(this->Hackers.Num()) {
		this->Hackers[0]->Destroy();
		this->Hackers.RemoveAt(0);
	}

	// And all user contexts.
	TArray<int> userKeys;
	this->Users.GetKeys(userKeys);

	while(this->Users.Num()) {
		int key = userKeys[0];
		userKeys.RemoveAt(0);

		this->Users[key]->Destroy();
		this->Users.Remove(key);
	}

	// Destroy all remaining data in the system context.
	this->MailProvider = nullptr;
	this->ProcessManager = nullptr;
	this->ComputerID = -1;
	this->CharacterID = -1;

}

int USystemContext::GetGameStat(FName InStatName) {
	return this->GetPeacenet()->GetGameStat(InStatName);
}

void USystemContext::SetGameStat(FName InStatName, int InValue) {
	this->GetPeacenet()->SetGameStat(InStatName, InValue);
}

void USystemContext::IncreaseGameStat(FName InStatName) {
	this->GetPeacenet()->IncreaseGameStat(InStatName);
}

bool USystemContext::IsSet(FString InSaveBoolean) {
	return this->GetPeacenet()->IsTrue(InSaveBoolean);
}

void USystemContext::SetSaveBoolean(FString InSaveBoolean, bool InValue) {
	this->GetPeacenet()->SetSaveValue(InSaveBoolean, InValue);
}

bool USystemContext::IsNewGame() {
	return this->GetPeacenet()->IsNewGame();
}

void USystemContext::UpdateInternalIdentity() {
	this->InternalIdentity.ID = -1;
	this->InternalIdentity.CharacterName = this->GetHostname();
	this->InternalIdentity.PreferredAlias = this->GetHostname();
	this->InternalIdentity.EmailAddress = this->GetHostname() + "@" + this->GetIPAddress();
	this->InternalIdentity.CharacterType = EIdentityType::NonPlayer;
	this->InternalIdentity.Skill = 0;
	this->InternalIdentity.Reputation = 0;
	this->InternalIdentity.ComputerID = this->GetComputer().ID;
}

TArray<UPayloadAsset*> USystemContext::GetPayloads() {
	TArray<UPayloadAsset*> Ret;
	for(auto Payload : this->GetPeacenet()->GetAllPayloads()) {
		if(Payload->IsUnlocked(this)) {
			Ret.Add(Payload);
		}
	}
	return Ret;
}

TArray<UExploit*> USystemContext::GetExploits() {
	TArray<UExploit*> Ret;
	for(auto Exploit : this->GetPeacenet()->GetExploits()) {
		if(Exploit->IsUnlocked(this)) {
			Ret.Add(Exploit);
		}
	}
	return Ret;
}

int USystemContext::GetUserIDFromUsername(FString InUsername) {
	for(auto User : this->GetComputer().Users) {
		if(User.Username == InUsername) {
			return User.ID;
		}
	}

	return -1;
}

int USystemContext::GetOpenConnectionCount() {
	return 0;
}

bool USystemContext::UsernameExists(FString InUsername) {
	auto Computer = this->GetComputer();

	for(auto& User : Computer.Users) {
		if(User.Username == InUsername) {
			return true;
		}
	}

	return false;
}

FString ReadFirstLine(FString InText) {
	if (InText.Contains("\n")) {
		int NewLineIndex;
		InText.FindChar('\n', NewLineIndex);
		return InText.Left(NewLineIndex).TrimStartAndEnd();
	} else {
		return InText.TrimStartAndEnd();
	}
}

FString USystemContext::GetHostname() {
	if (!CurrentHostname.IsEmpty()) {
		// Speed increase: No need to consult the filesystem for this.
		return CurrentHostname;
	}
	
	UPeacegateFileSystem* RootFS = this->GetFilesystem(0);
	if (RootFS->FileExists("/etc/hostname")) {
		EFilesystemStatusCode StatusCode;
		RootFS->ReadText("/etc/hostname", this->CurrentHostname, StatusCode);
		CurrentHostname = ReadFirstLine(CurrentHostname);
		return this->CurrentHostname;
	}

	CurrentHostname = "localhost";
	return CurrentHostname;
}

TArray<UPeacegateProgramAsset*> USystemContext::GetInstalledPrograms() {
	check(Peacenet);

	TArray<UPeacegateProgramAsset*> OutArray;

	for(auto Program : this->GetPeacenet()->Programs) {
		if(Program->IsUnlocked(this)) {
			OutArray.Add(Program);
		}
	}

	return OutArray;
}

TArray<UCommandInfo*> USystemContext::GetInstalledCommands() {
	check(this->GetPeacenet());

	TArray<UCommandInfo*> Ret;

	TArray<FName> CommandNames;
	GetPeacenet()->CommandInfo.GetKeys(CommandNames);

	for(auto Name : CommandNames) {
		UCommandInfo* Info = GetPeacenet()->CommandInfo[Name];

		if(Info->IsUnlocked(this)) {
			Ret.Add(Info);
		}
	}

	return Ret;
}

bool USystemContext::HasIdentity() {
	return this->Peacenet->IdentityExists(this->GetComputer().SystemIdentity);
}

bool USystemContext::OpenProgram(FName InExecutableName, UProgram*& OutProgram, bool InCheckForExistingWindow) {
	check(this->GetPeacenet());
	check(this->GetDesktop());

	UPeacegateProgramAsset* PeacegateProgram = nullptr;

	if(!this->GetPeacenet()->FindProgramByName(InExecutableName, PeacegateProgram)) {
		return false;
	}

	if(!PeacegateProgram->IsUnlocked(this)) {
		return false;
	}
	

	UProgram* Program = this->GetDesktop()->SpawnProgramFromClass(PeacegateProgram->ProgramClass, PeacegateProgram->FullName, PeacegateProgram->AppLauncherItem.Icon, PeacegateProgram->EnableMinimizeAndMaximize);

	check(Program);

	this->GetDesktop()->ShowProgramOnWorkspace(Program);

	return Program;
}

UPeacegateFileSystem * USystemContext::GetFilesystem(const int UserID) {
	if (!RegisteredFilesystems.Contains(UserID)) {
		UPeacegateFileSystem* NewFS = UCommonUtils::CreateFilesystem(this, UserID);
		TScriptDelegate<> ModifiedDelegate;
		ModifiedDelegate.BindUFunction(this, "HandleFileSystemEvent");
		NewFS->FilesystemOperation.Add(ModifiedDelegate);
		this->RegisteredFilesystems.Add(UserID, NewFS);
		return NewFS;
	}

	return this->RegisteredFilesystems[UserID];
}

bool USystemContext::TryGetTerminalCommand(FName CommandName, UProcess* OwningProcess, ATerminalCommand *& OutCommand, FString& InternalUsage, FString& FriendlyUsage) {
	check(Peacenet);

	UPeacegateProgramAsset* Program = nullptr;
	if (GetPeacenet()->FindProgramByName(CommandName, Program)) {
		if(!Program->IsUnlocked(this)) {
			return false;
		}

		AGraphicalTerminalCommand* GraphicalCommand = Cast<AGraphicalTerminalCommand>(ATerminalCommand::SpawnCommand<AGraphicalTerminalCommand>(OwningProcess->Fork(Program->ID.ToString())));
		GraphicalCommand->ProgramAsset = Program;
		GraphicalCommand->CommandInfo = Peacenet->CommandInfo[CommandName];
		OutCommand = GraphicalCommand;
		return true;
	}

	if (!GetPeacenet()->CommandInfo.Contains(CommandName)) {
		return false;
	}

	UCommandInfo* Info = GetPeacenet()->CommandInfo[CommandName];

	if(!Info->IsUnlocked(this)) {
		return false;
	}
	
 	FVector Location(0.0f, 0.0f, 0.0f);
	FRotator Rotation(0.0f, 0.0f, 0.0f);
 	FActorSpawnParameters SpawnInfo;

	OutCommand = this->GetPeacenet()->GetWorld()->SpawnActor<ATerminalCommand>(Info->CommandClass, Location, Rotation, SpawnInfo);

	if(!OutCommand) return false;

	OutCommand->MyProcess = OwningProcess->Fork(Info->ID.ToString());

	OutCommand->CommandInfo = Info;

	return true;
}

FString USystemContext::GetIPAddress() {
	check(this->GetPeacenet());

	return this->GetPeacenet()->GetIPAddress(this->GetComputer());
}

FUserInfo USystemContext::GetUserInfo(const int InUserID) {
	if (InUserID == -1) {
		FUserInfo AnonInfo;
		AnonInfo.IsAdminUser = false;
		AnonInfo.Username = "<anonymous>";
		return AnonInfo;
	}

	for (FUser User : GetComputer().Users) {
		if (User.ID == InUserID) {
			FUserInfo Info;
			Info.Username = User.Username;
			Info.IsAdminUser = (User.Domain == EUserDomain::Administrator);
			return Info;
		}
	}

	return FUserInfo();
}

void USystemContext::ShowWindowOnWorkspace(UProgram * InProgram) {
	// DEPRECATED IN FAVOUR OF UUserContext::ShowProgramOnWorkspace().
	if (Desktop && InProgram) {
		Desktop->ShowProgramOnWorkspace(InProgram);
	}
}

EUserDomain USystemContext::GetUserDomain(int InUserID) {
	if (InUserID == -1) {
		return EUserDomain::Anonymous;
	}

	for (FUser User : GetComputer().Users) {
		if (User.ID == InUserID) {
			return User.Domain;
		}
	}

	return EUserDomain::User;
}

FString USystemContext::GetUsername(int InUserID) {
	FUserInfo UserInfo = this->GetUserInfo(InUserID);
	return UserInfo.Username;
}

FString USystemContext::GetUserHomeDirectory(int UserID) {
	if (this->GetUserDomain(UserID) == EUserDomain::Anonymous) {
		return "/";
	}

	for (FUser User : GetComputer().Users) {
		if (User.ID == UserID) {
			if (User.Domain == EUserDomain::Administrator) {
				return TEXT("/root");
			}
			return TEXT("/home/") + User.Username;
		}
	}

	return FString();
}

bool USystemContext::Authenticate(const FString & Username, const FString & Password, int & UserID) {
	for (FUser User : GetComputer().Users) {
		if (User.Username == Username && User.Password == Password) {
			UserID = User.ID;
			return true;
		}
	}

	return false;
}

bool USystemContext::GetSuitableProgramForFileExtension(const FString & InExtension, UPeacegateProgramAsset *& OutProgram) {
	for(auto Program : this->GetInstalledPrograms()) {
		if (Program->SupportedFileExtensions.Contains(InExtension)) {
			OutProgram = Program;
			return true;
		}
	}
	return false;
}

bool USystemContext::IsIPAddress(FString InIPAddress) {
	return this->GetPeacenet()->IsIPAddress(InIPAddress);
}

UDesktopWidget* USystemContext::GetDesktop() {
	return this->Desktop;
}

FPeacenetIdentity& USystemContext::GetCharacter() {
	check(this->GetPeacenet());

	auto MyPeacenet = this->GetPeacenet();

	if(!this->HasIdentity())  {
		this->UpdateInternalIdentity();
		return this->InternalIdentity;
	}

	return MyPeacenet->GetCharacterByID(this->GetComputer().SystemIdentity);
}

UUserContext* USystemContext::GetHackerContext(int InUserID, UUserContext* HackingUser) {
	for(auto Hacker : Hackers) {
		if(Hacker->GetUserID() == InUserID && Hacker->GetHacker() == HackingUser) {
			return Hacker;
		}
	}

	UUserContext* NewHacker = NewObject<UUserContext>(this);
	NewHacker->Setup(this, InUserID, this->ProcessManager->CreateProcessAs("user-session", InUserID));
	NewHacker->SetHacker(HackingUser);
	Hackers.Add(NewHacker);
	return NewHacker;
}

UUserContext* USystemContext::GetUserContext(int InUserID) {
	if(this->Users.Contains(InUserID)) {
		return this->Users[InUserID];
	} else {
		UUserContext* User = NewObject<UUserContext>(this);
		User->Setup(this, InUserID, this->ProcessManager->CreateProcessAs("user-session", InUserID));
		Users.Add(InUserID, User);
		return User;
	}
}

FComputer& USystemContext::GetComputer() {
	check(this->GetPeacenet());

	auto MyPeacenet = this->GetPeacenet();

	return MyPeacenet->GetComputerByID(this->ComputerID);
}

APeacenetWorldStateActor* USystemContext::GetPeacenet() {
	return this->Peacenet;
}

void USystemContext::SetupDesktop(int InUserID) {
	check(!this->GetDesktop());

	this->AppendLog("Beginning desktop session - uid: " + FString::FromInt(InUserID));

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetPeacenet()->GetWorld(), 0);

	this->Desktop = CreateWidget<UDesktopWidget, APlayerController>(PlayerController, this->GetPeacenet()->DesktopClass);

	check(GetDesktop());

	this->Desktop->SystemContext = this;
	this->Desktop->UserID = InUserID;
}

void USystemContext::GetFolderTree(TArray<FFolder>& OutFolderTree) {
	OutFolderTree = GetComputer().Filesystem;
}

void USystemContext::PushFolderTree(const TArray<FFolder>& InFolderTree) {
	GetComputer().Filesystem = InFolderTree;
}

FText USystemContext::GetTimeOfDay() {
	return GetPeacenet()->GetTimeOfDay();
}

void USystemContext::ExecuteCommand(FString InCommand) {
	check(this->GetDesktop());

	this->GetDesktop()->ExecuteCommand(InCommand);
}

void USystemContext::HandleFileSystemEvent(EFilesystemEventType InType, FString InPath) {
	switch (InType) {
		case EFilesystemEventType::WriteFile:
			if (InPath == "/etc/hostname") {
				auto fs = GetFilesystem(0);
				EFilesystemStatusCode err;
				fs->ReadText("/etc/hostname", this->CurrentHostname, err);
				CurrentHostname = ReadFirstLine(CurrentHostname);

				this->AppendLog("Hostname changed to " + CurrentHostname);
			}
			break;
	}

	// If the path is within /var we might want to check to make sure the log still exists.
	if (InPath.StartsWith("/var")) {
		auto RootFS = GetFilesystem(0);
		EFilesystemStatusCode Anus;

		// Does /var/log not exist?
		if (!RootFS->DirectoryExists("/var/log")) {
			if (!RootFS->DirectoryExists("/var")) {
				RootFS->CreateDirectory("/var", Anus);
			}
			RootFS->CreateDirectory("/var/log", Anus);
		}

		// Does peacegate.log not exist?
		// if (!RootFS->FileExists("/var/log/system.log"))
		// {
		//	// write blank log.
		//	RootFS->WriteText("/var/log/system.log", "");
		//}
		// The above code was written during an episode of severe
		// acute hypocaffiemia in Michael VanOverbeek.

	}
}

TArray<UWallpaperAsset*> USystemContext::GetAvailableWallpapers() {
	TArray<UWallpaperAsset*> Ret;
	for (auto Wallpaper : this->GetPeacenet()->Wallpapers) {
		if(Wallpaper->IsDefault || Wallpaper->UnlockedByDefault || this->GetComputer().UnlockedWallpapers.Contains(Wallpaper->InternalID)) {
			Ret.Add(Wallpaper);
		}
	}
	return Ret;
}

void USystemContext::DisableWallpaper() {
	this->GetComputer().CurrentWallpaper = nullptr;
}

void USystemContext::SetCurrentWallpaper(UWallpaperAsset* InWallpaperAsset) {
	// Make sure it's not null.
	check(InWallpaperAsset);

	// If it's unlocked by default or already unlocked, we just set it.
	if(InWallpaperAsset->UnlockedByDefault || InWallpaperAsset->IsDefault || this->GetComputer().UnlockedWallpapers.Contains(InWallpaperAsset->InternalID)) {
		// Set the wallpaper.
		this->GetComputer().CurrentWallpaper = InWallpaperAsset->WallpaperTexture;
	} else {
		// BETA TODO: Announce item unlock.
		this->GetComputer().UnlockedWallpapers.Add(InWallpaperAsset->InternalID);

		// Set the wallpaper.
		this->GetComputer().CurrentWallpaper = InWallpaperAsset->WallpaperTexture;
	}
}

void USystemContext::UpdateSystemFiles() {
	// This function updates the system based on save data and in-game assets.
	//
	// A.K.A: This is the function that updates things like what wallpapers are installed.

	// So first we need a root fs context.
	UPeacegateFileSystem* RootFS = this->GetFilesystem(0);

	EFilesystemStatusCode Anus;

	// Does /var/log not exist?
	if (!RootFS->DirectoryExists("/var/log")) {
		if (!RootFS->DirectoryExists("/var")) {
			RootFS->CreateDirectory("/var", Anus);
		}
		RootFS->CreateDirectory("/var/log", Anus);
	}

	// Does peacegate.log not exist?
	if (!RootFS->FileExists("/var/log/peacegate.log")) {
		// write blank log.
		RootFS->WriteText("/var/log/peacegate.log", "");
	}
}

bool USystemContext::DnsResolve(FString InHost, FComputer& OutComputer, EConnectionError& OutConnectionError) {
	// Default the connection error to nothing.
	OutConnectionError = EConnectionError::None;

	// TODO: /etc/hosts support.

	// Map localhost, 127.0.0.1, and the system hostname to our public IP address, since 0.3.0
	// doesn't have a concept of LANs, network address traversal, etc.
	if(InHost == "127.0.0.1" || InHost == "localhost" || InHost == this->GetHostname()) {

		InHost = this->GetIPAddress();
	}

	// It's your fucking problem now, world state.
	return this->GetPeacenet()->DnsResolve(InHost, OutComputer, OutConnectionError);
}

UMailProvider* USystemContext::GetMailProvider() {
	return this->MailProvider;
}

void USystemContext::Crash() {
	// This behaves differently if we have a desktop than if we don't.
	//
	// If we don't have a desktop then we basically kick all hacker users off the
	// system and mark our computer as "crashed."  Essentially making it offline
	// and un-hackable.
	// 
	// If we have a desktop than this is the player version which presents a kernel
	// panic UI, essentially the "game over" screen, and restarts the game.
	if(this->GetDesktop()) {
		// If we're in a mission, silently fail it.  This ensures that the game
		// isn't in a fucked mission state because the mission is abandoned and the
		// player is in free roam.
		if(this->GetPeacenet()->IsInMission()) {
			this->GetPeacenet()->SilentlyFailMission();
		}

		// Show the crash.
		this->GetDesktop()->KernelPanic();
	} else {
		// Mark ourselves as crashed.
		this->GetComputer().Crashed = true;
	}

	// DANGER! The game is in a VERY broken state right now and it's up to us to fix it.
	// The "peacegate" process being killed, in fact, doesn't just cause fake problems, as
	// all user session processes (and thus all commands, programs, etc) either directly
	// or indirectly fork from it - and thus get killed.
	//
	// To fix the game state, we need to essentially nuke all User Contexts resulting in Unreal
	// doing a ton of garbage collection of the old game state, and all User Contexts resetting.
	this->Users.Empty();

	// That may put the desktop as well as any active console contexts in a broken state, however
	// they will regain proper state either at next simulation tick or next access.
	//
	// We also need to get rid of all filesystem contexts because they're assigned to user contexts.
	this->RegisteredFilesystems.Empty();

	// Restart the daemon manager.
	this->RestartDaemonManager();
}

void USystemContext::Setup(int InComputerID, int InCharacterID, APeacenetWorldStateActor* InPeacenet) {
	check(InPeacenet);

	// assign all our IDs and Peacenet.
	this->ComputerID = InComputerID;
	this->CharacterID = InCharacterID;
	this->Peacenet = InPeacenet;

	// create and initialize the process manager.
	this->ProcessManager = NewObject<UProcessManager>();
	this->ProcessManager->Initialize(this);

	this->MailProvider = NewObject<UMailProvider>(this);
	this->MailProvider->Setup(this);

	// Do we not have a preferred alias?
	if(!this->GetCharacter().PreferredAlias.Len()) {
		this->GetCharacter().PreferredAlias = this->GetCharacter().CharacterName;
	}

	// Now we need a filesystem.
	UPeacegateFileSystem* fs = this->GetFilesystem(0);

	// Any FS errors are reported here.
	EFilesystemStatusCode fsStatus = EFilesystemStatusCode::OK;

	// Create the logfile directories.
	if(!fs->DirectoryExists("/var")) {
		fs->CreateDirectory("/var", fsStatus);
	}

	if(!fs->DirectoryExists("/var/log")) {
		fs->CreateDirectory("/var/log", fsStatus);
	}

	this->AppendLog("System online.");
	this->AppendLog("Welcome to Peacegate OS.");
	this->AppendLog("IDENTITY: " + this->GetCharacter().CharacterName);
	this->AppendLog("IP ADDRESS: " + this->GetIPAddress());

	// Create /home if it doesn't exist.
	if(!fs->DirectoryExists("/home")) {
		fs->CreateDirectory("/home", fsStatus);
	}

	// Create the bin directory - which actually means something now.
	if(!fs->DirectoryExists("/bin")) {
		fs->CreateDirectory("/bin", fsStatus);
	}


	// Go through every user on the system.
	for(auto& user : this->GetComputer().Users) {
		// Get the home directory for the user.
		FString home = this->GetUserHomeDirectory(user.ID);

		// If the user's home directory doesn't exist, create it.
		if(!fs->DirectoryExists(home)) {
			fs->CreateDirectory(home, fsStatus);
			this->AppendLog("Creating home directory " + home + " for user " + user.Username);
		}

		// These sub-directories are important too.
		TArray<FString> homeDirs = {
			"Desktop",
			"Documents",
			"Downloads",
			"Music",
			"Pictures",
			"Videos"
		};

		for(auto subDir : homeDirs) {
			if(!fs->DirectoryExists(home + "/" + subDir)) {
				fs->CreateDirectory(home + "/" + subDir, fsStatus);
			}
		}
	}

	// Now we'll get all the installed terminal commands to show in /bin.
	TArray<UCommandInfo*> InstalledCommands = this->GetInstalledCommands();
	TArray<FName> CommandKeys;
	this->GetPeacenet()->CommandInfo.GetKeys(CommandKeys);
	
	for(int i = 0; i < CommandKeys.Num(); i++) {
		UCommandInfo* CommandInfo = this->GetPeacenet()->CommandInfo[CommandKeys[i]];
		if(InstalledCommands.Contains(CommandInfo)) {
			fs->SetFileRecord("/bin/" + CommandInfo->ID.ToString(), EFileRecordType::Command, i);
		}
	}


	// Make all programs show in /bin.
	TArray<UPeacegateProgramAsset*> InstalledPrograms = this->GetInstalledPrograms();

	for(int i = 0; i < this->GetPeacenet()->Programs.Num(); i++) {
		UPeacegateProgramAsset* ProgramAsset = this->GetPeacenet()->Programs[i];
		if(InstalledPrograms.Contains(ProgramAsset)) {
			fs->SetFileRecord("/bin/" + ProgramAsset->ID.ToString(), EFileRecordType::Program, i);
		}
	}

	// If the cryptowallets directory exists, delete it.
	if(fs->DirectoryExists("/usr/share/wallets")) {
		fs->Delete("/usr/share/wallets", true, fsStatus);
	}

	if(!fs->DirectoryExists("/usr")) {
		fs->CreateDirectory("/usr", fsStatus);

		if(!fs->DirectoryExists("/usr/share")) {
			fs->CreateDirectory("/usr/share", fsStatus);
		}

	}

	// Create and initialize the daemon manager.
	this->InitDaemonManager();
}

bool USystemContext::GetDaemonManager(UUserContext* InUserContext, UDaemonManager*& OutDaemonManager) {
	if(InUserContext->IsAdministrator() || InUserContext->IsPowerUser()) {
		OutDaemonManager = this->DaemonManager;
		return true;
	}
	OutDaemonManager = nullptr;
	return false;
}

void USystemContext::InitDaemonManager() {
	// Create the daemon manager if it is nullptr.
	if(!this->DaemonManager) {
		this->DaemonManager = NewObject<UDaemonManager>();
	}

	// Spawn a system process for the daemon manager.  All daemons will fork from it.
	UProcess* DaemonManagerProcess = this->ProcessManager->CreateProcess("system-daemon-manager");

	// Initialize the daemon manager.
	this->DaemonManager->Initialize(this, DaemonManagerProcess);

	// When the daemon manager ends, we'll restart the daemon manager.
	TScriptDelegate<> DaemonManagerEnded;
	DaemonManagerEnded.BindUFunction(this, "RestartDaemonManager");
	DaemonManagerProcess->OnKilled.Add(DaemonManagerEnded);
}

void USystemContext::RestartDaemonManager() {
	if(this->ProcessManager->IsActive()) {
		this->DaemonManager = nullptr;
		this->InitDaemonManager();
	}
}

FString USystemContext::GetEmailAddress() {
	return this->GetCharacter().EmailAddress;
}

void USystemContext::AppendLog(FString InLogText) {
	FString ExistingLog = "";
	UPeacegateFileSystem* FS = this->GetFilesystem(0);
	EFilesystemStatusCode EatMyFuckingBurger;
	if(FS->FileExists("/var/log/system.log")) {
		FS->ReadText("/var/log/system.log", ExistingLog, EatMyFuckingBurger);
	}

	ExistingLog = ExistingLog.TrimStartAndEnd();

	ExistingLog += "\r\n[" + this->GetTimeOfDay().ToString() + "] " + InLogText;

	ExistingLog = ExistingLog.TrimStartAndEnd();

	FS->WriteText("/var/log/system.log", ExistingLog);
}

TArray<int> USystemContext::GetRunningProcesses() {
	TArray<int> ret;
	for(UProcess* Process : this->ProcessManager->GetAllProcesses()) {
		ret.Add(Process->GetProcessID());
	}
	return ret;
}

bool USystemContext::GetProcess(int ProcessID, UUserContext* InUserContext, UProcess*& OutProcess, EProcessResult& OutProcessResult) {
	// Check to make sure the user context belongs to us.
	check(InUserContext->GetOwningSystem() == this);

	// Release builds: Have the owning system kill the process if the owning system isn't us.
	if(InUserContext->GetOwningSystem() != this) {
		// A properly-functioning Peacenet build should, under NO CIRCUMSTANCES, execute this line of code.
		// This is a protection from attempts to kill processes with user contexts that do not
		// belong to the system which owns the process.
		return InUserContext->GetOwningSystem()->GetProcess(ProcessID, InUserContext, OutProcess, OutProcessResult);
	}

	// Get the user ID of the user context.
	int UserID = InUserContext->GetUserID();

	// Kill the process at a low-level!
	return this->ProcessManager->GetProcess(ProcessID, UserID, OutProcess, OutProcessResult);

}

bool USystemContext::KillProcess(int ProcessID, UUserContext* UserContext, EProcessResult& OutKillResult) {
	// Check to make sure the user context belongs to us.
	check(UserContext->GetOwningSystem() == this);

	// Release builds: Have the owning system kill the process if the owning system isn't us.
	if(UserContext->GetOwningSystem() != this) {
		// A properly-functioning Peacenet build should, under NO CIRCUMSTANCES, execute this line of code.
		// This is a protection from attempts to kill processes with user contexts that do not
		// belong to the system which owns the process.
		return UserContext->GetOwningSystem()->KillProcess(ProcessID, UserContext, OutKillResult);
	}

	// Get the user ID of the user context.
	int UserID = UserContext->GetUserID();

	// Kill the process at a low-level!
	return this->ProcessManager->KillProcess(ProcessID, UserID, OutKillResult);
}

bool USystemContext::IsEnvironmentVariableSet(FString InVariable) {
	// Simply returns whether the computer has an environment variable set.
	return this->GetComputer().EnvironmentVariables.Contains(InVariable);
}

bool USystemContext::GetEnvironmentVariable(FString InVariable, FString& OutValue) {
	// Check if we have the variable:
	if(this->IsEnvironmentVariableSet(InVariable)) {
		// Retrieve the value.
		OutValue = this->GetComputer().EnvironmentVariables[InVariable];
		return true;
	}
	return false;
}

void USystemContext::SetEnvironmentVariable(FString InVariable, FString InValue) {
	// If we have a variable with the same name, we set it. Else, we add it.
	if(this->IsEnvironmentVariableSet(InVariable)) {
		this->GetComputer().EnvironmentVariables[InVariable] = InValue;
	} else {
		this->GetComputer().EnvironmentVariables.Add(InVariable, InValue);
	}
}

void USystemContext::UnsetEnvironmentVariable(FString InVariable) {
	// If we have the variable set, remove it.
	if(this->IsEnvironmentVariableSet(InVariable)) {
		this->GetComputer().EnvironmentVariables.Remove(InVariable);
	}
}
