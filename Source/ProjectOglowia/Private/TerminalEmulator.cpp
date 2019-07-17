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
 *******************************7*************************************************/

#include "TerminalEmulator.h"
#include "CommonUtils.h"
#include "UnrealString.h"

#include <cstdlib>
#include <cstring>
#include <wchar.h>
#include <ctype.h>
#include <locale>

#define UTF_INVALID   0xFFFD
#define UTF_SIZ       4
#define ESC_BUF_SIZ   (128*UTF_SIZ)
#define ESC_ARG_SIZ   16
#define STR_BUF_SIZ   ESC_BUF_SIZ
#define STR_ARG_SIZ   ESC_ARG_SIZ

#define LIMIT(x, a, b)		(x) = (x) < (a) ? (a) : (x) > (b) ? (b) : (x)
#define BETWEEN(x, a, b)    ((x) >= (a) && (x) <= (b))

#define IS_SET(flag)		((term.mode & (flag)) != 0)
#define ISCONTROLC0(c)		(BETWEEN(c, 0, 0x1f) || (c) == '\177')
#define ISCONTROLC1(c)		(BETWEEN(c, 0x80, 0x9f))
#define ISCONTROL(c)		(ISCONTROLC0(c) || ISCONTROLC1(c))
#define ISDELIM(u)		(u && this->WordDelimiters.Contains(FString::Chr(u)))

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

void UTerminalEmulator::NativePreConstruct()
{
    // Set rows and columns count.
    this->term.row = this->DefaultRowCount;
    this->term.col = this->DefaultColumnCount;
    
    // Initialize the screen.
    this->InitializeScreen();

    // Hello Blueprint.
    Super::NativePreConstruct();

    // Hello world.
    this->Write("Hello, \x1B[31mred\x1B[0m world!");
}

bool UTerminalEmulator::IsSelected(int x, int y) const
{
    // TODO: Selection support.
    return false;
}

void UTerminalEmulator::DrawGlyph(FTerminalDrawContext* DrawContext, FGlyph glyph, int x, int y) const
{
    if(glyph.mode & (uint16)EGlyphAttribute::ATTR_WDUMMY) return;

    bool reversed = glyph.mode && (uint16)EGlyphAttribute::ATTR_REVERSE;

    FLinearColor bg = (reversed) ? glyph.fg : glyph.bg;
    FLinearColor fg = (reversed) ? glyph.bg : glyph.fg;

    FSlateFontInfo font = this->DefaultFont;

    if(glyph.mode & ((uint16)EGlyphAttribute::ATTR_BOLD | (uint16)EGlyphAttribute::ATTR_ITALIC))
    {
        font = this->BoldItalicFont;
    }
    else if(glyph.mode & (uint16)EGlyphAttribute::ATTR_BOLD)
    {
        font = this->BoldFont;
    }
    else if(glyph.mode & (uint16)EGlyphAttribute::ATTR_ITALIC)
    {
        font = this->ItalicFont;
    }

    FString text = FString::Chr(glyph.u);

    float winx = x * cw;
    float winy = y * ch;

    DrawContext->DrawRect(bg, winx, winy, cw, ch);
    DrawContext->DrawString(text, font, fg, winx, winy);

}

void UTerminalEmulator::DrawLine(FTerminalDrawContext* DrawContext, FLine line, int x1, int y, int x2) const
{
    for(int x = x1; x < x2; x++)
    {
        FGlyph glyph = line[x];

        if(this->IsSelected(x, y))
            glyph.mode ^= (uint16)EGlyphAttribute::ATTR_REVERSE;
        
        this->DrawGlyph(DrawContext, glyph, x, y);
    }
}

void UTerminalEmulator::DrawRegion(FTerminalDrawContext* DrawContext, int x1, int y1, int x2, int y2) const
{
    for(int y = y1; y < y2; y++)
    {
        this->DrawLine(DrawContext, this->term.line[y], x1, y, x2);
    }
}

