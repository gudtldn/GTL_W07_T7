#pragma once

#include "GameFramework/Actor.h"
#include "UnrealEd/EditorPanel.h"

/**
 * A class is for GameJam
 */
class GameLobbyPanel : public UEditorPanel
{
public:
    GameLobbyPanel();

public:
    void Render() override;
    void OnResize(HWND hWnd) override;

private:
    void DrawBackground(const ImVec2& WindowSize);
    
    void DrawTitle(const ImVec2& WindowSize);
    
    bool CreateButton(const char* Label, const ImVec2& ButtonSize,  const ImVec4& InColorNormal, const ImVec4& InColorHover);

private:
    float Width, Height;

    ImFont* FontSize24;
    ImFont* FontSize32;
    ImFont* FontSize48;

    TArray<std::shared_ptr<FTexture>> Textures; 
};
