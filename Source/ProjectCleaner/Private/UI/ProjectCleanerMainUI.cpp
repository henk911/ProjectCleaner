﻿// Copyright 2021. Ashot Barkhudaryan. All Rights Reserved.

#include "UI/ProjectCleanerMainUI.h"
#include "UI/ProjectCleanerStyle.h"
#include "UI/ProjectCleanerStatisticsUI.h"
#include "UI/ProjectCleanerUnusedAssetsBrowserUI.h"
#include "UI/ProjectCleanerNonEngineFilesUI.h"
#include "UI/ProjectCleanerConfigsUI.h"
#include "UI/ProjectCleanerCorruptedFilesUI.h"
#include "UI/ProjectCleanerExcludeOptionsUI.h"
#include "UI/ProjectCleanerIndirectAssetsUI.h"
#include "UI/ProjectCleanerExcludedAssetsUI.h"
#include "StructsContainer.h"
// Engine Headers
#include "ToolMenus.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/SlateWrapperTypes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Framework/Docking/TabManager.h"
#include "UI/ProjectCleanerNotificationManager.h"
#include "Widgets/Notifications/SProgressBar.h"

static const FName UnusedAssetsTab = FName{ TEXT("UnusedAssetsTab") };
static const FName IndirectAssetsTab = FName{ TEXT("IndirectAssetsTab") };
static const FName NonEngineFilesTab = FName{ TEXT("NonEngineFilesTab") };
static const FName CorruptedFilesTab = FName{ TEXT("CorruptedFilesTab") };
static const FName ExcludedAssetsTab = FName{ TEXT("ExcludedAssetsTab") };

void SProjectCleanerMainUI::Construct(const FArguments& InArgs)
{
	if (InArgs._DataManager)
	{
		DataManager = InArgs._DataManager;
	}
	
	InitTabs();

	ensure(DataManager);
	DataManager->Update();

	ensure(TabManager.IsValid());

	const TSharedRef<SWidget> TabContents = TabManager->RestoreFrom(
		TabLayout.ToSharedRef(),
		TSharedPtr<SWindow>()
	).ToSharedRef();
	
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(FMargin{ 20.0f })
		.FillHeight(1.0f)
		[
			SNew(SSplitter)
			.Style(FEditorStyle::Get(), "ContentBrowser.Splitter")
			.PhysicalSplitterHandleSize(5.0f)
			+ SSplitter::Slot()
			.Value(0.35f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SScrollBox)
					.ScrollWhenFocusChanges(EScrollWhenFocusChanges::AnimatedScroll)
					.AnimateWheelScrolling(true)
					.AllowOverscroll(EAllowOverscroll::No)
					+ SScrollBox::Slot()
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.Padding(FMargin{ 20.0f, 20.0f })
							.AutoHeight()
							[
								SAssignNew(StatisticsUI, SProjectCleanerStatisticsUI)
								.DataManager(DataManager)
							]
							+ SVerticalBox::Slot()
							.Padding(FMargin{ 20.0f, 20.0f })
							.AutoHeight()
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SButton)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.Text(FText::FromString("Refresh"))
									.OnClicked_Raw(this, &SProjectCleanerMainUI::OnRefreshBtnClick)
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.Padding(FMargin{ 40.0f, 0.0f, 40.0f, 0.0f })
								[
									SNew(SButton)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.Text(FText::FromString("Delete Unused Assets"))
									.OnClicked_Raw(this, &SProjectCleanerMainUI::OnDeleteUnusedAssetsBtnClick)
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SButton)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.Text(FText::FromString("Delete Empty Folders"))
									.OnClicked_Raw(this, &SProjectCleanerMainUI::OnDeleteEmptyFolderClick)
								]
							]
							+ SVerticalBox::Slot()
							.Padding(FMargin{20.0f, 5.0f})
							.AutoHeight()
							[
								SAssignNew(CleanerConfigsUI, SProjectCleanerConfigsUI)
								.CleanerConfigs(DataManager->GetCleanerConfigs())
							]
							+ SVerticalBox::Slot()
							.Padding(FMargin{20.0f, 5.0f})
							.AutoHeight()
							[
								SAssignNew(ExcludeOptionUI, SProjectCleanerExcludeOptionsUI)
								.ExcludeOptions(DataManager->GetExcludeOptions())
							]
						]
					]
				]
			]
			+ SSplitter::Slot()
			[
				TabContents
			]
		]
	];
}

