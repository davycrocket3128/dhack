// Copyright (c) 2018 The Peacenet & Alkaline Thunder.

#pragma once

#include "CoreMinimal.h"
#include "UComputerService.h"
#include "UHackableAsset.h"
#include "FComputer.h"
#include "FNetMapScanEventArgs.generated.h"

UENUM(BlueprintType)
enum class ENetMapScanEventType : uint8
{
	ServiceFound,
	HostResolveFailure,
	FirewallDetected,
	ScanStarted
};

USTRUCT(BlueprintType)
struct PROJECTOGLOWIA_API FNetMapServiceInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "NetMap")
	UComputerService* ServiceAsset;

	UPROPERTY(BlueprintReadOnly, Category = "NetMap")
	FServiceInfo ServiceInfo;
};

/**
 * Represents an event fired by an nmap scan.
 */
USTRUCT(BlueprintType)
struct PROJECTOGLOWIA_API FNetMapScanEventArgs
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "NetMap")
	ENetMapScanEventType EventType;

	UPROPERTY(BlueprintReadOnly, Category = "NetMap")
	FNetMapServiceInfo Hackable;

	UPROPERTY(BlueprintReadOnly, Category = "NetMap")
	int FirewallStrength = -1;
};
