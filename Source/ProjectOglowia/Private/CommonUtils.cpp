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

#include "CommonUtils.h"
#include "Engine/Font.h"
#include "PeacenetSaveGame.h"
#include "TerminalEmulator.h"
#include "Parse.h"
#include "Window.h"
#include "PlatformApplicationMisc.h"
#include "SystemContext.h"
#include "Regex.h"
#include "GameFramework/PlayerController.h"
#include "UserContext.h"
#include "GameFramework/Actor.h"
#include "PeacenetWorldStateActor.h"
#include "Engine/Public/Engine.h"

bool UCommonUtils::WidgetsOverlap(const FGeometry& InFirstWidgetGeometry, const FGeometry& InSecondWidgetGeometry) {
	// Essentially we need to treat the geometries as rectangles and use a simple collision detection algorithm I learned
	// from the lovely, lovely St. Lawrence College dual credit program.
	FVector2D FirstTopCorner = InFirstWidgetGeometry.AbsolutePosition;
	FVector2D SecondTopCorner = InSecondWidgetGeometry.AbsolutePosition;
	FVector2D FirstSize = InFirstWidgetGeometry.GetAbsoluteSize();
	FVector2D SecondSize = InSecondWidgetGeometry.GetAbsoluteSize();
	
	float x1 = FirstTopCorner.X;
	float y1 = FirstTopCorner.Y;
	float w1 = x1 + FirstSize.X;
	float h1 = y1 + FirstSize.Y;

	float x2 = SecondTopCorner.X;
	float y2 = SecondTopCorner.Y;
	float w2 = x2 + SecondSize.X;
	float h2 = y2 + SecondSize.Y;

	// I believe...it goes something like this for the horizontal stuff?
	if(x1 > w2) return false;
	if(x2 > w1) return false;

	// And I think... like this for vertical?
	if(y1 > h2) return false;
	if(y2 > h1) return false;

	// And I THINK if we get this far, we're overlapping?
	return true;
}

bool UCommonUtils::GetWidgetIntersection(const FGeometry& InFirstWidgetGeometry, const FGeometry& InSecondWidgetGeometry, FVector2D& OutIntersectionSize) {
	// If the widgets don't intersect then return false.
	if(!UCommonUtils::WidgetsOverlap(InFirstWidgetGeometry, InSecondWidgetGeometry)) {
		return false;
	}

	// The returned intersection size goes here:
	float ix = 0.f, iy = 0.f;

	// Get the locations and sizes of each geometry.
	FVector2D loc1 = InFirstWidgetGeometry.AbsolutePosition;
	FVector2D loc2 = InSecondWidgetGeometry.AbsolutePosition;
	FVector2D size1 = InFirstWidgetGeometry.GetAbsoluteSize();
	FVector2D size2 = InSecondWidgetGeometry.GetAbsoluteSize();
	
	// Quick compiler macros because optimization.
	#define Min(a, b) ((a > b) ? (b) : (a))
	#define Max(a, b) ((a > b) ? (a) : (b))

	// "Borrowed" from MonoGame's Rectangle Intersect function.  Hopefully I don't get eaten alive by a lawyer lol.
	// https://github.com/MonoGame/MonoGame/blob/develop/MonoGame.Framework/Rectangle.cs
	// Can you even copyright math?
	float right_side = Min(loc1.X + size1.X, loc2.X + size2.X);
    float left_side = Max(loc1.X, loc2.X);
    float top_side = Max(loc1.Y, loc2.Y);
    float bottom_side = Min(loc1.Y + size1.Y, loc2.Y + size2.Y);

	// Store the intersection size!
	OutIntersectionSize = FVector2D(right_side - left_side, bottom_side - top_side);

	#undef Min
	#undef Max

	// We already know they intersect.
	return true;
}

FText UCommonUtils::GetFirstName(const FText& InText) {
	// Convert the text to a string.
	FString AsString = InText.ToString();

	// The resulting first name:
	FString FirstName;

	// Go through every character and append until we hit whitespace.
	for(int i = 0; i < AsString.Len(); i++) {
		TCHAR c = AsString[i];
		if(FChar::IsWhitespace(c)) {
			break;
		}
		FirstName += c;
	}

	// Return it.
	return FText::FromString(FirstName);
}

FString UCommonUtils::Aliasify(FString InString) {
	// FIXME: Possibly more efficient algorithm? - Michael

	// Convert the string to lowercase and trim whitespace from the start and end.
	InString = InString.ToLower().TrimStartAndEnd();

	// The transformed string:
	FString RetVal;

	// Go through all characters in the source string.
	for(int i = 0; i < InString.Len(); i++) {
		// Pull the current character:
		TCHAR c = InString[i];

		// Handle hyphens and periods:
		if(c == '-' || c == '.') {
			// Skip if the ret string is empty.
			if(!RetVal.Len()) {
				continue;
			}

			// Skip if the ret string ends with the character.
			if(RetVal.EndsWith(FString::Chr(c))) {
				continue;
			}

			// Add it!
			RetVal += c;
			continue;
		}

		// Handle non-alphanumeric characters as they can't appear in aliases.
		if(!FChar::IsAlnum(c)) {
			// If the return string is empty or ends with an underscore, then we'll skip.
			// This prevents the string from beginning with an underscore and having duplicate underscores in it.
			if(RetVal.EndsWith("_") || !RetVal.Len()) {
				continue;
			}

			// Otherwise, append an underscore to the string.
			RetVal += "_";
			continue;
		}

		// The character is alphanumeric if we get this far.
		RetVal += c;
	}

	return RetVal;
}