void UTerminalEmulator::DrawCursor(FTerminalDrawContext* DrawContext) const
{
    float winx = cdraw.cx * cw;
    float winy = cdraw.cy * ch;

    bool reversed = (cdraw.g.mode & (uint16)EGlyphAttribute::ATTR_REVERSE);

    FLinearColor bg = (reversed) ? cdraw.g.fg : cdraw.g.bg;
    FLinearColor fg = (reversed) ? cdraw.g.bg : cdraw.g.fg;
    
    DrawContext->DrawRect(bg, winx, winy, cw, ch);

    switch(this->CursorShape)
    {
        case ECursorShape::Block:
            DrawContext->DrawRect(fg, winx, winy, cw, ch);
            break;
        case ECursorShape::Bar:
            DrawContext->DrawRect(fg, winx, winy, this->CursorThickness, ch);
            break;
        case ECursorShape::Underline:
            DrawContext->DrawRect(fg, winx, winy + (ch - this->CursorThickness), cw, this->CursorThickness);
            break;
    }
}

int32 UTerminalEmulator::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    // Create a new draw context containing all of the Slate draw args we were given.
    FTerminalDrawContext* DrawContext = new FTerminalDrawContext(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, bParentEnabled);

    // Draw the screen.
    this->DrawRegion(DrawContext, 0, 0, this->term.col, this->term.row);

    // Draw the cursor.
    this->DrawCursor(DrawContext);

    // Get the layer ID of the draw context.
    LayerId = DrawContext->GetLayerId();

    // Return the layer ID.
    return LayerId;
}

void UTerminalEmulator::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    int cx = this->term.c.x;

    LIMIT(this->term.ocx, 0, this->term.col - 1);
    LIMIT(this->term.ocy, 0, this->term.row - 1);

    if(this->term.line[this->term.ocy][this->term.ocx].mode & (uint16)EGlyphAttribute::ATTR_WDUMMY)
    {
        this->term.ocx--;
    }
	if (term.line[term.c.y][cx].mode & (uint16)EGlyphAttribute::ATTR_WDUMMY)
    {
		cx--;
    }

    cdraw.cx = cx;
    cdraw.cy = term.c.y;
    cdraw.g = term.line[term.c.y][cx];
    cdraw.ocx = term.ocx;
    cdraw.ocy = term.ocy;
    cdraw.og = term.line[term.ocy][term.ocy];

    term.ocx = cx;
    term.ocy = term.c.y;

}

void UTerminalEmulator::InitializeScreen()
{
    // Measure a character so we know the width and height.
    UCommonUtils::MeasureChar(TEXT('#'), this->DefaultFont, this->cw, this->ch);

    // Completely empty the screen lines array.
    this->term.line.Empty();

    // Insert the rows.
    this->term.line.AddDefaulted(this->term.row);

    // Allocate all of the columns.
    for(int i = 0; i < this->term.row; i++)
    {
        this->term.line[i].SetCount(this->term.col);
        for(int j = 0; j < term.col; j++)
        {
            this->term.line[i][j].bg = this->BackgroundColor;
            this->term.line[i][j].fg = this->ForegroundColor.GetSpecifiedColor();
        }
    }

    // Set the cursor position to 0,0.
    this->term.c.x = 0;
    this->term.c.y = 0;

    // Set the cursor color.
    this->term.c.attr.bg = this->BackgroundColor;
    this->term.c.attr.fg = this->ForegroundColor.GetSpecifiedColor();
}

void copyTChar(TCHAR u, char* dest, int i)
{
    memcpy(dest+i, &u, sizeof(TCHAR));
}

