// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

#include "FancyFoldersSubsystem.h"

#include <ContentBrowserDataSource.h>
#include <ContentBrowserDataSubsystem.h>
#include <IContentBrowserDataModule.h>
#include <PathViewTypes.h>
#include <SAssetView.h>
#include <SPathView.h>
#include <Misc/EngineVersionComparison.h>

#include "HackedRedefinition.h"

#include "FancyFoldersSettings.h"

// TODO: Add option to clear data - icon & color
// TODO: On startup we should transform all the currently assigned colors to rules in the settings
// TODO: We need some way to also listen for color changes so they can be shared between users
// TODO: On Tick - check if any of the AssetViewWidgets changed their type

namespace Helpers
{
	void IterateOverWidgetsRecursively(const TArray<TSharedRef<SWidget>>& TopLevelWidgets, TFunctionRef<void(const TSharedRef<SWidget>& Widget)> Iterator)
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

	template <typename T>
	TSharedPtr<T> FindChildWidgetOfType(const TSharedRef<SWidget>& Parent, const FName& WidgetType = T::StaticWidgetClass().GetWidgetType())
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

		return StaticCastSharedPtr<T>(Result);
	}

	TMap<FName, FTreeItemPtr> GetInternalPathData(const TSharedRef<SPathView>& PathView)
	{
		class SInternalAccessPathView : public SPathView
		{
		public:
#if UE_VERSION_NEWER_THAN(5, 4, 4)
			TMap<FName, FTreeItemPtr> MyCoolGetter() const { return TreeData->VirtualPathToItem; }
#else
			TMap<FName, FTreeItemPtr> MyCoolGetter() const { return TreeItemLookup; }
#endif
		};

		TMap<FName, FTreeItemPtr> Data = reinterpret_cast<SInternalAccessPathView*>(&PathView.Get())->MyCoolGetter();
		return Data;
	}

	bool IsItemDeveloperContent(const FContentBrowserItem& InItem)
	{
		const FContentBrowserItemDataAttributeValue IsDeveloperAttributeValue = InItem.GetItemAttribute(ContentBrowserItemAttributes::ItemIsDeveloperContent);
		return IsDeveloperAttributeValue.IsValid() && IsDeveloperAttributeValue.GetValue<bool>();
	}

	bool IsItemCodeContent(const FContentBrowserItem& InItem)
	{
		return EnumHasAnyFlags(InItem.GetItemCategory(), EContentBrowserItemFlags::Category_Class);
	}

	template <typename T, typename U>
	TArray<TPair<T, U>> GetDifference(const TMap<T, U>& Lhs, const TMap<T, U>& Rhs, bool bIncludeDifferent)
	{
		TArray<TPair<T, U>> Result;
		for (auto It = Lhs.CreateConstIterator(); It; ++It)
		{
			auto RightIt = Rhs.Find(It->Key);
			if (!RightIt)
			{
				Result.Emplace(It->Key, It->Value);
			}
			else if (bIncludeDifferent && *RightIt != It->Value)
			{
				Result.Emplace(It->Key, It->Value);
			}
		}

		return Result;
	}
} // namespace Helpers

bool FContentBrowserFolder::IsOpenNow() const
{
	if (!GetFolderState.IsBound())
	{
		return false;
	}

	return GetFolderState.Execute();
}

bool FContentBrowserFolder::IsColumnViewNow() const
{
	return FolderImage->GetDesiredSize().X < 32.0 && FolderImage->GetDesiredSize().Y < 32.0;
}

FString FContentBrowserFolder::GetPackagePath() const
{
	FName ConvertedPath;
	IContentBrowserDataModule::Get().GetSubsystem()->TryConvertVirtualPath(FolderPath, ConvertedPath);
	return ConvertedPath.ToString();
}

FContentBrowserItem FContentBrowserFolder::GetContentBrowserItem() const
{
	return IContentBrowserDataModule::Get().GetSubsystem()->GetItemAtPath(FName(FolderPath), EContentBrowserItemTypeFilter::IncludeFolders);
}

bool FContentBrowserFolder::operator==(const FContentBrowserFolder& Other) const
{
	return FolderImage == Other.FolderImage;
}

UFancyFoldersSubsystem& UFancyFoldersSubsystem::Get()
{
	return *GEditor->GetEditorSubsystem<UFancyFoldersSubsystem>();
}

