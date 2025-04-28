setmetatable(_ENV, { __index = EngineSIU })

local ALuaActor = {}
ALuaActor.__index = ALuaActor

-- 인스턴스 생성 시 호출될 함수 (생성자 역할)
function ALuaActor:new(cpp_actor)
    local instance = setmetatable({}, ALuaActor)
    instance.cpp_actor = cpp_actor -- C++ 객체 참조 저장

    -- instance.health = 100      -- 인스턴스별 상태
    -- instance.mode = "Idle"

    return instance
end


-- PIE 또는 Spawn이 되었을 때 호출되는 함수
function ALuaActor:BeginPlay()

end


-- 매 프레임마다 호출되는 함수
function ALuaActor:Tick(delta_time)

end


-- 충돌 시 호출되는 함수
function ALuaActor:OnOverlap(other_actor)

end


function ALuaActor:Destroyed()

end


-- 액터가 모든 로직을 끝내고 정리되기 전에 호출되는 함수
function ALuaActor:EndPlay(reason)

end


local function create_actor_instance(cpp_actor)
    return ALuaActor:new(cpp_actor)
end

return create_actor_instance