void UTerminalEmulator::PutChar(TCHAR u)
{
    // TODO - ANSI escape sequences.

    // For now we'll just get text showing.
    FGlyph* gp = &term.line[term.c.y][term.c.x];

    if(IS_SET(MODE_WRAP) && term.c.state & (uint8)ECursorState::WrapNext)
    {
        gp->mode |= (uint16)EGlyphAttribute::ATTR_WRAP;
        this->NewLine(1);
        gp = &term.line[term.c.y][term.c.x];
    }

    if(IS_SET(MODE_INSERT) && term.c.x + 1 < term.col)
    {
        memmove(gp+1, gp, (term.col - term.c.x - 1) * sizeof(FGlyph));
    }

    if (term.c.x+1 > term.col) 
    {
		this->NewLine(1);
		gp = &term.line[term.c.y][term.c.x];
	}

    term.line[term.c.y][term.c.x] = term.c.attr;
    term.line[term.c.y][term.c.x].u = u;

 	if (term.c.x+1 < term.col) {
		this->MoveTo(term.c.x+1, term.c.y);
	} else {
		term.c.state |= (uint8)ECursorState::WrapNext;
    }
}

void UTerminalEmulator::NewLine(int firstCol)
{
    int y = this->term.c.y;

	if (y == this->term.bot) {
		this->ScrollUp(term.top, 1);
	} else {
		y++;
	}
	this->MoveTo(firstCol ? 0 : term.c.x, y);
}

void UTerminalEmulator::MoveTo(int x, int y)
{
    	int miny, maxy;

	if (term.c.state & (uint8)ECursorState::Origin) {
		miny = term.top;
		maxy = term.bot;
	} else {
		miny = 0;
		maxy = term.row - 1;
	}
	term.c.state &= ~(uint8)ECursorState::WrapNext;
	term.c.x = LIMIT(x, 0, term.col-1);
	term.c.y = LIMIT(y, miny, maxy);
}

void UTerminalEmulator::ScrollUp(int origin, int n)
{
    int i;
	FLine temp;

	LIMIT(n, 0, term.bot-origin+1);

	this->ClearRegion(0, origin, term.col-1, origin+n-1);
	
	for (i = origin; i <= term.bot-n; i++) {
		temp = term.line[i];
		term.line[i] = term.line[i+n];
		term.line[i+n] = temp;
	}

	this->SelectionScroll(origin, -n);
}

void UTerminalEmulator::SelectionScroll(int orig, int n)
{
    // TODO
}

void UTerminalEmulator::SelectionClear()
{
    // TODO
}

void UTerminalEmulator::ClearRegion(int x1, int y1, int x2, int y2)
{
    int x, y, temp;
	FGlyph *gp;

	if (x1 > x2)
		temp = x1, x1 = x2, x2 = temp;
	if (y1 > y2)
		temp = y1, y1 = y2, y2 = temp;

	LIMIT(x1, 0, term.col-1);
	LIMIT(x2, 0, term.col-1);
	LIMIT(y1, 0, term.row-1);
	LIMIT(y2, 0, term.row-1);

	for (y = y1; y <= y2; y++) {
		term.dirty[y] = 1;
		for (x = x1; x <= x2; x++) {
			gp = &term.line[y][x];
			if (this->IsSelected(x, y))
				this->SelectionClear();
			gp->fg = term.c.attr.fg;
			gp->bg = term.c.attr.bg;
			gp->mode = 0;
			gp->u = ' ';
		}
	}
}

void UTerminalEmulator::Write(FString InString, bool ShowControl)
{
    for(int i = 0; i < InString.Len(); i++)
    {
        TCHAR c = InString[i];
        if(ShowControl && ISCONTROL(c))
        {
            if(c & 0x80)
            {
                c &= 0x7f;
                this->PutChar('^');
                this->PutChar('[');
            }
            else if(c != '\r' && c != '\n' && c != '\t')
            {
                c ^= 0x40;
                this->PutChar('^');
            }
        }
        this->PutChar(c);
    }
}

FReply UTerminalEmulator::NativeOnKeyChar(const FGeometry& InGeometry, const FCharacterEvent& InCharEvent)
{
    this->PutChar(InCharEvent.GetCharacter());
    return FReply::Handled();
}