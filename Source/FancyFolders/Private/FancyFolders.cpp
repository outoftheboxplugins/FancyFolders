// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FancyFolders.h"

#include <ContentBrowserMenuContexts.h>
#include <Interfaces/IPluginManager.h>
#include <ToolMenus.h>

#include "FancyFoldersSettings.h"
#include "FancyFoldersSubsystem.h"

TArray<FString> FFancyFoldersModule::GetIconFoldersOnDisk()
{
	const FString ResourcesFolder = IPluginManager::Get().FindPlugin("FancyFolders")->GetBaseDir() / TEXT("Resources") / TEXT("Icons") + TEXT("/");
	const FString IconsFolder = ResourcesFolder + TEXT("*");

	TArray<FString> FoundDirectories;
	IFileManager::Get().FindFiles(FoundDirectories, *IconsFolder, false, true);

	for (auto& Directory : FoundDirectories)
	{
		Directory.InsertAt(0, ResourcesFolder);
	}

	return FoundDirectories;
}

void FFancyFoldersModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("FolderData", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FFolderDataCustomization::MakeInstance));

	FToolMenuOwnerScoped ToolMenuOwnerScoped(this);
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu");
	FToolMenuSection& Section = Menu->AddSection("FancyFolders", INVTEXT("Folder Icons"), FToolMenuInsert("PathViewFolderOptions", EToolMenuInsertType::After));

	Section.AddDynamicEntry(
		"FancyFoldersSelection",
		FNewToolMenuSectionDelegate::CreateLambda(
			[this](FToolMenuSection& InSection)
			{
				UContentBrowserFolderContext* Context = InSection.FindContext<UContentBrowserFolderContext>();

				InSection.AddSubMenu(
					"FolderIconOptions",
					INVTEXT("Set Folder Icon"),
					INVTEXT("Set a folder's icon"),
					FNewToolMenuChoice(FNewMenuDelegate::CreateRaw(this, &FFancyFoldersModule::BuildContextMenu, Context))
				);
			}
		)
	);
}

void FFancyFoldersModule::ShutdownModule()
{
	UToolMenus::UnregisterOwner(this);

	if (FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
	{
		PropertyModule->UnregisterCustomPropertyTypeLayout("FolderData");
	}
}

void FFancyFoldersModule::BuildContextMenu(FMenuBuilder& MenuBuilder, UContentBrowserFolderContext* Context)
{
	if (!Context || Context->NumAssetPaths <= 0)
	{
		return;
	}

	TArray<FString> IconsAvailable = GetIconFoldersOnDisk();
	for (const FString& IconPath : IconsAvailable)
	{
		const FString IconName = FPaths::GetBaseFilename(IconPath);
		MenuBuilder.AddMenuEntry(
			FText::FromString(IconName),
			FText::FromString(IconName), // TODO: Improve tooltip message
			FSlateIcon(), // TODO: Maybe here we can display the raw image as a little thumbnail?
			FUIAction(FExecuteAction::CreateLambda(
				[=, this]()
				{
					UFancyFoldersSubsystem::Get().SetFoldersIcon(IconName, Context->SelectedPackagePaths);
				}
			))
		);
	}
}

IMPLEMENT_MODULE(FFancyFoldersModule, FancyFolders)