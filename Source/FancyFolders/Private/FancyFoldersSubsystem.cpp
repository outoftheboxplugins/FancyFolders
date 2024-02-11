// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FancyFoldersSubsystem.h"

#include <ContentBrowserDataSubsystem.h>
#include <IContentBrowserDataModule.h>
#include <PathViewTypes.h>
#include <SAssetView.h>

#include "FancyFoldersSettings.h"

FString FContentBrowserFolder::GetPackagePath() const
{
	FName ConvertedPath;
	IContentBrowserDataModule::Get().GetSubsystem()->TryConvertVirtualPath(VirtualPath, ConvertedPath);
	return ConvertedPath.ToString();
}

UFancyFoldersSubsystem& UFancyFoldersSubsystem::Get()
{
	return *GEditor->GetEditorSubsystem<UFancyFoldersSubsystem>();
}

void UFancyFoldersSubsystem::SetFoldersIcon(const FString& Icon, TArray<FString> Folders)
{
	UFancyFoldersSettings* Settings = GetMutableDefault<UFancyFoldersSettings>();
	for (const auto& Folder : Folders)
	{
		// TODO: This should take the color the folder currently has instead of hardcoded red
		// TODO: We need some way to also listen for color changes so they can be shared between users
		Settings->PathAssignments.Emplace(Folder, FFolderData{FLinearColor::Red, FName(Icon)});
	}
}

void UFancyFoldersSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FSlateApplication& SlateApp = FSlateApplication::Get();
	SlateApp.OnPostTick().AddUObject(this, &ThisClass::OnPostTick);
}

void UFancyFoldersSubsystem::OnPostTick(float DeltaTime)
{
	// TODO: Check if any of the AssetViewWidgets changed their type
	for (const TSharedRef<SAssetView>& AssetView : AssetViewWidgets)
	{
	}

	RefreshAllFolders();
}

const FSlateBrush* UFancyFoldersSubsystem::GetIconForFolder(const FString& VirtualPath, bool bIsColumnView, const TDelegate<bool()>& GetOpenState) const
{
	const UFancyFoldersSettings* const Settings = GetDefault<UFancyFoldersSettings>();

	const bool bIsOpen = GetOpenState.IsBound() && GetOpenState.Execute();

	if (const FSlateBrush* CustomIcon = Settings->GetIconForPath(VirtualPath, bIsColumnView, bIsOpen))
	{
		return CustomIcon;
	}

	// Return to default behavior - normal icon / c++ icon / developer icon
	if (bIsColumnView)
	{
		if (bIsOpen)
		{
			return FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderOpen");
		}
		return FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderClosed");
	}
	return FAppStyle::GetBrush("ContentBrowser.ListViewFolderIcon");
}

void UFancyFoldersSubsystem::RefreshAllFolders()
{
	AssetViewWidgets = GetAllAssetViews();
	TArray<FContentBrowserFolder> Folders = GetAllFolders(AssetViewWidgets);

	// TODO: Find some way to prevent duplicates
	for (const FContentBrowserFolder& Folder : Folders)
	{
		AssignIconAndColor(Folder);
	}

	// TODO: Below we have the pathview stuff:

	class SMyCoolPath : public SPathView
	{
	public:
		TMap<FName, TWeakPtr<FTreeItem>> MyCoolGetter() const { return TreeItemLookup; }
	};

	TArray<TSharedRef<SPathView>> PathWidgets = GetAllPathWidgets();
	for (const TSharedRef<SPathView>& PathWidget : PathWidgets)
	{
		TMap<FName, TWeakPtr<FTreeItem>> Data = reinterpret_cast<SMyCoolPath*>(&PathWidget.Get())->MyCoolGetter();

		const TSharedPtr<SWidget> FoundTreeView = FindChildWidgetOfType(PathWidget, TEXT("STreeView< TSharedPtr<FTreeItem> >"));
		if (!FoundTreeView)
		{
			return;
		}

		TSharedPtr<STreeView<TSharedPtr<FTreeItem>>> TreeViewPtr = StaticCastSharedPtr<STreeView<TSharedPtr<FTreeItem>>>(FoundTreeView);
		for (const TTuple<FName, TWeakPtr<FTreeItem>>& Entry : Data)
		{
			if (const TSharedPtr<ITableRow> Widget = TreeViewPtr->WidgetFromItem(Entry.Value.Pin()))
			{
				if (const TSharedPtr<SWidget> FoundImage = FindChildWidgetOfType(Widget->GetContent().ToSharedRef(), TEXT("SImage")))
				{
					FContentBrowserFolder Folder = {Entry.Key, FoundImage.ToSharedRef()};
					AssignIconAndColor(
						Folder,
						TDelegate<bool()>::CreateLambda(
							[=]()
							{
								return TreeViewPtr->IsItemExpanded(Entry.Value.Pin());
							}
						)
					);
				}
			}
		}
	}
}