SProjectCleanerMainUI::~SProjectCleanerMainUI()
{
	TabManager->UnregisterTabSpawner(UnusedAssetsTab);
	TabManager->UnregisterTabSpawner(IndirectAssetsTab);
	TabManager->UnregisterTabSpawner(NonEngineFilesTab);
	TabManager->UnregisterTabSpawner(CorruptedFilesTab);
	TabManager->UnregisterTabSpawner(ExcludedAssetsTab);
}

void SProjectCleanerMainUI::InitTabs()
{
	const auto DummyTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	TabManager = FGlobalTabmanager::Get()->NewTabManager(DummyTab);
	TabManager->SetCanDoDragOperation(false);
	TabLayout = FTabManager::NewLayout("ProjectCleanerTabLayout")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.4f)
			->AddTab(UnusedAssetsTab, ETabState::OpenedTab)
			->AddTab(ExcludedAssetsTab, ETabState::OpenedTab)
			->AddTab(IndirectAssetsTab, ETabState::OpenedTab)
			->AddTab(NonEngineFilesTab, ETabState::OpenedTab)
			->AddTab(CorruptedFilesTab, ETabState::OpenedTab)
			->SetForegroundTab(UnusedAssetsTab)
		)
	);
	
	TabManager->RegisterTabSpawner(UnusedAssetsTab, FOnSpawnTab::CreateRaw(
		this,
		&SProjectCleanerMainUI::OnUnusedAssetTabSpawn)
	);
	TabManager->RegisterTabSpawner(ExcludedAssetsTab, FOnSpawnTab::CreateRaw(
		this,
		&SProjectCleanerMainUI::OnExcludedAssetsTabSpawn)
	);
	TabManager->RegisterTabSpawner(IndirectAssetsTab, FOnSpawnTab::CreateRaw(
		this,
		&SProjectCleanerMainUI::OnIndirectAssetsTabSpawn)
	);
	TabManager->RegisterTabSpawner(NonEngineFilesTab, FOnSpawnTab::CreateRaw(
		this,
		&SProjectCleanerMainUI::OnNonEngineFilesTabSpawn)
	);
	TabManager->RegisterTabSpawner(CorruptedFilesTab, FOnSpawnTab::CreateRaw(
		this,
		&SProjectCleanerMainUI::OnCorruptedFilesTabSpawn)
	);
}

void SProjectCleanerMainUI::Update() const
{
	// CleanerManager->UpdateData();
	// UpdateUIData();
}

void SProjectCleanerMainUI::UpdateUIData() const
{
	// if (StatisticsUI.IsValid())
	// {
	// 	StatisticsUI.Pin()->SetData(CleanerManager->GetCleanerData());
	// }
	//
	// if (UnusedAssetsBrowserUI.IsValid())
	// {
	// 	UnusedAssetsBrowserUI.Pin()->SetUIData(
	// 		CleanerManager->GetCleanerData().UnusedAssets,
	// 		CleanerManager->GetCleanerConfigs(),
	// 		CleanerManager->GetCleanerData().PrimaryAssetClasses
	// 	);
	// }
	//
	// if (NonEngineFilesUI.IsValid())
	// {
	// 	// NonEngineFilesUI.Pin()->SetNonEngineFiles(CleanerManager->GetCleanerData().NonEngineFiles);
	// }
	//
	// if (CorruptedFilesUI.IsValid())
	// {
	// 	// CorruptedFilesUI.Pin()->SetCorruptedFiles(CleanerManager->GetCleanerData().CorruptedAssets);
	// }
	//
	// if (IndirectAssetsUI.IsValid())
	// {
	// 	// IndirectAssetsUI.Pin()->SetIndirectFiles(CleanerManager->GetCleanerData().IndirectFileInfos);
	// }
	//
	// if (ExcludedAssetsUI.IsValid())
	// {
	// 	// ExcludedAssetsUI.Pin()->SetUIData(
	// 	// 	CleanerManager->GetCleanerData().ExcludedAssets,
	// 	// 	CleanerManager->GetCleanerData().LinkedAssets,
	// 	// 	CleanerManager->GetCleanerData().PrimaryAssetClasses,
	// 	// 	CleanerManager->GetCleanerConfigs()
	// 	// );
	// }
}

