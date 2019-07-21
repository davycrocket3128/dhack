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
#include "ConsoleColor.h"

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

void UTerminalEmulator::NativePreConstruct()
{
    // Initialize the pty.
    this->InitializePty();

    // Set rows and columns count.
    this->term.row = this->DefaultRowCount;
    this->term.col = this->DefaultColumnCount;
    
    // Initialize the screen.
    this->InitializeScreen();

    // Measure and set minimum size of the UMG widget.
    float w = 0.f, h = 0.f;
    UCommonUtils::MeasureChar('#', this->DefaultFont, w, h);
    this->SetMinimumDesiredSize(FVector2D(w * this->term.col, h * this->term.row));

    // Hello Blueprint.
    Super::NativePreConstruct();
}

#define MIN(a, b)		((a) < (b) ? (a) : (b))

void UTerminalEmulator::PutTab(int n)
{
    int x = this->term.c.x;
    if (n > 0) {
		while (x < term.col && n--)
			for (++x; x < term.col; ++x)
				/* nothing */ ;
	} else if (n < 0) {
		while (x > 0 && n++)
			for (--x; x > 0; --x)
				/* nothing */ ;
	}
	term.c.x = LIMIT(x, 0, term.col-1);

}

void UTerminalEmulator::Resize(int col, int row)
{
    	int i;
	int minrow = MIN(row, term.row);
	int mincol = MIN(col, term.col);
	FCursor c;

	check(!(col < 1 || row < 1));
	
    /*
	 * slide screen to keep cursor where we expect it -
	 * tscrollup would work here, but we can optimize to
	 * memmove because we're freeing the earlier lines
	 */
	for (i = 0; i <= term.c.y - row; i++) {
		this->term.line[i] = FLine();
	}
	/* ensure that both src and dst are not NULL */
	if (i > 0) {
		for(int j = i; j < i + row; j++)
        {
            this->term.line[j - i] = this->term.line[j];
            this->term.line[j] = FLine();
        }
	}
	for (i += row; i < term.row; i++) {
		this->term.line[i] = FLine();
	}

	/* resize to new height */
	this->term.line.SetNumZeroed(row);
    
	/* resize each row to new width, zero-pad if needed */
	for (i = 0; i < minrow; i++) {
		term.line[i].Resize(col);
	}

	/* allocate any new rows */
	for (/* i = minrow */; i < row; i++) {
		term.line[i] = FLine();
		term.line[i].SetCount(col);
        for(int j = 0; j < col; j++)
        {
            term.line[i][j] = this->term.c.attr;
            term.line[i][j].u = '\0';
        }
	}
	
	/* update terminal size */
	term.col = col;
	term.row = row;
	/* reset scrolling region */
	term.top = 0;
    term.bot = row - 1;
	/* make use of the LIMIT in tmoveto */
	this->MoveTo(term.c.x, term.c.y);
	/* Clearing both screens (it makes dirty all lines) */
	c = term.c;
	for (i = 0; i < 2; i++) {
		if (mincol < col && 0 < minrow) {
			this->ClearRegion(mincol, 0, col - 1, minrow - 1);
		}
		if (0 < col && minrow < row) {
			this->ClearRegion(0, minrow, col - 1, row - 1);
		}
	}
	term.c = c;

}

void UTerminalEmulator::InitializePty()
{
    // Set up the terminal options
    FPtyOptions options;
    options.LFlag = ICANON | ECHO;
    options.C_cc[VERASE] = '\b';
    options.C_cc[VEOL] = '\r';
    options.C_cc[VEOL2] = '\n';

    // Create the pseudo terminal streams.
    UPtyStream::CreatePty(this->Master, this->Slave, options);
}

bool UTerminalEmulator::IsSelected(int x, int y) const
{
    // TODO: Selection support.
    return false;
}

