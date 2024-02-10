// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <EditorSubsystem.h>

#include "FolderIconsSubsystem.generated.h"


class SAssetView;
class FContentBrowserItemDataUpdate;

struct FContentBrowserFolder
{
	FString VirtualPath;
	TSharedRef<SWidget> Widget;

	FString GetPackagePath() const;

	bool operator==(const FContentBrowserFolder&) const = default;
};

UCLASS()
class UFolderIconsSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
public:
	static UFolderIconsSubsystem& Get();

	void SetFoldersIcon(const FString& Icon, TArray<FString> Folders);
	const FSlateBrush* GetIconForFolder(const FString& VirtualPath, bool bIsColumnView) const;

private:
	// Begin UEditorSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// End UEditorSubsystem interface

	void OnPostTick(float DeltaTime);

	void RefreshAllFolders();
	TArray<TSharedRef<SAssetView>> GetAllAssetViews();
	TArray<FContentBrowserFolder> GetAllFolders(const TArray<TSharedRef<SAssetView>>& AssetViews);
	void AssignIconAndColor(const FContentBrowserFolder& Folder);

	static TSharedRef<SWidget> FindChildWidgetOfType(const TSharedRef<SWidget>& Parent, const FName& WidgetType);
	static void IterateOverWidgetsRecursively(const TArray<TSharedRef<SWidget>>& TopLevelWidgets, TFunctionRef<void(const TSharedRef<SWidget>& Widget)> Iterator);


	TArray<TSharedRef<SAssetView>> AssetViewWidgets;
};
