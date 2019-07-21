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

void UCommonUtils::ReorderCanvasPanel(UCanvasPanel* InCanvasPanel, UWindow* InFocusWindow)
{
	// First we need to collect a list of all widget slots in ascending Z order.
	// This will allow us to "normalize" everything.
	TArray<UCanvasPanelSlot*> SortedSlots;

	int ChildCount = InCanvasPanel->GetChildrenCount();
	for(int i = 0; i < ChildCount; i++)
	{
		auto Widget = InCanvasPanel->GetChildAt(i);
		UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Widget->Slot);
		int ZOrder = Slot->GetZOrder();
		bool Added = false;
		for(int j = 0; j < SortedSlots.Num(); j++)
		{
			if(SortedSlots[j]->GetZOrder() > ZOrder)
			{
				Added = true;
				SortedSlots.Insert(Slot, j);
				break;
			}
		}
		if(!Added) SortedSlots.Add(Slot);
	}

	// Now, we can start rearranging everything properly.
	for(int i = 0; i < SortedSlots.Num(); i++)
		SortedSlots[i]->SetZOrder(i);

	// The above should have preserved the visual locations of each widget.
	
	// Now we can work on the focus window if we have one.
	if(InFocusWindow)
	{
		// Grab the ZOrder of the focus window before setting it to a sentinel value.
		UCanvasPanelSlot* FocusSlot = Cast<UCanvasPanelSlot>(InFocusWindow->Slot);
		int FocusZ = FocusSlot->GetZOrder();
		FocusSlot->SetZOrder(-1);

		for(int i = 0; i < SortedSlots.Num(); i++)
		{
			UCanvasPanelSlot* Slot = SortedSlots[i];
			if(Slot->GetZOrder() == -1) continue; // Skip the focus window.
			if(Slot->GetZOrder() > FocusZ)
				Slot->SetZOrder(Slot->GetZOrder() - 1);
		}

		FocusSlot->SetZOrder(SortedSlots.Num() - 1);
	}
}

