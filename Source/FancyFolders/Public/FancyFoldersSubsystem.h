// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

#pragma once

#include <ContentBrowserItem.h>
#include <EditorSubsystem.h>

#include "FancyFoldersSubsystem.generated.h"

class SPathView;
class SAssetView;

using FOnGetFolderState = TDelegate<bool()>;

/**
 * Struct representing the information about folder - path, image widget, state
 */
struct FContentBrowserFolder
{
	bool operator==(const FContentBrowserFolder& Other) const;
	/**
	 * VirtualPath of the FolderPath
	 */
	FName FolderPath;
	/**
	 * Widget representing the folder's image
	 */
	TSharedRef<SImage> FolderImage;
	/**
	 * Delegate used to determine if the folder is open or closed
	 */
	FOnGetFolderState GetFolderState;
	/**
	 * Returns the current open/closed state of the folder
	 */
	bool IsOpenNow() const;
	/**
	 * Returns the current normal/column state of the folder
	 */
	bool IsColumnViewNow() const;
	/**
	 * Converts a virtual path such as /All/Plugins -> /Plugins or /All/Game -> /Game
	 */
	FString GetPackagePath() const;
	/**
	 * Returns the matching ContentBrowserItem based on the Folder's VirtualPath
	 */
	FContentBrowserItem GetContentBrowserItem() const;
};

/**
 * Subsystem responsible for replacing all folder image widget delegates with enhanced getters to show custom icons & colors
 */
UCLASS()
class UFancyFoldersSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
public:
	/**
	 * Convenience method to retrieve the FancyFolder Editor subsystem.
	 */
	static UFancyFoldersSubsystem& Get();
	/**
	 * Assigns a certain icon to each folder in the array
	 */
	void SetFoldersIcon(const FString& Icon, TArray<FString> Folders);

private:
	// Begin UEditorSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// End UEditorSubsystem interface
	/**
	 * Callback executed after each SlateApplication's Tick
	 */
	void OnPostTick(float DeltaTime);
	/**
	 * Assigns the new fancy delegates to a folder instance, so we can override the folder's icon & color
	 */
	void AssignIconAndColor(const FContentBrowserFolder& Folder);
	/**
	 * Callback executed to determine a folder's icon
	 */
	const FSlateBrush* GetIconForFolder(FContentBrowserFolder Folder) const;
	/**
	 * Callback executed to determine a folder's color
	 */
	FSlateColor GetColorForFolder(FContentBrowserFolder Folder) const;
	/**
	 * Ensures all visible AssetView folder images are using the fancy delegates
	 */
	void RefreshAssetViewFolders();
	/**
	 * Ensures all visible PathView folder images are using the fancy delegates
	 */
	void RefreshPathViewFolders();
	/**
	 * Gets all the visible AssetView widgets from the editor windows
	 */
	TArray<TSharedRef<SAssetView>> GetAllAssetViews();
	/**
	 * Gets all the visible PathView widgets from the editor windows
	 */
	TArray<TSharedRef<SPathView>> GetAllPathWidgets();
	/**
	 * Ensures the editor data from the GEditorPerProjectIni->PathColor and the FancyFolder color data are in sync
	 */
	void SyncFolderColorData();
	/**
	 * PathColors values from last FolderColorData sync
	 */
	TMap<FString, FLinearColor> CachedPathColors;
};
