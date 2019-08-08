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
#include "PeacegateFileSystem.h"
#include "SlateFontInfo.h"
#include "UserColor.h"
#include "FileOpenResult.h"
#include "Camera/CameraComponent.h"
#include "ConsoleColor.h"
#include "UMG/Public/Components/CanvasPanel.h"
#include "UMG/Public/Components/CanvasPanelSlot.h"
#include "Blueprint/UserWidget.h"
#include "CommonUtils.generated.h"

class UPeacenetSaveGame;
class USystemContext;
class UUserContext;
class APlayerController;
class UTerminalEmulator;
class UWindow;

/**
 * Common utilities used throughout the entire game.
 */
UCLASS(Blueprintable)
class PROJECTOGLOWIA_API UCommonUtils : public UObject
{
	GENERATED_BODY()

public:
	// Given the geometry data for two UMG widgets, this function returns a value indicating whether the widgets overlap.
	// This function is used in the Peacegate Tutorial during the Window Management tutorial to see if the Terminal is
	// within or overlapping with the highlighted area on the desktop.
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool WidgetsOverlap(const FGeometry& InFirstWidgetGeometry, const FGeometry& InSecondWidgetGeometry);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool GetWidgetIntersection(const FGeometry& InFirstWidgetGeometry, const FGeometry& InSecondWidgetGeometry, FVector2D& OutIntersectionSize);

	// Converts the specified string into a value suitable for use as a Peacegate username, preferred alias or
	// email username.
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FString Aliasify(FString InString);

	UFUNCTION(BlueprintCallable, Category = "Common Utils")
	static void ReorderCanvasPanel(UCanvasPanel* InCanvasPanel, UWindow* InFocusWindow);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Upgrades")
	static bool UpgradeDependsOn(UUserContext* UserContext, USystemUpgrade* Target, USystemUpgrade* Dependency);

	UFUNCTION(BlueprintCallable, Category = "Bootup")
	static float PrintKernelMessages(UTerminalEmulator* InConsole);
	
	UFUNCTION(BlueprintCallable, Category = "Peacegate|Setup")
	static void ParseCharacterName(const FString InCharacterName, FString& OutUsername, FString& OutHostname);

private:
	template<typename T>
	static int Partition(TArray<T>& InArray, int Start, int End, TFunction<bool(const T& InA, const T& InB)> InComparer);

	template<typename T>
	static void QuickSortInternal(TArray<T>& InArray, int Start, int End, TFunction<bool(const T& InA, const T& InB)> InComparer);
public:
	
	template<typename T>
	static TArray<T> QuickSort(TArray<T> InArray, TFunction<bool(const T& InA, const T& InB)> InComparer);

	template<typename T>
	static bool BinarySearch(TArray<T> InArray, TFunction<int(const T& InA)> InComparer, T& OutElement);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Common Utils")
	static FText GetConnectionError(EConnectionError InError);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Common Utilities for The Peacenet")
	static FLinearColor GetForegroundColor(FLinearColor InColor);

	UFUNCTION(BlueprintCallable, Category = "Peacegate", BlueprintPure)
	static FText GetFriendlyFilesystemStatusCode(const EFilesystemStatusCode InStatusCode);

	UFUNCTION(BlueprintCallable, Category = "Peacegate")
	static UPeacegateFileSystem* CreateFilesystem(USystemContext* InSystemContext, int InUserID);

	UFUNCTION(BlueprintCallable, Category = "Terminal")
	static FLinearColor GetUserColor(EUserColor InColor);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trigonometry Bullshit")
	static float GetRotation(FVector2D InA, FVector2D InB);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "URL parsing")
	static void ParseURL(FString InURL, FString& OutUsername, FString& OutHost, int& OutPort, FString& OutPath, bool& HasPath, bool& HasUser, bool& HasPort);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Common")
	static void GetFriendlyFileOpenText(EFileOpenResult InResult, FString& OutTitle, FString& OutDescription);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Console")
	static FLinearColor GetConsoleColor(EConsoleColor InConsoleColor);

	static void MeasureChar(const TCHAR InChar, const FSlateFontInfo& InSlateFont, float& OutWidth, float& OutHeight);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Clipboard")
	static bool GetClipboardText(FString& OutText);

	UFUNCTION(BlueprintCallable, Category = "Clipboard")
	static void PutClipboardText(FString InText);

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Common Utils")
	static bool GetPlayerUserContext(APlayerController* InPlayerController, UUserContext*& OutUserContext);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Parsing")
	static void ParseHost(FString InHost, FString& OutAddress, bool& HasPort, int& OutPort);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	static void SetEnableBloom(UCameraComponent* InCamera, bool InEnableBloom);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cool Shit")
	static FText GetRichTextSegment(const FText& InSourceText, int InEndIndex, bool& FoundIncompleteTag, int& TrueEndIndex);
};

template<typename T>
inline int UCommonUtils::Partition(TArray<T>& InArray, int Start, int End, TFunction<bool(const T&InA, const T&InB)> InComparer)
{
	T& pivot = InArray[End];
	int i = Start;
	for (int j = Start; j < End - 1; j++)
	{
		if (InComparer(InArray[j], pivot))
		{
			if (i != j)
			{
				T& temp = InArray[i];
				InArray[i] = InArray[j];
				InArray[j] = temp;
			}
			i++;
		}
	}
	T& iTemp = InArray[i];
	InArray[i] = InArray[End];
	InArray[End] = iTemp;
	return i;
}

template<typename T>
inline void UCommonUtils::QuickSortInternal(TArray<T>& InArray, int Start, int End, TFunction<bool(const T&InA, const T&InB)> InComparer)
{
	if (Start < End)
	{
		int p = Partition(InArray, Start, End, InComparer);
		QuickSortInternal(InArray, Start, p - 1, InComparer);
		QuickSortInternal(InArray, p + 1, End, InComparer);
	}
}

template<typename T>
inline TArray<T> UCommonUtils::QuickSort(TArray<T> InArray, TFunction<bool(const T&InA, const T&InB)> InComparer)
{
	TArray<T> Sort = InArray;
	QuickSortInternal(Sort, 0, Sort.Num() - 1, InComparer);
	return Sort;
}

template<typename T>
inline bool UCommonUtils::BinarySearch(TArray<T> InArray, TFunction<int(const T&InA)> InComparer, T & OutElement)
{
	if (!InArray.Num())
		return false;

	// NOTE: THIS ALGORITHM ONLY WORKS IF THE ARRAY IS SORTED. Don't be a fucking drunky. Use a QuickSort.

	int min = 0;
	int max = InArray.Num();

	while (max - min != 0)
	{
		int average = (min + max) / 2;

		int guessResult = InComparer(InArray[average]);

		if (guessResult == 0)
		{
			OutElement = InArray[average];
			return true;
		}
		else if (guessResult > 0)
		{
			min = average + 1;
		}
		else
		{
			max = average - 1;
		}
	}

	return false;
}
