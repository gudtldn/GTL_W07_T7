---@meta

--[[!
@module ALuaActor
@description Lua stub for the C++ ALuaActor class, inheriting from AActor.
             Represents an Actor designed to interact with Lua scripts in Unreal Engine.
             Provides type hints for Lua scripting environments (e.g., UnLua, sluaunreal).
]]

---@class AActor : UObject -- Placeholder for the base AActor class (assuming UObject is base)
--- Represents the base class for objects that can be placed or spawned in a level.
--- NOTE: Define AActor more completely in its own stub file if needed.
local AActor = {} -- Placeholder definition

---@class ALuaActor : AActor
--- An Actor class specifically designed for Lua integration. Provides standard Actor lifecycle events.
--- Actors are typically spawned into the world by the engine (e.g., via `World:SpawnActor()`) rather than created directly with `.new()` in Lua scripts.
local ALuaActor = {}

--- Internal helper class defining constructor overloads. Access via `ALuaActor.new`.
--- Note: Actors are usually spawned via `World:SpawnActor(ALuaActor, ...)`, not `ALuaActor.new()`.
--- This `.new()` stub primarily serves documentation or potential binding patterns.
---@class ALuaActor.Constructor
local Constructor = {}

--- Creates a new ALuaActor instance (primarily for documentation or specific binding patterns).
--- In typical Unreal Engine usage, Actors are spawned using engine functions.
---@overload fun():ALuaActor Creates a default ALuaActor instance (before it's added to the world).
---@return ALuaActor
function Constructor:new(...) end -- Stub function definition

--- Factory function to create new ALuaActor objects (use with caution, prefer `World:SpawnActor`).
ALuaActor.new = Constructor.new


--[[ Actor Lifecycle Methods (Overridden from AActor, potentially callable/overridable from Lua) ]]
-- These functions are called automatically by the engine at specific points in the Actor's lifecycle.
-- Lua bindings might allow you to define Lua functions that get called by these C++ implementations,
-- or potentially call the base C++ implementation using `Super:FunctionName()` syntax.

--- Called when the game starts or when spawned. This is where initialization logic often goes.
--- The Lua binding might allow defining a Lua `BeginPlay` function on the Lua side.
---@param self ALuaActor
function ALuaActor:BeginPlay() end

--- Called every frame.
--- The Lua binding might allow defining a Lua `Tick` function on the Lua side.
---@param self ALuaActor
---@param DeltaTime number The time in seconds since the last frame.
function ALuaActor:Tick(DeltaTime) end

--- Called when this actor is explicitly destroyed. Good place for cleanup before garbage collection.
--- The Lua binding might allow defining a Lua `Destroyed` function on the Lua side.
---@param self ALuaActor
function ALuaActor:Destroyed() end

--- Called when the actor is removed from play. Provides a reason for removal.
--- The Lua binding might allow defining a Lua `EndPlay` function on the Lua side.
---@param self ALuaActor
---@param EndPlayReason EEndPlayReason Enum value indicating why play ended (e.g., Destroyed, LevelTransition, Quit). Define EEndPlayReason enum if needed.
function ALuaActor:EndPlay(EndPlayReason) end


--[[ Placeholder for EEndPlayReason Enum ]]
-- Add actual enum values based on your Unreal Engine version if needed for type hinting.
---@alias EEndPlayReason integer|string -- Placeholder type

--[[ Additional Notes:
-- - ALuaActor might expose other properties or functions bound from C++. Add them here as needed.
-- - Common Actor functions like GetActorLocation, SetActorLocation, GetWorld, DestroyActor etc.,
--   would typically be defined in the AActor stub, which ALuaActor inherits.
-- - The `DECLARE_CLASS` macro is C++ specific and doesn't directly translate to Lua stubs.
]]

return ALuaActor