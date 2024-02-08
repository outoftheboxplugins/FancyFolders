// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <EditorSubsystem.h>

#include "FolderIconsSubsystem.generated.h"


class SAssetView;
class FContentBrowserItemDataUpdate;

struct FContentBrowserFolder
{
	FString Path;
	TSharedRef<SWidget> Widget;

	bool operator==(const FContentBrowserFolder&) const = default;
};

UCLASS()
class UFolderIconsSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
public:
	void SetFoldersIcon(const FString& Icon, TArray<FString> Folders);
	const FSlateBrush* GetIconForFolder(const FString& VirtualPath) const;

private:
	// Begin UEditorSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// End UEditorSubsystem interface

	void OnPostTick(float DeltaTime);
	void OnSettingsChanged(UObject* Settings, FPropertyChangedEvent& PropertyChangedEvent);
	void OnAssetPathChanged(const FString& AssetPath);
	void OnItemDataUpdated(TArrayView<const FContentBrowserItemDataUpdate> ItemDataUpdate);
	void OnItemDataRefreshed();
	void OnItemDataDiscoveryComplete();

	void RefreshAllFolders();
	void IterateOverWidgetsRecursively(const TArray<TSharedRef<SWidget>>& TopLevelWidgets, TFunctionRef<void(const TSharedRef<SWidget>& Widget)> Iterator);
	TArray<TSharedRef<SAssetView>> GetAllAssetViews();
	TArray<FContentBrowserFolder> GetAllFolders(const TArray<TSharedRef<SAssetView>>& AssetViews);
	void AssignIconAndColor(const FContentBrowserFolder& Folder);

	TArray<TSharedRef<SAssetView>> AssetViewWidgets;

	bool bRefreshNextTick = false;
};
