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


#include "USystemContext.h"
#include "Kismet/GameplayStatics.h"
#include "PeacenetWorldStateActor.h"
#include "UDesktopWidget.h"
#include "UProceduralGenerationEngine.h"
#include "Exploit.h"
#include "UPeacegateFileSystem.h"
#include "CommonUtils.h"
#include "PeacegateProgramAsset.h"
#include "UUserContext.h"
#include "FAdjacentNode.h"
#include "UProgram.h"
#include "URainbowTable.h"
#include "WallpaperAsset.h"
#include "UGraphicalTerminalCommand.h"
#include "CommandInfo.h"
#include "PayloadAsset.h"

TArray<UPayloadAsset*> USystemContext::GetPayloads()
{
	TArray<UPayloadAsset*> UnlockedPayloads = this->GetComputer().Payloads;

	for(auto Payload : this->GetPeacenet()->GetAllPayloads())
	{
		if(Payload->UnlockedByDefault)
		{
			if(!UnlockedPayloads.Contains(Payload))
				UnlockedPayloads.Add(Payload);
		}
	}

	return UnlockedPayloads;
}

TArray<UExploit*> USystemContext::GetExploits()
{
	return this->GetComputer().Exploits;
}

FString USystemContext::GetProcessUsername(FPeacegateProcess InProcess)
{
	return this->GetUserInfo(InProcess.UID).Username;
}

int USystemContext::GetUserIDFromUsername(FString InUsername)
{
	for(auto User : this->GetComputer().Users)
	{
		if(User.Username == InUsername)
			return User.ID;
	}

	return -1;
}

int USystemContext::GetOpenConnectionCount()
{
	return 0;
}

bool USystemContext::UsernameExists(FString InUsername)
{
	auto Computer = this->GetComputer();

	for(auto& User : Computer.Users)
	{
		if(User.Username == InUsername)
			return true;
	}

	return false;
}

FString ReadFirstLine(FString InText)
{
	if (InText.Contains("\n"))
	{
		int NewLineIndex;
		InText.FindChar('\n', NewLineIndex);
		return InText.Left(NewLineIndex).TrimStartAndEnd();
	}
	else
	{
		return InText.TrimStartAndEnd();
	}
}

FString USystemContext::GetHostname()
{
	if (!CurrentHostname.IsEmpty())
	{
		// Speed increase: No need to consult the filesystem for this.
		return CurrentHostname;
	}
	
	UPeacegateFileSystem* RootFS = this->GetFilesystem(0);
	if (RootFS->FileExists("/etc/hostname"))
	{
		EFilesystemStatusCode StatusCode;
		RootFS->ReadText("/etc/hostname", this->CurrentHostname, StatusCode);
		CurrentHostname = ReadFirstLine(CurrentHostname);
		return this->CurrentHostname;
	}

	CurrentHostname = "localhost";
	return CurrentHostname;
}

TArray<UPeacegateProgramAsset*> USystemContext::GetInstalledPrograms()
{
	check(Peacenet);

	TArray<UPeacegateProgramAsset*> OutArray;

	for(auto Program : this->GetPeacenet()->Programs)
	{
		if(GetPeacenet()->GameType->GameRules.DoUnlockables)
		{
			if(!GetComputer().InstalledPrograms.Contains(Program->ID) && !Program->IsUnlockedByDefault)
				continue;
		}
		OutArray.Add(Program);
	}

	return OutArray;
}

TArray<UCommandInfo*> USystemContext::GetInstalledCommands()
{
	check(this->GetPeacenet());

	TArray<UCommandInfo*> Ret;

	TArray<FName> CommandNames;
	GetPeacenet()->CommandInfo.GetKeys(CommandNames);

	for(auto Name : CommandNames)
	{
		UCommandInfo* Info = GetPeacenet()->CommandInfo[Name];

		if(GetPeacenet()->GameType->GameRules.DoUnlockables)
		{
			if(!GetComputer().InstalledCommands.Contains(Name) && !Info->UnlockedByDefault)
				continue;
		}
		Ret.Add(Info);
	}

	return Ret;
}

