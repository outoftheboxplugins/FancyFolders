// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FolderIconsSubsystem.h"

#include <Algo/Compare.h>
#include <ContentBrowserDataSubsystem.h>
#include <ContentBrowserModule.h>
#include <IContentBrowserDataModule.h>

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "FolderIconsSettings.h"
#include "FolderIconsStyle.h"
#include "Interfaces/IMainFrameModule.h"

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
	TSharedPtr<SWidget> FindWidgetOfTypeInHierarchy(const TSharedRef<SWidget>& Widget)
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

			if (TSharedPtr<SWidget> FoundChild = FindWidgetOfTypeInHierarchy<TWidgetType>(ChildWidget.ToSharedRef()))
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
	if (bRefreshNextTick)
	{
		RefreshFolderIcons();
		bRefreshNextTick = false;
	}
}

void UFolderIconsSubsystem::RefreshFolderIcons()
{
	TArray<FContentBrowserFolder> WidgetsFound;

	TArray<TSharedRef<SWidget>> WidgetsToCheck;
	WidgetsToCheck.Append(FSlateApplication::Get().GetTopLevelWindows());

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
			const TSharedPtr<SWidget> ChildWidget = Children->GetChildAt(i);
			if (!ChildWidget)
			{
				continue;
			}

			WidgetsToCheck.Add(ChildWidget.ToSharedRef());

			const TSharedPtr<FTagMetaData> MetaTag = ChildWidget->GetMetaData<FTagMetaData>();
			if (!MetaTag)
			{
				continue;
			}

			const FString TagString = MetaTag->Tag.ToString();
			// TODO: Find a better way to confirm this is a virtual path
			if (!TagString.StartsWith("/"))
			{
				continue;
			}

			// TODO: Check if we transform TEXT("SImage") to a template, static or a GET_CLASS_CHECKED
			if (const TSharedPtr<SWidget> FoundImage = FindWidgetOfTypeInHierarchy<SImage>(ChildWidget.ToSharedRef()))
			{
				WidgetsFound.Emplace(ConvertVirtualPathToInvariantPathString(TagString), FoundImage.ToSharedRef());
			}
		}
	}

	// TODO: Perform a better check to see if WidgetsFound is different from CurrentAssetWidgets
	if (!Algo::Compare(WidgetsFound, CurrentAssetWidgets))
	{
		// TODO: Find some way to prevent duplicates
		CurrentAssetWidgets = WidgetsFound;
		for (const auto& AssetWidget : CurrentAssetWidgets)
		{
			const TSharedRef<SImage> Image = StaticCastSharedRef<SImage>(AssetWidget.Widget);
			TWeakObjectPtr<UFolderIconsSubsystem> WeakThis = TWeakObjectPtr<UFolderIconsSubsystem>(this);
			Image->SetImage(TAttribute<const FSlateBrush*>::CreateLambda(
				[WeakThis, AssetWidget]()
				{
					const FSlateBrush* Brush = WeakThis.IsValid() ? WeakThis->GetIconForFolder(AssetWidget.Path) : nullptr;
					return Brush;
				}
			));
		}
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
