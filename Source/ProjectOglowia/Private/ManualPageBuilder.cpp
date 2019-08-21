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

#include "ManualPageBuilder.h"

void UManualPageBuilder::SetItemType(const FText& InItemType) {
    this->ManualPage.ItemType = InItemType;
}

void UManualPageBuilder::SetID(FName InManualID) {
    this->ManualPage.ID = InManualID;
}

void UManualPageBuilder::SetFullName(const FText& InFullName) {
    this->ManualPage.FullName = InFullName;
}

void UManualPageBuilder::SetSummary(const FText& InSummary) {
    this->ManualPage.Summary = InSummary;
}

void UManualPageBuilder::SetMetadata(const FText& InMetaName, const FText& InMetaContent) {
    for(auto& Metadata : this->ManualPage.ManualMetadata) {
        if(Metadata.Title.EqualTo(InMetaName)) {
            Metadata.Content = InMetaContent;
            return;
        }
    }

    FManualMetadata NewMeta;
    NewMeta.Title = InMetaName;
    NewMeta.Content = InMetaContent;
    this->ManualPage.ManualMetadata.Add(NewMeta);
}

FManualPage UManualPageBuilder::GetManualPage() {
    return this->ManualPage;
}