FLinearColor UCommonUtils::GetConsoleColor(EConsoleColor InConsoleColor)
{
	switch(InConsoleColor)
	{
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

float UCommonUtils::PrintKernelMessages(UTerminalEmulator* InConsole)
{
	// Yes. I know. This looks like it came out of Philip Adams' asshole.  Just looking at it gives me RSI.
	// That's why I wasn't dumb enough to write this by hand, I wrote a script that generated this code from a real
	// Linux dmesg log.  -- Michael
	InConsole->WriteLine(FText::FromString("[    0.000000] Linux version 4.15.0-50-generic (buildd@lcy01-amd64-013) (gcc version 7.3.0 (Ubuntu 7.3.0-16ubuntu3)) #54-Ubuntu SMP Mon May 6 18:46:08 UTC 2019 (Ubuntu 4.15.0-50.54-generic 4.15.18)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Command line: BOOT_IMAGE=/boot/vmlinuz-4.15.0-50-generic root=UUID=b9df59e6-c806-4851-befa-12402bca5828 ro console=tty1 console=ttyS0"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] KERNEL supported cpus:"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   Intel GenuineIntel"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   AMD AuthenticAMD"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   Centaur CentaurHauls"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] x86/fpu: Supporting XSAVE feature 0x001: 'x87 floating point registers'"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] x86/fpu: Supporting XSAVE feature 0x002: 'SSE registers'"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] x86/fpu: Supporting XSAVE feature 0x004: 'AVX registers'"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] x86/fpu: xstate_offset[2]:  576, xstate_sizes[2]:  256"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] x86/fpu: Enabled xstate features 0x7, context size is 832 bytes, using 'standard' format."), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] e820: BIOS-provided physical RAM map:"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BIOS-e820: [mem 0x0000000000000000-0x000000000009fbff] usable"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BIOS-e820: [mem 0x000000000009fc00-0x000000000009ffff] reserved"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BIOS-e820: [mem 0x00000000000f0000-0x00000000000fffff] reserved"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BIOS-e820: [mem 0x0000000000100000-0x00000000bfffafff] usable"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BIOS-e820: [mem 0x00000000bfffb000-0x00000000bfffffff] reserved"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BIOS-e820: [mem 0x00000000feffc000-0x00000000feffffff] reserved"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BIOS-e820: [mem 0x00000000fffc0000-0x00000000ffffffff] reserved"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] NX (Execute Disable) protection: active"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] SMBIOS 2.4 present."), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] DMI: DigitalOcean Droplet, BIOS 20171212 12/12/2017"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Hypervisor detected: KVM"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] e820: update [mem 0x00000000-0x00000fff] usable ==> reserved"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] e820: remove [mem 0x000a0000-0x000fffff] usable"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] e820: last_pfn = 0xbfffb max_arch_pfn = 0x400000000"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] MTRR default type: write-back"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] MTRR fixed ranges enabled:"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   00000-9FFFF write-back"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   A0000-BFFFF uncachable"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   C0000-FFFFF write-protect"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] MTRR variable ranges enabled:"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   0 base 00C0000000 mask FFC0000000 uncachable"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   1 disabled"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   2 disabled"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   3 disabled"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   4 disabled"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   5 disabled"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   6 disabled"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   7 disabled"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] x86/PAT: Configuration [0-7]: WB  WC  UC- UC  WB  WP  UC- WT  "), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] found SMP MP-table at [mem 0x000f1280-0x000f128f] mapped at [        (ptrval)]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Scanning 1 areas for low memory corruption"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Base memory trampoline at [        (ptrval)] 99000 size 24576"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Using GB pages for direct mapping"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BRK [0x1b741000, 0x1b741fff] PGTABLE"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BRK [0x1b742000, 0x1b742fff] PGTABLE"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BRK [0x1b743000, 0x1b743fff] PGTABLE"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BRK [0x1b744000, 0x1b744fff] PGTABLE"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BRK [0x1b745000, 0x1b745fff] PGTABLE"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] BRK [0x1b746000, 0x1b746fff] PGTABLE"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] RAMDISK: [mem 0x3515d000-0x368a5fff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: Early table checksum verification disabled"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: RSDP 0x00000000000F10F0 000014 (v00 BOCHS )"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: RSDT 0x00000000BFFFE470 000034 (v01 BOCHS  BXPCRSDT 00000001 BXPC 00000001)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: FACP 0x00000000BFFFFF80 000074 (v01 BOCHS  BXPCFACP 00000001 BXPC 00000001)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: DSDT 0x00000000BFFFE4B0 001135 (v01 BOCHS  BXPCDSDT 00000001 BXPC 00000001)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: FACS 0x00000000BFFFFF40 000040"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: SSDT 0x00000000BFFFF720 000819 (v01 BOCHS  BXPCSSDT 00000001 BXPC 00000001)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: APIC 0x00000000BFFFF630 000078 (v01 BOCHS  BXPCAPIC 00000001 BXPC 00000001)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: HPET 0x00000000BFFFF5F0 000038 (v01 BOCHS  BXPCHPET 00000001 BXPC 00000001)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: Local APIC address 0xfee00000"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] No NUMA configuration found"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Faking a node at [mem 0x0000000000000000-0x00000000bfffafff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] NODE_DATA(0) allocated [mem 0xbffd0000-0xbfffafff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] kvm-clock: cpu 0, msr 0:bff4f001, primary cpu clock"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] kvm-clock: Using msrs 4b564d01 and 4b564d00"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] kvm-clock: using sched offset of 5499243520160660 cycles"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] clocksource: kvm-clock: mask: 0xffffffffffffffff max_cycles: 0x1cd42e4dffb, max_idle_ns: 881590591483 ns"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Zone ranges:"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   DMA      [mem 0x0000000000001000-0x0000000000ffffff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   DMA32    [mem 0x0000000001000000-0x00000000bfffafff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   Normal   empty"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   Device   empty"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Movable zone start for each node"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Early memory node ranges"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   node   0: [mem 0x0000000000001000-0x000000000009efff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   node   0: [mem 0x0000000000100000-0x00000000bfffafff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Reserved but unavailable: 103 pages"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Initmem setup node 0 [mem 0x0000000000001000-0x00000000bfffafff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] On node 0 totalpages: 786329"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   DMA zone: 64 pages used for memmap"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   DMA zone: 21 pages reserved"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   DMA zone: 3998 pages, LIFO batch:0"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   DMA32 zone: 12224 pages used for memmap"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000]   DMA32 zone: 782331 pages, LIFO batch:31"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: PM-Timer IO Port: 0xb008"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: Local APIC address 0xfee00000"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: LAPIC_NMI (acpi_id[0xff] dfl dfl lint[0x1])"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] IOAPIC[0]: apic_id 0, version 17, address 0xfec00000, GSI 0-23"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 0 global_irq 2 dfl dfl)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 5 global_irq 5 high level)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 9 global_irq 9 high level)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 10 global_irq 10 high level)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: INT_SRC_OVR (bus 0 bus_irq 11 global_irq 11 high level)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: IRQ0 used by override."), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: IRQ5 used by override."), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: IRQ9 used by override."), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: IRQ10 used by override."), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: IRQ11 used by override."), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Using ACPI (MADT) for SMP configuration information"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ACPI: HPET id: 0x8086a201 base: 0xfed00000"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] smpboot: Allowing 1 CPUs, 0 hotplug CPUs"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] PM: Registered nosave memory: [mem 0x00000000-0x00000fff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] PM: Registered nosave memory: [mem 0x0009f000-0x0009ffff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] PM: Registered nosave memory: [mem 0x000a0000-0x000effff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] PM: Registered nosave memory: [mem 0x000f0000-0x000fffff]"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] e820: [mem 0xc0000000-0xfeffbfff] available for PCI devices"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Booting paravirtualized kernel on KVM"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] clocksource: refined-jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645519600211568 ns"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] random: get_random_bytes called from start_kernel+0x99/0x4fd with crng_init=0"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] setup_percpu: NR_CPUS:8192 nr_cpumask_bits:1 nr_cpu_ids:1 nr_node_ids:1"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] percpu: Embedded 46 pages/cpu @        (ptrval) s151552 r8192 d28672 u2097152"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] pcpu-alloc: s151552 r8192 d28672 u2097152 alloc=1*2097152"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] pcpu-alloc: [0] 0 "), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] KVM setup async PF for cpu 0"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] kvm-stealtime: cpu 0, msr bfc24040"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] PV qspinlock hash table entries: 256 (order: 0, 4096 bytes)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 774020"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Policy zone: DMA32"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Kernel command line: BOOT_IMAGE=/boot/vmlinuz-4.15.0-50-generic root=UUID=b9df59e6-c806-4851-befa-12402bca5828 ro console=tty1 console=ttyS0"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Calgary: detecting Calgary via BIOS EBDA area"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Calgary: Unable to locate Rio Grande table in EBDA - bailing!"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Memory: 3043344K/3145316K available (12300K kernel code, 2474K rwdata, 4276K rodata, 2412K init, 2416K bss, 101972K reserved, 0K cma-reserved)"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=1, Nodes=1"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] Kernel/User page tables isolation: enabled"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.000000] ftrace: allocating 39233 entries in 154 pages"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004000] Hierarchical RCU implementation."), 0.004f);
	InConsole->WriteLine(FText::FromString("[    0.004000] 	RCU restricting CPUs from NR_CPUS=8192 to nr_cpu_ids=1."), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004000] 	Tasks RCU enabled."), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004000] RCU: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=1"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004000] NR_IRQS: 524544, nr_irqs: 256, preallocated irqs: 16"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004000] Console: colour VGA+ 80x25"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004000] console [tty1] enabled"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004000] console [ttyS0] enabled"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004000] ACPI: Core revision 20170831"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004000] ACPI: 2 ACPI AML tables successfully acquired and loaded"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004000] clocksource: hpet: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 19112604467 ns"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004000] hpet clockevent registered"), 0.f);
	InConsole->WriteLine(FText::FromString("[    0.004015] APIC: Switch to symmetric I/O mode setup"), 1.499988E-05f);
	InConsole->WriteLine(FText::FromString("[    0.006994] x2apic enabled"), 0.002979f);
	InConsole->WriteLine(FText::FromString("[    0.008011] Switched APIC routing to physical x2apic."), 0.001017f);
	InConsole->WriteLine(FText::FromString("[    0.013537] ..TIMER: vector=0x30 apic1=0 pin1=2 apic2=-1 pin2=-1"), 0.005526f);
	InConsole->WriteLine(FText::FromString("[    0.016000] tsc: Detected 2199.998 MHz processor"), 0.002463001f);
	InConsole->WriteLine(FText::FromString("[    0.016013] Calibrating delay loop (skipped) preset value.. 4399.99 BogoMIPS (lpj=8799992)"), 1.29994E-05f);
	InConsole->WriteLine(FText::FromString("[    0.020007] pid_max: default: 32768 minimum: 301"), 0.003993999f);
	InConsole->WriteLine(FText::FromString("[    0.024050] Security Framework initialized"), 0.004043f);
	InConsole->WriteLine(FText::FromString("[    0.026040] Yama: becoming mindful."), 0.001990002f);
	InConsole->WriteLine(FText::FromString("[    0.028054] AppArmor: AppArmor initialized"), 0.002014f);
	InConsole->WriteLine(FText::FromString("[    0.030867] Dentry cache hash table entries: 524288 (order: 10, 4194304 bytes)"), 0.002812998f);
	InConsole->WriteLine(FText::FromString("[    0.033566] Inode-cache hash table entries: 262144 (order: 9, 2097152 bytes)"), 0.002699003f);
	InConsole->WriteLine(FText::FromString("[    0.036120] Mount-cache hash table entries: 8192 (order: 4, 65536 bytes)"), 0.002553999f);
	InConsole->WriteLine(FText::FromString("[    0.040094] Mountpoint-cache hash table entries: 8192 (order: 4, 65536 bytes)"), 0.003973998f);
	InConsole->WriteLine(FText::FromString("[    0.044037] mce: CPU supports 10 MCE banks"), 0.003943f);
	InConsole->WriteLine(FText::FromString("[    0.052067] Last level iTLB entries: 4KB 64, 2MB 8, 4MB 8"), 0.008030001f);
	InConsole->WriteLine(FText::FromString("[    0.056010] Last level dTLB entries: 4KB 64, 2MB 0, 4MB 0, 1GB 4"), 0.003943f);
	InConsole->WriteLine(FText::FromString("[    0.060011] Spectre V2 : Mitigation: Full generic retpoline"), 0.004000999f);
	InConsole->WriteLine(FText::FromString("[    0.062668] Spectre V2 : Spectre v2 / SpectreRSB mitigation: Filling RSB on context switch"), 0.002657004f);
	InConsole->WriteLine(FText::FromString("[    0.064012] Speculative Store Bypass: Vulnerable"), 0.001343995f);
	InConsole->WriteLine(FText::FromString("[    0.068050] MDS: Vulnerable: Clear CPU buffers attempted, no microcode"), 0.004037999f);
	InConsole->WriteLine(FText::FromString("[    0.084816] Freeing SMP alternatives memory: 36K"), 0.016766f);
	InConsole->WriteLine(FText::FromString("[    0.088585] TSC deadline timer enabled"), 0.003768995f);
	InConsole->WriteLine(FText::FromString("[    0.088604] smpboot: CPU0: Intel(R) Xeon(R) CPU E5-2650 v4 @ 2.20GHz (family: 0x6, model: 0x4f, stepping: 0x1)"), 1.900643E-05f);
	InConsole->WriteLine(FText::FromString("[    0.092229] Performance Events: Broadwell events, Intel PMU driver."), 0.003624998f);
	InConsole->WriteLine(FText::FromString("[    0.110917] ... version:                2"), 0.018688f);
	InConsole->WriteLine(FText::FromString("[    0.112145] ... bit width:              48"), 0.001227997f);
	InConsole->WriteLine(FText::FromString("[    0.114319] ... generic registers:      4"), 0.002173997f);
	InConsole->WriteLine(FText::FromString("[    0.116015] ... value mask:             0000ffffffffffff"), 0.001696005f);
	InConsole->WriteLine(FText::FromString("[    0.120015] ... max period:             000000007fffffff"), 0.004000001f);
	InConsole->WriteLine(FText::FromString("[    0.121998] ... fixed-purpose events:   3"), 0.001982994f);
	InConsole->WriteLine(FText::FromString("[    0.123960] ... event mask:             000000070000000f"), 0.001962006f);
	InConsole->WriteLine(FText::FromString("[    0.124151] Hierarchical SRCU implementation."), 0.0001909956f);
	InConsole->WriteLine(FText::FromString("[    0.127470] smp: Bringing up secondary CPUs ..."), 0.003319003f);
	InConsole->WriteLine(FText::FromString("[    0.128018] smp: Brought up 1 node, 1 CPU"), 0.0005480051f);
	InConsole->WriteLine(FText::FromString("[    0.129914] smpboot: Max logical packages: 1"), 0.001895994f);
	InConsole->WriteLine(FText::FromString("[    0.131922] smpboot: Total of 1 processors activated (4399.99 BogoMIPS)"), 0.002008006f);
	InConsole->WriteLine(FText::FromString("[    0.132521] devtmpfs: initialized"), 0.0005989969f);
	InConsole->WriteLine(FText::FromString("[    0.134226] x86/mm: Memory block size: 128MB"), 0.001704991f);
	InConsole->WriteLine(FText::FromString("[    0.136652] evm: security.selinux"), 0.002425998f);
	InConsole->WriteLine(FText::FromString("[    0.138322] evm: security.SMACK64"), 0.001670003f);
	InConsole->WriteLine(FText::FromString("[    0.139640] evm: security.SMACK64EXEC"), 0.001318008f);
	InConsole->WriteLine(FText::FromString("[    0.140022] evm: security.SMACK64TRANSMUTE"), 0.0003819913f);
	InConsole->WriteLine(FText::FromString("[    0.144017] evm: security.SMACK64MMAP"), 0.003995001f);
	InConsole->WriteLine(FText::FromString("[    0.145705] evm: security.apparmor"), 0.001688004f);
	InConsole->WriteLine(FText::FromString("[    0.147354] evm: security.ima"), 0.001649007f);
	InConsole->WriteLine(FText::FromString("[    0.148014] evm: security.capability"), 0.0006599873f);
	InConsole->WriteLine(FText::FromString("[    0.150246] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns"), 0.002232f);
	InConsole->WriteLine(FText::FromString("[    0.152044] futex hash table entries: 256 (order: 2, 16384 bytes)"), 0.001798004f);
	InConsole->WriteLine(FText::FromString("[    0.154990] pinctrl core: initialized pinctrl subsystem"), 0.002946004f);
	InConsole->WriteLine(FText::FromString("[    0.156241] RTC time: 13:03:04, date: 05/23/19"), 0.001250997f);
	InConsole->WriteLine(FText::FromString("[    0.158388] NET: Registered protocol family 16"), 0.002147004f);
	InConsole->WriteLine(FText::FromString("[    0.160120] audit: initializing netlink subsys (disabled)"), 0.001731992f);
	InConsole->WriteLine(FText::FromString("[    0.162680] cpuidle: using governor ladder"), 0.002560005f);
	InConsole->WriteLine(FText::FromString("[    0.164015] cpuidle: using governor menu"), 0.001334995f);
	InConsole->WriteLine(FText::FromString("[    0.168029] audit: type=2000 audit(1558616582.810:1): state=initialized audit_enabled=0 res=1"), 0.004014f);
	InConsole->WriteLine(FText::FromString("[    0.171650] ACPI: bus type PCI registered"), 0.003621012f);
	InConsole->WriteLine(FText::FromString("[    0.172014] acpiphp: ACPI Hot Plug PCI Controller Driver version: 0.5"), 0.0003639907f);
	InConsole->WriteLine(FText::FromString("[    0.174809] PCI: Using configuration type 1 for base access"), 0.002794996f);
	InConsole->WriteLine(FText::FromString("[    0.177553] HugeTLB registered 1.00 GiB page size, pre-allocated 0 pages"), 0.002744004f);
	InConsole->WriteLine(FText::FromString("[    0.180021] HugeTLB registered 2.00 MiB page size, pre-allocated 0 pages"), 0.002468005f);
	return 0.180021f;
}