TArray<FAdjacentNodeInfo> USystemContext::ScanForAdjacentNodes()
{
	check(this->GetPeacenet());

	int CharID = this->GetCharacter().ID;

	TArray<FAdjacentNodeInfo> Ret;

	for(auto& OtherIdentity : this->GetPeacenet()->GetAdjacentNodes(this->GetCharacter()))
	{
		FAdjacentNodeInfo Node;
		Node.NodeName = OtherIdentity.CharacterName;
		Node.Link = FAdjacentNode();
		Node.Link.NodeA = CharID;
		Node.Link.NodeB = OtherIdentity.ID;
		Ret.Add(Node);

		if(!this->GetPeacenet()->SaveGame->PlayerDiscoveredNodes.Contains(OtherIdentity.ID))
		{
			this->GetPeacenet()->SaveGame->PlayerDiscoveredNodes.Add(OtherIdentity.ID);
		}
	}

	return Ret;
}

bool USystemContext::OpenProgram(FName InExecutableName, UProgram*& OutProgram, bool InCheckForExistingWindow)
{
	check(this->GetPeacenet());
	check(this->GetDesktop());

	UPeacegateProgramAsset* PeacegateProgram = nullptr;

	if(!this->GetPeacenet()->FindProgramByName(InExecutableName, PeacegateProgram))
		return false;

	if(this->GetPeacenet()->GameType->GameRules.DoUnlockables)
	{
		if(!this->GetComputer().InstalledPrograms.Contains(InExecutableName) && !PeacegateProgram->IsUnlockedByDefault)
		{
			return false;
		}
	}

	UProgram* Program = this->GetDesktop()->SpawnProgramFromClass(PeacegateProgram->ProgramClass, PeacegateProgram->FullName, PeacegateProgram->AppLauncherItem.Icon, PeacegateProgram->EnableMinimizeAndMaximize);

	check(Program);

	this->GetDesktop()->ShowProgramOnWorkspace(Program);

	return Program;
}

UPeacegateFileSystem * USystemContext::GetFilesystem(const int UserID)
{
	if (!RegisteredFilesystems.Contains(UserID))
	{
		UPeacegateFileSystem* NewFS = UCommonUtils::CreateFilesystem(this, UserID);
		TScriptDelegate<> ModifiedDelegate;
		ModifiedDelegate.BindUFunction(this, "HandleFileSystemEvent");
		NewFS->FilesystemOperation.Add(ModifiedDelegate);
		this->RegisteredFilesystems.Add(UserID, NewFS);
		return NewFS;
	}

	return this->RegisteredFilesystems[UserID];
}

bool USystemContext::TryGetTerminalCommand(FName CommandName, ATerminalCommand *& OutCommand, FString& InternalUsage, FString& FriendlyUsage)
{
	check(Peacenet);

	UPeacegateProgramAsset* Program = nullptr;
	if (GetPeacenet()->FindProgramByName(CommandName, Program))
	{
		if(GetPeacenet()->GameType->GameRules.DoUnlockables)
		{
			if(!GetComputer().InstalledPrograms.Contains(CommandName) && !Program->IsUnlockedByDefault)
			{
				return false;
			}
		}

 		FVector Location(0.0f, 0.0f, 0.0f);
		 FRotator Rotation(0.0f, 0.0f, 0.0f);
 		FActorSpawnParameters SpawnInfo;

		AGraphicalTerminalCommand* GraphicalCommand = this->GetPeacenet()->GetWorld()->SpawnActor<AGraphicalTerminalCommand>(Location, Rotation, SpawnInfo);
		GraphicalCommand->ProgramAsset = Program;
		GraphicalCommand->CommandInfo = Peacenet->CommandInfo[CommandName];
		OutCommand = GraphicalCommand;
		return true;
	}

	if (!GetPeacenet()->CommandInfo.Contains(CommandName))
	{
		return false;
	}

	UCommandInfo* Info = GetPeacenet()->CommandInfo[CommandName];

	if(GetPeacenet()->GameType->GameRules.DoUnlockables)
	{
		if(!GetComputer().InstalledCommands.Contains(CommandName) && !Info->UnlockedByDefault)
		{
			return false;
		}
	}

 	FVector Location(0.0f, 0.0f, 0.0f);
	FRotator Rotation(0.0f, 0.0f, 0.0f);
 	FActorSpawnParameters SpawnInfo;

	OutCommand = this->GetPeacenet()->GetWorld()->SpawnActor<ATerminalCommand>(Info->CommandClass, Location, Rotation, SpawnInfo);

	OutCommand->CommandInfo = Info;

	return true;
}

FString USystemContext::GetIPAddress()
{
	check(this->GetPeacenet());

	return this->GetPeacenet()->GetIPAddress(this->GetComputer());
}