void UFancyFoldersSubsystem::SetFoldersIcon(const FString& Icon, TArray<FString> Folders)
{
	for (const FString& Folder : Folders)
	{
		UFancyFoldersSettings* Settings = GetMutableDefault<UFancyFoldersSettings>();
		Settings->UpdateOrCreateAssignmentIcon(Folder, FName(Icon));
	}
}

void UFancyFoldersSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication& SlateApp = FSlateApplication::Get();
		SlateApp.OnPostTick().AddUObject(this, &ThisClass::OnPostTick);
	}
}

void UFancyFoldersSubsystem::OnPostTick(float DeltaTime)
{
	SyncFolderColorData();

	RefreshAssetViewFolders();
	RefreshPathViewFolders();
}

void UFancyFoldersSubsystem::AssignIconAndColor(const FContentBrowserFolder& Folder)
{
	const FContentBrowserItem ContentBrowserFolder = Folder.GetContentBrowserItem();
	if (!ContentBrowserFolder.IsValid())
	{
		return;
	}

	const TSharedRef<SImage> Image = Folder.FolderImage;
	Image->SetImage(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateUObject(this, &ThisClass::GetIconForFolder, Folder)));
	Image->SetColorAndOpacity(TAttribute<FSlateColor>::Create(TAttribute<FSlateColor>::FGetter::CreateUObject(this, &ThisClass::GetColorForFolder, Folder)));
}

const FSlateBrush* UFancyFoldersSubsystem::GetIconForFolder(FContentBrowserFolder Folder) const
{
	const UFancyFoldersSettings* const Settings = GetDefault<UFancyFoldersSettings>();

	const bool bIsOpen = Folder.IsOpenNow();
	const bool bIsColumnView = Folder.IsColumnViewNow();

	if (const FSlateBrush* CustomIcon = Settings->GetIconForPath(Folder.GetPackagePath(), bIsColumnView, bIsOpen))
	{
		return CustomIcon;
	}

	const FContentBrowserItem ContentBrowserFolder = Folder.GetContentBrowserItem();
	const bool bCodeFolder = Helpers::IsItemCodeContent(ContentBrowserFolder);
	const bool bDeveloperFolder = Helpers::IsItemDeveloperContent(ContentBrowserFolder);

	const FSlateBrush* FolderBrush;
	const FSlateBrush* FolderOpenBrush;
	const FSlateBrush* FolderClosedBrush;

	if (bCodeFolder)
	{
		FolderBrush = FAppStyle::GetBrush("ContentBrowser.ListViewCodeFolderIcon");
		FolderOpenBrush = FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderOpenCode");
		FolderClosedBrush = FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderClosedCode");
	}
	else if (bDeveloperFolder)
	{
		FolderBrush = FAppStyle::GetBrush("ContentBrowser.ListViewDeveloperFolderIcon");
		FolderOpenBrush = FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderOpenDeveloper");
		FolderClosedBrush = FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderClosedDeveloper");
	}
	else
	{
		FolderBrush = FAppStyle::GetBrush("ContentBrowser.ListViewFolderIcon");
		FolderOpenBrush = FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderOpen");
		FolderClosedBrush = FAppStyle::GetBrush("ContentBrowser.AssetTreeFolderClosed");
	}

	if (bIsColumnView)
	{
		if (bIsOpen)
		{
			return FolderOpenBrush;
		}

		return FolderClosedBrush;
	}

	return FolderBrush;
}

FSlateColor UFancyFoldersSubsystem::GetColorForFolder(FContentBrowserFolder Folder) const
{
	const UFancyFoldersSettings* const Settings = GetDefault<UFancyFoldersSettings>();

	if (const TOptional<FLinearColor> CustomColor = Settings->GetColorForPath(Folder.GetPackagePath()))
	{
		return CustomColor.GetValue();
	}

	return AssetViewUtils::GetDefaultColor();
}

void UFancyFoldersSubsystem::RefreshAssetViewFolders()
{
	TArray<TSharedRef<SWidget>> AssetViewWidgets = TArray<TSharedRef<SWidget>>(GetAllAssetViews());

	Helpers::IterateOverWidgetsRecursively(
		AssetViewWidgets,
		[this](const TSharedRef<SWidget>& Widget)
		{
			const TSharedPtr<FTagMetaData> MetaTag = Widget->GetMetaData<FTagMetaData>();
			if (!MetaTag)
			{
				return;
			}

			const FName& PathTag = MetaTag->Tag;
			// TODO: Find a better way to confirm this is a virtual path
			if (!PathTag.ToString().StartsWith("/"))
			{
				return;
			}

			if (const TSharedPtr<SImage> FoundImage = Helpers::FindChildWidgetOfType<SImage>(Widget))
			{
				FContentBrowserFolder Folder = {PathTag, FoundImage.ToSharedRef()};
				AssignIconAndColor(Folder);
			}
		}
	);
}

