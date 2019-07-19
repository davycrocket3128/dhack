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

#include "TerminalDrawContext.h"
#include "SlateBrush.h"
#include "SlateColorBrush.h"
#include "SlateCore.h"

FVector2D FTerminalDrawContext::GetSize()
{
    return this->AllottedGeometry.GetDrawSize();
}

void FTerminalDrawContext::DrawRect(FLinearColor color, float x, float y, float w, float h)
{
    FSlateBrush brush = FSlateColorBrush(FLinearColor::White);

    FSlateDrawElement::MakeBox(this->DrawElements, this->LayerId, this->AllottedGeometry.ToPaintGeometry(FVector2D(x, y), FVector2D(w, h)), &brush, ESlateDrawEffect::None, color);
    this->LayerId++;
}

void FTerminalDrawContext::DrawString(FString string, FSlateFontInfo font, FLinearColor color, float x, float y)
{
    // Ignore the alpha channel in the event we try to render transparent text
    // (i.e, background is transparent and text attribute has ATTR_REVERSE bit set to 1)
    color.A = 1.f;

    FSlateDrawElement::MakeText(
        this->DrawElements,
		this->LayerId,
		this->AllottedGeometry.ToOffsetPaintGeometry(FVector2D(x, y)),
		FText::FromString(string),
		font,
		ESlateDrawEffect::None,
        color);

    this->LayerId++;
}