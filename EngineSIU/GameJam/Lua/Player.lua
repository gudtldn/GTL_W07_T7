setmetatable(_ENV, { __index = EngineSIU })

-- 전역 Player 인스턴스
Player = nil

local ALuaPlayer = {}
ALuaPlayer.__index = ALuaPlayer

local DEFAULT_MAX_CHARGE    = 500   -- 최대 충전값
local DEFAULT_CHARGE_SPEED  = 50    -- 충전 속도 (기본)

function ALuaPlayer:new(cpp_actor)
    local inst = setmetatable({}, ALuaPlayer)
    inst.cpp_actor      = cpp_actor
    inst.IsCharging     = false
    inst.ChargeForce    = 0
    inst.ChargeDir       =  1      -- 1: 증가, -1: 감소
    inst.MaxChargeForce  = DEFAULT_MAX_CHARGE
    inst.ChargeSpeed     = DEFAULT_CHARGE_SPEED

    -- 즉시 전역에 등록 (BeginPlay 이전)
    Player = inst
    return inst
end

function ALuaPlayer:BeginPlay()
    -- 입력 리스너 등록 (C++ 쪽에서 MouseDown/Up 이벤트를 Lua로 디스패치)
    local pos = self.cpp_actor:GetActorLocation()
    SetCameraLocation(pos)

end

function ALuaPlayer:Tick(delta_time)
    if self.IsCharging then
        self.ChargeForce = self.ChargeForce + delta_time * self.ChargeSpeed * self.ChargeDir

        -- 최대값 도달 시 방향 반전
        if self.ChargeForce >= self.MaxChargeForce then
            self.ChargeForce = self.MaxChargeForce
            self.ChargeDir   = -1
        elseif self.ChargeForce <= 0 then
            self.ChargeForce = 0
            self.ChargeDir   = 1
        end
    end
end

function ALuaPlayer:OnOverlap(other_actor) end
function ALuaPlayer:Destroyed() end
function ALuaPlayer:EndPlay(reason) end

-- C++에서 호출되는 커스텀 이벤트 핸들러 예시
function ALuaPlayer:OnLeftMouseDown()
    self.IsCharging  = true
    self.ChargeForce = 0
    self.ChargeDir   = 1   
    print("[OnLeftMouseDown]")
end

function ALuaPlayer:OnLeftMouseUp()
    if not self.IsCharging then return end
    self.IsCharging = false
    local dir   = GetCameraForwardVector()
    local pos = self.cpp_actor:GetActorLocation()
    -- Heart 액터 스폰 (C++ 팩토리 함수)
    self.cpp_actor:SpawnHeart(pos, dir, self.ChargeForce, GameMode.CurrentPlayerIndex)
    self.ChargeForce = 0
    GameMode:NextTurn()
    print("[OnLeftMouseUp]")
end

local function create_actor_instance(cpp_actor)
    return ALuaPlayer:new(cpp_actor)
end

return create_actor_instance