void UTerminalEmulator::DrawGlyph(FTerminalDrawContext* DrawContext, FGlyph glyph, int x, int y) const
{
    if(glyph.mode & (uint16)EGlyphAttribute::ATTR_WDUMMY) return;

    bool reversed = glyph.mode & (uint16)EGlyphAttribute::ATTR_REVERSE;

    FLinearColor bg = (reversed) ? glyph.fg : glyph.bg;
    FLinearColor fg = (reversed) ? glyph.bg : glyph.fg;

    FSlateFontInfo font = this->DefaultFont;

    bool bold = glyph.mode & (uint16)EGlyphAttribute::ATTR_BOLD;
    bool italic = glyph.mode & (uint16)EGlyphAttribute::ATTR_ITALIC;
    
    if(bold && italic)
        font = this->BoldItalicFont;
    else if(bold)
        font = this->BoldFont;
    else if(italic)
        font = this->ItalicFont;

    bool faint = glyph.mode & (uint16)EGlyphAttribute::ATTR_BOLD_FAINT;
    if(faint)
    {
        bg.R *= 0.5f;
        bg.G *= 0.5f;
        bg.B *= 0.5f;
        fg.R *= 0.5f;
        fg.G *= 0.5f;
        fg.B *= 0.5f;
    }


    FString text = FString::Chr(glyph.u);

    float winx = x * cw;
    float winy = y * ch;

    DrawContext->DrawRect(bg, winx, winy, cw, ch);
    if((glyph.mode & (uint16)EGlyphAttribute::ATTR_INVISIBLE) == 0)
    {
        if((glyph.mode & (uint16)EGlyphAttribute::ATTR_BLINK) && !this->ShowBlinking)
            return;
        DrawContext->DrawString(text, font, fg, winx, winy);
        if(glyph.mode & (uint16)EGlyphAttribute::ATTR_UNDERLINE)
        {
            DrawContext->DrawRect(fg, winx, winy + (ch - 2), cw, 2);
        }
    }



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
    if(!this->HasAnyUserFocus()) return;

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

FReply UTerminalEmulator::NativeOnMouseButtonUp( const FGeometry& InGeometry, const FPointerEvent& InMouseEvent )
{
	return FReply::Handled().SetUserFocus(this->TakeWidget());
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

void UTerminalEmulator::TtyRead()
{
    // The string that will be written.
    FString buffer;

    TCHAR c;
    // Keep reading till we run out of characters to read.
    while(this->Slave->ReadChar(c))
    {
        // Append the character to the string IF it's not null.
        if(c == '\0') continue;
        buffer += c;
    }

    // Write to the terminal display!
    this->Write(buffer, false);
}

void UTerminalEmulator::WriteLine(const FText& InText, float InTime)
{
    FTerminalEmulatorWrite write;
    write.Text = FText::Format(NSLOCTEXT("Terminal", "Line", "{0}\r\n"), InText);
    write.Time = InTime;
    this->TextQueue.Add(write);
}

void UTerminalEmulator::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    if(this->TextQueue.Num())
    {
        if(this->TextQueue[0].Time < 0)
        {
            FString str = this->TextQueue[0].Text.ToString();
            for(TCHAR c : str)
                this->Master->WriteChar(c);
            this->TextQueue.RemoveAt(0);
        }
        this->TextQueue[0].Time -= InDeltaTime;
    }

    // Handle blink timing.
    if(this->BlinkingTimeout <= 0.f)
    {
        this->ShowBlinking = true;
    }
    else
    {
        this->BlinkTime += InDeltaTime;
        if(this->BlinkTime >= this->BlinkingTimeout)
        {
            this->ShowBlinking = !this->ShowBlinking;
            this->BlinkTime = 0.f;
        }
    }

    // Get the size of our geometry so we can check if we need to resize.
    FVector2D drawSize = MyGeometry.GetLocalSize();

    // Measure a character so we know how big each cell is.
    float w = 0.f, h = 0.f;
    UCommonUtils::MeasureChar('#', this->DefaultFont, w, h);

    // Get the amount of rows and columns that can fit in our current size.
    int col = FMath::RoundToInt(drawSize.X / w);
    int row = FMath::RoundToInt(drawSize.Y / h);

    // If the size doesn't match, we should resize.
    if(this->term.row != row || this->term.col != col)
    {
        this->Resize(col, row);
    }

    // Read from the tty and write to the terminal.
    this->TtyRead();

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

    this->term.top = 0;
    this->term.bot = this->term.row - 1;

    // Set the cursor color.
    this->term.c.attr.bg = this->BackgroundColor;
    this->term.c.attr.fg = this->ForegroundColor.GetSpecifiedColor();
}

void copyTChar(TCHAR u, char* dest, int i)
{
    memcpy(dest+i, &u, sizeof(TCHAR));
}

void UTerminalEmulator::HandleControlCode(TCHAR c)
{
    switch(c)
    {
        case '\t':
            this->PutTab(1);
            break;
        case '\b': // backspace
            this->MoveTo(term.c.x-1, term.c.y);
            break;
        case '\r':
            this->MoveTo(0, term.c.y);
            break;
        case '\v':
        case '\n':
        case '\f':
            this->NewLine(IS_SET(MODE_CRLF));
            break;
        case '\x1B': // Escape start.
            this->StartEscapeSequence();
            break;
    }
}

void UTerminalEmulator::StartEscapeSequence()
{
    this->escaping = true;
    this->currentArg = "";
    this->escapeArgs.Empty();
    this->escapeMultiLen = false;
    this->escapeType = '\0';
}

void UTerminalEmulator::HandleEscapeChar(TCHAR c)
{
    // Read another char if the current one is a left square brace.
    if(c == '[')
    {
        this->escapeMultiLen = true;
        return;
    }

    // If the character is a digit then we're a multi-length escape code
    // and the digit gets appended to the current argument.
    if(c >= '0' && c <= '9' && this->escapeMultiLen)
    {
        this->currentArg += c;
        return;
    }

    // If we are a semicolon then it's an argument separator.
    if(c == ';' && this->escapeMultiLen)
    {
        // Push the current argument onto the argument array and then
        // clear it.
        this->escapeArgs.Add(FCString::Atoi(*this->currentArg));
        this->currentArg = "";
        return;
    }

    // If we're anything else then the escape sequence is over.
    
    // If there's still an argument we'll quickly push that:
    if(this->currentArg.Len() > 0)
    {
        this->escapeArgs.Add(FCString::Atoi(*this->currentArg));
        this->currentArg = "";
    }

    // Current char becomes type of escape sequence.
    this->escapeType = c;

    // Handle the escape sequence.
    this->HandleEscapeSequence();

    // Reset the escape state.
    this->StartEscapeSequence();
    this->escaping = false;
}

void UTerminalEmulator::ScrollDown(int origin, int n)
{
    int i;
    FLine temp;

    LIMIT(n, 0, term.bot - origin + 1);
    this->ClearRegion(0, term.bot-n+1, term.col-1, term.bot);

    for (i = term.bot; i >= origin+n; i--) {
		temp = term.line[i];
		term.line[i] = term.line[i-n];
		term.line[i-n] = temp;
	}
}

void UTerminalEmulator::UpdateTerminalMode()
{
    for(int a : this->escapeArgs)
    {
        switch(a)
        {
            case 0: // Reset everything.
                this->term.c.attr.mode = 0;
                this->term.c.attr.bg = this->BackgroundColor;
                this->term.c.attr.fg = this->ForegroundColor.GetSpecifiedColor();
                break;
            case 1: // Bright (bold)
                this->term.c.attr.mode |= (uint16)EGlyphAttribute::ATTR_BOLD;
                break;
            case 2: // Dim.
                this->term.c.attr.mode |= (uint16)EGlyphAttribute::ATTR_FAINT;
                break;
            case 3: // Italic.
                this->term.c.attr.mode |= (uint16)EGlyphAttribute::ATTR_ITALIC;
                break;
            case 4: // Underlined.
                this->term.c.attr.mode |= (uint16)EGlyphAttribute::ATTR_UNDERLINE;
                break;
            case 5: // Blinky blinky inky dinky.
                this->term.c.attr.mode |= (uint16)EGlyphAttribute::ATTR_BLINK;
                break;
            case 6: // Blind Man's Dream.
                this->term.c.attr.mode |= (uint16)EGlyphAttribute::ATTR_REVERSE;
                break;
            case 7: // Hidey hidey motherfucker!
                this->term.c.attr.mode |= (uint16)EGlyphAttribute::ATTR_INVISIBLE;
                break;
        }

        if(a >= 40 && a <= 47)
        {
            // Background color
            this->term.c.attr.bg = UCommonUtils::GetConsoleColor((EConsoleColor)(a - 40));
        }

        if(a >= 30 && a <= 37)
        {
            // Foreground color
            this->term.c.attr.fg = UCommonUtils::GetConsoleColor((EConsoleColor)(a - 30));
        }
    }
}

void UTerminalEmulator::HandleEscapeSequence()
{
    switch(escapeType)
    {
        case 'm': // Set Terminal Mode - sets the cursor attributes.
            this->UpdateTerminalMode();
            break;
        case 'H': // Cursor Home
        case 'f': // Force Cursor Position
            // if we're not given a row and column, then move home.
            // Otherwise move to the row and column.
            if(this->escapeArgs.Num() == 2)
            {
                this->MoveTo(this->escapeArgs[1], this->escapeArgs[0]);
            }
            else
            {
                this->MoveTo(0, 0);
            }
            break;
        case 'A': // Move up.
            if(this->escapeArgs.Num())
            {
                this->MoveTo(this->term.c.x, this->term.c.y - this->escapeArgs[0]);
            }
            else
            {
                this->MoveTo(this->term.c.x, this->term.c.y - 1);
            }
            break;
        case 'B':
            if(this->escapeArgs.Num())
            {
                this->MoveTo(this->term.c.x, this->term.c.y + this->escapeArgs[0]);
            }
            else
            {
                this->MoveTo(this->term.c.x, this->term.c.y + 1);
            }
            break;
        case 'C':
            if(this->escapeArgs.Num())
            {
                this->MoveTo(this->term.c.x + this->escapeArgs[0], this->term.c.y);
            }
            else
            {
                this->MoveTo(this->term.c.x + 1, this->term.c.y);
            }
            break;
        case 'D':
            // If we are multi-len this is cursor backward.  Otherwise this scrolls the screen up a line.
            if(!this->escapeMultiLen)
            {
                this->ScrollDown(this->term.top, 1);
                break;
            }

            if(this->escapeArgs.Num())
            {
                this->MoveTo(this->term.c.x - this->escapeArgs[0], this->term.c.y);
            }
            else
            {
                this->MoveTo(this->term.c.x - 1, this->term.c.y);
            }
            break;
        case 's': // Save cursor position.
            this->SavedCursor.x = this->term.c.x;
            this->SavedCursor.y = this->term.c.y;
            break;
        case 'u': // Restore cursor position.
            this->term.c.x = this->SavedCursor.x;
            this->term.c.y = this->SavedCursor.y;
            break;
        case '7': // save cursor with attributes.
            this->SavedCursor = this->term.c;
            break;
        case '8': //unsave cursor and attributes.
            this->term.c = this->SavedCursor;
            break;
        case 'n': // Device status
            switch(this->escapeArgs[0])
            {
                case 6: // Cursor position.
                    this->WriteInput("\x1B[" + FString::FromInt(this->term.c.y) + ";" + FString::FromInt(this->term.c.x) + "R");
                    break;
                case 5: // are we functioning?
                    this->WriteInput("\x1B[0n"); // Yes.
                    break;
            }
            break;
        case 'c': // Reset device.
            this->NativePreConstruct();
            break;
        case 'h':
            if(this->escapeArgs.Num() && this->escapeArgs[0] == 7)
            {
                this->term.mode |= MODE_WRAP;
            }
            break;
        case 'l':
            if(this->escapeArgs.Num() && this->escapeArgs[0] == 7)
            {
                this->term.mode &= ~MODE_WRAP;
            }
            break;
        case 'r': // Scroll Screen
            // If we're given arguments, they're the area at which we're allowed to scroll.
            if(this->escapeArgs.Num() == 2)
            {
                this->term.top = this->escapeArgs[0];
                this->term.bot = this->escapeArgs[1];
            }
            else 
            {
                // Entire screen can be scrolled.
                this->term.top = 0;
                this->term.bot = this->term.row - 1;
            }
            break;
        case 'M': // Scroll up.
            this->ScrollUp(this->term.top, 1);
            break;
        case 'K': // Erasing.
            if(this->escapeArgs.Num() && this->escapeArgs[0] != 0)
            {
                switch(this->escapeArgs[0])
                {
                    case 1: // erase start of line.
                        for(int i = 0; i < this->term.c.x; i++)
                        {
                            this->term.line[this->term.c.y][i] = this->term.c.attr;
                            this->term.line[this->term.c.y][i].u = '\0';
                        }
                        break;
                    case 2: // Entire line.
                        for(int i = 0; i < this->term.col; i++)
                        {
                            this->term.line[this->term.c.y][i] = this->term.c.attr;
                            this->term.line[this->term.c.y][i].u = '\0';
                        }
                        break;
                }
                return;
            }
            // Erase end of line.
            for(int i = this->term.c.x; i < this->term.col; i++)
            {
                this->term.line[this->term.c.y][i] = term.c.attr;
                this->term.line[this->term.c.y][i].u = '\0';
            }
            break;
        case 'J':
            if(this->escapeArgs.Num() && this->escapeArgs[0] != 0)
            {
                switch(this->escapeArgs[0])
                {
                    case 1:
                        this->ClearRegion(0, 0, this->term.col - 1, this->term.row - this->term.c.y - 1);
                        break;
                    case 2:
                        this->ClearRegion(0, 0, this->term.col - 1, this->term.row - 1);
                        this->MoveTo(0, 0);
                        break;
                }
                return;
            }
            this->ClearRegion(0, this->term.c.y, this->term.col - 1, this->term.row - this->term.c.y - 1);
            break;
    }
}

void UTerminalEmulator::WriteInput(FString data)
{
    for(TCHAR c : data)
    {
        this->Slave->WriteChar(c);
    }
}

void UTerminalEmulator::PutChar(TCHAR u)
{
    if(this->escaping)
    {
        this->HandleEscapeChar(u);
        return;
    }
    // Handle control codes.
    if(ISCONTROL(u))
    {
        this->HandleControlCode(u);
        return;
    }

    // For now we'll just get text showing.
    FGlyph* gp = &term.line[term.c.y][term.c.x];

    if(IS_SET(MODE_WRAP) && term.c.state & (uint8)ECursorState::WrapNext)
    {
        // gp->mode |= (uint16)EGlyphAttribute::ATTR_WRAP;
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

FReply UTerminalEmulator::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    

    return FReply::Unhandled();
}

FReply UTerminalEmulator::NativeOnKeyChar(const FGeometry& InGeometry, const FCharacterEvent& InCharEvent)
{
    this->Slave->WriteChar(InCharEvent.GetCharacter());
    return FReply::Handled();
}

FReply UTerminalEmulator::NativeOnFocusReceived(const FGeometry & InGeometry, const FFocusEvent & InFocusEvent)
{
	return FReply::Handled();
}

UConsoleContext* UTerminalEmulator::CreateConsoleContext(UUserContext* PeacegateUser)
{
    check(PeacegateUser);

    UConsoleContext* Console = NewObject<UConsoleContext>();
    Console->Setup(this->Master, PeacegateUser);
    Console->OnTtyUpdate(this, "TtyRead");
    Console->OnGetSize(this, "ReportTerminalSize");

    return Console;
}

void UTerminalEmulator::ReportTerminalSize(int& Rows, int& Cols)
{
    Rows = this->term.row;
    Cols = this->term.col;
}