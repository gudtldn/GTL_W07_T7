setmetatable(_ENV, { __index = EngineSIU })

-- 전역 Coaches 배열
_G.Coaches = _G.Coaches or {}

local ALuaCoach = {}
ALuaCoach.__index = ALuaCoach

function ALuaCoach:new(cpp_actor)
    local inst = setmetatable({}, ALuaCoach)
    inst.cpp_actor   = cpp_actor
    inst.Name = " "
    inst.Affection   = 0
    inst.MaxAffection= 100

    table.insert(_G.Coaches, inst)
    return inst
end

function ALuaCoach:SetCoachName(coachName)
    self.Name = coachName
end

function ALuaCoach:BeginPlay()
    -- UI 초기화, 프로퍼티 노출 등
end

function ALuaCoach:Tick(delta_time)
    -- 1) 고정 속도 (units/sec)
    local speed = 7 + self.Affection / 10

    -- 2) 랜덤 방향(0~2π) 생성
    local angle = math.random() * 2 * math.pi

    -- 3) 프레임별 이동량 = 속도 × delta_time
    local dx = math.cos(angle) * speed * delta_time
    local dy = math.sin(angle) * speed * delta_time

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
end

function ALuaCoach:Destroyed() end
function ALuaCoach:EndPlay(reason) end

function ALuaCoach:OnOverlap(other_actor)

    if other_actor.cpp_actor:GetClassName() == "ALuaHeartActor" then

        local playerIdx = other_actor.OwnerIndex
        -- 색상 일치 시 +10, 아니면 -5
        if other_actor.Color == self.Color then
            self.Affection = math.min(self.Affection + 10, self.MaxAffection)
        else
            self.Affection = math.max(self.Affection - 5, 0)
        end
        self.cpp_actor:SetAffection(self.Affection)

        -- 최대에 도달했으면 승리 처리
        if self.Affection >= self.MaxAffection then
            GameMode:EndGame(playerIdx)
        end
    end

end

local function create_actor_instance(cpp_actor)
    return ALuaCoach:new(cpp_actor)
end

return create_actor_instance
