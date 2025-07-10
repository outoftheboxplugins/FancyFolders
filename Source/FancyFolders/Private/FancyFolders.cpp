// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

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
	Menu->AddDynamicSection("FancyFolders", FNewToolMenuDelegate::CreateRaw(this, &FFancyFoldersModule::ExtendFolderContextMenu));
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

	if (Context->NumAssetPaths == 1)
	{
		const FString& SinglePath = Context->SelectedPackagePaths[0];

		if (UFancyFoldersSubsystem::Get().HasFolderIcon(SinglePath))
		{
			MenuBuilder.AddMenuEntry(
				INVTEXT("Clear"),
				INVTEXT("Clear the folder's icon."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda(
					[=, this]()
					{
						UFancyFoldersSubsystem::Get().ClearFolderIcon(SinglePath);
					}
				))
			);

			MenuBuilder.AddMenuSeparator();
		}
	}

	TArray<FString> IconsAvailable = GetIconFoldersOnDisk();
	for (const FString& IconPath : IconsAvailable)
	{
		const FString IconName = FPaths::GetBaseFilename(IconPath);
		MenuBuilder.AddMenuEntry(
			FText::FromString(IconName),
			FText::Format(INVTEXT("Set {0} as the folder's icon."), FText::FromString(IconName)),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda(
				[=, this]()
				{
					UFancyFoldersSubsystem::Get().SetFoldersIcon(IconName, Context->SelectedPackagePaths);
				}
			))
		);
	}
}

void FFancyFoldersModule::ExtendFolderContextMenu(UToolMenu* InMenu)
{
	FToolMenuSection& Section = InMenu->FindOrAddSection("PathViewFolderOptions");

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
					FNewToolMenuChoice(FNewMenuDelegate::CreateRaw(this, &FFancyFoldersModule::BuildContextMenu, Context)),
					false,
					FSlateIcon(FAppStyle::Get().GetStyleSetName(), "Icons.FolderOpen")
				);
			}
		)
	);
}

IMPLEMENT_MODULE(FFancyFoldersModule, FancyFolders)