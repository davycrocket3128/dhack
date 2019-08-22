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
#include "Engine/GameInstance.h"
#include "PeacenetSettings.h"
#include "PeacegateDaemon.h"
#include "Profile.h"
#include "DaemonType.h"
#include "Computer.h"
#include "PeacenetGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSettingsAppliedEvent, UPeacenetSettings*, InSettings);

class USystemContext;
class UPeacenetGameTypeAsset;

USTRUCT()
struct FDaemonInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TSubclassOf<UPeacegateDaemon> DaemonClass;

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FText FriendlyName;

	UPROPERTY()
	FText Description;

	UPROPERTY()
	EDaemonType DaemonType;
};

/**
 * A game instance specifically for Peacenet.
 */
UCLASS(BlueprintType)
class PROJECTOGLOWIA_API UPeacenetGameInstance : public UGameInstance
{
	friend APeacenetWorldStateActor;

	GENERATED_BODY()

private:
	UPROPERTY()
	UProfile* Profile;

	UPROPERTY()
	TMap<FName, FDaemonInfo> RegisteredDaemons;

protected:
	UFUNCTION()
	void WipeInvalidProfiles();

	UFUNCTION()
	void SaveProfile(int SlotID, UPeacenetSaveGame* InSaveGame);

	UFUNCTION()
	bool ProfileExists(int SlotID);

	UFUNCTION()
	FString GetSlotName(int SlotID);

	UFUNCTION()
	void RegisterPeacegateDaemon(TSubclassOf<UPeacegateDaemon> InDaemonClass, FName Name, FText FriendlyName, FText Description, EDaemonType DaemonType = EDaemonType::AllSystems);

	UFUNCTION()
	void RegisterDaemons();

	UFUNCTION()
	int GetNextSaveSlotId();

	UFUNCTION()
	void GeneratePeacegateData(FProfileData& InProfile);

	UFUNCTION()
	void GenerateImportantPlayerFiles(FProfileData& InProfileData, FComputer& InComputer);

	UFUNCTION()
	void WriteHomeFolders(FComputer& InComputer, FFolder& InParent);

public:
	UPROPERTY()
	TArray<UPeacenetGameTypeAsset*> GameTypes;

	// The settings for the current instance of The Peacenet.
	UPROPERTY()
	UPeacenetSettings* Settings;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FSettingsAppliedEvent SettingsApplied;

	UFUNCTION(BlueprintCallable)
	bool ConvertOldSave(FString InUsername, FString& OutUsername, FString& OutPassword, FText& OutError);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	UPeacenetSettings* GetSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadSettings();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Peacenet")
	TArray<UPeacenetGameTypeAsset*> const& GetGameTypes() const;

	UFUNCTION(BlueprintCallable, Category = "Peacenet")
	void CreateWorld(FString InCharacterName, UPeacenetGameTypeAsset* InGameType);

	UFUNCTION()
	TArray<FDaemonInfo> GetDaemonsForSystem(USystemContext* InSystem);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool HasOldSaveFile();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetMostRecentCredentials(FString& Username, FString& Password);

	UFUNCTION(BlueprintCallable)
	bool LoadGame(APlayerController* InPlayerController, FString Username, FString Password, APeacenetWorldStateActor*& WorldState);

	UFUNCTION(BlueprintCallable)
	bool CreateNewUser(FString InUsername, FString InPassword);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FProfileData> GetProfiles();

public: // UGameInstance overrides.
	virtual void Init() override;
	virtual void Shutdown() override;
};
