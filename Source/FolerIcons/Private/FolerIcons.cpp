// Copyright Epic Games, Inc. All Rights Reserved.

#include "FolerIcons.h"

#include "Algo/Compare.h"
#include "ContentBrowserDataSubsystem.h"
#include "ContentBrowserMenuContexts.h"
#include "FolderIconsStyle.h"
#include "IContentBrowserDataModule.h"
#include "ToolMenus.h"

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
		if(!Children)
		{
			return {}; 
		}

		for(int i = 0; i< Children->Num(); i++)
		{
			const TSharedPtr<SWidget> ChildWidget = Children->GetChildAt(i);
			if(!ChildWidget)
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

void FFolerIconsModule::StartupModule()
{
	if (FSlateApplication::IsInitialized())
	{
		FFolderIconsStyle::Initialize();

		PresetIcons.Emplace(TEXT("Maps"), "Icons.Level");
		FSlateApplication::Get().OnPostTick().AddRaw(this, &FFolerIconsModule::OnApplicationTick);

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
						FNewToolMenuChoice(FNewMenuDelegate::CreateRaw(this, &FFolerIconsModule::BuildContextMenu, Context))
					);
				}
			)
		);
	}
}

void FFolerIconsModule::ShutdownModule()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnPostTick().RemoveAll(this);
	}
}

void FFolerIconsModule::OnApplicationTick(float DeltaTime)
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
			if(FoundImage)
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
void FFolerIconsModule::BuildContextMenu(FMenuBuilder& MenuBuilder, UContentBrowserFolderContext* Context)
{
	if (!Context || Context->NumAssetPaths <= 0)
	{
		return;
	}
	const TArray<FString>& SelectedPaths = Context->GetSelectedPackagePaths();

	const TArray<FName> StandardIcons =
		{
			"FolderIcons.Apple",
			"FolderIcons.Android",
			"FolderIcons.Play",
			"FolderIcons.Textures",
			"FolderIcons.Sound",
			"FolderIcons.Fonts",
			"FolderIcons.Code",
			"FolderIcons.Physics",
			"FolderIcons.Bin",
			"FolderIcons.Windows",
			"FolderIcons.Movies",
			"FolderIcons.Random",
		};

	for (const auto& Icon : StandardIcons)
	{
		MenuBuilder.AddMenuEntry(FText::FromName(Icon), FText::FromName(Icon), FSlateIcon(), FUIAction(
			FExecuteAction::CreateLambda(
		[=, this]()
		{
			for (const FString& Path : Context->SelectedPackagePaths)
			{
				Icons.Emplace(Path, Icon);
			}
		}
	)));
	}
}

/*
void FFolerIconsModule::BuildContextMenu(FToolMenuSection& InSection)
{
InSection.AddSubMenu(
	"RunStopUtilityWidget",
	FNewToolMenuSectionDelegate::CreateLambda(
		[=, this](FToolMenuSection& InSubSection)
		{

		}
	)
);
*/

/*
InSection.AddMenuEntry(
	"SetIcon",
	INVTEXT("SetRandomIcon"),
	INVTEXT("SetRandomIcon"),
	FSlateIcon(FAppStyle::GetAppStyleSetName(), "MediaAsset.AssetActions.Play.Small"),
	FUIAction(FExecuteAction::CreateLambda(
		[=, this]()
		{
			for (const FString& Path : Context->SelectedPackagePaths)
			{
				static int32 IconIndex = 0;
				IconIndex = (IconIndex + 1) % StandardIcons.Num();

				Icons.Emplace(Path, StandardIcons[IconIndex]);
			}
		}
	))
);
}
*/


void FFolerIconsModule::AddIconsToWidgets()
{
	// TODO: Find some way to prevent duplicates
	for (const TTuple<FString, TSharedRef<SWidget>>& AssetWidget : CurrentAssetWidgets)
	{
		const TSharedRef<SImage> Image = StaticCastSharedRef<SImage>(AssetWidget.Value);
		Image->SetImage(TAttribute<const FSlateBrush*>::CreateRaw(this, &FFolerIconsModule::GetIconForFolder, AssetWidget.Key));
	}
}

const FSlateBrush* FFolerIconsModule::GetIconForFolder(FString VirtualPath) const
{
	if (Icons.Contains(VirtualPath))
	{
		return FFolderIconsStyle::Get().GetBrush(Icons[VirtualPath]);
	}

	for (const TTuple<FString, FName>& Preset : PresetIcons)
	{
		if (VirtualPath.EndsWith(Preset.Key))
		{
			return FFolderIconsStyle::Get().GetBrush("FolderIcons.Apple");
		}
	}

	return nullptr;
}

IMPLEMENT_MODULE(FFolerIconsModule, FolerIcons)