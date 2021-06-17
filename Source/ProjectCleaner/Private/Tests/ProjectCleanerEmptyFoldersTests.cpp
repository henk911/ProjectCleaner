#include "CoreTypes.h"
#include "Misc/AutomationTest.h"
#include "ProjectCleanerHelper.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/Paths.h"


#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectClanerEmptyFoldersQuery, "ProjectCleaner.EmptyFolders.Query", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectClanerEmptyFoldersDelete, "ProjectCleaner.EmptyFolders.Delete", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

struct ProjectCleanerTestLogHelper 
{
	static void PrintEmptyFolders(const TArray<FString>& EmptyFolders)
	{
		for (const auto& Folder : EmptyFolders)
		{
			UE_LOG(LogTemp, Display, TEXT("%s"), *Folder);
		}
	}
};


bool FProjectClanerEmptyFoldersQuery::RunTest(const FString& Parameters)
{
	TArray<FString> EmptyFolders;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// EmptyFolders with 'ScanDeveloperContents' options disabled
	{
		UE_LOG(LogTemp, Display, TEXT("[==================]"));

		PlatformFile.CreateDirectory(*(FPaths::ProjectContentDir() + TEXT("EmptyFolder")));

		// checking if newly created folder founded
		ProjectCleanerHelper::GetEmptyFolders(EmptyFolders, false);
		ProjectCleanerTestLogHelper::PrintEmptyFolders(EmptyFolders);

		TestEqual(TEXT("Empty folders count"), EmptyFolders.Num(), 1);

		// cleanup
		PlatformFile.DeleteDirectory(*(FPaths::ProjectContentDir() + TEXT("EmptyFolder")));
		EmptyFolders.Empty();
	}

	// EmptyFolders with 'ScanDeveloperContents' options enabled
	{
		UE_LOG(LogTemp, Display, TEXT("[==================]"));

		ProjectCleanerHelper::GetEmptyFolders(EmptyFolders, true);
		ProjectCleanerTestLogHelper::PrintEmptyFolders(EmptyFolders);

		TestEqual(TEXT("Empty folders count (with 'ScanDeveloperContent' options enabled)"), EmptyFolders.Num(), 0);

		EmptyFolders.Empty();
	}

	// EmptyFolders hierarchy with 'ScanDeveloperContents' options disabled
	{
		UE_LOG(LogTemp, Display, TEXT("[==================]"));

		// creating folder hierarchy
		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f2/f3")));

		ProjectCleanerHelper::GetEmptyFolders(EmptyFolders, false);
		ProjectCleanerTestLogHelper::PrintEmptyFolders(EmptyFolders);

		TestEqual(TEXT("Empty folders Hierarchy count"), EmptyFolders.Num(), 4);

		// cleanup
		PlatformFile.DeleteDirectoryRecursively(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot")));
		EmptyFolders.Empty();
	}

	{
		UE_LOG(LogTemp, Display, TEXT("[==================]"));

		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f2")));
		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f3")));
		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f4")));
		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f5")));
		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f4/f6")));

		ProjectCleanerHelper::GetEmptyFolders(EmptyFolders, false);

		TestEqual(TEXT("Empty folders Hierarchy nested with multiple folders count"), EmptyFolders.Num(), 7);

		// cleanup
		PlatformFile.DeleteDirectoryRecursively(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot")));
		EmptyFolders.Empty();
	}

	{
		UE_LOG(LogTemp, Display, TEXT("[==================]"));

		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f2/f3/f4/f5/f6/f7/f8/f9/f10/f11/f12/f13/f14/f15/f16/f17/f18/f19/f20")));
		ProjectCleanerHelper::GetEmptyFolders(EmptyFolders, false);
		ProjectCleanerTestLogHelper::PrintEmptyFolders(EmptyFolders);

		TestEqual(TEXT("Nested (20 lvl) Hierarchy empty folders count"), EmptyFolders.Num(), 21);

		// cleanup
		PlatformFile.DeleteDirectoryRecursively(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot")));
		EmptyFolders.Empty();

	}

	// EmptyFolders hierarchy with 'ScanDeveloperContents' options enabled
	{
		UE_LOG(LogTemp, Display, TEXT("[==================]"));

		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("f1/f2/f3")));
		PlatformFile.CreateDirectoryTree(*(FPaths::GameUserDeveloperDir() + TEXT("Collections/f4/f5")));
		PlatformFile.CreateDirectoryTree(*(FPaths::GameUserDeveloperDir() + TEXT("f6/f7/f8")));
		PlatformFile.CreateDirectoryTree(*(FPaths::GameDevelopersDir() + TEXT("f9/f10/f11")));

		ProjectCleanerHelper::GetEmptyFolders(EmptyFolders, true);
		ProjectCleanerTestLogHelper::PrintEmptyFolders(EmptyFolders);

		TestEqual(TEXT("Empty folders nested hierarchy count (with 'ScanDeveloperContent' option enabled)"), EmptyFolders.Num(), 11);

		// cleanup
		PlatformFile.DeleteDirectoryRecursively(*(FPaths::ProjectContentDir() + TEXT("f1/")));
		PlatformFile.DeleteDirectoryRecursively(*(FPaths::GameUserDeveloperDir() + TEXT("Collections/f4/")));
		PlatformFile.DeleteDirectoryRecursively(*(FPaths::GameUserDeveloperDir() + TEXT("f6/")));
		PlatformFile.DeleteDirectoryRecursively(*(FPaths::GameDevelopersDir() + TEXT("f9/")));
		EmptyFolders.Empty();
	}

	return true;
}

bool FProjectClanerEmptyFoldersDelete::RunTest(const FString& Parameters)
{
	TArray<FString> EmptyFolders;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const auto AssetRegistry = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	bool Result = false;
	// Empty Folder creation and deletion check (with "ScanDeveloperContent" option disabled)
	{
		UE_LOG(LogTemp, Display, TEXT("[==================]"));

		PlatformFile.CreateDirectory(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderTest")));
		ProjectCleanerHelper::GetEmptyFolders(EmptyFolders, false);
		ProjectCleanerTestLogHelper::PrintEmptyFolders(EmptyFolders);
		Result = ProjectCleanerHelper::DeleteEmptyFolders(AssetRegistry, EmptyFolders);


		TestTrue(TEXT("Deleting folders result"), Result);
		TestEqual(TEXT("Empty Folders count after deletion must"), EmptyFolders.Num(), 0);

		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f2/f3")));
		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f4")));
		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f5")));
		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f6")));
		PlatformFile.CreateDirectoryTree(*(FPaths::ProjectContentDir() + TEXT("EmptyFolderRoot/f1/f6/f7")));

		ProjectCleanerHelper::GetEmptyFolders(EmptyFolders, false);
		ProjectCleanerTestLogHelper::PrintEmptyFolders(EmptyFolders);
		Result = ProjectCleanerHelper::DeleteEmptyFolders(AssetRegistry, EmptyFolders);

		TestTrue(TEXT("Deleting folder hierarchy result"), Result);
		TestEqual(TEXT("Empty Folders count after deletion must"), EmptyFolders.Num(), 0);
	}

	// Empty Folder creation and deletion check (with "ScanDeveloperContent" option enabled)
	{
		UE_LOG(LogTemp, Display, TEXT("[==================]"));

		PlatformFile.CreateDirectory(*(FPaths::ProjectContentDir() + TEXT("f1")));
		PlatformFile.CreateDirectory(*(FPaths::GameUserDeveloperDir() + TEXT("f2")));
		PlatformFile.CreateDirectory(*(FPaths::GameUserDeveloperDir() + TEXT("Collections/f3")));
		PlatformFile.CreateDirectory(*(FPaths::GameDevelopersDir() + TEXT("f4")));

		ProjectCleanerHelper::GetEmptyFolders(EmptyFolders, true);
		ProjectCleanerTestLogHelper::PrintEmptyFolders(EmptyFolders);
		Result = ProjectCleanerHelper::DeleteEmptyFolders(AssetRegistry, EmptyFolders);

		TestTrue(TEXT("Deleting folders result(with 'ScanDeveloperContent' option enabled)"), Result);
		TestEqual(TEXT("Empty Folders count after deletion must(with 'ScanDeveloperContent' option enabled)"), EmptyFolders.Num(), 0);

	}

	return true;
}

#endif