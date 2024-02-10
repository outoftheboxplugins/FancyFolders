// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FolderIconsSubsystem.h"

#include <Algo/Compare.h>
#include <ContentBrowserDataSubsystem.h>
#include <ContentBrowserItemData.h>
#include <ContentBrowserModule.h>
#include <IContentBrowserDataModule.h>

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "FolderIconsSettings.h"
#include "FolderIconsStyle.h"
#include "Interfaces/IMainFrameModule.h"
#include "SAssetView.h"

namespace
{
	/**
	 * Converts a virtual path such as /All/Plugins -> /Plugins or /All/Game -> /Game
	 */
	FString ConvertVirtualPathToInvariantPathString(const FString& VirtualPath)
	{
		FName ConvertedPath;
		IContentBrowserDataModule::Get().GetSubsystem()->TryConvertVirtualPath(FName(VirtualPath), ConvertedPath);
		return ConvertedPath.ToString();
	}
} // namespace

UFolderIconsSubsystem& UFolderIconsSubsystem::Get()
{
	return *GEditor->GetEditorSubsystem<UFolderIconsSubsystem>();
}

void UFolderIconsSubsystem::SetFoldersIcon(const FString& Icon, TArray<FString> Folders)
{
	UFolderIconsSettings* Settings = GetMutableDefault<UFolderIconsSettings>();
	for (const auto& Folder : Folders)
	{
		// TODO: This should take the color the folder currently has instead of hardcoded red
		// TODO: We need some way to also listen for color changes so they can be shared between users
		Settings->Assigned.Emplace(Folder, FLinearColor::Red, FName(Icon));
	}
}

void UFolderIconsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FSlateApplication& SlateApp = FSlateApplication::Get();
	SlateApp.OnPostTick().AddUObject(this, &ThisClass::OnPostTick);
}

void UFolderIconsSubsystem::OnPostTick(float DeltaTime)
{
	// TODO: Check if any of the AssetViewWidgets changed their type
	for (const TSharedRef<SAssetView>& AssetView : AssetViewWidgets)
	{
	}

	RefreshAllFolders();
}

const FSlateBrush* UFolderIconsSubsystem::GetIconForFolder(const FString& VirtualPath, bool bIsColumnView) const
{
	const UFolderIconsSettings* const Settings = GetDefault<UFolderIconsSettings>();

	for (const FFolderIconPreset& Assigned : Settings->Assigned)
	{
		if (Assigned.FolderName == VirtualPath)
		{
			return Assigned.GetIcon(bIsColumnView);
		}
	}

	for (const FFolderIconPreset& Preset : Settings->Presets)
	{
		// TODO: Maybe in the future, support regex matching?
		const FString FolderName = FPaths::GetBaseFilename(VirtualPath);
		if (FolderName == Preset.FolderName)
		{
			return Preset.GetIcon(bIsColumnView);
		}
	}

	const FSlateBrush* NormalViewIcon = FAppStyle::GetBrush("ContentBrowser.ListViewFolderIcon");
	const FSlateBrush* ColumnViewIcon = FAppStyle::GetBrush("ContentBrowser.ColumnViewFolderIcon");

	return bIsColumnView ? ColumnViewIcon : NormalViewIcon;
}

void UFolderIconsSubsystem::RefreshAllFolders()
{
	AssetViewWidgets = GetAllAssetViews();
	TArray<FContentBrowserFolder> Folders = GetAllFolders(AssetViewWidgets);

	// TODO: Find some way to prevent duplicates
	for (const FContentBrowserFolder& Folder : Folders)
	{
		AssignIconAndColor(Folder);
	}
}

