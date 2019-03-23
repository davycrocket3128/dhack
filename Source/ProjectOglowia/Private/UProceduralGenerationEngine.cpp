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


#include "UProceduralGenerationEngine.h"
#include "UPeacenetSaveGame.h"
#include "Base64.h"
#include "FFirewallRule.h"
#include "FEntityPosition.h"
#include "UComputerService.h"
#include "UUserContext.h"
#include "AdjacentLinkType.h"
#include "LootableFile.h"
#include "ProtocolVersion.h"
#include "CommonUtils.h"
#include "WallpaperAsset.h"
#include "UPeacegateFileSystem.h"
#include "UMarkovChain.h"
#include "PeacenetWorldStateActor.h"

FRandomStream& UProceduralGenerationEngine::GetRNG()
{
    return this->RNG;
}

void UProceduralGenerationEngine::PlaceLootableFiles(UUserContext* InUserContext)
{
    // If the system is a player, then we stop right now.
    if(InUserContext->GetOwningSystem()->GetCharacter().CharacterType == EIdentityType::Player) return;

    // The amount of lootables we are EVER allowed to generate in an NPC.
    int MaxLootables = this->LootableFiles.Num() / 2;

    // If that ends up being 0, then we stop right there.
    if(!MaxLootables) return;

    // The amount of lootables this NPC gets.
    int LootableCount = RNG.RandRange(0, MaxLootables);

    // Get a filesystem context for this user.
    UPeacegateFileSystem* Filesystem = InUserContext->GetFilesystem();

    // Keep going while there are lootables to generate.
    while(LootableCount)
    {
        // Pick a random lootable! Any lootable!
        ULootableFile* Lootable = this->LootableFiles[RNG.RandRange(0, this->LootableFiles.Num() - 1)];

        // This is where the file will go.
        FString BaseDirectory = "/";

        // Desktop or documents?
        int Dice = RNG.RandRange(0, 6);

        // Where should it go?
        switch(Lootable->SpawnLocation)
        {
            case EFileSpawnLocation::Anywhere:
                // TODO: Support picking of random folders.
                break;
            case EFileSpawnLocation::HomeFolder:
                BaseDirectory = InUserContext->GetHomeDirectory();
                break;
            case EFileSpawnLocation::Binaries:
                BaseDirectory = "/bin";
                break;
            case EFileSpawnLocation::DesktopOrDocuments:
                if(Dice % 2 == 0)
                    BaseDirectory = InUserContext->GetHomeDirectory() + "/Desktop";
                else
                    BaseDirectory = InUserContext->GetHomeDirectory() + "/Documents";
                break;
            case EFileSpawnLocation::Pictures:
                BaseDirectory = InUserContext->GetHomeDirectory() + "/Pictures";
                break;
            case EFileSpawnLocation::Downloads:
                BaseDirectory = InUserContext->GetHomeDirectory() + "/Downloads";
                break;
            case EFileSpawnLocation::Music:
                BaseDirectory = InUserContext->GetHomeDirectory() + "/Music";
                break;
            case EFileSpawnLocation::Videos:
                BaseDirectory = InUserContext->GetHomeDirectory() + "/Videos";
                break;
        }

        // Now that we have a base directory, get the file path.
        FString FilePath = BaseDirectory + "/" + Lootable->FileName.ToString();

        // If the file already exists, continue.
        if(Filesystem->FileExists(FilePath)) continue;

        // Spawn the file.
        Lootable->FileContents->SpawnFile(FilePath, Filesystem);

        // Decrease the lootable count.
        LootableCount--;
    }
}

void UProceduralGenerationEngine::GenerateFirewallRules(FComputer& InComputer)
{
    // Don't do this if the computer already has firewall rules!
    if(InComputer.FirewallRules.Num())
        return;

    TArray<UComputerService*> Services = this->Peacenet->GetServicesFor(InComputer.ComputerType);

    // This gets the skill level of this computer's owning entity if any.
    int Skill = this->Peacenet->SaveGame->GetSkillOf(InComputer);

    for(int i = 0; i < Services.Num(); i++)
    {
        if(Services[i]->IsDefault || RNG.RandRange(0, 6) % 2)
        {
            FFirewallRule Rule;
            Rule.Port = Services[i]->Port;
            Rule.Service = this->GetProtocol(Services[i], Skill);
            Rule.IsFiltered = false;
            InComputer.FirewallRules.Add(Rule);
        }
    }
}

