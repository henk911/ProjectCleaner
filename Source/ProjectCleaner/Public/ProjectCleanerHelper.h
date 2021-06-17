// Copyright 2021. Ashot Barkhudaryan. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FAssetRegistryModule;
struct FSourceCodeFile;

class ProjectCleanerHelper
{
public:
	// query
	static void GetEmptyFolders(TArray<FString>& EmptyFolders, const bool bScanDeveloperContents);
	static void GetProjectFilesFromDisk(TSet<FName>& ProjectFiles);
	static void GetSourceCodeFilesFromDisk(TArray<FSourceCodeFile>& SourceCodeFiles);
	// delete
	static bool DeleteEmptyFolders(const FAssetRegistryModule* AssetRegistry, TArray<FString>& EmptyFolders);
private:
	static bool FindAllEmptyFolders(const FString& FolderPath, TArray<FString>& EmptyFolders);
};