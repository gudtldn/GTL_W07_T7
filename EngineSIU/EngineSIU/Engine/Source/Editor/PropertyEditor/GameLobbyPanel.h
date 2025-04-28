#pragma once

#include "GameFramework/Actor.h"
#include "UnrealEd/EditorPanel.h"

/**
 * A class is for GameJam
 */
class GameLobbyPanel : public UEditorPanel
{
public:
    GameLobbyPanel() = default;

public:
    void Render() override;
    void OnResize(HWND hWnd) override;
};
