function EngineSIU.LuaImGui()
    local Coaches = _G.Coaches
    
    ImGui.SetNextWindowPos(10, 10)
    ImGui.SetNextWindowSize(300, 300)
    ImGui.Begin("Game Property")

    for i = 1, 3 do
        if Coaches[i] then
            ImGui.Text(Coaches[i].Name)
            ImGui.Text("'s Love : ")
            ImGui.SameLine()
            ImGui.Text(string.format("%d", Coaches[i].Affection))
        end
    end

    ImGui.Separator()
    
    -- ChargeForce 표시
    if Player then
        local cf = Player.ChargeForce or 0;
        ImGui.Text(string.format("Charge Force: %.2f", cf))
    else
        ImGui.Text("Charge Force : N/A")
    end

    -- 현재 턴 플레이어
    ImGui.Text(string.format("This Turn Player: %d", GameMode.CurrentPlayerIndex))
    ImGui.Spacing()

    ImGui.Button("Restart", 75, 35, function()
        ImGui.SetGameMode("Lobby")
    end)

    ImGui.End()
end