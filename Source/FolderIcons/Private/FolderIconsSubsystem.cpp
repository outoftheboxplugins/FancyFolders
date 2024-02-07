// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FolderIconsSubsystem.h"

#include "Algo/Compare.h"
#include "ContentBrowserDataSubsystem.h"
#include "FolderIconsSettings.h"
#include "FolderIconsStyle.h"
#include "IContentBrowserDataModule.h"

namespace
{
	bool AreTheSame(const TMap<FString, TSharedRef<SWidget>>& A, const TMap<FString, TSharedRef<SWidget>>& B)
	{
		TArray<TSharedRef<SWidget>> KeysA;
		A.GenerateValueArray(KeysA);

		TArray<TSharedRef<SWidget>> KeysB;
		B.GenerateValueArray(KeysB);

		return Algo::Compare(KeysA, KeysB);
	}

	/**
	 * Converts a virtual path such as /All/Plugins -> /Plugins or /All/Game -> /Game
	 */
	FString ConvertVirtualPathToInvariantPathString(const FString& VirtualPath)
	{
		FName ConvertedPath;
		IContentBrowserDataModule::Get().GetSubsystem()->TryConvertVirtualPath(FName(VirtualPath), ConvertedPath);
		return ConvertedPath.ToString();
	}

	TSharedPtr<SWidget> FindWidgetOfTypeInHierarchy(TSharedRef<SWidget> Widget, FString Type)
	{
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

			if (ChildWidget->GetType().Compare(TEXT("SImage")) == 0)
			{
				return ChildWidget;
			}

			if (TSharedPtr<SWidget> FoundChild = FindWidgetOfTypeInHierarchy(ChildWidget.ToSharedRef(), Type))
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
		Settings->Assigned.Emplace(Folder, FLinearColor::Red, FName(Icon));
	}

	AddIconsToWidgets();
}

void UFolderIconsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UFolderIconsSettings* Settings = GetMutableDefault<UFolderIconsSettings>();
	Settings->OnSettingChanged().AddUObject(this, &ThisClass::OnSettingsChanged);
}

void UFolderIconsSubsystem::Deinitialize()
{
}

void UFolderIconsSubsystem::Tick(float DeltaTime)
{
	TMap<FString, TSharedRef<SWidget>> WidgetsFound;

	TArray<TSharedRef<SWidget>> WidgetsToCheck;

	const TArray<TSharedRef<SWindow>> AllWindows = FSlateApplication::Get().GetTopLevelWindows();
	for (const TSharedRef<SWindow>& Window : AllWindows)
	{
		WidgetsToCheck.Add(Window);
	}

	while (!WidgetsToCheck.IsEmpty())
	{
		const TSharedPtr<SWidget> CurrentWidget = WidgetsToCheck.Pop();
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

			const auto FoundImage = FindWidgetOfTypeInHierarchy(ChildWidget.ToSharedRef(), TEXT("SImage"));
			if (FoundImage)
			{
				WidgetsFound.Emplace(ConvertVirtualPathToInvariantPathString(TagString), FoundImage.ToSharedRef());
			}
		}
	}

	// check if WidgetsFound is different from CurrentAssetWidgets
	if (!AreTheSame(WidgetsFound, CurrentAssetWidgets))
	{
		CurrentAssetWidgets = WidgetsFound;
		AddIconsToWidgets();
	}
}

ETickableTickType UFolderIconsSubsystem::GetTickableTickType() const
{
	return IsTemplate() ? ETickableTickType::Never : ETickableTickType::Always;
}

UWorld* UFolderIconsSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

bool UFolderIconsSubsystem::IsTickableInEditor() const
{
	return true;
}

TStatId UFolderIconsSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFolderIconsSubsystem, STATGROUP_Tickables);
}

void UFolderIconsSubsystem::AddIconsToWidgets()
{
	// TODO: Find some way to prevent duplicates
	for (const TTuple<FString, TSharedRef<SWidget>>& AssetWidget : CurrentAssetWidgets)
	{
		const TSharedRef<SImage> Image = StaticCastSharedRef<SImage>(AssetWidget.Value);
		TWeakObjectPtr<UFolderIconsSubsystem> WeakThis = TWeakObjectPtr<UFolderIconsSubsystem>(this);
		Image->SetImage(TAttribute<const FSlateBrush*>::CreateLambda(
			[WeakThis, AssetWidget]()
			{
				const FSlateBrush* Brush = WeakThis.IsValid() ? WeakThis->GetIconForFolder(AssetWidget.Key) : nullptr;
				return Brush;
			}
		));
	}
}

const FSlateBrush* UFolderIconsSubsystem::GetIconForFolder(const FString& VirtualPath) const
{
	const UFolderIconsSettings* const Settings = GetDefault<UFolderIconsSettings>();

	for (const FFolderIconPreset& Assigned : Settings->Assigned)
	{
		if (Assigned.FolderName.Contains(VirtualPath))
		{
			return FFolderIconsStyle::Get().GetBrush(Assigned.Icon);
		}
	}

	for (const FFolderIconPreset& Preset : Settings->Presets)
	{
		// TODO: Improve this, everything after the slash should match, not just end with.
		if (VirtualPath.EndsWith(Preset.FolderName))
		{
			return FFolderIconsStyle::Get().GetBrush(Preset.Icon);
		}
	}

	return FAppStyle::GetBrush("ContentBrowser.ListViewFolderIcon");
}

void UFolderIconsSubsystem::OnSettingsChanged(UObject* Settings, FPropertyChangedEvent& PropertyChangedEvent)
{
}