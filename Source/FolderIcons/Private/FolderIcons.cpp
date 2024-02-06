// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FolderIcons.h"

#include <ContentBrowserMenuContexts.h>
#include <Interfaces/IPluginManager.h>
#include <ToolMenus.h>

#include "FolderIconsSubsystem.h"

TArray<FString> FFolderIconsModule::GetFolderIconsOnDisk()
{
	const FString ResourcesFolder = IPluginManager::Get().FindPlugin("FolderIcons")->GetBaseDir() / TEXT("Resources");
	const FString IconsFolder = ResourcesFolder / TEXT("Icons");

	TArray<FString> FoundFiles;
	IFileManager::Get().FindFilesRecursive(FoundFiles, *IconsFolder, TEXT("*.svg"), true, false);

	return FoundFiles;
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

	TArray<FString> IconsAvailable = GetFolderIconsOnDisk();
	for (const FString& IconPath : IconsAvailable)
	{
		const FString DisplayName = FPaths::GetBaseFilename(IconPath);
		MenuBuilder.AddMenuEntry(
			FText::FromString(IconPath),
			FText::FromString(IconPath),
			// TODO: Maybe here we can display the raw image as a little thumbnail?
			FSlateIcon(),
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