void UCommonUtils::ReorderCanvasPanel(UCanvasPanel* InCanvasPanel, UWindow* InFocusWindow) {
	// First we need to collect a list of all widget slots in ascending Z order.
	// This will allow us to "normalize" everything.
	TArray<UCanvasPanelSlot*> SortedSlots;

	int ChildCount = InCanvasPanel->GetChildrenCount();
	for(int i = 0; i < ChildCount; i++) {
		auto Widget = InCanvasPanel->GetChildAt(i);
		UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Widget->Slot);
		int ZOrder = Slot->GetZOrder();
		bool Added = false;
		for(int j = 0; j < SortedSlots.Num(); j++) {
			if(SortedSlots[j]->GetZOrder() > ZOrder) {
				Added = true;
				SortedSlots.Insert(Slot, j);
				break;
			}
		}
		if(!Added) {
			 SortedSlots.Add(Slot);
		}
	}

	// Now, we can start rearranging everything properly.
	for(int i = 0; i < SortedSlots.Num(); i++) {
		SortedSlots[i]->SetZOrder(i);
	}

	// The above should have preserved the visual locations of each widget.
	
	// Now we can work on the focus window if we have one.
	if(InFocusWindow) {
		// Grab the ZOrder of the focus window before setting it to a sentinel value.
		UCanvasPanelSlot* FocusSlot = Cast<UCanvasPanelSlot>(InFocusWindow->Slot);
		int FocusZ = FocusSlot->GetZOrder();
		FocusSlot->SetZOrder(-1);

		for(int i = 0; i < SortedSlots.Num(); i++) {
			UCanvasPanelSlot* Slot = SortedSlots[i];
			if(Slot->GetZOrder() == -1) continue; // Skip the focus window.
			if(Slot->GetZOrder() > FocusZ) {
				Slot->SetZOrder(Slot->GetZOrder() - 1);
			}
		}

		FocusSlot->SetZOrder(SortedSlots.Num() - 1);
	}
}

bool UCommonUtils::GetPlayerUserContext(APlayerController* InPlayerController, UUserContext*& OutUserContext) {
	// The reason we need a player controller is that we need a way to get context as to where the player is.
	// By that I mean, in 3D space.  What level are they in?  What actors are in tere?
	//
	// This context will allow us to find the Peacenet World State Actor even when this function is called outside
	// of Peacegate or in a place where a User Context/System Context is inaccessible.  That's the whole point
	// of this function in 0.3.0, so that you can get a user context anywhere.
	//
	// Really, any object that properly implements UObject::GetWorld() - any actor, UMG widget, etc. will work
	// but taking a Player Controller ABSOLUTELY ensures that function will get us what we need.

	// We're gonna need the world context object right now.
	UWorld* WorldContext = InPlayerController->GetWorld();

	// If that's nullptr, something has SEVERELY FUCKED UP in Unreal Engine.
	check(WorldContext);

	// graceful, silent failure in release builds.
	if(!WorldContext) {
		return false;
	}

	// Allows us to iterate through the current world for any Peacenet world state actors.
	TActorIterator<APeacenetWorldStateActor> ActorItr(WorldContext, APeacenetWorldStateActor::StaticClass(), EActorIteratorFlags::SkipPendingKill);

	// Keep on goin' til we run outta gas.
	while(ActorItr) {
		// Try to get a pointer to a Peacenet world state actor.
		APeacenetWorldStateActor* Peacenet = *ActorItr;

		// If it's valid...
		if(Peacenet) {
			// WE HAVE A PEACENET CONTEXT!
			//
			// So now we can get the player user.
			//
			// Essentially this is a matter of getting the player system context and, from there,
			// getting the desktop user session.
			USystemContext* PlayerSystem = Peacenet->GetSystemContext(Peacenet->GetPlayerComputer().ID);

			// Make sure it's valid.
			if(!PlayerSystem) {
				return false;
			}

			// Now we can get the desktop session:
			UDesktopWidget* PlayerDesktop = PlayerSystem->GetDesktop();

			// Make sure that's valid too.
			if(!PlayerDesktop) {
				return false;
			}

			// If the player desktop doesn't have an active session then we return false.
			if(!PlayerDesktop->IsSessionActive()) {
				return false;
			}

			// Get the player user from the desktop and return true!
			OutUserContext = PlayerDesktop->GetUserContext();
			return true;
		}

		// Go to the next possible actor.
		++ActorItr;
	}

	 return false;
}

FText UCommonUtils::GetConnectionError(EConnectionError InConnectionError) {
	switch(InConnectionError) {
		case EConnectionError::None:
			return NSLOCTEXT("Connection", "Success", "Connection successful.");
		case EConnectionError::HostNotFound:
			return NSLOCTEXT("Connection", "HostNotFound", "Could not resolve host.");
		case EConnectionError::ConnectionRefused:
			return NSLOCTEXT("Connection", "Refused", "Connection refused.");
		case EConnectionError::ConnectionTimedOut:
			return NSLOCTEXT("Connection", "Timeout", "Connection timed out.");
		default:
			return NSLOCTEXT("Connection", "Unknown", "Unspecified error.");
	}
}

void UCommonUtils::ParseHost(FString InHost, FString& OutAddress, bool& HasPort, int& OutPort) {
	// Regex pattern used to parse address and port out of host.
	FRegexPattern HostPattern("(.*):(\\d*)");

	// Create a matcher that uses the pattern and host.
	FRegexMatcher Matcher(HostPattern, InHost);

	// If the pattern matches then we have a port.
	if(Matcher.FindNext()) {
		HasPort = true;

		// The address is whatever's in the first capture group.
		OutAddress = Matcher.GetCaptureGroup(1);

		// The port is in the second capture group.
		OutPort = FCString::Atoi(*Matcher.GetCaptureGroup(2));
	} else {
		// No port!
		HasPort = false;
		OutPort = -1;
		OutAddress = InHost;
	}
}

bool UCommonUtils::UpgradeDependsOn(UUserContext* UserContext, USystemUpgrade* Target, USystemUpgrade* Dependency) {
	if(Target->Dependencies.Num() == 0) {
		return false;
	}

	for(auto TargetDep : Target->Dependencies) {
		if(TargetDep == Dependency) {
			return true;
		}

		bool result = UpgradeDependsOn(UserContext, TargetDep, Dependency);
		if(result) return true;
	}

	return false;
}

