// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

#pragma once

// This was copied from SPathView.cpp because for some reason the definition is inside the .cpp file
// Note: VirtualPathToItem was made public on purpose so we can access the data.

struct FPathViewData
{
public:
	FPathViewData(FName InContentBrowserName, bool InFlat)
		: OwningContentBrowserName(InContentBrowserName)
		, bFlat(InFlat)
		, FolderPathTextFilter(decltype(FolderPathTextFilter)::FItemToStringArray::CreateStatic(
			  [](FStringView Input, TArray<FString>& Out) { Out.Emplace(FString(Input)); }))
	{
	}

	~FPathViewData() { }

	uint64 GetVersion()
	{
		return Version;
	}

	TArray<TSharedPtr<FTreeItem>>* GetVisibleRootItems()
	{
		return &VisibleRootItems;
	}

	TTextFilter<FStringView>& GetFolderPathTextFilter()
	{
		return FolderPathTextFilter;
	}

	void PopulateFullFolderTree(const FContentBrowserDataCompiledFilter& InFilter);
	void PopulateWithFavorites(const FContentBrowserDataCompiledFilter& InFilter);
	void FilterFullFolderTree();
	void ClearItemFilterState();
	void SortRootItems();
	TSharedRef<FTreeItem> AddFolderItem(FContentBrowserItemData&& InItemData);

	void RemoveFolderItem(const TSharedRef<FTreeItem>& InItem);

	TSharedPtr<FTreeItem> FindTreeItem(FName InVirtualPath, bool bVisibleOnly = false);
	TSharedPtr<FTreeItem> FindBestItemForPath(FStringView InVirtualPath);

	void ProcessDataUpdates(TConstArrayView<FContentBrowserItemDataUpdate> InUpdatedItems,
		const FContentBrowserDataCompiledFilter& InFilter);

protected:
	TSharedRef<FTreeItem> AddFolderItemInternal(FContentBrowserItemData&& InItemData,
		TMap<FName, TSharedPtr<FTreeItem>>* OldItemsByInvariantPath);

	TSharedPtr<FTreeItem> TryRemoveFolderItemInternal(const FContentBrowserItemData& InItem);
	TSharedPtr<FTreeItem> TryRemoveFolderItemInternal(const FContentBrowserMinimalItemData& InKey);

	bool PassesTextFilter(const TSharedPtr<FTreeItem>& InItem);

	struct FEmptyFolderFilter
	{
		TOptional<FContentBrowserFolderContentsFilter> FolderFilter;
		EContentBrowserIsFolderVisibleFlags FolderFlags;
	};
	FEmptyFolderFilter GetEmptyFolderFilter(const FContentBrowserDataCompiledFilter& CompiledDataFilter) const;

	uint64 Version;
	TArray<TSharedPtr<FTreeItem>> RootItems;
	TArray<TSharedPtr<FTreeItem>> VisibleRootItems;

public:
	TMap<FName, TSharedPtr<FTreeItem>> VirtualPathToItem;

private:
	TMap<FName, TSharedPtr<FTreeItem>> InvariantPathToItem;
	FName OwningContentBrowserName;
	bool bFlat;
	TTextFilter<FStringView> FolderPathTextFilter;
};
