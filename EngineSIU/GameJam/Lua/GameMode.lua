setmetatable(_ENV, { __index = EngineSIU })

local ALuaGameMode = {}
ALuaGameMode.__index = ALuaGameMode

function ALuaGameMode:new(cpp_actor)
    local o = setmetatable({}, ALuaGameMode)
    o.cpp_actor         = cpp_actor
    o.Player            = nil     -- BeginPlay 에 채울 예정
    o.Coaches           = {}  
    o.CurrentPlayerIndex     = 1
    o.PlayerNum         = 4
    o.MaxAffection      = 30
    o.GameOver          = false
    o.Winner            = 0
    return o
end

function ALuaGameMode:BeginPlay()
    -- BeginPlay 시점에 전역에서 Player/Coaches 가져오기
    self.Player  = Player
    self.Coaches = _G.Coaches

    for idx, coach in ipairs(self.Coaches) do
        coach:SetCoachColor(idx - 1) -- Lua 배열의 경우 1부터 index가 시작도니다고 함
    end

end

function ALuaGameMode:Tick(delta_time)
    if self.GameOver then return end
    -- (필요 시) 타이머나 UI 업데이트 로직
end

-- 플레이어 턴 넘기기
function ALuaGameMode:NextTurn()
    self.CurrentPlayerIndex = self.CurrentPlayerIndex  % self.PlayerNum + 1
    -- C++ HUD 업데이트 알림
    -- self.cpp_actor:UpdateCurrentPlayer(self.CurrentPlayer)
end

-- 승리 처리
function ALuaGameMode:EndGame(winnerIndex)
    if self.GameOver then 
        return
    else
        self.GameOver = true
        self.Winner = winnerIndex
    end
end

-- 색출 일어나진 않음
function ALuaGameMode:OnOverlap(other_actor) end
function ALuaGameMode:Destroyed() end

function ALuaGameMode:EndPlay(reason) 
    LOG_INFO("EndPlay")
    
end

local function create_actor_instance(cpp_actor)
    local inst = ALuaGameMode:new(cpp_actor)
    _G.GameMode = inst
    return inst
end

return create_actor_instance