TSharedRef<SDockTab> SProjectCleanerMainUI::OnUnusedAssetTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	const auto UnusedAssetsUIRef =
		SAssignNew(UnusedAssetsBrowserUI, SProjectCleanerUnusedAssetsBrowserUI)
		.DataManager(DataManager);
	
	// UnusedAssetsUIRef->OnUserDeletedAssets = FOnUserDeletedAssets::CreateRaw(
	// 	this,
	// 	&SProjectCleanerMainUI::OnUserDeletedAssets
	// );
	//
	// UnusedAssetsUIRef->OnUserExcludedAssets = FOnUserExcludedAssets::CreateRaw(
	// 	this,
	// 	&SProjectCleanerMainUI::OnUserExcludedAssets
	// );
	//
	// UnusedAssetsUIRef->OnUserExcludedAssetsOfType = FOnUserExcludedAssetsOfType::CreateRaw(
	// 	this,
	// 	&SProjectCleanerMainUI::OnUserExcludedAssetsOfType
	// );
	
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.OnCanCloseTab_Lambda([] { return false; })
		.Label(NSLOCTEXT("UnusedAssetsTab", "TabTitle", "Unused Assets"))
		.ShouldAutosize(true)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.Padding(20.0f)
			[
				UnusedAssetsUIRef
			]
		];
}

TSharedRef<SDockTab> SProjectCleanerMainUI::OnExcludedAssetsTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	// const auto ExcludedAssetsUIRef = SAssignNew(ExcludedAssetsUI, SProjectCleanerExcludedAssetsUI)
	// // .ExcludedAssets(&CleanerManager->GetCleanerData().ExcludedAssets)
	// // .LinkedAssets(&CleanerManager->GetCleanerData().LinkedAssets)
	// .PrimaryAssetClasses(&CleanerManager->GetCleanerData().PrimaryAssetClasses)
	// .CleanerConfigs(CleanerManager->GetCleanerConfigs());
	//
	// // ExcludedAssetsUIRef->OnUserIncludedAssets = FOnUserIncludedAsset::CreateRaw(
	// // 	this,
	// // 	&SProjectCleanerMainUI::OnUserIncludedAssets
	// // );
	//
	// return SNew(SDockTab)
	// 	.TabRole(ETabRole::NomadTab)
	// 	.OnCanCloseTab_Lambda([] { return false; })
	// 	.Label(NSLOCTEXT("ExcludedAssetsTab", "TabTitle", "Excluded Assets"))
	// 	.ShouldAutosize(true)
	// 	[
	// 		SNew(SOverlay)
	// 		+ SOverlay::Slot()
	// 		.Padding(20.0f)
	// 		[
	// 			ExcludedAssetsUIRef
	// 		]
	// 	];
	return SNew(SDockTab);
}

TSharedRef<SDockTab> SProjectCleanerMainUI::OnNonEngineFilesTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	// return SNew(SDockTab)
	// 	.TabRole(ETabRole::PanelTab)
	// 	.OnCanCloseTab_Lambda([] {return false; })
	// 	.Label(NSLOCTEXT("NonEngineFilesTab", "TabTitle", "Non Engine Files"))
	// 	[
	// 		SAssignNew(NonEngineFilesUI, SProjectCleanerNonEngineFilesUI)
	// 		// .NonEngineFiles(&CleanerManager->GetCleanerData().NonEngineFiles)
	// 	];
	return SNew(SDockTab);
}

