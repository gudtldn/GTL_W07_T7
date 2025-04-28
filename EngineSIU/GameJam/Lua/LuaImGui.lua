function EngineSIU.LuaImGui()
    local Coaches = _G.Coaches
    
    ImGui.SetNextWindowPos(10, 10)
    ImGui.SetNextWindowSize(300, 300)
    ImGui.Begin("Game Property")

    local n = #Coaches

    for i = n-2, n do
        if Coaches[i] then
            ImGui.Text(string.format("%s's Love : %d", Coaches[i].Name, Coaches[i].cpp_actor:GetAffection()))
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

    if (GameMode.GameOver) then

        ImGui.SetCenterPos();
        ImGui.SetNextWindowSize(300, 100)

        ImGui.Begin("WINNER WINNER CHICKEN DINNER")
        ImGui.Text(string.format("Winner Player: %d", GameMode.Winner))
        ImGui.End()
    end
end