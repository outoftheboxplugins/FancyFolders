// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

#pragma once

// This was copied from SPathView.cpp because for some reason the definition is inside the .cpp file
// Note: Everything was made public and all the functions were removed for simplicity. As long as the memory layout is the same, it should work.

struct FPathViewData
{
	uint64 Version;
	TArray<TSharedPtr<FTreeItem>> RootItems;
	TArray<TSharedPtr<FTreeItem>> VisibleRootItems;
	TMap<FName, TSharedPtr<FTreeItem>> VirtualPathToItem;
	TMap<FName, TSharedPtr<FTreeItem>> InvariantPathToItem;
	FName OwningContentBrowserName;
	bool bFlat;
	TTextFilter<FStringView> FolderPathTextFilter;
};
