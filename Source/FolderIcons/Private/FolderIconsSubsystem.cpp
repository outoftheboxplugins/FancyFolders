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

	template <typename TWidgetType>
	TSharedPtr<SWidget> FindChildWidgetOfType(const TSharedRef<SWidget>& Widget)
	{
		const FName Type = TWidgetType::StaticWidgetClass().GetWidgetType();

		FChildren* Children = Widget->GetChildren();
		if (!Children)
		{
			return {};
		}

		for (int i = 0; i < Children->Num(); i++)
		{
			const TSharedPtr<SWidget> ChildWidget = Children->GetChildAt(i);
			if (!ChildWidget)
			{
				continue;
			}

			if (ChildWidget->GetType().Compare(Type) == 0)
			{
				return ChildWidget;
			}

			if (TSharedPtr<SWidget> FoundChild = FindChildWidgetOfType<TWidgetType>(ChildWidget.ToSharedRef()))
			{
				return FoundChild;
			}
		}

		return {};
	}
} // namespace

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
	UFolderIconsSettings* Settings = GetMutableDefault<UFolderIconsSettings>();
	Settings->OnSettingChanged().AddUObject(this, &ThisClass::OnSettingsChanged);

	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.GetOnAssetPathChanged().AddUObject(this, &ThisClass::OnAssetPathChanged);

	FSlateApplication& SlateApp = FSlateApplication::Get();
	SlateApp.OnPostTick().AddUObject(this, &ThisClass::OnPostTick);

	UContentBrowserDataSubsystem* ContentBrowserData = IContentBrowserDataModule::Get().GetSubsystem();
	ContentBrowserData->OnItemDataUpdated().AddUObject(this, &ThisClass::OnItemDataUpdated);
	ContentBrowserData->OnItemDataRefreshed().AddUObject(this, &ThisClass::OnItemDataRefreshed);
	ContentBrowserData->OnItemDataDiscoveryComplete().AddUObject(this, &ThisClass::OnItemDataDiscoveryComplete);
}

void UFolderIconsSubsystem::OnPostTick(float DeltaTime)
{
	// TODO: Check if any of the AssetViewWidgets changed their type
	for (const TSharedRef<SAssetView>& AssetView : AssetViewWidgets)
	{
	}

	if (bRefreshNextTick)
	{
		RefreshAllFolders();
		bRefreshNextTick = false;
	}
}

const FSlateBrush* UFolderIconsSubsystem::GetIconForFolder(const FString& VirtualPath) const
{
	const UFolderIconsSettings* const Settings = GetDefault<UFolderIconsSettings>();

	for (const FFolderIconPreset& Assigned : Settings->Assigned)
	{
		if (Assigned.FolderName == VirtualPath)
		{
			return FFolderIconsStyle::Get().GetBrush(Assigned.Icon);
		}
	}

	for (const FFolderIconPreset& Preset : Settings->Presets)
	{
		// TODO: Maybe in the future, support regex matching?
		const FString FolderName = FPaths::GetBaseFilename(VirtualPath);
		if (FolderName == Preset.FolderName)
		{
			return FFolderIconsStyle::Get().GetBrush(Preset.Icon);
		}
	}

	return FAppStyle::GetBrush("ContentBrowser.ListViewFolderIcon");
}

void UFolderIconsSubsystem::OnSettingsChanged(UObject* Settings, FPropertyChangedEvent& PropertyChangedEvent)
{
}

void UFolderIconsSubsystem::OnAssetPathChanged(const FString& AssetPath)
{
	bRefreshNextTick = true;
}

void UFolderIconsSubsystem::OnItemDataUpdated(TArrayView<const FContentBrowserItemDataUpdate> ItemDataUpdate)
{
	bRefreshNextTick = true;
}

void UFolderIconsSubsystem::OnItemDataRefreshed()
{
	bRefreshNextTick = true;
}

void UFolderIconsSubsystem::OnItemDataDiscoveryComplete()
{
	bRefreshNextTick = true;
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
		const TSharedPtr<SWidget> CurrentWidget = WidgetsToCheck.Pop();

		if (CurrentWidget->GetType() == TEXT("SWindow"))
		{
			const TSharedPtr<SWindow> WidgetAsWindow = StaticCastSharedPtr<SWindow>(CurrentWidget);
			WidgetsToCheck.Append(WidgetAsWindow->GetChildWindows());
		}

		FChildren* Children = CurrentWidget ? CurrentWidget->GetChildren() : nullptr;
		if (!Children)
		{
			continue;
		}

		for (int i = 0; i < Children->Num(); i++)
		{
			const TSharedRef<SWidget> ChildWidget = Children->GetChildAt(i);

			WidgetsToCheck.Add(ChildWidget);

			Iterator(ChildWidget);
		}
	}
}

TArray<TSharedRef<SAssetView>> UFolderIconsSubsystem::GetAllAssetViews()
{
	TArray<TSharedRef<SAssetView>> Result;
	const FName AssetViewType = SAssetView::StaticWidgetClass().GetWidgetType();

	TArray<TSharedRef<SWidget>> TopLevelWidgets;
	TopLevelWidgets.Append(FSlateApplication::Get().GetTopLevelWindows());

	IterateOverWidgetsRecursively(
		TopLevelWidgets,
		[&Result, AssetViewType](const TSharedRef<SWidget>& Widget)
		{
			if (Widget->GetType() == AssetViewType)
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
			if (const TSharedPtr<SWidget> FoundImage = FindChildWidgetOfType<SImage>(Widget))
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
	TWeakObjectPtr<UFolderIconsSubsystem> WeakThis = TWeakObjectPtr<UFolderIconsSubsystem>(this);
	Image->SetImage(TAttribute<const FSlateBrush*>::CreateLambda(
		[WeakThis, Folder]()
		{
			const FSlateBrush* Brush = WeakThis.IsValid() ? WeakThis->GetIconForFolder(Folder.Path) : nullptr;
			return Brush;
		}
	));
}
