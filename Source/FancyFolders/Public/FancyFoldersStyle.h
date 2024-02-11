// Copyright Out-of-the-Box Plugins 2018-2024. All Rights Reserved.

#pragma once

#include <Styling/SlateStyle.h>

class FFancyFoldersStyle final : public FSlateStyleSet
{
public:
	FFancyFoldersStyle();
	virtual ~FFancyFoldersStyle() override;

	static FFancyFoldersStyle& Get();
};