FUserInfo USystemContext::GetUserInfo(const int InUserID)
{
	if (InUserID == -1)
	{
		FUserInfo AnonInfo;
		AnonInfo.IsAdminUser = false;
		AnonInfo.Username = "<anonymous>";
		return AnonInfo;
	}

	for (FUser User : GetComputer().Users)
	{
		if (User.ID == InUserID)
		{
			FUserInfo Info;
			Info.Username = User.Username;
			Info.IsAdminUser = (User.Domain == EUserDomain::Administrator);
			return Info;
		}
	}

	return FUserInfo();
}

void USystemContext::ShowWindowOnWorkspace(UProgram * InProgram)
{
	// DEPRECATED IN FAVOUR OF UUserContext::ShowProgramOnWorkspace().
	if (Desktop && InProgram)
	{
		Desktop->ShowProgramOnWorkspace(InProgram);
	}
}

EUserDomain USystemContext::GetUserDomain(int InUserID)
{
	if (InUserID == -1)
	{
		return EUserDomain::Anonymous;
	}

	for (FUser User : GetComputer().Users)
	{
		if (User.ID == InUserID)
		{
			return User.Domain;
		}
	}

	return EUserDomain::User;
}

FString USystemContext::GetUsername(int InUserID)
{
	FUserInfo UserInfo = this->GetUserInfo(InUserID);
	return UserInfo.Username;
}

FString USystemContext::GetUserHomeDirectory(int UserID)
{
	if (this->GetUserDomain(UserID) == EUserDomain::Anonymous)
	{
		return "/";
	}

	for (FUser User : GetComputer().Users)
	{
		if (User.ID == UserID)
		{
			if (User.Domain == EUserDomain::Administrator)
				return TEXT("/root");
			return TEXT("/home/") + User.Username;
		}
	}

	return FString();
}

bool USystemContext::Authenticate(const FString & Username, const FString & Password, int & UserID)
{
	for (FUser User : GetComputer().Users)
	{
		if (User.Username == Username && User.Password == Password)
		{
			UserID = User.ID;
			return true;
		}
	}

	return false;
}

bool USystemContext::GetSuitableProgramForFileExtension(const FString & InExtension, UPeacegateProgramAsset *& OutProgram)
{
	for(auto Program : this->GetPeacenet()->Programs)
	{
		if(this->GetPeacenet()->GameType->GameRules.DoUnlockables)
		{
			if(!this->GetComputer().InstalledPrograms.Contains(Program->ID) && !Program->IsUnlockedByDefault)
			{
				continue;
			}
		}

		if (Program->SupportedFileExtensions.Contains(InExtension))
		{
			OutProgram = Program;
			return true;
		}
	}
	return false;
}

bool USystemContext::IsIPAddress(FString InIPAddress)
{
	return this->GetPeacenet()->SaveGame->ComputerIPMap.Contains(InIPAddress);
}

UDesktopWidget* USystemContext::GetDesktop()
{
	return this->Desktop;
}

FPeacenetIdentity& USystemContext::GetCharacter()
{
	check(this->GetPeacenet());

	auto MyPeacenet = this->GetPeacenet();

	int CharacterIndex = 0;
	FPeacenetIdentity Character;

	check(MyPeacenet->SaveGame->GetCharacterByID(this->CharacterID, Character, CharacterIndex));

	return MyPeacenet->SaveGame->Characters[CharacterIndex];
}

UUserContext* USystemContext::GetHackerContext(int InUserID, UUserContext* HackingUser)
{
	for(auto Hacker : Hackers)
	{
		if(Hacker->GetUserID() == InUserID && Hacker->GetHacker() == HackingUser)
		{
			return Hacker;
		}
	}

	UUserContext* NewHacker = NewObject<UUserContext>(this);
	NewHacker->Setup(this, InUserID);
	NewHacker->SetHacker(HackingUser);
	Hackers.Add(NewHacker);
	return NewHacker;
}

UUserContext* USystemContext::GetUserContext(int InUserID)
{
	if(this->Users.Contains(InUserID))
	{
		return this->Users[InUserID];
	}
	else
	{
		UUserContext* User = NewObject<UUserContext>(this);
		User->Setup(this, InUserID);
		Users.Add(InUserID, User);
		return User;
	}
}

