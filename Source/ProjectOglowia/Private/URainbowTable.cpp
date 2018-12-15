// Copyright (c) 2018 The Peacenet & Alkaline Thunder.

#include "URainbowTable.h"
#include "USystemContext.h"
#include "UHashFunctions.h"
#include "UPeacegateFileSystem.h"

void URainbowTable::Setup(USystemContext* InSystem, FString InPath, bool InShouldAutoFlush)
{
	// Some safeguards against drunk developers like Alkaline
	check(InSystem);
	check(!InPath.TrimStartAndEnd().IsEmpty());
	check(InPath.StartsWith("/"));
	check(!InPath.EndsWith("/"));

	// Store whether we should auto-flush.
	this->ShouldAutoFlush = InShouldAutoFlush;

	// We get owned by the calling system context.
	this->SystemContext = InSystem;

	// And, we get a root filesystem.
	this->Filesystem = this->SystemContext->GetFilesystem(0);

	// More debug asserts to correct the mistakes of drunk devs.
	check(this->SystemContext);
	check(this->Filesystem);

	// Now set the path for the rainbow table.
	this->RainbowTablePath = InPath;

	// Check to make sure the rainbow table IS NOT A FUCKING FOLDER.
	check(!this->Filesystem->DirectoryExists(this->RainbowTablePath));

	// Load/create the new Rainbow Table database.
	this->ReloadTable();
}

void URainbowTable::AddPassword(FString InPassword)
{
	if (!this->RainbowTable->GetColumnValues("Rainbow Table", "Password").Contains(InPassword))
	{
		TMap<FString, FString> Row;
		Row.Add("Password", InPassword);
		Row.Add("MD5 Hash", UHashFunctions::MD5Hash(InPassword));
		Row.Add("SHA256 Hash", UHashFunctions::SHA256Hash(InPassword));
		Row.Add("CRC Hash", UHashFunctions::CrcHash(InPassword));

		this->RainbowTable->AddRowToTableChecked("Rainbow Table", Row);

		if(this->ShouldAutoFlush)
		{
			this->Flush();
		}
	}
}

void URainbowTable::ReloadTable()
{
	// Now we can check to see if we have a rainbow table to read.
	if (this->Filesystem->FileExists(this->RainbowTablePath))
	{
		// The rainbow table is just a database, like any .db file. So we can use the functions in UDatabase and UDatabaseParser to help us.

		// This value is populated by the filesystem engine for certain operations and allows us to check what really happened with read/write ops. If something went wrong, it'll tell you, instead of asserting... since the player can easily fuck things up.
		EFilesystemStatusCode DatabaseLoadStatus;

		// TODO: Pre-emptive recovery/recreation of the database if this fails. We want to be silent about it, we don't want to assert. The player can break this.
		check(UDatabase::ReadFromFile(this->Filesystem, this->RainbowTablePath, DatabaseLoadStatus, this->RainbowTable));
	}
	else
	{
		// Create a new UDatabase object.
		this->RainbowTable = NewObject<UDatabase>();
	}

	// Update the table format.
	this->UpdateTableFormat();
}

void URainbowTable::UpdateTableFormat()
{
	// Make sure this is in fact a rainbow table.
	if (!this->RainbowTable->TableExists("Rainbow Table"))
	{
		this->RainbowTable->AddTable("Rainbow Table");
	}

	// Add columns for the various hash functions. These don't actually modify the database unless the column doesn't exist.
	this->RainbowTable->AddColumnToTable("Rainbow Table", "Password");
	this->RainbowTable->AddColumnToTable("Rainbow Table", "MD5 Hash");
	this->RainbowTable->AddColumnToTable("Rainbow Table", "SHA256 Hash");
	this->RainbowTable->AddColumnToTable("Rainbow Table", "CRC Hash");

	// Save the rainbow table.
	if(this->ShouldAutoFlush)
	{
		this->Flush();
	}
}

void URainbowTable::Flush()
{
	this->Filesystem->WriteText(this->RainbowTablePath, UDatabaseParser::SerializeDatabase(this->RainbowTable->Tables));
}