UProtocolVersion* UProceduralGenerationEngine::GetProtocol(UComputerService* InService, int InSkill)
{
    int i = 0;
    int count = 100;
    UProtocolVersion* protocol = nullptr;

    while(count > 0)
    {
        UProtocolVersion* protocolVersion = this->ProtocolVersions[i];
        if(protocolVersion->Protocol != InService)
        {
            i++;
            if(i >= this->ProtocolVersions.Num())
                i=0;
            continue;
        }
        if(protocolVersion->MinimumSkillLevel > InSkill)        
        {
            i++;
            if(i >= this->ProtocolVersions.Num())
                i=0;
            continue;
        }

        protocol = protocolVersion;

        i++;
        if(i >= this->ProtocolVersions.Num())
            i=0;
        
        count -= RNG.RandRange(0, 10);
    }

    return protocol;
}

TArray<FString> UProceduralGenerationEngine::GetMarkovData(EMarkovTrainingDataUsage InUsage)
{
    TArray<FString> Ret;
    for(auto Markov : this->Peacenet->MarkovData)
    {
        if(Markov->Usage == InUsage)
        {
            Ret.Append(Markov->TrainingData);
        }
    }
    return Ret;
}

FString UProceduralGenerationEngine::GenerateIPAddress()
{
    uint8 Byte1, Byte2, Byte3, Byte4 = 0;

    Byte1 = (uint8)RNG.RandRange(0, 255);

    // The other three are easy.
    Byte2 = (uint8)RNG.RandRange(0, 255);
    Byte3 = (uint8)RNG.RandRange(0, 255);
    Byte4 = (uint8)RNG.RandRange(0, 255);
    
    // We only support IPv4 in 2025 lol.
    return FString::FromInt(Byte1) + "." + FString::FromInt(Byte2) + "." + FString::FromInt(Byte3) + "." + FString::FromInt(Byte4);
}

void UProceduralGenerationEngine::ClearNonPlayerEntities()
{
    TArray<int> ComputersToRemove;
    TArray<int> CharactersToRemove;
    
    // Collect all computers that are NPC-owned.
    for(int i = 0; i < this->Peacenet->SaveGame->Computers.Num(); i++)
    {
        FComputer& Computer = this->Peacenet->SaveGame->Computers[i];
        if(Computer.OwnerType == EComputerOwnerType::NPC)
        {
            ComputersToRemove.Add(i);
        }
    }

    // Collect all characters to remove.
    for(int i = 0; i < this->Peacenet->SaveGame->Characters.Num(); i++)
    {
        FPeacenetIdentity& Character = this->Peacenet->SaveGame->Characters[i];
        if(Character.CharacterType == EIdentityType::NonPlayer)
        {
            CharactersToRemove.Add(i);
        }
    }

    // Remove all characters..
    while(CharactersToRemove.Num())
    {
        this->Peacenet->SaveGame->Characters.RemoveAt(CharactersToRemove[0]);
        CharactersToRemove.RemoveAt(0);
        for(int i = 0; i < CharactersToRemove.Num(); i++)
            CharactersToRemove[i]--;
    }

    // Remove all computers.
    while(ComputersToRemove.Num())
    {
        this->Peacenet->SaveGame->Computers.RemoveAt(ComputersToRemove[0]);
        ComputersToRemove.RemoveAt(0);
        for(int i = 0; i < ComputersToRemove.Num(); i++)
            ComputersToRemove[i]--;
    }

    // Clear entity adjacentness.
    this->Peacenet->SaveGame->AdjacentNodes.Empty();

    // Remove all entity positions that aren't player.
    TArray<int> EntityPositionsToRemove;
    int EntityPositionsRemoved=0;
    for(int i = 0; i < this->Peacenet->SaveGame->EntityPositions.Num(); i++)
    {
        FPeacenetIdentity Identity;
        int IdentityIndex;
        bool result = this->Peacenet->SaveGame->GetCharacterByID(this->Peacenet->SaveGame->EntityPositions[i].EntityID, Identity, IdentityIndex);
        if(result)
        {
           if(Identity.CharacterType != EIdentityType::Player) 
           {
               EntityPositionsToRemove.Add(i);
           }
        }
        else
        {
            EntityPositionsToRemove.Add(i);
        }
    }
    while(EntityPositionsToRemove.Num())
    {
        this->Peacenet->SaveGame->EntityPositions.RemoveAt(EntityPositionsToRemove[0] - EntityPositionsRemoved);
        EntityPositionsRemoved++;
        EntityPositionsToRemove.RemoveAt(0);
    }

    // Clear out the player's known entities.
    this->Peacenet->SaveGame->PlayerDiscoveredNodes.Empty();
    this->Peacenet->SaveGame->PlayerDiscoveredNodes.Add(this->Peacenet->SaveGame->PlayerCharacterID);
    

    // Fix up entity IDs.
    this->Peacenet->SaveGame->FixEntityIDs();
}

