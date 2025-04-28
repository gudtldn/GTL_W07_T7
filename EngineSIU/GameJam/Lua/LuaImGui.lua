function EngineSIU.LuaImGui()
    ImGui.SetNextWindowPos(30, 100)
    ImGui.SetNextWindowSize(300, 200)
    ImGui.Begin("Game Property")
    ImGui.Text("Coach1 Love : ")
    ImGui.Text("Coach2 Love : ")
    ImGui.Text("Coach3 Love : ")
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
    ImGui.End()
end