FLinearColor UCommonUtils::GetForegroundColor(FLinearColor InColor)
{
	float r = InColor.R;
	float g = InColor.G;
	float b = InColor.B;

	if((r + g + b) / 3 <= 0.6f)
	{
		return FLinearColor(1.f, 1.f, 1.f, 1.f);
	}
	else {
		return FLinearColor(0.f, 0.f, 0.f, 1.f);
	}
}

FText UCommonUtils::GetRichTextSegment(const FText& InSourceText, int InEndIndex, bool& FoundIncompleteTag, int& TrueEndIndex)
{
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
	for(i = 0; i < SourceString.Len() && i < InEndIndex; i++)
	{
		// Get the current character.
		TCHAR c = SourceString[i];

		// Are we outside of a tag?
		if(!InRichTag)
		{
			// Check if the current character is the start of a tag.
			if(c == '<')
			{
				// We're now in a tag.
				InRichTag = true;
				TagIdentifier = FString::Chr(c);
				continue;
			}
		}
		else
		{
			// If we are in a tag, add the current character to the identifier...
			TagIdentifier = TagIdentifier.AppendChar(c);

			// And check if the current character is the ending of a tag.
			if(c == '>')
			{
				// We're no longer in the tag.
				InRichTag = false;
			}
		}
	}

	// If we're still in a tag then continue until we hit the end tag.
	if(InRichTag)
	{
		FoundIncompleteTag = true;
		for(i = i; i < SourceString.Len(); i++)
		{
			TCHAR c = SourceString[i];
			TagIdentifier = TagIdentifier.AppendChar(c);
			if(c == '>')
			{
				InRichTag = false;
				break;
			}
		}	
	}
	else
	{
		FoundIncompleteTag = false;
	}

	// Wherever i is now, we can grab a substring starting from index 0 to i and that's
	// the text we're going to return.
	FString ReturnedText = SourceString.Left(i);

	// Unless the tag identifier isn't empty and is not equal to "</>".
	if(TagIdentifier.Len() && TagIdentifier != "</>")
	{
		// Then we append "</>" to the end of returned text.
		ReturnedText = ReturnedText.Append("</>");
	}

	// Output where we left off in the string.
	TrueEndIndex = i;

	// And then we return it.
	return FText::FromString(ReturnedText);
}

