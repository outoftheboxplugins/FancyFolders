// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <EditorSubsystem.h>

#include "FolderIconsSubsystem.generated.h"

struct FContentBrowserFolder
{
	FString Path;
	TSharedRef<SWidget> Widget;

	bool operator==(const FContentBrowserFolder&) const = default;
};

UCLASS()
class UFolderIconsSubsystem : public UEditorSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
public:
	void SetFoldersIcon(const FString& Icon, TArray<FString> Folders);
	const FSlateBrush* GetIconForFolder(const FString& VirtualPath) const;

private:
	// Begin UEditorSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// End UEditorSubsystem interface

	// Begin FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual bool IsTickableInEditor() const override;
	virtual TStatId GetStatId() const override;
	// End FTickableGameObject interface

	void RefreshFolderIcons();
	TArray<FContentBrowserFolder> CurrentAssetWidgets;

	void OnSettingsChanged(UObject* Settings, FPropertyChangedEvent& PropertyChangedEvent);
};
