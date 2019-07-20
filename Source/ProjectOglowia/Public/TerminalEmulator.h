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
#include "Blueprint/UserWidget.h"
#include "TerminalDrawContext.h"
#include "Fonts/SlateFontInfo.h"
#include "PtyStream.h"
#include "ConsoleContext.h"
#include "UserContext.h"
#include "TerminalEmulator.generated.h"

enum term_mode {
	MODE_WRAP        = 1 << 0,
	MODE_INSERT      = 1 << 1,
	MODE_ALTSCREEN   = 1 << 2,
	MODE_CRLF        = 1 << 3,
	MODE_ECHO        = 1 << 4,
	MODE_PRINT       = 1 << 5,
	MODE_UTF8        = 1 << 6,
	MODE_SIXEL       = 1 << 7,
};

UENUM(BlueprintType)
enum class ECursorShape : uint8
{
    Block,
    Bar,
    Underline
};

UENUM()
enum class EGlyphAttribute  : uint16
{
	ATTR_NULL       = 0,
	ATTR_BOLD       = 1 << 0,
	ATTR_FAINT      = 1 << 1,
	ATTR_ITALIC     = 1 << 2,
	ATTR_UNDERLINE  = 1 << 3,
	ATTR_BLINK      = 1 << 4,
	ATTR_REVERSE    = 1 << 5,
	ATTR_INVISIBLE  = 1 << 6,
	ATTR_STRUCK     = 1 << 7,
	ATTR_WRAP       = 1 << 8,
	ATTR_WIDE       = 1 << 9,
	ATTR_WDUMMY     = 1 << 10,
	ATTR_BOLD_FAINT = ATTR_BOLD | ATTR_FAINT,
};

UENUM()
enum class ECursorState : uint8
{
    Default,
    WrapNext,
    Origin
};

USTRUCT()
struct FGlyph
{
    GENERATED_BODY()

public:
	TCHAR u;           /* character code */
	uint16 mode;      /* attribute flags */
	FLinearColor fg;      /* foreground  */
	FLinearColor bg;      /* background  */
};

USTRUCT()
struct FLine
{
    GENERATED_BODY()

private:
    TArray<FGlyph> glyphs;

public:
    void SetCount(int n)
    {
        glyphs.Empty();
        glyphs.AddZeroed(n);
    }

    FGlyph& operator[](int i) { return glyphs[i]; }

    int Num() { return glyphs.Num(); }

    void Resize(int n)
    {
        this->glyphs.SetNumZeroed(n);
    }
};

USTRUCT()
struct FCursor
 {
     GENERATED_BODY()

public:
    UPROPERTY()
	FGlyph attr; /* current char attributes */
	
    UPROPERTY()
    int x;

    UPROPERTY()
	int y;

    UPROPERTY()
    uint8 state;
};

// Internal representation of the screen ported from suckless terminal.
USTRUCT()
struct FTerminalScreen
{
    GENERATED_BODY()

public:
    TArray<TCHAR> trantbl; /* charset table translation */

	UPROPERTY()
    int row;      /* nb row */
	
    UPROPERTY()
    int col;      /* nb col */
	
    UPROPERTY()    
    TArray<FLine> line;   /* screen */
	
    UPROPERTY()
    TArray<FLine> alt;    /* alternate screen */
	
    UPROPERTY()
    TArray<int> dirty;   /* dirtyness of lines */
	
    UPROPERTY()
    FCursor c;    /* cursor */
	
    UPROPERTY()
    int ocx;      /* old cursor col */
	
    UPROPERTY()
    int ocy;      /* old cursor row */
	
    UPROPERTY()
    int top;      /* top    scroll limit */
	
    UPROPERTY()
    int bot;      /* bottom scroll limit */
	
    UPROPERTY()
    uint32 mode = MODE_WRAP;     /* terminal mode flags */
	
    UPROPERTY()
    int esc;      /* escape state flags */
		
    UPROPERTY()
    int charset;  /* current charset */
	
    UPROPERTY()
    int icharset; /* selected charset for sequence */
	
    UPROPERTY()
    TArray<int> tabs;
};

USTRUCT()
struct FCursorDraw
{
    GENERATED_BODY()

public:
    UPROPERTY()
    int cx;