FComputer& USystemContext::GetComputer()
{
	check(this->GetPeacenet());

	auto MyPeacenet = this->GetPeacenet();

	int ComputerIndex = 0;
	FComputer Computer;

	check(MyPeacenet->SaveGame->GetComputerByID(this->ComputerID, Computer, ComputerIndex));

	return MyPeacenet->SaveGame->Computers[ComputerIndex];
}

APeacenetWorldStateActor* USystemContext::GetPeacenet()
{
	return this->Peacenet;
}

URainbowTable* USystemContext::GetRainbowTable()
{
	return this->RainbowTable;
}

void USystemContext::SetupDesktop(int InUserID)
{
	check(!this->GetDesktop());

	this->AppendLog("Beginning desktop session - uid: " + FString::FromInt(InUserID));

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetPeacenet()->GetWorld(), 0);

	this->Desktop = CreateWidget<UDesktopWidget, APlayerController>(PlayerController, this->GetPeacenet()->DesktopClass);

	check(GetDesktop());

	this->Desktop->SystemContext = this;
	this->Desktop->UserID = InUserID;
}

void USystemContext::GetFolderTree(TArray<FFolder>& OutFolderTree)
{
	OutFolderTree = GetComputer().Filesystem;
}

void USystemContext::PushFolderTree(const TArray<FFolder>& InFolderTree)
{
	GetComputer().Filesystem = InFolderTree;
}

FText USystemContext::GetTimeOfDay()
{
	return GetPeacenet()->GetTimeOfDay();
}

void USystemContext::ExecuteCommand(FString InCommand)
{
	check(this->GetDesktop());

	this->GetDesktop()->ExecuteCommand(InCommand);
}

void USystemContext::HandleFileSystemEvent(EFilesystemEventType InType, FString InPath)
{
	switch (InType)
	{
	case EFilesystemEventType::WriteFile:
		if (InPath == "/etc/hostname")
		{
			auto fs = GetFilesystem(0);
			EFilesystemStatusCode err;
			fs->ReadText("/etc/hostname", this->CurrentHostname, err);
			CurrentHostname = ReadFirstLine(CurrentHostname);

			this->AppendLog("Hostname changed to " + CurrentHostname);
		}
		break;
	}

	// If the path is within /var we might want to check to make sure the log still exists.
	if (InPath.StartsWith("/var"))
	{
		auto RootFS = GetFilesystem(0);

		EFilesystemStatusCode Anus;

		// Does /var/log not exist?
		if (!RootFS->DirectoryExists("/var/log"))
		{
			if (!RootFS->DirectoryExists("/var"))
			{
				RootFS->CreateDirectory("/var", Anus);
			}
			RootFS->CreateDirectory("/var/log", Anus);
		}

		// Does peacegate.log not exist?
		if (!RootFS->FileExists("/var/log/system.log"))
		{
			// write blank log.
			RootFS->WriteText("/var/log/system.log", "");
		}

	}
}

TArray<UWallpaperAsset*> USystemContext::GetAvailableWallpapers()
{
	TArray<UWallpaperAsset*> Ret;
	for (auto Wallpaper : this->GetPeacenet()->Wallpapers)
	{
		if(Wallpaper->IsDefault || Wallpaper->UnlockedByDefault || this->GetComputer().UnlockedWallpapers.Contains(Wallpaper->InternalID))
		{
			Ret.Add(Wallpaper);
		}
	}
	return Ret;
}

void USystemContext::SetCurrentWallpaper(UWallpaperAsset* InWallpaperAsset)
{
	// Make sure it's not null.
	check(InWallpaperAsset);

	// If it's unlocked by default or already unlocked, we just set it.
	if(InWallpaperAsset->UnlockedByDefault || InWallpaperAsset->IsDefault || this->GetComputer().UnlockedWallpapers.Contains(InWallpaperAsset->InternalID))
	{
		// Set the wallpaper.
		this->GetComputer().CurrentWallpaper = InWallpaperAsset->WallpaperTexture;
	}
	else
	{
		// BETA TODO: Announce item unlock.
		this->GetComputer().UnlockedWallpapers.Add(InWallpaperAsset->InternalID);

		// Set the wallpaper.
		this->GetComputer().CurrentWallpaper = InWallpaperAsset->WallpaperTexture;
	}
}