void UFancyFoldersSubsystem::IterateOverWidgetsRecursively(const TArray<TSharedRef<SWidget>>& TopLevelWidgets, TFunctionRef<void(const TSharedRef<SWidget>& Widget)> Iterator)
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

TSharedPtr<SWidget> UFancyFoldersSubsystem::FindChildWidgetOfType(const TSharedRef<SWidget>& Parent, const FName& WidgetType)
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

	return Result;
}

TArray<TSharedRef<SAssetView>> UFancyFoldersSubsystem::GetAllAssetViews()
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

TArray<FContentBrowserFolder> UFancyFoldersSubsystem::GetAllFolders(const TArray<TSharedRef<SAssetView>>& AssetViews)
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

			const auto& PathTag = MetaTag->Tag;
			// TODO: Find a better way to confirm this is a virtual path
			if (!PathTag.ToString().StartsWith("/"))
			{
				return;
			}

			// TODO: Check if we transform TEXT("SImage") to a template, static or a GET_CLASS_CHECKED
			if (const TSharedPtr<SWidget> FoundImage = FindChildWidgetOfType(Widget, TEXT("SImage")))
			{
				Result.Emplace(PathTag, FoundImage.ToSharedRef());
			}
		}
	);

	return Result;
}

void UFancyFoldersSubsystem::AssignIconAndColor(const FContentBrowserFolder& Folder, TDelegate<bool()> GetIsOpen)
{
	const FContentBrowserItem ContentBrowserFolder = IContentBrowserDataModule::Get().GetSubsystem()->GetItemAtPath(FName(Folder.VirtualPath), EContentBrowserItemTypeFilter::IncludeFolders);
	if (!ContentBrowserFolder.IsValid())
	{
		return;
	}

	const TSharedRef<SImage> Image = StaticCastSharedRef<SImage>(Folder.Widget);
	const bool bIsColumnView = Image->GetDesiredSize().X < 32.0 && Image->GetDesiredSize().Y < 32.0;
	Image->SetImage(TAttribute<const FSlateBrush*>::CreateLambda(
		[Folder, bIsColumnView, GetIsOpen]()
		{
			return Get().GetIconForFolder(Folder.GetPackagePath(), bIsColumnView, GetIsOpen);
		}
	));
}

TArray<TSharedRef<SPathView>> UFancyFoldersSubsystem::GetAllPathWidgets()
{
	TArray<TSharedRef<SPathView>> Result;

	TArray<TSharedRef<SWidget>> TopLevelWidgets;
	TopLevelWidgets.Append(FSlateApplication::Get().GetTopLevelWindows());

	IterateOverWidgetsRecursively(
		TopLevelWidgets,
		[&Result](const TSharedRef<SWidget>& Widget)
		{
			if (Widget->GetType() == TEXT("SPathView"))
			{
				Result.Add(StaticCastSharedRef<SPathView>(Widget));
			}
		}
	);

	return Result;
}