TSharedRef<SDockTab> SProjectCleanerMainUI::OnCorruptedFilesTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	// return SNew(SDockTab)
	// 	.TabRole(ETabRole::PanelTab)
	// 	.OnCanCloseTab_Lambda([] {return false; })
	// 	.Label(NSLOCTEXT("CorruptedFilesTab", "TabTitle", "Corrupted Files"))
	// 	[
	// 		SAssignNew(CorruptedFilesUI, SProjectCleanerCorruptedFilesUI)
	// 		// .CorruptedFiles(&CleanerManager->GetCleanerData().CorruptedAssets)
	// 	];
	return SNew(SDockTab);
}

TSharedRef<SDockTab> SProjectCleanerMainUI::OnIndirectAssetsTabSpawn(const FSpawnTabArgs& SpawnTabArgs)
{
	// return SNew(SDockTab)
	// 	.TabRole(ETabRole::PanelTab)
	// 	.OnCanCloseTab_Lambda([] {return false; })
	// 	.Label(NSLOCTEXT("IndirectAssetsTab", "TabTitle", "Assets Used Indirectly"))
	// 	[
	// 		SAssignNew(IndirectAssetsUI, SProjectCleanerIndirectAssetsUI)
	// 		// .IndirectFileInfos(&CleanerManager->GetCleanerData().IndirectFileInfos)
	// 	];
	return SNew(SDockTab);
}

void SProjectCleanerMainUI::OnUserDeletedAssets() const
{
	Update();
}

void SProjectCleanerMainUI::OnUserExcludedAssets(const TArray<FAssetData>& Assets) const
{
	if (!Assets.Num()) return;
	
	// for (const auto& Asset : Assets)
	// {
	// 	// CleanerManager->GetCleanerData().UserExcludedAssets.AddUnique(Asset);
	// }
	
	// Update();
}

void SProjectCleanerMainUI::OnUserIncludedAssets(const TArray<FAssetData>& Assets, const bool CleanFilters) const
{
	// if (!Assets.Num()) return;
	//
	// if (CleanFilters)
	// {
	// 	// CleanerManager->GetCleanerData().UserExcludedAssets.Empty();
	// 	// CleanerManager->GetCleanerData().ExcludedAssets.Empty();
	// 	// CleanerManager->GetCleanerData().LinkedAssets.Empty();
	// 	// CleanerManager->GetExcludeOptions()->Classes.Empty();
	// 	// CleanerManager->GetExcludeOptions()->Paths.Empty();
	// }
	// else
	// {
	// 	// if user tries to include assets that are filtered by path or class,
	// 	// we wont include them and notify user about it
	// 	TArray<FAssetData> AssetsExcludedByFilter;
	// 	bool ShowNotification = false;
	// 	for (const auto& Asset : Assets)
	// 	{
	// 		// if (CleanerManager->IsExcludedByClass(Asset))
	// 		// {
	// 		// 	AssetsExcludedByFilter.AddUnique(Asset);
	// 		// 	ShowNotification = true;
	// 		// }
	// 		//
	// 		// if (CleanerManager->IsExcludedByPath(Asset))
	// 		// {
	// 		// 	ShowNotification = true;
	// 		// 	AssetsExcludedByFilter.AddUnique(Asset);
	// 		// }
	// 	}
	//
	// 	if (ShowNotification)
	// 	{
	// 		ProjectCleanerNotificationManager::AddTransient(
	// 			FText::FromString(FStandardCleanerText::CantIncludeSomeAssets),
	// 			SNotificationItem::CS_None,
	// 			3.0f
	// 		);
	// 	}
	// 	
	// 	// CleanerManager->GetCleanerData().UserExcludedAssets.RemoveAllSwap([&](const FAssetData& Elem)
	// 	// {
	// 	// 	return Assets.Contains(Elem) && !AssetsExcludedByFilter.Contains(Elem);
	// 	// });
	// }
	//
	// Update();
}

