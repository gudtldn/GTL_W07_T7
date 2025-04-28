setmetatable(_ENV, { __index = EngineSIU })

-- 전역 Player 인스턴스
Player = nil

local ALuaPlayer = {}
ALuaPlayer.__index = ALuaPlayer

function ALuaPlayer:new(cpp_actor)
    local inst = setmetatable({}, ALuaPlayer)
    inst.cpp_actor      = cpp_actor
    inst.IsCharging     = false
    inst.ChargeForce    = 0

    -- 즉시 전역에 등록 (BeginPlay 이전)
    Player = inst
    return inst
end

function ALuaPlayer:BeginPlay()
    -- 입력 리스너 등록 (C++ 쪽에서 MouseDown/Up 이벤트를 Lua로 디스패치)
end

function ALuaPlayer:Tick(delta_time)
    if self.IsCharging then
        self.ChargeForce = self.ChargeForce + delta_time * 50
    end
end

function ALuaPlayer:OnOverlap(other_actor) end
function ALuaPlayer:Destroyed() end
function ALuaPlayer:EndPlay(reason) end

-- C++에서 호출되는 커스텀 이벤트 핸들러 예시
function ALuaPlayer:OnLeftMouseDown()
    self.IsCharging  = true
    self.ChargeForce = 0
    print("[OnLeftMouseDown]")
end

function ALuaPlayer:OnLeftMouseUp()
    if not self.IsCharging then return end
    self.IsCharging = false
    local dir   = self.cpp_actor:GetAimDirection()
    -- Heart 액터 스폰 (C++ 팩토리 함수)
    self.cpp_actor:SpawnHeart(color, dir, self.ChargeForce, GameMode.CurrentPlayerIndex)
    GameMode:NextTurn()
    print("[OnLeftMouseUp]")
end

local function create_actor_instance(cpp_actor)
    return ALuaPlayer:new(cpp_actor)
end

return create_actor_instance