void USystemContext::UpdateSystemFiles()
{
	// This function updates the system based on save data and in-game assets.
	//
	// A.K.A: This is the function that updates things like what wallpapers are installed.

	// So first we need a root fs context.
	UPeacegateFileSystem* RootFS = this->GetFilesystem(0);

	EFilesystemStatusCode Anus;

	// Does /var/log not exist?
	if (!RootFS->DirectoryExists("/var/log"))
	{
		if (!RootFS->DirectoryExists("/var"))
		{
			RootFS->CreateDirectory("/var", Anus);
		}
		RootFS->CreateDirectory("/var/log", Anus);
	}

	// Does peacegate.log not exist?
	if (!RootFS->FileExists("/var/log/peacegate.log"))
	{
		// write blank log.
		RootFS->WriteText("/var/log/peacegate.log", "");
	}

	// This is also where we init our rainbow table.
	this->RainbowTable = NewObject<URainbowTable>(this);
	this->RainbowTable->Setup(this, "/etc/rainbow_table.db", true);
}

void USystemContext::Setup(int InComputerID, int InCharacterID, APeacenetWorldStateActor* InPeacenet)
{
	check(InPeacenet);

	// assign all our IDs and Peacenet.
	this->ComputerID = InComputerID;
	this->CharacterID = InCharacterID;
	this->Peacenet = InPeacenet;

	// Do we not have a preferred alias?
	if(!this->GetCharacter().PreferredAlias.Len())
	{
		this->GetCharacter().PreferredAlias = this->GetCharacter().CharacterName;
	}

	// Do we not have an email address?
	if(!this->GetCharacter().EmailAddress.Len())
	{
		this->GetCharacter().EmailAddress = this->GetCharacter().PreferredAlias.Replace(TEXT(" "), TEXT("_")) + "@" + this->GetPeacenet()->GetProcgen()->ChooseEmailDomain();
	}

	// Now we need a filesystem.
	UPeacegateFileSystem* fs = this->GetFilesystem(0);

	// Any FS errors are reported here.
	EFilesystemStatusCode fsStatus = EFilesystemStatusCode::OK;

	// Create the logfile directories.
	if(!fs->DirectoryExists("/var"))
		fs->CreateDirectory("/var", fsStatus);

	if(!fs->DirectoryExists("/var/log"))
		fs->CreateDirectory("/var/log", fsStatus);

	this->AppendLog("System online.");
	this->AppendLog("Welcome to Peacegate OS.");
	this->AppendLog("IDENTITY: " + this->GetCharacter().CharacterName);
	this->AppendLog("IP ADDRESS: " + this->GetIPAddress());

	// Create /home if it doesn't exist.
	if(!fs->DirectoryExists("/home"))
		fs->CreateDirectory("/home", fsStatus);

	// Create the bin directory - which actually means something now.
	if(!fs->DirectoryExists("/bin"))
		fs->CreateDirectory("/bin", fsStatus);


	// Go through every user on the system.
	for(auto& user : this->GetComputer().Users)
	{
		// should we generate loots after this?
		bool generateLoots = false;

		// Get the home directory for the user.
		FString home = this->GetUserHomeDirectory(user.ID);

		// If the user's home directory doesn't exist, create it.
		if(!fs->DirectoryExists(home))
		{
			fs->CreateDirectory(home, fsStatus);
			generateLoots = true;

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

		for(auto subDir : homeDirs)
		{
			if(!fs->DirectoryExists(home + "/" + subDir))
			{
				generateLoots = true;
				fs->CreateDirectory(home + "/" + subDir, fsStatus);
			}
		}

		if(generateLoots)
		{
			this->GetPeacenet()->GetProcgen()->PlaceLootableFiles(this->GetUserContext(user.ID));
		}
	}

	// Now we go through all the exploits in the game and, if they're
	// unlocked by default, we unlock them.
	for(auto exploit : this->GetPeacenet()->GetExploits())
	{
		if(exploit->UnlockedByDefault)
		{
			if(!this->GetComputer().Exploits.Contains(exploit))
				this->GetComputer().Exploits.Add(exploit);
		}
	}

	// Now we'll get all the installed terminal commands to show in /bin.
	TArray<UCommandInfo*> InstalledCommands = this->GetInstalledCommands();
	TArray<FName> CommandKeys;
	this->GetPeacenet()->CommandInfo.GetKeys(CommandKeys);
	
	for(int i = 0; i < CommandKeys.Num(); i++)
	{
		UCommandInfo* CommandInfo = this->GetPeacenet()->CommandInfo[CommandKeys[i]];
		if(CommandInfo->UnlockedByDefault || InstalledCommands.Contains(CommandInfo))
		{
			fs->SetFileRecord("/bin/" + CommandInfo->ID.ToString(), EFileRecordType::Command, i);
		}
	}


	// Make all programs show in /bin.
	TArray<UPeacegateProgramAsset*> InstalledPrograms = this->GetInstalledPrograms();

	for(int i = 0; i < this->GetPeacenet()->Programs.Num(); i++)
	{
		UPeacegateProgramAsset* ProgramAsset = this->GetPeacenet()->Programs[i];
		if(ProgramAsset->IsUnlockedByDefault || InstalledPrograms.Contains(ProgramAsset))
		{
			fs->SetFileRecord("/bin/" + ProgramAsset->ID.ToString(), EFileRecordType::Program, i);
		}
	}

	// If we are a player, auto-scan for adjacent nodes so the player desktop gets populated.
	if(this->GetCharacter().CharacterType == EIdentityType::Player)
	{
		// Scan for adjacent identities so the player doesn't have to.
		this->ScanForAdjacentNodes();
	}
}

FString USystemContext::GetEmailAddress()
{
	return this->GetCharacter().EmailAddress;
}

void USystemContext::AppendLog(FString InLogText)
{
	FString ExistingLog = "";
	UPeacegateFileSystem* FS = this->GetFilesystem(0);
	EFilesystemStatusCode EatMyFuckingBurger;
	if(FS->FileExists("/var/log/system.log"))
		FS->ReadText("/var/log/system.log", ExistingLog, EatMyFuckingBurger);

	ExistingLog = ExistingLog.TrimStartAndEnd();

	ExistingLog += "\r\n[" + this->GetTimeOfDay().ToString() + "] " + InLogText;

	ExistingLog = ExistingLog.TrimStartAndEnd();

	FS->WriteText("/var/log/system.log", ExistingLog);
}

TArray<FPeacegateProcess> USystemContext::GetRunningProcesses()
{
	return this->Processes;
}

int USystemContext::StartProcess(FString Name, FString FilePath, int UserID)
{
	int NewPID = 0;
	for(auto Process : this->GetRunningProcesses())
	{
		if(NewPID <= Process.PID)
			NewPID = Process.PID + 1;
	}

	FPeacegateProcess NewProcess;
	NewProcess.PID = NewPID;
	NewProcess.UID = UserID;
	NewProcess.ProcessName = Name;
	NewProcess.FilePath = FilePath;
	this->Processes.Add(NewProcess);

	this->AppendLog("Process started - pid " + FString::FromInt(NewPID) + " - uid " + FString::FromInt(UserID) + " - name " + Name);

	return NewProcess.PID;
}

void USystemContext::FinishProcess(int ProcessID)
{
	for(int i = 0; i < Processes.Num(); i++)
	{
		FPeacegateProcess p = Processes[i];
		if(p.PID == ProcessID)
		{
			this->Processes.RemoveAt(i);
			this->AppendLog("Process " + FString::FromInt(ProcessID) + " killed.");
			return;
		}
	}
}

bool USystemContext::IsEnvironmentVariableSet(FString InVariable)
{
	// Simply returns whether the computer has an environment variable set.
	return this->GetComputer().EnvironmentVariables.Contains(InVariable);
}

bool USystemContext::GetEnvironmentVariable(FString InVariable, FString& OutValue)
{
	// Check if we have the variable:
	if(this->IsEnvironmentVariableSet(InVariable))
	{
		// Retrieve the value.
		OutValue = this->GetComputer().EnvironmentVariables[InVariable];
		return true;
	}
	return false;
}

void USystemContext::SetEnvironmentVariable(FString InVariable, FString InValue)
{
	// If we have a variable with the same name, we set it. Else, we add it.
	if(this->IsEnvironmentVariableSet(InVariable))
	{
		this->GetComputer().EnvironmentVariables[InVariable] = InValue;
	}
	else
	{
		this->GetComputer().EnvironmentVariables.Add(InVariable, InValue);
	}
}

void USystemContext::UnsetEnvironmentVariable(FString InVariable)
{
	// If we have the variable set, remove it.
	if(this->IsEnvironmentVariableSet(InVariable))
	{
		this->GetComputer().EnvironmentVariables.Remove(InVariable);
	}
}