void SProjectCleanerMainUI::OnUserExcludedAssetsOfType(const TArray<FAssetData>& Assets) const
{
	// if (!Assets.Num()) return;
	//
	// for (const auto& Asset : Assets)
	// {
	// 	const auto LoadedAsset = Asset.GetAsset();
	// 	const auto BlueprintAsset = Cast<UBlueprint>(LoadedAsset);
	// 	if (BlueprintAsset && BlueprintAsset->GeneratedClass)
	// 	{
	// 		CleanerManager->GetExcludeOptions()->Classes.AddUnique(BlueprintAsset->GeneratedClass);
	// 	}
	// 	else
	// 	{
	// 		auto AssetClass = Asset.GetClass();
	// 		if(!AssetClass) continue;
	// 		CleanerManager->GetExcludeOptions()->Classes.AddUnique(AssetClass);
	// 	}
	// }
	//
	// Update();
}

FReply SProjectCleanerMainUI::OnRefreshBtnClick() const
{
	// Update();
	DataManager->Update();
	return FReply::Handled();
}

FReply SProjectCleanerMainUI::OnDeleteUnusedAssetsBtnClick() const
{
	// if (CleanerManager->GetCleanerData().UnusedAssets.Num() == 0)
	// {
	// 	ProjectCleanerNotificationManager::AddTransient(
	// 		FText::FromString(FStandardCleanerText::NoAssetsToDelete),
	// 		SNotificationItem::ECompletionState::CS_Fail,
	// 		3.0f
	// 	);
	//
	// 	return FReply::Handled();
	// }
	//
	// const auto ConfirmationWindowStatus = ShowConfirmationWindow(
	// 	FText::FromString(FStandardCleanerText::AssetsDeleteWindowTitle),
	// 	FText::FromString(FStandardCleanerText::AssetsDeleteWindowContent)
	// );
	// if (IsConfirmationWindowCanceled(ConfirmationWindowStatus))
	// {
	// 	return FReply::Handled();
	// }
	//
	// // CleanerManager->DeleteUnusedAssets();
	// UpdateUIData();
	
	return FReply::Handled();
}

FReply SProjectCleanerMainUI::OnDeleteEmptyFolderClick() const
{
	// if (CleanerManager->GetCleanerData().EmptyFolders.Num() == 0)
	// {
	// 	ProjectCleanerNotificationManager::AddTransient(
	// 		FText::FromString(FStandardCleanerText::NoEmptyFolderToDelete),
	// 		SNotificationItem::ECompletionState::CS_Fail,
	// 		3.0f
	// 	);
	// 	
	// 	return FReply::Handled();
	// }
	//
	// const auto ConfirmationWindowStatus = ShowConfirmationWindow(
	// 	FText::FromString(FStandardCleanerText::EmptyFolderWindowTitle),
	// 	FText::FromString(FStandardCleanerText::EmptyFolderWindowContent)
	// );
	// if (IsConfirmationWindowCanceled(ConfirmationWindowStatus))
	// {
	// 	return FReply::Handled();
	// }
	//
	// // CleanerManager->DeleteEmptyFolders();
	// UpdateUIData();

	return FReply::Handled();
}

EAppReturnType::Type SProjectCleanerMainUI::ShowConfirmationWindow(const FText& Title, const FText& ContentText) const
{
	return FMessageDialog::Open(
		EAppMsgType::YesNo,
		ContentText,
		&Title
	);
}

bool SProjectCleanerMainUI::IsConfirmationWindowCanceled(EAppReturnType::Type Status)
{
	return Status == EAppReturnType::Type::No || Status == EAppReturnType::Cancel;
}