    UPROPERTY()
    int cy;

    UPROPERTY()
    FGlyph g;

    UPROPERTY()
    int ocx;

    UPROPERTY()
    int ocy;

    UPROPERTY()
    FGlyph og;
};

UCLASS(Blueprintable, BlueprintType)
class PROJECTOGLOWIA_API UTerminalEmulator : public UUserWidget
{
    GENERATED_BODY()

private:
    UPROPERTY()
    UPtyStream* Master;

    UPROPERTY()
    bool escaping = false;

    UPROPERTY()
    bool escapeMultiLen = false;

    UPROPERTY()
    TArray<int> escapeArgs;

    TCHAR escapeType;

    UPROPERTY()
    FString currentArg = "";

    UPROPERTY()
    UPtyStream* Slave;

    UPROPERTY()
    FString WordDelimeters = " ";

    UPROPERTY()
    FTerminalScreen term;

    UPROPERTY()
    FCursorDraw cdraw;

    UPROPERTY()
    float cw;

    UPROPERTY()
    float ch;

    UPROPERTY()
    float BlinkTime = 0.f;

    UPROPERTY()
    bool ShowBlinking = true;

    FCursor SavedCursor;

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    float DoubleClickTimeout = 0.3f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    float TripleClickTimeout = 0.6f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    int DefaultRowCount = 24;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    int DefaultColumnCount = 80;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    bool AllowAlternateScreen = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    float BlinkingTimeout = 0.8f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    float CursorThickness = 2;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    float BellVolume = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    int TabSpaces = 8;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    FLinearColor BackgroundColor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    FLinearColor CursorColor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    FLinearColor ReverseCursorColor;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    ECursorShape CursorShape;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Configuration")
    FLinearColor MouseCursorColor;    

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Fonts")
    FSlateFontInfo DefaultFont;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Fonts")
    FSlateFontInfo BoldFont;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Fonts")
    FSlateFontInfo ItalicFont;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Fonts")
    FSlateFontInfo BoldItalicFont;

private:
    UFUNCTION()
    void InitializeScreen();

    UFUNCTION()
    void InitializePty();

    bool IsSelected(int x, int y) const;
    void DrawGlyph(FTerminalDrawContext* DrawContext, FGlyph glyph, int x, int y) const;
    void DrawLine(FTerminalDrawContext* DrawContext, FLine line, int x1, int y, int x2) const;
    void DrawRegion(FTerminalDrawContext* DrawContext, int x1, int y1, int x2, int y2) const;
    void DrawCursor(FTerminalDrawContext* DrawContext) const;
    void PutChar(TCHAR c);
    void NewLine(int firstCol);
    void MoveTo(int x, int y);
    void ScrollUp(int origin, int n);
    void ClearRegion(int x1, int y1, int x2, int y2);
    void SelectionScroll(int origin, int n);
    void SelectionClear();
    void HandleControlCode(TCHAR c);
    void Write(FString InString, bool ShowControl = true);
    void StartEscapeSequence();
    void HandleEscapeChar(TCHAR c);
    void HandleEscapeSequence();
    void WriteInput(FString data);
    void ScrollDown(int origin, int n);
    void UpdateTerminalMode();
    void Resize(int col, int row);

protected:
    UFUNCTION()
    void TtyRead();

    UFUNCTION()
    void ReportTerminalSize(int& Rows, int& Cols);

    // so we can get mouse/keyboard input from UMG
    virtual bool NativeIsInteractable() const override { return true; }
    virtual bool NativeSupportsKeyboardFocus() const override { return true; }
    
    // Handles receiving of user focus.
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

    // Occurs when it's time for the terminal to render - creates a new draw context and starts rendering the terminal with it.
    virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

    // This is where we'll update the terminal's state.
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Occurs before the widget is constructed in the UI.
    virtual void NativePreConstruct() override;

    // Handles keyboard input for the terminal.
    virtual FReply NativeOnKeyChar(const FGeometry& InGeometry, const FCharacterEvent& InCharEvent) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    virtual FReply NativeOnMouseButtonUp( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent ) override;

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Terminal Emulator")
    UConsoleContext* CreateConsoleContext(UUserContext* PeacegateUser);
};