void UProceduralGenerationEngine::GenerateIdentityPosition(FPeacenetIdentity& Pivot, FPeacenetIdentity& Identity)
{
    const float MIN_DIST_FROM_PIVOT = 50.f;
    const float MAX_DIST_FROM_PIVOT = 400.f;


    FVector2D Test;
    if(this->Peacenet->SaveGame->GetPosition(Identity.ID, Test))
        return;

    FVector2D PivotPos;
    bool PivotResult = this->Peacenet->SaveGame->GetPosition(Pivot.ID, PivotPos);

    if(!PivotResult)
    {
        for(auto EntityID : this->Peacenet->SaveGame->GetAllEntities())
        {
            FVector2D PivotSquared; // the pivot point of the pivot's new position.
            if(this->Peacenet->SaveGame->GetPosition(EntityID, PivotSquared))
            {
                PivotResult = true;

                PivotPos = FVector2D(0.f, 0.f);
                do
                {
                    PivotPos.X = PivotSquared.X + (RNG.FRandRange(MIN_DIST_FROM_PIVOT, MAX_DIST_FROM_PIVOT) - (MAX_DIST_FROM_PIVOT/2.f));
                    PivotPos.Y = PivotSquared.Y + (RNG.FRandRange(MIN_DIST_FROM_PIVOT, MAX_DIST_FROM_PIVOT) - (MAX_DIST_FROM_PIVOT/2.f));
                } while(this->Peacenet->SaveGame->LocationTooCloseToEntity(PivotPos, MIN_DIST_FROM_PIVOT));

                this->Peacenet->SaveGame->SetEntityPosition(Pivot.ID, PivotPos);

                break;
            }
        }
    }

    check(PivotResult);

    FVector2D NewPos = FVector2D(0.f, 0.f);
    do
    {
        NewPos.X = PivotPos.X + (RNG.FRandRange(MIN_DIST_FROM_PIVOT, MAX_DIST_FROM_PIVOT) - (MAX_DIST_FROM_PIVOT/2.f));
        NewPos.Y = PivotPos.Y + (RNG.FRandRange(MIN_DIST_FROM_PIVOT, MAX_DIST_FROM_PIVOT) - (MAX_DIST_FROM_PIVOT/2.f));
    } while(this->Peacenet->SaveGame->LocationTooCloseToEntity(NewPos, MIN_DIST_FROM_PIVOT));

    this->Peacenet->SaveGame->SetEntityPosition(Identity.ID, NewPos);

}