void UCommonUtils::ParseURL(FString InURL, FString& OutUsername, FString& OutHost, int& OutPort, FString& OutPath, bool& HasPath, bool& HasUser, bool& HasPort)
{
	HasPort=false;
	HasPath=false;
	HasUser=false;

	FString DefaultUser = "root";
	FString UserDelim = "@";
	FString PortDelim = ":";
	FString PathDelim = "/";

	if(InURL.Contains(PathDelim))
	{
		int PathStart = InURL.Find(PathDelim);
		OutPath = InURL.RightChop(PathStart);
		InURL.RemoveFromEnd(OutPath);
		HasPath = true;
	}

	if(InURL.Contains(PortDelim))
	{
		int PortStart = InURL.Find(PortDelim, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		FString PortString = InURL.RightChop(PortStart+1);
		if(PortString.Len())
		{
			HasPort = true;
			OutPort = FCString::Atoi(*PortString);
		}
		InURL = InURL.Left(PortStart);
	}

	if(InURL.Contains(UserDelim))
	{
		int UserEnd = InURL.Find(UserDelim);
		OutUsername = InURL.Left(UserEnd);
		InURL = InURL.RightChop(UserEnd+1);
		HasUser = true;
	}

	OutHost = InURL;
}

void UCommonUtils::GetFriendlyFileOpenText(EFileOpenResult InResult, FString& OutTitle, FString& OutDescription)
{
	OutTitle = "";
	OutDescription = "";

	switch(InResult)
	{
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

bool UCommonUtils::GetClipboardText(FString& OutText)
{
	FPlatformApplicationMisc::ClipboardPaste(OutText);
	return OutText.Len();
}

void UCommonUtils::PutClipboardText(FString InText)
{
	FPlatformApplicationMisc::ClipboardCopy(*InText);
}

FText UCommonUtils::GetFriendlyFilesystemStatusCode(const EFilesystemStatusCode InStatusCode)
{
	switch (InStatusCode)
	{
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

UPeacegateFileSystem * UCommonUtils::CreateFilesystem(USystemContext* InSystemContext, int InUserID)
{
	UPeacegateFileSystem* FS = NewObject<UPeacegateFileSystem>();
	FS->SystemContext = InSystemContext;
	FS->Initialize(InUserID);
	return FS;
}

FLinearColor UCommonUtils::GetTerminalColor(ETerminalColor InColor)
{
	switch (InColor)
	{
	case ETerminalColor::Black:
	default:
		return FLinearColor::Black;
	case ETerminalColor::Blue:
		return FLinearColor(0.f, 0.f, 0.5f, 1.f);
	case ETerminalColor::Red:
		return FLinearColor(0.5f, 0.f, 0.f, 1.f);
	case ETerminalColor::Green:
		return FLinearColor(0.f, 0.5f, 0.f, 1.f);
	case ETerminalColor::Aqua:
		return FLinearColor(0.f, 0.5f, 0.5f, 1.f);
	case ETerminalColor::Purple:
		return FLinearColor(0.5f, 0.f, 0.5f, 1.f);
	case ETerminalColor::Yellow:
		return FLinearColor(0.5f, 0.5f, 0.f, 1.f);
	case ETerminalColor::Gray:
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
	case ETerminalColor::White:
		return FLinearColor(0.75F, 0.75f, 0.75f, 1.f);
	case ETerminalColor::LightBlue:
		return FLinearColor(0.f, 0.f, 1.f, 1.f);
	case ETerminalColor::LightGreen:
		return FLinearColor(0.f, 1.f, 0.f, 1.f);
	case ETerminalColor::LightRed:
		return FLinearColor(1.f, 0.f, 0.f, 1.f);
	case ETerminalColor::LightAqua:
		return FLinearColor(0.f, 1.f, 1.f, 1.f);
	case ETerminalColor::LightPurple:
		return FLinearColor(1.f, 0.f, 1.f, 1.f);
	case ETerminalColor::LightYellow:
		return FLinearColor(1.f, 1.f, 0.f, 1.f);
	case ETerminalColor::BrightWhite:
		return FLinearColor(1.f, 1.f, 1.f, 1.f);
	}
}

FString UCommonUtils::GetTerminalColorCode(ETerminalColor InColor)
{
	switch (InColor)
	{
	case ETerminalColor::Black:
	default:
		return "&0";
	case ETerminalColor::Blue:
		return "&1";
	case ETerminalColor::Red:
		return "&4";
	case ETerminalColor::Green:
		return "&2";
	case ETerminalColor::Aqua:
		return "&3";
	case ETerminalColor::Purple:
		return "&5";
	case ETerminalColor::Yellow:
		return "&6";
	case ETerminalColor::Gray:
		return "&8";
	case ETerminalColor::White:
		return "&7";
	case ETerminalColor::LightBlue:
		return "&9";
	case ETerminalColor::LightGreen:
		return "&A";
	case ETerminalColor::LightRed:
		return "&C";
	case ETerminalColor::LightAqua:
		return "&B";
	case ETerminalColor::LightPurple:
		return "&D";
	case ETerminalColor::LightYellow:
		return "&E";
	case ETerminalColor::BrightWhite:
		return "&F";
	}
}

bool UCommonUtils::IsColorCode(FString InControlCode, ETerminalColor& OutColor)
{
	if (!InControlCode.StartsWith("&"))
		return false;

	if (InControlCode.Len() != 2)
		return false;

	// Get rid of the "&" at the start so we can parse as hex
	InControlCode.RemoveAt(0, 1);

	int Code = FParse::HexNumber(InControlCode.GetCharArray().GetData());
	if (Code == 0 && InControlCode != "0")
		return false; //HexNumber returns 0 if the code is invalid.

	// Enums are awesome because we can do this.
	OutColor = (ETerminalColor)Code;
	return true;
}

void UCommonUtils::MeasureChar(const TCHAR InChar, const FSlateFontInfo & InSlateFont, float & OutWidth, float & OutHeight)
{
	float x, y = 0;

	const UFont* RawFont = Cast<UFont>(InSlateFont.FontObject);

	float MeasureSize = RawFont->LegacyFontSize;
	float RealSize = InSlateFont.Size;
	float Scale = RealSize / MeasureSize;

	RawFont->GetCharSize(InChar, x, y);

	OutWidth = x * Scale;
	OutHeight = y * Scale;
}

void UCommonUtils::SetEnableBloom(UCameraComponent * InCamera, bool InEnableBloom)
{
	auto PostProcessSettings = InCamera->PostProcessSettings;
	PostProcessSettings.bOverride_BloomIntensity = InEnableBloom;
	InCamera->PostProcessSettings = PostProcessSettings;
}

void UCommonUtils::ParseCharacterName(const FString InCharacterName, FString & OutUsername, FString & OutHostname)
{
	// No sense doing this if there's only whitespace
	if (InCharacterName.IsEmpty())
		return;

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

	for (auto Char : NameChars)
	{
		if (!ValidUnixUsernameChars.Contains(FString(1, &Char)))
		{
			InvalidChar = Char;
			break;
		}
	}

	// Did that for loop above change us?
	if (InvalidChar != TEXT('\0'))
	{
		NameString.Split(FString(1, &InvalidChar), &FirstName, &Rem);
	}
	else
	{
		FirstName = NameString;
	}

	OutUsername = FirstName;
	OutHostname = FirstName + TEXT("-pc");
}

float UCommonUtils::GetRotation(FVector2D InA, FVector2D InB)
{
	float adj = InA.X - InB.X;
    float opp = InA.Y - InB.Y;
    return FMath::RadiansToDegrees<float>(FMath::Atan2(opp, adj)/* - FMath::PI*/);
}