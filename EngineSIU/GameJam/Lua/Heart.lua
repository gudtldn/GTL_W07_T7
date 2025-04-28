setmetatable(_ENV, { __index = EngineSIU })

local ALuaHeart = {}
ALuaHeart.__index = ALuaHeart

function ALuaHeart:new(cpp_actor)
    local inst = setmetatable({}, ALuaHeart)
    inst.cpp_actor       = cpp_actor
    inst.Color           = cpp_actor.Color         -- C++가 초기화한 색상
    inst.OwnerIndex      = cpp_actor.OwnerIndex    -- 던진 플레이어 인덱스
    return inst
end

function ALuaHeart:BeginPlay()
    -- nothing
end

function ALuaHeart:Tick(delta_time)
    -- C++ 쪽 물리 업데이트
    self.cpp_actor:Move(delta_time)
end

function ALuaHeart:OnOverlap(other_actor)
    -- Coach.lua 쪽 OnOverlap 로 호출
    other_actor:OnOverlap(self)
    -- 충돌 후 자신은 제거
    self.cpp_actor:Destroy()
end

function ALuaHeart:Destroyed() end
function ALuaHeart:EndPlay(reason) end

local function create_actor_instance(cpp_actor)
    return ALuaHeart:new(cpp_actor)
end

return create_actor_instance
