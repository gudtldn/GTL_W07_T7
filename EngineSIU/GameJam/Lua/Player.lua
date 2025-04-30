setmetatable(_ENV, { __index = EngineSIU })

-- 전역 Player 인스턴스
Player = nil

local ALuaPlayer = {}
ALuaPlayer.__index = ALuaPlayer

local DEFAULT_MAX_CHARGE    = 700   -- 최대 충전값
local DEFAULT_MIN_CHARGE    = 300   -- 최소 충전값
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

    -- SpringArm 동작하는지 테스트 하는 코드 

    -- 1) 고정 속도 (units/sec) 
    local speed = 0.39

    local dx = 0
    local dy = speed * delta_time

    -- 4) 현재 위치 가져오기
    local loc = self.cpp_actor:GetActorLocation()
    local newX = loc.X + dx
    local newY = loc.Y + dy

    -- 5) -15~15 범위로 클램프
    if newX < 2 then newX = 2
    elseif newX >  23 then newX =  23 end

    if newY < -13 then newY = -13
    elseif newY >  13 then newY =  13 end

    -- 6) 위치 적용 (Z 축은 그대로)
    self.cpp_actor:SetActorLocation(
        FVector(newX, newY, loc.Z)
    )

    -- SpringArm 테스트 끝
end

function ALuaPlayer:OnOverlap(other_actor) end
function ALuaPlayer:Destroyed() end
function ALuaPlayer:EndPlay(reason) end

-- C++에서 호출되는 커스텀 이벤트 핸들러 예시
function ALuaPlayer:OnLeftMouseDown()
    self.IsCharging  = true
    self.ChargeForce = DEFAULT_MIN_CHARGE
    self.ChargeDir   = 1   
    print("[OnLeftMouseDown]")
    local fadeAmount = CameraEffect.GetFadeAmount();
    CameraEffect.StartCameraFade(fadeAmount, 0.7, 1.0, FLinearColor(0.7, 0.2, 0.2, 1), true, true)
end

function ALuaPlayer:OnLeftMouseUp()
    if not self.IsCharging then return end
    self.IsCharging = false
    local dir   = GetCameraForwardVector()
    local pos = GetCameraLocation()
    -- Heart 액터 스폰 (C++ 팩토리 함수)
    self.cpp_actor:SpawnHeart(pos, dir, self.ChargeForce, GameMode.CurrentPlayerIndex)
    self.ChargeForce = 0
    GameMode:NextTurn()
    print("[OnLeftMouseUp]")

    local fadeAmount = CameraEffect.GetFadeAmount();
    CameraEffect.StartCameraFade(fadeAmount, 0.0, 1.0, FLinearColor(0.7, 0.2, 0.2, 1), true, true)
end

local function create_actor_instance(cpp_actor)
    return ALuaPlayer:new(cpp_actor)
end

return create_actor_instance