void UFolderIconsSubsystem::IterateOverWidgetsRecursively(const TArray<TSharedRef<SWidget>>& TopLevelWidgets, TFunctionRef<void(const TSharedRef<SWidget>& Widget)> Iterator)
{
	TArray<TSharedRef<SWidget>> WidgetsToCheck;
	WidgetsToCheck.Append(TopLevelWidgets);

	while (!WidgetsToCheck.IsEmpty())
	{
		const TSharedRef<SWidget> CurrentWidget = WidgetsToCheck.Pop();
		Iterator(CurrentWidget);

		if (CurrentWidget->GetType() == TEXT("SWindow"))
		{
			const TSharedRef<SWindow> WidgetAsWindow = StaticCastSharedRef<SWindow>(CurrentWidget);
			WidgetsToCheck.Append(WidgetAsWindow->GetChildWindows());
		}

		FChildren* Children = CurrentWidget->GetChildren();
		if (!Children)
		{
			continue;
		}

		for (int i = 0; i < Children->Num(); i++)
		{
			const TSharedRef<SWidget> ChildWidget = Children->GetChildAt(i);
			WidgetsToCheck.Add(ChildWidget);
		}
	}
}

TSharedRef<SWidget> UFolderIconsSubsystem::FindChildWidgetOfType(const TSharedRef<SWidget>& Parent, const FName& WidgetType)
{
	TSharedPtr<SWidget> Result;

	TArray<TSharedRef<SWidget>> TopLevelWidgets;
	TopLevelWidgets.Add(Parent);

	IterateOverWidgetsRecursively(
		TopLevelWidgets,
		[&Result, WidgetType](const TSharedRef<SWidget>& Widget)
		{
			if (Widget->GetType() == WidgetType && Widget->GetVisibility() == EVisibility::Visible)
			{
				Result = Widget;
			}
		}
	);

	return Result.ToSharedRef();
}

TArray<TSharedRef<SAssetView>> UFolderIconsSubsystem::GetAllAssetViews()
{
	TArray<TSharedRef<SAssetView>> Result;

	TArray<TSharedRef<SWidget>> TopLevelWidgets;
	TopLevelWidgets.Append(FSlateApplication::Get().GetTopLevelWindows());

	IterateOverWidgetsRecursively(
		TopLevelWidgets,
		[&Result](const TSharedRef<SWidget>& Widget)
		{
			if (Widget->GetType() == TEXT("SAssetView"))
			{
				Result.Add(StaticCastSharedRef<SAssetView>(Widget));
			}
		}
	);

	return Result;
}

TArray<FContentBrowserFolder> UFolderIconsSubsystem::GetAllFolders(const TArray<TSharedRef<SAssetView>>& AssetViews)
{
	TArray<FContentBrowserFolder> Result;

	TArray<TSharedRef<SWidget>> TopLevelWidgets;
	TopLevelWidgets.Append(AssetViews);

	IterateOverWidgetsRecursively(
		TopLevelWidgets,
		[&Result](const TSharedRef<SWidget>& Widget)
		{
			const TSharedPtr<FTagMetaData> MetaTag = Widget->GetMetaData<FTagMetaData>();
			if (!MetaTag)
			{
				return;
			}

			const FString TagString = MetaTag->Tag.ToString();
			// TODO: Find a better way to confirm this is a virtual path
			if (!TagString.StartsWith("/"))
			{
				return;
			}

			// TODO: Check if we transform TEXT("SImage") to a template, static or a GET_CLASS_CHECKED
			if (const TSharedPtr<SWidget> FoundImage = FindChildWidgetOfType(Widget, TEXT("SImage")))
			{
				Result.Emplace(ConvertVirtualPathToInvariantPathString(TagString), FoundImage.ToSharedRef());
			}
		}
	);

	return Result;
}

void UFolderIconsSubsystem::AssignIconAndColor(const FContentBrowserFolder& Folder)
{
	const TSharedRef<SImage> Image = StaticCastSharedRef<SImage>(Folder.Widget);
	const bool bIsColumnView = Image->GetDesiredSize().X < 32.0 && Image->GetDesiredSize().Y < 32.0;
	Image->SetImage(TAttribute<const FSlateBrush*>::CreateLambda(
		[Folder, bIsColumnView]()
		{
			return Get().GetIconForFolder(Folder.Path, bIsColumnView);
		}
	));
}
