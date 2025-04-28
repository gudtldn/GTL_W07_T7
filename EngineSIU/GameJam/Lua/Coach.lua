setmetatable(_ENV, { __index = EngineSIU })

-- 전역 Coaches 배열
Coaches = Coaches or {}

local ALuaCoach = {}
ALuaCoach.__index = ALuaCoach

function ALuaCoach:new(cpp_actor)
    local inst = setmetatable({}, ALuaCoach)
    inst.cpp_actor   = cpp_actor
    inst.Name = " "
    inst.Affection   = 0
    inst.MaxAffection= 100

    table.insert(Coaches, inst)
    return inst
end

function ALuaCoach:SetCoachName(coachName)
    self.Name = coachName
end

function ALuaCoach:BeginPlay()
    -- UI 초기화, 프로퍼티 노출 등
end

function ALuaCoach:Tick(delta_time) end
function ALuaCoach:Destroyed() end
function ALuaCoach:EndPlay(reason) end

function ALuaCoach:OnOverlap(other_actor)

    if other_actor.cpp_actor:GetClassName() == "ALuaHeartActor" then

        self.cpp_actor:SetActorLocation(
        self.cpp_actor:GetActorLocation()
        + FVector(0, 0, 10)
        )

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

        self.cpp_actor:Destroy()
    else
        self.cpp_actor:Destroy()
    
    end

end

local function create_actor_instance(cpp_actor)
    return ALuaCoach:new(cpp_actor)
end

return create_actor_instance
