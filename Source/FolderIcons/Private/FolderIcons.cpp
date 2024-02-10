// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FolderIcons.h"

#include <ContentBrowserMenuContexts.h>
#include <Interfaces/IPluginManager.h>
#include <ToolMenus.h>

#include "FolderIconsStyle.h"
#include "FolderIconsSubsystem.h"

TArray<FString> FFolderIconsModule::GetIconFoldersOnDisk()
{
	const FString ResourcesFolder = IPluginManager::Get().FindPlugin("FolderIcons")->GetBaseDir() / TEXT("Resources") / TEXT("Icons") + TEXT("/");
	const FString IconsFolder = ResourcesFolder + TEXT("*");

	TArray<FString> FoundDirectories;
	IFileManager::Get().FindFiles(FoundDirectories, *IconsFolder, false, true);

	for (auto& Directory : FoundDirectories)
	{
		Directory.InsertAt(0, ResourcesFolder);
	}

	return FoundDirectories;
}

void FFolderIconsModule::StartupModule()
{
	FToolMenuOwnerScoped ToolMenuOwnerScoped(this);
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu");
	FToolMenuSection& Section = Menu->AddSection("FolderIcons", INVTEXT("Folder Icons"), FToolMenuInsert("PathViewFolderOptions", EToolMenuInsertType::After));

	Section.AddDynamicEntry(
		"FolderIconsSelection",
		FNewToolMenuSectionDelegate::CreateLambda(
			[this](FToolMenuSection& InSection)
			{
				UContentBrowserFolderContext* Context = InSection.FindContext<UContentBrowserFolderContext>();

				InSection.AddSubMenu(
					"FolderIconOptions",
					INVTEXT("Set Folder Icon"),
					INVTEXT("Set a folder's icon"),
					FNewToolMenuChoice(FNewMenuDelegate::CreateRaw(this, &FFolderIconsModule::BuildContextMenu, Context))
				);
			}
		)
	);
}

void FFolderIconsModule::ShutdownModule()
{
	UToolMenus::UnregisterOwner(this);
}

void FFolderIconsModule::BuildContextMenu(FMenuBuilder& MenuBuilder, UContentBrowserFolderContext* Context)
{
	if (!Context || Context->NumAssetPaths <= 0)
	{
		return;
	}

	TArray<FString> IconsAvailable = GetIconFoldersOnDisk();
	for (const FString& IconPath : IconsAvailable)
	{
		const FString DisplayName = FPaths::GetBaseFilename(IconPath);
		MenuBuilder.AddMenuEntry(
			FText::FromString(DisplayName),
			FText::FromString(DisplayName),
			// TODO: Maybe here we can display the raw image as a little thumbnail?
			FSlateIcon(TEXT("FolderIconsStyle"), FName(DisplayName)),
			FUIAction(FExecuteAction::CreateLambda(
				[=, this]()
				{
					UFolderIconsSubsystem* FolderIconsSubsystem = GEditor->GetEditorSubsystem<UFolderIconsSubsystem>();
					FolderIconsSubsystem->SetFoldersIcon(DisplayName, Context->SelectedPackagePaths);
				}
			))
		);
	}
}

IMPLEMENT_MODULE(FFolderIconsModule, FolderIcons)