void UFancyFoldersSubsystem::RefreshPathViewFolders()
{
	TArray<TSharedRef<SPathView>> PathWidgets = GetAllPathWidgets();
	for (const TSharedRef<SPathView>& PathWidget : PathWidgets)
	{
		TMap<FName, FTreeItemPtr> Data = Helpers::GetInternalPathData(PathWidget);

#if UE_VERSION_NEWER_THAN(5, 4, 4)
		const FName PathWidgetType = TEXT("STreeView<TSharedPtr<FTreeItem>>");
#else
		const FName PathWidgetType =  TEXT("STreeView< TSharedPtr<FTreeItem> >");
#endif

		const TSharedPtr<STreeView<TSharedPtr<FTreeItem>>> TreeViewPtr = Helpers::FindChildWidgetOfType<STreeView<TSharedPtr<FTreeItem>>>(PathWidget, PathWidgetType);
		if (!TreeViewPtr)
		{
			return;
		}

		for (const TTuple<FName, FTreeItemPtr>& Entry : Data)
		{
			TWeakPtr<FTreeItem> EntryValue = Entry.Value;

			if (const TSharedPtr<ITableRow> Widget = TreeViewPtr->WidgetFromItem(EntryValue.Pin()))
			{
				if (const TSharedPtr<SImage> FoundImage = Helpers::FindChildWidgetOfType<SImage>(Widget->GetContent().ToSharedRef()))
				{
					FContentBrowserFolder Folder = {
						Entry.Key,
						FoundImage.ToSharedRef(),
						TDelegate<bool()>::CreateLambda(
							[=]()
							{
								return TreeViewPtr->IsItemExpanded(EntryValue.Pin());
							}
						)};
					AssignIconAndColor(Folder);
				}
			}
		}
	}
}

TArray<TSharedRef<SAssetView>> UFancyFoldersSubsystem::GetAllAssetViews()
{
	TArray<TSharedRef<SAssetView>> Result;

	TArray<TSharedRef<SWidget>> TopLevelWidgets;
	TopLevelWidgets.Append(FSlateApplication::Get().GetTopLevelWindows());

	Helpers::IterateOverWidgetsRecursively(
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

TArray<TSharedRef<SPathView>> UFancyFoldersSubsystem::GetAllPathWidgets()
{
	TArray<TSharedRef<SPathView>> Result;

	TArray<TSharedRef<SWidget>> TopLevelWidgets;
	TopLevelWidgets.Append(FSlateApplication::Get().GetTopLevelWindows());

	Helpers::IterateOverWidgetsRecursively(
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

void UFancyFoldersSubsystem::SyncFolderColorData()
{
	TMap<FString, FLinearColor> CurrentPathColors;
	{
		TArray<FString> Section;
		GConfig->GetSection(TEXT("PathColor"), Section, GEditorPerProjectIni);

		for (int32 SectionIndex = 0; SectionIndex < Section.Num(); SectionIndex++)
		{
			FString EntryStr = Section[SectionIndex];
			EntryStr.TrimStartInline();

			FString PathStr;
			FString ColorStr;
			if (EntryStr.Split(TEXT("="), &PathStr, &ColorStr))
			{
				FLinearColor CurrentColor;
				if (CurrentColor.InitFromString(ColorStr))
				{
					CurrentPathColors.Emplace(PathStr, CurrentColor);
				}
			}
		}
	}

	TArray<TTuple<FString, FLinearColor>> NewColors = Helpers::GetDifference(CurrentPathColors, CachedPathColors, true);
	TArray<TTuple<FString, FLinearColor>> RemovedColors = Helpers::GetDifference(CachedPathColors, CurrentPathColors, false);

	UFancyFoldersSettings* Settings = GetMutableDefault<UFancyFoldersSettings>();
	for (auto Color : NewColors)
	{
		Settings->UpdateOrCreateAssignmentColor(Color.Key, Color.Value);
	}

	for (auto Color : RemovedColors)
	{
		Settings->UpdateOrCreateAssignmentColor(Color.Key, AssetViewUtils::GetDefaultColor());
	}

	CachedPathColors = CurrentPathColors;
}
