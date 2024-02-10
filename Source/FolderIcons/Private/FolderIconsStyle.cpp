// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#include "FolderIconsStyle.h"

#include <Styling/SlateStyleRegistry.h>

#include "FolderIcons.h"

FFolderIconsStyle::FFolderIconsStyle() : FSlateStyleSet(TEXT("FolderIconsStyle"))
{
	const TArray<FString> IconFolders = FFolderIconsModule::GetIconFoldersOnDisk();
	for (const FString& Folder : IconFolders)
	{
		const FString Icon = FPaths::GetBaseFilename(Folder, true);

		const FName NormalIcon = *FString::Printf(TEXT("%s.Normal"), *Icon);
		Set(NormalIcon, new FSlateVectorImageBrush(Folder / TEXT("Normal.svg"), FVector2D(64, 64)));

		const FName ColumnIcon = *FString::Printf(TEXT("%s.Column"), *Icon);
		Set(ColumnIcon, new FSlateVectorImageBrush(Folder / TEXT("Column.svg"), FVector2D(16, 16)));
	}

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FFolderIconsStyle& FFolderIconsStyle::Get()
{
	static FFolderIconsStyle Inst;
	return Inst;
}

FFolderIconsStyle::~FFolderIconsStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}