FLinearColor UCommonUtils::GetConsoleColor(EConsoleColor InConsoleColor) {
	switch(InConsoleColor) {
		default:
		case EConsoleColor::Black:
			return FLinearColor(0.f, 0.f, 0.f);
		case EConsoleColor::White:
			return FLinearColor(1.f, 1.f, 1.f);
		case EConsoleColor::Red:
			return FLinearColor(1.f, 0.f, 0.f);
		case EConsoleColor::Green:
			return FLinearColor(0.f, 1.f, 0.f);
		case EConsoleColor::Blue:
			return FLinearColor(0.f, 0.f, 1.f);
		case EConsoleColor::Yellow:
			return FLinearColor(1.f, 1.f, 0.f);
		case EConsoleColor::Magenta:
			return FLinearColor(1.f, 0.f, 1.f);
		case EConsoleColor::Cyan:
			return FLinearColor(0.f, 1.f, 1.f);
	}
}

float UCommonUtils::GetLogDelayTime(const FText& InText) {
	FString AsString = InText.ToString();

	if(AsString.Contains("[") && AsString.Contains("]")) {
		int Index = -1;
		if(AsString.FindChar(']', Index)) {
			FString Timestamp = AsString.Left(Index);
			Timestamp.RemoveAt(0, 1);
			Timestamp = Timestamp.TrimStartAndEnd();
			return FCString::Atof(*Timestamp);
		} else {
			return 0.f;
		}
	} else {
		return 0.f;
	}
}

