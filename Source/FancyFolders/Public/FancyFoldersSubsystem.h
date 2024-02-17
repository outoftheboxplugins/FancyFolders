// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <EditorSubsystem.h>

#include "SPathView.h"

#include "FancyFoldersSubsystem.generated.h"

class SAssetView;
class FContentBrowserItemDataUpdate;

using FOnGetFolderState = TDelegate<bool()>;

struct FContentBrowserFolder
{
	FName FolderPath;
	TSharedRef<SImage> FolderImage;
	FOnGetFolderState GetFolderState;

	bool IsOpenNow() const;
	bool IsColumnViewNow() const;
	/**
	 * Converts a virtual path such as /All/Plugins -> /Plugins or /All/Game -> /Game
	 */
	FString GetPackagePath() const;
	FContentBrowserItem GetContentBrowserItem() const;

	bool operator==(const FContentBrowserFolder& Other) const;
};

UCLASS()
class UFancyFoldersSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
public:
	static UFancyFoldersSubsystem& Get();

	void SetFoldersIcon(const FString& Icon, TArray<FString> Folders);

private:
	// Begin UEditorSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// End UEditorSubsystem interface

	void OnPostTick(float DeltaTime);

	void AssignIconAndColor(const FContentBrowserFolder& Folder);
	const FSlateBrush* GetIconForFolder(FContentBrowserFolder Folder) const;
	FSlateColor GetColorForFolder(FContentBrowserFolder Folder) const;

	void RefreshAllFolders();
	void RefreshAssetViewFolders();
	void RefreshPathViewFolders();

	TArray<TSharedRef<SAssetView>> GetAllAssetViews();
	TArray<TSharedRef<SPathView>> GetAllPathWidgets();

	void SyncFolderColorData();

	TMap<FString, FLinearColor> CachedPathColors;
};
