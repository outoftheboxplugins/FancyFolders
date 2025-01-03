// Copyright Out-of-the-Box Plugins 2018-2025. All Rights Reserved.

#pragma once

#include <Styling/SlateStyle.h>

/**
 * Slate style set for FancyFolder plugin
 */
class FFancyFoldersStyle final : public FSlateStyleSet
{
public:
	FFancyFoldersStyle();
	virtual ~FFancyFoldersStyle() override;
	/**
	 * Access the singleton instance of this style set
	 */
	static FFancyFoldersStyle& Get();
};