setmetatable(_ENV, { __index = EngineSIU })

local ALuaActor = {}
ALuaActor.__index = ALuaActor

-- 인스턴스 생성 시 호출될 함수 (생성자 역할)
function ALuaActor:new(cpp_actor)
    local instance = setmetatable({}, ALuaActor)
    instance.cpp_actor = cpp_actor -- C++ 객체 참조 저장

    print("[new]")

    instance.test = 1;

    instance.acc_time = 0

    return instance
end


-- PIE 또는 Spawn이 되었을 때 호출되는 함수
function ALuaActor:BeginPlay()
    print("[BeginPlay]")
end


-- 매 프레임마다 호출되는 함수
function ALuaActor:Tick(delta_time)
    -- print("[Tick]", delta_time)
    self.cpp_actor:SetActorLocation(
        self.cpp_actor:GetActorLocation()
        + self.cpp_actor:GetActorForwardVector()
        * FVector(10)  -- TODO: 나중에 sol::overload로 오버로드 필요
        * delta_time
    )

    self.cpp_actor:SetActorRotation(
        self.cpp_actor:GetActorRotation()
        + FRotator(0, 175, 0)  -- TODO: 나중에 sol::overload로 오버로드 필요
        * delta_time
    )

    -- self.acc_time = self.acc_time + delta_time

    -- local offset = math.sin(self.acc_time) * 500  -- sin 함수를 이용한 좌우 이동
    -- local new_location = self.cpp_actor:GetActorLocation()
    -- new_location.X = new_location.X + offset
    -- self.cpp_actor:SetActorLocation(new_location * delta_time)
end


-- 충돌 시 호출되는 함수
function ALuaActor:OnOverlap(other_actor)
    self.cpp_actor:SetActorLocation(
        self.cpp_actor:GetActorLocation()
        + self.cpp_actor:GetActorForwardVector()
        + FVector.new(0, 0, 10)
    )
    print("[OnOverlap]", other_actor)
end


function ALuaActor:Destroyed()
    print("[Destroyed]")
end


-- 액터가 모든 로직을 끝내고 정리되기 전에 호출되는 함수
function ALuaActor:EndPlay(reason)
    print("[EndPlay]", reason)
end


local function create_actor_instance(cpp_actor)
    print("Creating Lua Actor Instance")
    return ALuaActor:new(cpp_actor)
end

return create_actor_instance
