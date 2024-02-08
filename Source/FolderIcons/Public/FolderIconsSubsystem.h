// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <EditorSubsystem.h>

#include "FolderIconsSubsystem.generated.h"

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
	void RefreshFolderIcons();

	void OnSettingsChanged(UObject* Settings, FPropertyChangedEvent& PropertyChangedEvent);
	void OnAssetPathChanged(const FString& AssetPath);
	void OnItemDataUpdated(TArrayView<const FContentBrowserItemDataUpdate> ItemDataUpdate);
	void OnItemDataRefreshed();
	void OnItemDataDiscoveryComplete();

	TArray<FContentBrowserFolder> CurrentAssetWidgets;


	bool bRefreshNextTick = false;
};