void UProceduralGenerationEngine::GenerateAdjacentNodes(FPeacenetIdentity& InIdentity)
{   
    // Don't generate any new links if there are any existing links from this NPC.
    // PATCH: Before, this would check for any links to and from the NPC, that's a problem. Now we only check for links from the NPC.
    if(this->Peacenet->SaveGame->GetAdjacents(InIdentity.ID, EAdjacentLinkType::AToB).Num())
        return;

    const int MIN_ADJACENTS = 2;
    const int MAX_ADJACENTS = 8;
    const int MAX_SKILL_DIFFERENCE = 3;


    int Adjacents = RNG.RandRange(MIN_ADJACENTS, MAX_ADJACENTS);

    while(Adjacents)
    {
        FPeacenetIdentity& LinkedIdentity = this->Peacenet->SaveGame->Characters[RNG.RandRange(0, this->Peacenet->SaveGame->Characters.Num()-1)];

        if(LinkedIdentity.ID == InIdentity.ID)
            continue;

        if(this->Peacenet->SaveGame->AreAdjacent(InIdentity.ID, LinkedIdentity.ID))
            continue;

        if(this->Peacenet->GameType->GameRules.DoSkillProgression)
        {
            int Difference = FMath::Abs(InIdentity.Skill - LinkedIdentity.Skill);
            if(Difference > MAX_SKILL_DIFFERENCE)
                continue;
        }

        this->Peacenet->SaveGame->AddAdjacent(InIdentity.ID, LinkedIdentity.ID);

        this->GenerateIdentityPosition(InIdentity, LinkedIdentity);

        Adjacents--;
    }
}

void UProceduralGenerationEngine::GenerateNonPlayerCharacters()
{
    this->ClearNonPlayerEntities();
    UE_LOG(LogTemp, Display, TEXT("Cleared old NPCs if any..."));

    for(int i = 0; i < 1000; i++)
    {
        this->GenerateNonPlayerCharacter();
    }
}

FPeacenetIdentity& UProceduralGenerationEngine::GenerateNonPlayerCharacter()
{
    FPeacenetIdentity Identity;
    Identity.ID = this->Peacenet->SaveGame->Characters.Num();
    Identity.CharacterType = EIdentityType::NonPlayer;

    bool IsMale = RNG.RandRange(0, 6) % 2;

    FString CharacterName;
    do
    {
        if(IsMale)
        {
            CharacterName = MaleNameGenerator->GetMarkovString(0);
        }
        else
        {
            CharacterName = FemaleNameGenerator->GetMarkovString(0);
        }

        CharacterName = CharacterName + " " + LastNameGenerator->GetMarkovString(0);
    } while(this->Peacenet->SaveGame->CharacterNameExists(CharacterName));

    Identity.CharacterName = CharacterName;

    Identity.Skill = RNG.RandRange(1, this->Peacenet->GameType->GameRules.MaximumSkillLevel);

    float Reputation = RNG.GetFraction();
    bool IsBadRep = RNG.RandRange(0, 6) % 2;
    if(IsBadRep)
        Reputation = -Reputation;
    
    Identity.Reputation = Reputation;

    FString Username;
    FString Hostname;

    if(this->RNG.RandRange(0, 6) % 2)
    {
        UCommonUtils::ParseCharacterName(CharacterName, Username, Hostname);
    }
    else
    {
        Username = this->UsernameGenerator->GetMarkovString(0);
        Hostname = Username + "-pc";
    }

    FComputer& IdentityComputer = this->GenerateComputer(Hostname, EComputerType::Personal, EComputerOwnerType::NPC);

    FUser RootUser;
    FUser NonRootUser;
    
    RootUser.Username = "root";
    RootUser.ID = 0;
    RootUser.Domain = EUserDomain::Administrator;

    NonRootUser.ID = 1;
    NonRootUser.Username = Username;
    NonRootUser.Domain = EUserDomain::PowerUser;

    RootUser.Password = this->GeneratePassword(Identity.Skill*5);
    NonRootUser.Password = this->GeneratePassword(Identity.Skill*3);
    
    IdentityComputer.Users.Add(RootUser);
    IdentityComputer.Users.Add(NonRootUser);
    
    Identity.ComputerID = IdentityComputer.ID;

    FString IPAddress;
    do
    {
        IPAddress = this->GenerateIPAddress();
    } while(this->Peacenet->SaveGame->IPAddressAllocated(IPAddress));

    this->Peacenet->SaveGame->ComputerIPMap.Add(IPAddress, IdentityComputer.ID);

    this->Peacenet->SaveGame->Characters.Add(Identity);

    UE_LOG(LogTemp, Display, TEXT("Generated NPC: %s"), *Identity.CharacterName);

    return this->Peacenet->SaveGame->Characters[this->Peacenet->SaveGame->Characters.Num()-1];
}