TArray<FText> UCommonUtils::GetKernelMessages() {
	TArray<FText> ret;

	// Yes. I know. This looks like it came out of Philip Adams' asshole.  Just looking at it gives me RSI.
	// That's why I wasn't dumb enough to write this by hand, I wrote a script that generated this code from a real
	// Linux dmesg log.  -- Michael
	//fixed it --Richie
	//
	// Okay major update, it's now being pushed into an array rather than a console because of the new main menu UI not using a 
	// console.  - Michael
	ret.Add(FText::FromString("[    0.000000] Linux version 4.15.0-50-generic (buildd@lcy01-amd64-013) (gcc version 7.9.0 (peacegate 7.9.0-16peacegate3)) #54-peacegate SMP Mon May 6 18:46:08 UTC 2030 (Peacegate 4.15.0-50.54-generic 4.15.18)"));
	ret.Add(FText::FromString("[    0.000000] Command line: BOOT_IMAGE=/boot/vmlinuz-4.15.0-50-generic root=UUID=b9df59e6-c806-4851-befa-12402bca5828 ro console=tty1 console=ttyS0"));
	ret.Add(FText::FromString("[    0.000000] KERNEL supported cpus:"));
	ret.Add(FText::FromString("[    0.000000]   Intel GenuineIntel"));
	ret.Add(FText::FromString("[    0.000000]   AMD AuthenticAMD"));
	ret.Add(FText::FromString("[    0.000000]   Centaur CentaurHauls"));
	ret.Add(FText::FromString("[    0.000000] x86/fpu: Supporting XSAVE feature 0x001: 'x87 floating point registers'"));
	ret.Add(FText::FromString("[    0.000000] x86/fpu: Supporting XSAVE feature 0x002: 'SSE registers'"));
	ret.Add(FText::FromString("[    0.000000] x86/fpu: Supporting XSAVE feature 0x004: 'AVX registers'"));
	ret.Add(FText::FromString("[    0.000000] x86/fpu: xstate_offset[2]:  576, xstate_sizes[2]:  256"));
	ret.Add(FText::FromString("[    0.000000] x86/fpu: Enabled xstate features 0x7, context size is 832 bytes, using 'standard' format."));
	ret.Add(FText::FromString("[    0.000000] e820: BIOS-provided physical RAM map:"));
	ret.Add(FText::FromString("[    0.000000] BIOS-e820: [mem 0x0000000000000000-0x000000000009fbff] usable"));
	ret.Add(FText::FromString("[    0.000000] BIOS-e820: [mem 0x000000000009fc00-0x000000000009ffff] reserved"));
	ret.Add(FText::FromString("[    0.000000] BIOS-e820: [mem 0x00000000000f0000-0x00000000000fffff] reserved"));
	ret.Add(FText::FromString("[    0.000000] BIOS-e820: [mem 0x0000000000100000-0x00000000bfffafff] usable"));
	ret.Add(FText::FromString("[    0.000000] BIOS-e820: [mem 0x00000000bfffb000-0x00000000bfffffff] reserved"));
	ret.Add(FText::FromString("[    0.000000] BIOS-e820: [mem 0x00000000feffc000-0x00000000feffffff] reserved"));
	ret.Add(FText::FromString("[    0.000000] BIOS-e820: [mem 0x00000000fffc0000-0x00000000ffffffff] reserved"));
	ret.Add(FText::FromString("[    0.000000] NX (Execute Disable) protection: active"));
	ret.Add(FText::FromString("[    0.000000] SMBIOS 2.4 present."));
	ret.Add(FText::FromString("[    0.000000] DMI: DigitalOcean Droplet, BIOS 20301212 12/12/2030"));
	ret.Add(FText::FromString("[    0.000000] Hypervisor detected: KVM"));
	ret.Add(FText::FromString("[    0.000000] e820: update [mem 0x00000000-0x00000fff] usable ==> reserved"));
	ret.Add(FText::FromString("[    0.000000] e820: remove [mem 0x000a0000-0x000fffff] usable"));
	ret.Add(FText::FromString("[    0.000000] e820: last_pfn = 0xbfffb max_arch_pfn = 0x400000000"));
	ret.Add(FText::FromString("[    0.000000] MTRR default type: write-back"));
	ret.Add(FText::FromString("[    0.000000] MTRR fixed ranges enabled:"));
	ret.Add(FText::FromString("[    0.000000]   00000-9FFFF write-back"));
	ret.Add(FText::FromString("[    0.000000]   A0000-BFFFF uncachable"));
	ret.Add(FText::FromString("[    0.000000]   C0000-FFFFF write-protect"));
	ret.Add(FText::FromString("[    0.000000] MTRR variable ranges enabled:"));
	ret.Add(FText::FromString("[    0.000000]   0 base 00C0000000 mask FFC0000000 uncachable"));
	ret.Add(FText::FromString("[    0.000000]   1 disabled"));
	ret.Add(FText::FromString("[    0.000000]   2 disabled"));
	ret.Add(FText::FromString("[    0.000000]   3 disabled"));
	ret.Add(FText::FromString("[    0.000000]   4 disabled"));
	ret.Add(FText::FromString("[    0.000000]   5 disabled"));
	ret.Add(FText::FromString("[    0.000000]   6 disabled"));
	ret.Add(FText::FromString("[    0.000000]   7 disabled"));
	ret.Add(FText::FromString("[    0.000000] x86/PAT: Configuration [0-7]: WB  WC  UC- UC  WB  WP  UC- WT  "));
	ret.Add(FText::FromString("[    0.000000] found SMP MP-table at [mem 0x000f1280-0x000f128f] mapped at [        (ptrval)]"));
	ret.Add(FText::FromString("[    0.000000] Scanning 1 areas for low memory corruption"));
	ret.Add(FText::FromString("[    0.000000] Base memory trampoline at [        (ptrval)] 99000 size 24576"));
	ret.Add(FText::FromString("[    0.000000] Using GB pages for direct mapping"));
	ret.Add(FText::FromString("[    0.000000] BRK [0x1b741000, 0x1b741fff] PGTABLE"));
	ret.Add(FText::FromString("[    0.000000] BRK [0x1b742000, 0x1b742fff] PGTABLE"));
	ret.Add(FText::FromString("[    0.000000] BRK [0x1b743000, 0x1b743fff] PGTABLE"));
	ret.Add(FText::FromString("[    0.000000] BRK [0x1b744000, 0x1b744fff] PGTABLE"));
	ret.Add(FText::FromString("[    0.000000] BRK [0x1b745000, 0x1b745fff] PGTABLE"));
	ret.Add(FText::FromString("[    0.000000] BRK [0x1b746000, 0x1b746fff] PGTABLE"));
	ret.Add(FText::FromString("[    0.000000] RAMDISK: [mem 0x3515d000-0x368a5fff]"));
	ret.Add(FText::FromString("[    0.000000] ACPI: Early table checksum verification disabled"));
	ret.Add(FText::FromString("[    0.000000] ACPI: RSDP 0x00000000000F10F0 000014 (v00 BOCHS )"));
	ret.Add(FText::FromString("[    0.000000] ACPI: RSDT 0x00000000BFFFE470 000034 (v01 BOCHS  BXPCRSDT 00000001 BXPC 00000001)"));
	ret.Add(FText::FromString("[    0.000000] ACPI: FACP 0x00000000BFFFFF80 000074 (v01 BOCHS  BXPCFACP 00000001 BXPC 00000001)"));
	ret.Add(FText::FromString("[    0.000000] ACPI: DSDT 0x00000000BFFFE4B0 001135 (v01 BOCHS  BXPCDSDT 00000001 BXPC 00000001)"));
	ret.Add(FText::FromString("[    0.000000] ACPI: FACS 0x00000000BFFFFF40 000040"));
	ret.Add(FText::FromString("[    0.000000] ACPI: SSDT 0x00000000BFFFF720 000819 (v01 BOCHS  BXPCSSDT 00000001 BXPC 00000001)"));
	ret.Add(FText::FromString("[    0.000000] ACPI: APIC 0x00000000BFFFF630 000078 (v01 BOCHS  BXPCAPIC 00000001 BXPC 00000001)"));
	ret.Add(FText::FromString("[    0.000000] ACPI: HPET 0x00000000BFFFF5F0 000038 (v01 BOCHS  BXPCHPET 00000001 BXPC 00000001)"));
	ret.Add(FText::FromString("[    0.000000] ACPI: Local APIC address 0xfee00000"));
	ret.Add(FText::FromString("[    0.000000] No NUMA configuration found"));
	ret.Add(FText::FromString("[    0.000000] Faking a node at [mem 0x0000000000000000-0x00000000bfffafff]"));
	ret.Add(FText::FromString("[    0.000000] NODE_DATA(0) allocated [mem 0xbffd0000-0xbfffafff]"));
	ret.Add(FText::FromString("[    0.000000] kvm-clock: cpu 0, msr 0:bff4f001, primary cpu clock"));
	ret.Add(FText::FromString("[    0.000000] kvm-clock: Using msrs 4b564d01 and 4b564d00"));
	ret.Add(FText::FromString("[    0.000000] kvm-clock: using sched offset of 5499243520160660 cycles"));
	ret.Add(FText::FromString("[    0.000000] clocksource: kvm-clock: mask: 0xffffffffffffffff max_cycles: 0x1cd42e4dffb, max_idle_ns: 881590591483 ns"));
	ret.Add(FText::FromString("[    0.000000] Zone ranges:"));
	ret.Add(FText::FromString("[    0.000000]   DMA      [mem 0x0000000000001000-0x0000000000ffffff]"));
	ret.Add(FText::FromString("[    0.000000]   DMA32    [mem 0x0000000001000000-0x00000000bfffafff]"));
	ret.Add(FText::FromString("[    0.000000]   Normal   empty"));
	ret.Add(FText::FromString("[    0.000000]   Device   empty"));
	ret.Add(FText::FromString("[    0.000000] Movable zone start for each node"));
	ret.Add(FText::FromString("[    0.000000] Early memory node ranges"));
	ret.Add(FText::FromString("[    0.000000]   node   0: [mem 0x0000000000001000-0x000000000009efff]"));
	ret.Add(FText::FromString("[    0.000000]   node   0: [mem 0x0000000000100000-0x00000000bfffafff]"));
	ret.Add(FText::FromString("[    0.000000] Reserved but unavailable: 103 pages"));
	ret.Add(FText::FromString("[    0.000000] Initmem setup node 0 [mem 0x0000000000001000-0x00000000bfffafff]"));
	ret.Add(FText::FromString("[    0.000000] On node 0 totalpages: 786329"));
	ret.Add(FText::FromString("[    0.000000]   DMA zone: 64 pages used for memmap"));
	ret.Add(FText::FromString("[    0.000000]   DMA zone: 21 pages reserved"));
	ret.Add(FText::FromString("[    0.000000]   DMA zone: 3998 pages, LIFO batch:0"));
	ret.Add(FText::FromString("[    0.000000]   DMA32 zone: 12224 pages used for memmap"));
	ret.Add(FText::FromString("[    0.000000]   DMA32 zone: 782331 pages, LIFO batch:31"));
	ret.Add(FText::FromString("[    0.000000] ACPI: PM-Timer IO Port: 0xb008"));
	ret.Add(FText::FromString("[    0.000000] ACPI: Local APIC address 0xfee00000"));
	ret.Add(FText::FromString("[    0.000000] ACPI: LAPIC_NMI (acpi_id[0xff] dfl dfl lint[0x1])"));
	ret.Add(FText::FromString("[    0.000000] IOAPIC[0]: apic_id 0, version 17, address 0xfec00000, GSI 0-23"));
	ret.Add(FText::FromString("[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 0 global_irq 2 dfl dfl)"));
	ret.Add(FText::FromString("[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 5 global_irq 5 high level)"));
	ret.Add(FText::FromString("[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 9 global_irq 9 high level)"));
	ret.Add(FText::FromString("[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 10 global_irq 10 high level)"));
	ret.Add(FText::FromString("[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 11 global_irq 11 high level)"));
	ret.Add(FText::FromString("[    0.000000] ACPI: IRQ0 used by override."));
	ret.Add(FText::FromString("[    0.000000] ACPI: IRQ5 used by override."));
	ret.Add(FText::FromString("[    0.000000] ACPI: IRQ9 used by override."));
	ret.Add(FText::FromString("[    0.000000] ACPI: IRQ10 used by override."));
	ret.Add(FText::FromString("[    0.000000] ACPI: IRQ11 used by override."));
	ret.Add(FText::FromString("[    0.000000] Using ACPI (MADT) for SMP configuration information"));
	ret.Add(FText::FromString("[    0.000000] ACPI: HPET id: 0x8086a201 base: 0xfed00000"));
	ret.Add(FText::FromString("[    0.000000] smpboot: Allowing 1 CPUs, 0 hotplug CPUs"));
	ret.Add(FText::FromString("[    0.000000] PM: Registered nosave memory: [mem 0x00000000-0x00000fff]"));
	ret.Add(FText::FromString("[    0.000000] PM: Registered nosave memory: [mem 0x0009f000-0x0009ffff]"));
	ret.Add(FText::FromString("[    0.000000] PM: Registered nosave memory: [mem 0x000a0000-0x000effff]"));
	ret.Add(FText::FromString("[    0.000000] PM: Registered nosave memory: [mem 0x000f0000-0x000fffff]"));
	ret.Add(FText::FromString("[    0.000000] e820: [mem 0xc0000000-0xfeffbfff] available for PCI devices"));
	ret.Add(FText::FromString("[    0.000000] Booting paravirtualized kernel on KVM"));
	ret.Add(FText::FromString("[    0.000000] clocksource: refined-jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645519600211568 ns"));
	ret.Add(FText::FromString("[    0.000000] random: get_random_bytes called from start_kernel+0x99/0x4fd with crng_init=0"));
	ret.Add(FText::FromString("[    0.000000] setup_percpu: NR_CPUS:8192 nr_cpumask_bits:1 nr_cpu_ids:1 nr_node_ids:1"));
	ret.Add(FText::FromString("[    0.000000] percpu: Embedded 46 pages/cpu @        (ptrval) s151552 r8192 d28672 u2097152"));
	ret.Add(FText::FromString("[    0.000000] pcpu-alloc: s151552 r8192 d28672 u2097152 alloc=1*2097152"));
	ret.Add(FText::FromString("[    0.000000] pcpu-alloc: [0] 0 "));
	ret.Add(FText::FromString("[    0.000000] KVM setup async PF for cpu 0"));
	ret.Add(FText::FromString("[    0.000000] kvm-stealtime: cpu 0, msr bfc24040"));
	ret.Add(FText::FromString("[    0.000000] PV qspinlock hash table entries: 256 (order: 0, 4096 bytes)"));
	ret.Add(FText::FromString("[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 774020"));
	ret.Add(FText::FromString("[    0.000000] Policy zone: DMA32"));
	ret.Add(FText::FromString("[    0.000000] Kernel command line: BOOT_IMAGE=/boot/vmlinuz-4.15.0-50-generic root=UUID=b9df59e6-c806-4851-befa-12402bca5828 ro console=tty1 console=ttyS0"));
	ret.Add(FText::FromString("[    0.000000] Calgary: detecting Calgary via BIOS EBDA area"));
	ret.Add(FText::FromString("[    0.000000] Calgary: Unable to locate Rio Grande table in EBDA - bailing!"));
	ret.Add(FText::FromString("[    0.000000] Memory: 3043344K/3145316K available (12300K kernel code, 2474K rwdata, 4276K rodata, 2412K init, 2416K bss, 101972K reserved, 0K cma-reserved)"));
	ret.Add(FText::FromString("[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=1, Nodes=1"));
	ret.Add(FText::FromString("[    0.000000] Kernel/User page tables isolation: enabled"));
	ret.Add(FText::FromString("[    0.000000] ftrace: allocating 39233 entries in 154 pages"));
	ret.Add(FText::FromString("[    0.004000] Hierarchical RCU implementation."));
	ret.Add(FText::FromString("[    0.004000] 	RCU restricting CPUs from NR_CPUS=8192 to nr_cpu_ids=1."));
	ret.Add(FText::FromString("[    0.004000] 	Tasks RCU enabled."));
	ret.Add(FText::FromString("[    0.004000] RCU: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=1"));
	ret.Add(FText::FromString("[    0.004000] NR_IRQS: 524544, nr_irqs: 256, preallocated irqs: 16"));
	ret.Add(FText::FromString("[    0.004000] Console: colour VGA+ 80x25"));
	ret.Add(FText::FromString("[    0.004000] console [tty1] enabled"));
	ret.Add(FText::FromString("[    0.004000] console [ttyS0] enabled"));
	ret.Add(FText::FromString("[    0.004000] ACPI: Core revision 20170831"));
	ret.Add(FText::FromString("[    0.004000] ACPI: 2 ACPI AML tables successfully acquired and loaded"));
	ret.Add(FText::FromString("[    0.004000] clocksource: hpet: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 19112604467 ns"));
	ret.Add(FText::FromString("[    0.004000] hpet clockevent registered"));
	ret.Add(FText::FromString("[    0.004015] APIC: Switch to symmetric I/O mode setup"));
	ret.Add(FText::FromString("[    0.006994] x2apic enabled"));
	ret.Add(FText::FromString("[    0.008011] Switched APIC routing to physical x2apic."));
	ret.Add(FText::FromString("[    0.013537] ..TIMER: vector=0x30 apic1=0 pin1=2 apic2=-1 pin2=-1"));
	ret.Add(FText::FromString("[    0.016000] tsc: Detected 2199.998 MHz processor"));
	ret.Add(FText::FromString("[    0.016013] Calibrating delay loop (skipped) preset value.. 4399.99 BogoMIPS (lpj=8799992)"));
	ret.Add(FText::FromString("[    0.020007] pid_max: default: 32768 minimum: 301"));
	ret.Add(FText::FromString("[    0.024050] Security Framework initialized"));
	ret.Add(FText::FromString("[    0.026040] Yama: becoming mindful."));
	ret.Add(FText::FromString("[    0.028054] AppArmor: AppArmor initialized"));
	ret.Add(FText::FromString("[    0.030867] Dentry cache hash table entries: 524288 (order: 10, 4194304 bytes)"));
	ret.Add(FText::FromString("[    0.033566] Inode-cache hash table entries: 262144 (order: 9, 2097152 bytes)"));
	ret.Add(FText::FromString("[    0.036120] Mount-cache hash table entries: 8192 (order: 4, 65536 bytes)"));
	ret.Add(FText::FromString("[    0.040094] Mountpoint-cache hash table entries: 8192 (order: 4, 65536 bytes)"));
	ret.Add(FText::FromString("[    0.044037] mce: CPU supports 10 MCE banks"));
	ret.Add(FText::FromString("[    0.052067] Last level iTLB entries: 4KB 64, 2MB 8, 4MB 8"));
	ret.Add(FText::FromString("[    0.056010] Last level dTLB entries: 4KB 64, 2MB 0, 4MB 0, 1GB 4"));
	ret.Add(FText::FromString("[    0.060011] Spectre V2 : Mitigation: Full generic retpoline"));
	ret.Add(FText::FromString("[    0.062668] Spectre V2 : Spectre v2 / SpectreRSB mitigation: Filling RSB on context switch"));
	ret.Add(FText::FromString("[    0.064012] Speculative Store Bypass: Vulnerable"));
	ret.Add(FText::FromString("[    0.068050] MDS: Vulnerable: Clear CPU buffers attempted, no microcode"));
	ret.Add(FText::FromString("[    0.084816] Freeing SMP alternatives memory: 36K"));
	ret.Add(FText::FromString("[    0.088585] TSC deadline timer enabled"));
	ret.Add(FText::FromString("[    0.088604] smpboot: CPU0: Intel(R) Xeon(R) CPU E5-2650 v4 @ 2.20GHz (family: 0x6, model: 0x4f, stepping: 0x1)"));
	ret.Add(FText::FromString("[    0.092229] Performance Events: Broadwell events, Intel PMU driver."));
	ret.Add(FText::FromString("[    0.110917] ... version:                2"));
	ret.Add(FText::FromString("[    0.112145] ... bit width:              48"));
	ret.Add(FText::FromString("[    0.114319] ... generic registers:      4"));
	ret.Add(FText::FromString("[    0.116015] ... value mask:             0000ffffffffffff"));
	ret.Add(FText::FromString("[    0.120015] ... max period:             000000007fffffff"));
	ret.Add(FText::FromString("[    0.121998] ... fixed-purpose events:   3"));
	ret.Add(FText::FromString("[    0.123960] ... event mask:             000000070000000f"));
	ret.Add(FText::FromString("[    0.124151] Hierarchical SRCU implementation."));
	ret.Add(FText::FromString("[    0.127470] smp: Bringing up secondary CPUs ..."));
	ret.Add(FText::FromString("[    0.128018] smp: Brought up 1 node, 1 CPU"));
	ret.Add(FText::FromString("[    0.129914] smpboot: Max logical packages: 1"));
	ret.Add(FText::FromString("[    0.131922] smpboot: Total of 1 processors activated (4399.99 BogoMIPS)"));
	ret.Add(FText::FromString("[    0.132521] devtmpfs: initialized"));
	ret.Add(FText::FromString("[    0.134226] x86/mm: Memory block size: 128MB"));
	ret.Add(FText::FromString("[    0.136652] evm: security.selinux"));
	ret.Add(FText::FromString("[    0.138322] evm: security.SMACK64"));
	ret.Add(FText::FromString("[    0.139640] evm: security.SMACK64EXEC"));
	ret.Add(FText::FromString("[    0.140022] evm: security.SMACK64TRANSMUTE"));
	ret.Add(FText::FromString("[    0.144017] evm: security.SMACK64MMAP"));
	ret.Add(FText::FromString("[    0.145705] evm: security.apparmor"));
	ret.Add(FText::FromString("[    0.147354] evm: security.ima"));
	ret.Add(FText::FromString("[    0.148014] evm: security.capability"));
	ret.Add(FText::FromString("[    0.150246] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns"));
	ret.Add(FText::FromString("[    0.152044] futex hash table entries: 256 (order: 2, 16384 bytes)"));
	ret.Add(FText::FromString("[    0.154990] pinctrl core: initialized pinctrl subsystem"));
	ret.Add(FText::FromString("[    0.156241] RTC time: 13:03:04, date: 05/23/19"));
	ret.Add(FText::FromString("[    0.158388] NET: Registered protocol family 16"));
	ret.Add(FText::FromString("[    0.160120] audit: initializing netlink subsys (disabled)"));
	ret.Add(FText::FromString("[    0.162680] cpuidle: using governor ladder"));
	ret.Add(FText::FromString("[    0.164015] cpuidle: using governor menu"));
	ret.Add(FText::FromString("[    0.168029] audit: type=2000 audit(1558616582.810:1): state=initialized audit_enabled=0 res=1"));
	ret.Add(FText::FromString("[    0.171650] ACPI: bus type PCI registered"));
	ret.Add(FText::FromString("[    0.172014] acpiphp: ACPI Hot Plug PCI Controller Driver version: 0.5"));
	ret.Add(FText::FromString("[    0.174809] PCI: Using configuration type 1 for base access"));
	ret.Add(FText::FromString("[    0.177553] HugeTLB registered 1.00 GiB page size, pre-allocated 0 pages"));
	ret.Add(FText::FromString("[    0.180021] HugeTLB registered 2.00 MiB page size, pre-allocated 0 pages"));

	// I need Kaylin's love just to give my brain enough energy to even want to comprehend the amount of RAM this array's going to take up
	// especially when it's marshaled into Blueprint land by Unreal Engine 4.  Good fucking grief.  - Michael
	return ret;
}

