---@meta

--[[!
@module FVector2D
@description Lua stub for the C++ FVector2D struct, using the .new() factory pattern.
             Represents a 2D vector with X, Y components.
             Provides type hints for Sol3 bindings or similar frameworks.
]]

---@class FVector2D
--- Represents a 2D vector with float components (X, Y).
--- Use `FVector2D.new(...)` to create new instances.
---@field X number The X component (read/write).
---@field Y number The Y component (read/write).
local FVector2D = {}

--[[ Type Aliases (if needed, e.g., for FString if not mapping directly to string) ]]
-- None strictly required based on the provided C++ code, assuming FString maps to string.


--- Internal helper class defining constructor overloads. Access via `FVector2D.new`.
---@class FVector2D.Constructor
local Constructor = {}

--- Creates a new FVector2D instance.
---@overload fun():FVector2D Creates a zero vector (0, 0).
---@overload fun(InX: number, InY: number):FVector2D Creates a vector from individual float components.
---@overload fun(Scalar: number):FVector2D Creates a vector with both components set to Scalar (e.g., FVector2D.new(1.0) -> (1, 1)).
---@return FVector2D
function Constructor:new(...) end -- Stub function definition

--- Factory function to create new FVector2D objects. Refer to overloads for usage.
FVector2D.new = Constructor.new


--[[ Static Constants (Bound as read-only properties) ]]

---@type FVector2D @Vector (0, 0). (Read Only)
FVector2D.ZeroVector = nil
---@type FVector2D @Vector (1, 1). (Read Only)
FVector2D.OneVector = nil


--[[ Member Methods (Called using colon syntax: `myVector:Method()`) ]]

--- Returns a string representation of the vector (e.g., "X=1.0 Y=2.0").
--- Exact format depends on the C++ FString implementation.
---@param self FVector2D
---@return string The string representation.
function FVector2D:ToString() end

--- **[Modifies self]** Initializes this vector by parsing a string representation.
--- The string is expected to contain "X=" and "Y=".
---@param self FVector2D
---@param InSourceString string The string to parse. Assumes FString maps to Lua string.
---@return boolean True if parsing succeeded, false otherwise (reflects C++ return). The vector is invalid if false is returned.
function FVector2D:InitFromString(InSourceString) end


--[[ Operators (Bound via Sol3 using metamethods: __add, __sub, __mul, __div, __eq) ]]
-- Note: Compound assignment operators (+=) from C++ are usually not directly bound as operators in Lua. Use `vec = vec + other`.
-- Note: `!=` is handled automatically by Lua's `~=` if `==` (__eq) is defined.

--- Adds two FVector2Ds component-wise. Returns a new FVector2D.
---@operator add(FVector2D): FVector2D

--- Subtracts another FVector2D component-wise. Returns a new FVector2D.
---@operator sub(FVector2D): FVector2D

--- Multiplies each component by a scalar. Returns a new FVector2D.
---@operator mul(number): FVector2D

--- Divides each component by a scalar. Returns a new FVector2D.
---@operator div(number): FVector2D

--- Checks for *exact* equality (both components must match precisely).
---@operator eq(FVector2D): boolean


return FVector2D