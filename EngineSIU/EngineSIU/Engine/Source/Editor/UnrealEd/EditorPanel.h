#pragma once

#ifndef __ICON_FONT_INDEX__

#define __ICON_FONT_INDEX__
#define DEFAULT_FONT		0
#define	FEATHER_FONT		1
#define DEFAULT_FONT_24     2
#define DEFAULT_FONT_32     3
#define DEFAULT_FONT_48     4

#endif // !__ICON_FONT_INDEX__

class UEditorPanel
{
public:
    virtual ~UEditorPanel() = default;
    virtual void Render() = 0;
    virtual void OnResize(HWND hWnd) = 0;
};