FLinearColor UCommonUtils::GetForegroundColor(FLinearColor InColor) {
	float r = InColor.R;
	float g = InColor.G;
	float b = InColor.B;

	if((r + g + b) / 3 <= 0.6f) {
		return FLinearColor(1.f, 1.f, 1.f, 1.f);
	} else {
		return FLinearColor(0.f, 0.f, 0.f, 1.f);
	}
}

FText UCommonUtils::GetRichTextSegment(const FText& InSourceText, int InEndIndex, bool& FoundIncompleteTag, int& TrueEndIndex) {
	// Convert the source text to a string, that way we can more easily perform parsing
	// on it.
	FString SourceString = InSourceText.ToString();

	// Are we currently in a rich tag?
	bool InRichTag = false;

	// The current tag text.
	FString TagIdentifier = "";

	// Keep track of the loop index outside of the loop.
	int i = 0;

	// Go through every character in the source string UNTIL THE END INDEX.
	for(i = 0; i < SourceString.Len() && i < InEndIndex; i++) {
		// Get the current character.
		TCHAR c = SourceString[i];

		// Are we outside of a tag?
		if(!InRichTag) {
			// Check if the current character is the start of a tag.
			if(c == '<') {
				// We're now in a tag.
				InRichTag = true;
				TagIdentifier = FString::Chr(c);
				continue;
			}
		} else {
			// If we are in a tag, add the current character to the identifier...
			TagIdentifier = TagIdentifier.AppendChar(c);

			// And check if the current character is the ending of a tag.
			if(c == '>') {
				// We're no longer in the tag.
				InRichTag = false;
			}
		}
	}

	// If we're still in a tag then continue until we hit the end tag.
	if(InRichTag) {
		FoundIncompleteTag = true;
		for(i = i; i < SourceString.Len(); i++) {
			TCHAR c = SourceString[i];
			TagIdentifier = TagIdentifier.AppendChar(c);
			if(c == '>') {
				InRichTag = false;
				break;
			}
		}	
	} else {
		FoundIncompleteTag = false;
	}

	// Wherever i is now, we can grab a substring starting from index 0 to i and that's
	// the text we're going to return.
	FString ReturnedText = SourceString.Left(i);

	// Unless the tag identifier isn't empty and is not equal to "</>".
	if(TagIdentifier.Len() && TagIdentifier != "</>") {
		// Then we append "</>" to the end of returned text.
		ReturnedText = ReturnedText.Append("</>");
	}

	// Output where we left off in the string.
	TrueEndIndex = i;

	// And then we return it.
	return FText::FromString(ReturnedText);
}