FString UProceduralGenerationEngine::GeneratePassword(int InLength)
{
    FString Chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890`~!@#$%^&*()_+-=[]{}\\|'\":;?.,<>";

    FString Ret;
    for(int i = 0; i < InLength; i++)
    {
        TCHAR Char = Chars[RNG.RandRange(0, Chars.Len()-1)];
        Ret.AppendChar(Char);
    }

    return Ret;
}

void UProceduralGenerationEngine::Initialize(APeacenetWorldStateActor* InPeacenet)
{
    check(!this->Peacenet);
    check(InPeacenet);

    this->Peacenet = InPeacenet;

    // Initialize the world seed if the game is new.
    if(Peacenet->SaveGame->IsNewGame)
    {
        // Get the player character.
        FPeacenetIdentity Character;
        int CharacterIndex = -1;
        bool result = Peacenet->SaveGame->GetCharacterByID(Peacenet->SaveGame->PlayerCharacterID, Character, CharacterIndex);
        check(result);

        // This creates a hash out of the character name which we can seed the RNG with.
        // Thus making the player's name dictate how the world generates.
        TArray<TCHAR> Chars = Character.CharacterName.GetCharArray();
        int Hash = FCrc::MemCrc32(Chars.GetData(), Chars.Num() * sizeof(TCHAR));

        // Store the seed in the save file in case we need it. WHICH WE FUCKING WILL LET ME TELL YOU.
        Peacenet->SaveGame->WorldSeed = Hash;
    }

    // Recall when we set the world seed in the save file?
    // This is where we need it.
    this->RNG = FRandomStream(this->Peacenet->SaveGame->WorldSeed);

    // Now that we have an RNG, we can begin creating markov chains!
    this->MaleNameGenerator = NewObject<UMarkovChain>(this);
    this->FemaleNameGenerator = NewObject<UMarkovChain>(this);
    this->LastNameGenerator = NewObject<UMarkovChain>(this);
    
    // Set them all up with the data they need.
    this->MaleNameGenerator->Init(this->GetMarkovData(EMarkovTrainingDataUsage::MaleFirstNames), 3, RNG);
    this->FemaleNameGenerator->Init(this->GetMarkovData(EMarkovTrainingDataUsage::FemaleFirstNames), 3, RNG);
    this->LastNameGenerator->Init(this->GetMarkovData(EMarkovTrainingDataUsage::LastNames), 3, RNG);
    
    // get username generator.
    this->UsernameGenerator = NewObject<UMarkovChain>(this);

    // initialize it.
    this->UsernameGenerator->Init(this->GetMarkovData(EMarkovTrainingDataUsage::Usernames), 2, RNG);

    if(this->Peacenet->SaveGame->IsNewGame)
    {
        // PASS 1: GENERATE NPC IDENTITIES.
        this->GenerateNonPlayerCharacters();

        // PASS 2: GENERATE STORY CHARACTERS
        // TODO

        // PASS 3: GENERATE CHARACTER RELATIONSHIPS
        this->GenerateCharacterRelationships();
    }

    // Array of temporary file assets.
    TArray<ULootableFile*> TempFiles;

    // Populate it with ALL lootable files in the game.
    this->Peacenet->LoadAssets<ULootableFile>("LootableFile", TempFiles);

    // Filter out any files that have bad contents.
    for(auto File : TempFiles)
    {
        if(!File->FileContents)
            continue;

        // Add it to the REAL lootables list.
        this->LootableFiles.Add(File);
    }

    // Now we do that but for all the protocol versions in the game.
    TArray<UProtocolVersion*> Protocols;
    this->Peacenet->LoadAssets<UProtocolVersion>("ProtocolVersion", Protocols);

    for(auto Protocol : Protocols)
    {
        if(!Protocol->Protocol) continue;

        this->ProtocolVersions.Add(Protocol);
    }
}

void UProceduralGenerationEngine::GenerateCharacterRelationships()
{
    // We will need to remove all relationships that are between any character and a non-player.
    TArray<int> RelationshipsToRemove;
    for(int i = 0; i < this->Peacenet->SaveGame->CharacterRelationships.Num(); i++)
    {
        FCharacterRelationship& Relationship = this->Peacenet->SaveGame->CharacterRelationships[i];

        FPeacenetIdentity First;
        FPeacenetIdentity Second;
        int FirstIndex, SecondIndex;

        bool FirstResult = this->Peacenet->SaveGame->GetCharacterByID(Relationship.FirstEntityID, First, FirstIndex);
        bool SecondResult = this->Peacenet->SaveGame->GetCharacterByID(Relationship.SecondEntityID, Second, SecondIndex);
        
        check(FirstResult && SecondResult);

        if(First.CharacterType != EIdentityType::Player || Second.CharacterType != EIdentityType::Player)
        {
            RelationshipsToRemove.Add(i);
        }
    }

    int RelationshipsRemoved = 0;
    while(RelationshipsToRemove.Num())
    {
        this->Peacenet->SaveGame->CharacterRelationships.RemoveAt(RelationshipsToRemove[0] - RelationshipsRemoved);
        RelationshipsRemoved++;
        RelationshipsToRemove.RemoveAt(0);
    }

    bool ConsiderReputation = this->Peacenet->GameType->GameRules.ConsiderReputations;

    if(ConsiderReputation)
    {
        TArray<FPeacenetIdentity> GoodReps;
        TArray<FPeacenetIdentity> BadReps;
        
        // First pass collects all NPCs and sorts them between good and bad reputations.
        for(int i = 0; i < this->Peacenet->SaveGame->Characters.Num(); i++)
        {
            FPeacenetIdentity Identity = this->Peacenet->SaveGame->Characters[i];

            if(Identity.CharacterType == EIdentityType::Player)
                continue;

            if(Identity.Reputation < 0)
                BadReps.Add(Identity);
            else
                GoodReps.Add(Identity);
        }

        // Second pass goes through every NPC, looks at their reputation, and chooses relationships from the correct list.
        for(int i = 0; i < this->Peacenet->SaveGame->Characters.Num(); i++)
        {
            FPeacenetIdentity First = this->Peacenet->SaveGame->Characters[i];

            if(First.CharacterType == EIdentityType::Player)
                continue;

            bool Bad = First.Reputation < 0;

            bool MakeEnemy = RNG.RandRange(0, 6) % 2;

            FPeacenetIdentity Second;

            do
            {
                if(MakeEnemy)
                {
                    if(Bad)
                        Second = GoodReps[RNG.RandRange(0, GoodReps.Num() - 1)];
                    else
                        Second = BadReps[RNG.RandRange(0, BadReps.Num() - 1)];
                }
                else
                {
                    if(Bad)
                        Second = BadReps[RNG.RandRange(0, BadReps.Num() - 1)];
                    else
                        Second = GoodReps[RNG.RandRange(0, GoodReps.Num() - 1)];
                }
            } while(this->Peacenet->SaveGame->RelatesWith(First.ID, Second.ID) || Second.CharacterType == EIdentityType::Player);
        
            FCharacterRelationship Relationship;
            Relationship.FirstEntityID = First.ID;
            Relationship.SecondEntityID = Second.ID;
            
            if(MakeEnemy)
            {
                Relationship.RelationshipType = ERelationshipType::Enemy;
            }
            else
            {
                Relationship.RelationshipType = ERelationshipType::Friend;
            }

            this->Peacenet->SaveGame->CharacterRelationships.Add(Relationship);
        }
    }
    else
    {
        for(int i = 0; i < this->Peacenet->SaveGame->Characters.Num(); i++)
        {
            FPeacenetIdentity& FirstChar = this->Peacenet->SaveGame->Characters[i];
            if(FirstChar.CharacterType == EIdentityType::Player)
                continue;
            FPeacenetIdentity Second;
            do
            {
                Second = this->Peacenet->SaveGame->Characters[RNG.RandRange(0, this->Peacenet->SaveGame->Characters.Num()-1)];
            } while(this->Peacenet->SaveGame->RelatesWith(FirstChar.ID, Second.ID) || Second.CharacterType == EIdentityType::Player);

            FCharacterRelationship Relationship;
            Relationship.FirstEntityID = FirstChar.ID;
            Relationship.SecondEntityID = Second.ID;

            bool Enemy = RNG.RandRange(0, 6) % 2;

            if(Enemy)
                Relationship.RelationshipType = ERelationshipType::Enemy;
            else
                Relationship.RelationshipType = ERelationshipType::Friend;

            this->Peacenet->SaveGame->CharacterRelationships.Add(Relationship);
        }
    }
}

FComputer& UProceduralGenerationEngine::GenerateComputer(FString InHostname, EComputerType InComputerType, EComputerOwnerType InOwnerType)
{
    FComputer Ret;

    // Set up the core metadata.
    Ret.ID = this->Peacenet->SaveGame->Computers.Num();
    Ret.OwnerType = InOwnerType;
    Ret.ComputerType = InComputerType;

    // Get a random wallpaper.
    UWallpaperAsset* Wallpaper = this->Peacenet->Wallpapers[RNG.RandRange(0, this->Peacenet->Wallpapers.Num()-1)];
    Ret.UnlockedWallpapers.Add(Wallpaper->InternalID);
    Ret.CurrentWallpaper = Wallpaper->WallpaperTexture;

    // Create the barebones filesystem.
    FFolder Root;
    Root.FolderID = 0;
    Root.FolderName = "";
    Root.ParentID = -1;

    FFolder RootHome;
    RootHome.FolderID = 1;
    RootHome.FolderName = "root";
    RootHome.ParentID = 0;

    FFolder UserHome;
    UserHome.FolderID = 2;
    UserHome.FolderName = "home";
    UserHome.ParentID = 0;

    FFolder Etc;
    Etc.FolderID = 3;
    Etc.FolderName = "etc";
    Etc.ParentID = 0;

    // Write the hostname to a file.
    FFileRecord HostnameFile;
    HostnameFile.ID = 0;
    HostnameFile.Name = "hostname";
    HostnameFile.RecordType = EFileRecordType::Text;
    HostnameFile.ContentID = 0;

    FTextFile HostnameText;
    HostnameText.ID = 0;
    HostnameText.Content = InHostname;
    Ret.TextFiles.Add(HostnameText);
    Ret.FileRecords.Add(HostnameFile);

    // Write the file in /etc.
    Etc.FileRecords.Add(HostnameFile.ID);

    // Link up the three folders to the root.
    Root.SubFolders.Add(RootHome.FolderID);
    Root.SubFolders.Add(Etc.FolderID);
    Root.SubFolders.Add(UserHome.FolderID);
    
    // Add all the folders to the computer's disk.
    Ret.Filesystem.Add(Root);
    Ret.Filesystem.Add(Etc);
    Ret.Filesystem.Add(RootHome);
    Ret.Filesystem.Add(UserHome);
    
    // Add the computer to the save file.
    this->Peacenet->SaveGame->Computers.Add(Ret);

    // Grab the index of that computer in the save.
    int ComputerIndex = this->Peacenet->SaveGame->Computers.Num() - 1;

    UE_LOG(LogTemp, Display, TEXT("Computer generated..."));

    // Return it.
    return this->Peacenet->SaveGame->Computers[ComputerIndex];
}