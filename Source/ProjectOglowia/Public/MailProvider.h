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
#include "Email.h"
#include "FPeacenetIdentity.h"
#include "MailMessage.h"
#include "MailProvider.generated.h"

class USystemContext;
class UPeacenetSaveGame;
class APeacenetWorldStateActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewMailMessage, UMailMessage*, InMessage);

UCLASS(BlueprintType)
class PROJECTOGLOWIA_API UMailProvider : public UObject
{
    GENERATED_BODY()

private:
    UPROPERTY()
    USystemContext* OwningSystem;

public:
    UFUNCTION()
    int GetIdentityID();

    UFUNCTION()
    void Setup(USystemContext* InOwningSystem);

    UFUNCTION()
    UPeacenetSaveGame* GetSaveGame();

    UFUNCTION()
    APeacenetWorldStateActor* GetPeacenet();

    UFUNCTION()
    TArray<FEmail> GetMailMessages();

    UFUNCTION()
    TArray<FEmail> GetInbox();

    UFUNCTION()
    TArray<FEmail> GetOutbox();

    UFUNCTION()
    void SendMailInternal(TArray<int> InRecipients, FString InSubject, FString InMessageText, int InReplyTo = -1);

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mail Provider")
    int GetInboxCount();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mail Provider")
    int GetOutboxCount();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mail Provider")
    int GetMissionsCount();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mail Provider")
    TArray<UMailMessage*> GetMessagesInInbox();

public:
    UFUNCTION()
    void NotifyReceivedMessage(int InMessageID);

public:
    UPROPERTY(BlueprintAssignable)
    FNewMailMessage NewMessageReceived;
};