void UCommonUtils::ParseURL(FString InURL, FString& OutUsername, FString& OutHost, int& OutPort, FString& OutPath, bool& HasPath, bool& HasUser, bool& HasPort) {
	HasPort=false;
	HasPath=false;
	HasUser=false;

	FString DefaultUser = "root";
	FString UserDelim = "@";
	FString PortDelim = ":";
	FString PathDelim = "/";

	if(InURL.Contains(PathDelim)) {
		int PathStart = InURL.Find(PathDelim);
		OutPath = InURL.RightChop(PathStart);
		InURL.RemoveFromEnd(OutPath);
		HasPath = true;
	}

	if(InURL.Contains(PortDelim)) {
		int PortStart = InURL.Find(PortDelim, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		FString PortString = InURL.RightChop(PortStart+1);
		if(PortString.Len()) {
			HasPort = true;
			OutPort = FCString::Atoi(*PortString);
		}
		InURL = InURL.Left(PortStart);
	}

	if(InURL.Contains(UserDelim)) {
		int UserEnd = InURL.Find(UserDelim);
		OutUsername = InURL.Left(UserEnd);
		InURL = InURL.RightChop(UserEnd+1);
		HasUser = true;
	}

	OutHost = InURL;
}

void UCommonUtils::GetFriendlyFileOpenText(EFileOpenResult InResult, FString& OutTitle, FString& OutDescription) {
	OutTitle = "";
	OutDescription = "";

	switch(InResult) {
		case EFileOpenResult::FileNotFound:
			OutTitle = "File not found";
			OutDescription = "The system could not find the file specified.";
			break;
		case EFileOpenResult::PermissionDenied:
			OutTitle = "Access denied";
			OutDescription = "You don't have permission to open this file.";
			break;
		case EFileOpenResult::NoSuitableProgram:
			OutTitle = "Can't open file";
			OutDescription = "There are no programs installed that can open this file.";
			break;
	}
}

bool UCommonUtils::GetClipboardText(FString& OutText) {
	FPlatformApplicationMisc::ClipboardPaste(OutText);
	return OutText.Len();
}

void UCommonUtils::PutClipboardText(FString InText) {
	FPlatformApplicationMisc::ClipboardCopy(*InText);
}

FText UCommonUtils::GetFriendlyFilesystemStatusCode(const EFilesystemStatusCode InStatusCode) {
	switch (InStatusCode) {
		case EFilesystemStatusCode::OK:
			return FText();
		case EFilesystemStatusCode::DirectoryNotEmpty:
			return NSLOCTEXT("Peacegate", "DirectoryNotEmpty", "Directory not empty.");
		case EFilesystemStatusCode::FileOrDirectoryExists:
			return NSLOCTEXT("Peacegate", "FileOrDirectoryExists", "File or directory exists.");
		case EFilesystemStatusCode::FileOrDirectoryNotFound:
			return NSLOCTEXT("Peacegate", "FileOrDirectoryNotFound", "File or directory not found.");
		case EFilesystemStatusCode::PermissionDenied:
			return NSLOCTEXT("Peacegate", "PermissionDenied", "Permission denied.");
		default:
			return NSLOCTEXT("Peacegate", "UnknownError", "An unknown error has occurred.");
	}
}

UPeacegateFileSystem * UCommonUtils::CreateFilesystem(USystemContext* InSystemContext, int InUserID) {
	UPeacegateFileSystem* FS = NewObject<UPeacegateFileSystem>();
	FS->SystemContext = InSystemContext;
	FS->Initialize(InUserID);
	return FS;
}

FLinearColor UCommonUtils::GetUserColor(EUserColor InColor) {
	FColor Result = FColor(0x00, 0x00, 0x00, 0xFF);

	switch(InColor) {
		case EUserColor::Peaceful:
			Result = FColor(0x1B, 0xAA, 0xF7, 0xFF);
			break;
		case EUserColor::Sky:
			Result = FColor(0x01, 0xDF, 0xD7, 0xFF);
			break;
		case EUserColor::Blood:
			Result = FColor(0xF7, 0x1B, 0x1B, 0xFF);
			break;
		case EUserColor::Noir:
			Result = FColor(0x75, 0x75, 0x75, 0xFF);
			break;
		case EUserColor::Fire:
			Result = FColor(0xF7, 0x94, 0x1B, 0xFF);
			break;
		case EUserColor::Slate:
			Result = FColor(0x94, 0x44, 0xFF, 0xFF);
			break;
		case EUserColor::Caramel:
			Result = FColor(0xC7, 0x62, 0x17, 0xFF);
			break;
		case EUserColor::Serenity:
			Result = FColor(0x2C, 0xD3, 0x1D, 0xFF);
			break;
		
	}

	return FLinearColor(Result);
}

void UCommonUtils::MeasureChar(const TCHAR InChar, const FSlateFontInfo & InSlateFont, float & OutWidth, float & OutHeight) {
	float x, y = 0;

	const UFont* RawFont = Cast<UFont>(InSlateFont.FontObject);

	float MeasureSize = RawFont->LegacyFontSize;
	float RealSize = InSlateFont.Size;
	float Scale = RealSize / MeasureSize;

	RawFont->GetCharSize(InChar, x, y);

	OutWidth = x * Scale;
	OutHeight = y * Scale;
}

void UCommonUtils::SetEnableBloom(UCameraComponent * InCamera, bool InEnableBloom) {
	auto PostProcessSettings = InCamera->PostProcessSettings;
	PostProcessSettings.bOverride_BloomIntensity = InEnableBloom;
	InCamera->PostProcessSettings = PostProcessSettings;
}

void UCommonUtils::ParseCharacterName(const FString InCharacterName, FString & OutUsername, FString & OutHostname) {
	// No sense doing this if there's only whitespace
	if (InCharacterName.IsEmpty()) {
		return;
	}

	// Unix usernames can only be lower-case.
	FString NameString = InCharacterName.ToLower();

	// this will be the username.
	FString FirstName;
	FString Rem;

	// These characters are valid as name chars.
	const FString ValidUnixUsernameChars = TEXT("abcdefghijklmnopqrstuvwxyz0123456789_-");

	// the first char that isn't valid.
	TCHAR InvalidChar = TEXT('\0');

	// the chars in the name string
	TArray<TCHAR> NameChars = NameString.GetCharArray();

	for (auto Char : NameChars) {
		if (!ValidUnixUsernameChars.Contains(FString(1, &Char))) {
			InvalidChar = Char;
			break;
		}
	}

	// Did that for loop above change us?
	if (InvalidChar != TEXT('\0')) {
		NameString.Split(FString(1, &InvalidChar), &FirstName, &Rem);
	} else {
		FirstName = NameString;
	}

	OutUsername = FirstName;
	OutHostname = FirstName + TEXT("-pc");
}

float UCommonUtils::GetRotation(FVector2D InA, FVector2D InB) {
	float adj = InA.X - InB.X;
    float opp = InA.Y - InB.Y;
    return FMath::RadiansToDegrees<float>(FMath::Atan2(opp, adj)/* - FMath::PI*/);
}