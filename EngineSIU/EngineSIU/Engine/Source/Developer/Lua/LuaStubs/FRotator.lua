---@meta

--[[!
@module FRotator
@description Lua stub for the C++ FRotator struct, using the .new() factory pattern.
             Represents a rotation in 3D space using Pitch, Yaw, and Roll angles (typically in degrees).
             Provides type hints for Sol3 bindings or similar frameworks.
]]

---@class FRotator
--- Represents a rotation using Pitch (Y-axis), Yaw (Z-axis), and Roll (X-axis) angles in degrees.
--- Use `FRotator.new(...)` to create new instances.
---@field Pitch number Rotation around the Y axis (degrees).
---@field Yaw number Rotation around the Z axis (degrees).
---@field Roll number Rotation around the X axis (degrees).
local FRotator = {}

--[[ Type Aliases for dependent types. Adjust if necessary ]]
---@alias FMatrix { } -- Placeholder definition (Structure depends heavily on binding)


--- Internal helper class defining constructor overloads. Access via `FRotator.new`.
---@class FRotator.Constructor
local Constructor = {}

--- Creates a new FRotator instance.
---@overload fun():FRotator Creates a zero rotator (Pitch=0, Yaw=0, Roll=0).
---@overload fun(InPitch: number, InYaw: number, InRoll: number):FRotator Creates a rotator from individual Pitch, Yaw, Roll components (degrees).
---@overload fun(Other: FRotator):FRotator Creates a copy of another FRotator. (Usually handled by Lua assignment, but reflects C++ constructor).
---@overload fun(InVector: FVector):FRotator Creates a rotator from a direction vector. The Roll component will be zero.
---@overload fun(InQuat: FQuat):FRotator Creates a rotator from a quaternion.
---@return FRotator
function Constructor:new(...) end -- Stub function definition

--- Factory function to create new FRotator objects. Refer to overloads for usage.
FRotator.new = Constructor.new


--[[ Static Constants (Example: Often exists in engine code) ]]
-- NOTE: The provided C++ snippet doesn't show static constants like ZeroRotator,
--       but they often exist. Add them here if they are bound.
-- ---@type FRotator @Rotator (0, 0, 0). (Read Only)
-- FRotator.ZeroRotator = nil


--[[ Member Methods (Called using colon syntax: `myRotator:Method()`) ]]

--- Checks if this rotator is nearly zero within a tolerance.
---@param self FRotator
---@param Tolerance? number Maximum absolute value allowed for Pitch, Yaw, and Roll (defaults to KINDA_SMALL_NUMBER).
---@return boolean True if all components are close to zero.
function FRotator:IsNearlyZero(Tolerance) end

--- Checks if this rotator is exactly zero (Pitch=0, Yaw=0, Roll=0).
---@param self FRotator
---@return boolean True if all components are exactly 0.0.
function FRotator:IsZero() end

--- Checks if this rotator is equal to another within a tolerance. Compares normalized versions.
---@param self FRotator
---@param Other FRotator The rotator to compare against.
---@param Tolerance? number Maximum difference allowed between components (defaults to KINDA_SMALL_NUMBER).
---@return boolean True if rotators represent nearly the same orientation.
function FRotator:Equals(Other, Tolerance) end

--- Adds delta Pitch, Yaw, and Roll to this rotator and returns the result as a new rotator.
--- Does not modify the original rotator.
---@param self FRotator
---@param DeltaPitch number Amount to add to Pitch.
---@param DeltaYaw number Amount to add to Yaw.
---@param DeltaRoll number Amount to add to Roll.
---@return FRotator A new FRotator with the added rotation.
function FRotator:Add(DeltaPitch, DeltaYaw, DeltaRoll) end

--- Creates a rotator from a quaternion. (Note: Naming might be confusing, constructor often used instead).
---@param self FRotator This parameter might be ignored if the method acts statically despite being instance-bound.
---@param InQuat FQuat The quaternion to convert.
---@return FRotator A new FRotator representing the quaternion's orientation.
function FRotator:FromQuaternion(InQuat) end

--- Converts this rotator to a quaternion representation.
---@param self FRotator
---@return FQuat The equivalent quaternion.
function FRotator:ToQuaternion() end

--- Converts this rotator to a direction vector (normalized). Represents the direction the rotator is 'facing' (typically along the X-axis after rotation).
---@param self FRotator
---@return FVector The direction vector.
function FRotator:ToVector() end

--- Converts this rotator into a 4x4 rotation matrix.
---@param self FRotator
---@return FMatrix The equivalent rotation matrix.
function FRotator:ToMatrix() end

--- Clamps an angle to the range (-180, 180].
---@param self FRotator Instance needed perhaps for context, but operates on input angle.
---@param Angle number The angle to clamp (in degrees).
---@return number The clamped angle (degrees).
function FRotator:Clamp(Angle) end -- Note: C++ name seems ambiguous; might refer to ClampAxis.

--- Returns a normalized version of this rotator, ensuring angles are within (-180, 180].
--- Does not modify the original rotator.
---@param self FRotator
---@return FRotator A new FRotator with normalized angles.
function FRotator:GetNormalized() end

--- **[Modifies self]** Normalizes this rotator's angles in-place to be within (-180, 180].
---@param self FRotator
function FRotator:Normalize() end

--- Returns a string representation of the rotator (e.g., "P=10.0 Y=20.0 R=30.0").
--- Exact format depends on C++ implementation.
---@param self FRotator
---@return string The string representation.
function FRotator:ToString() end

--- **[Modifies self]** Initializes this rotator by parsing a string representation.
--- The string is expected to contain "P=", "Y=", "R=".
---@param self FRotator
---@param InSourceString string The string to parse. Assumes FString maps to Lua string.
---@return boolean True if parsing succeeded, false otherwise (reflects C++ return). The rotator is invalid if false is returned.
function FRotator:InitFromString(InSourceString) end


--[[ Operators (Bound via Sol3 using metamethods: __add, __sub, __mul, __div, __unm, __eq) ]]
-- Note: Compound assignment operators (+=, -=, *=, /=) are usually not directly bound.

--- Adds two FRotators component-wise. Returns a new FRotator. Normalization might occur depending on binding.
---@operator add(FRotator): FRotator

--- Subtracts another FRotator component-wise. Returns a new FRotator. Normalization might occur.
---@operator sub(FRotator): FRotator

--- Multiplies Pitch, Yaw, Roll by a scalar. Returns a new FRotator.
---@operator mul(number): FRotator

--- Divides this FRotator's components by another's component-wise. Returns a new FRotator. (Use with caution).
---@operator div(FRotator): FRotator
--- Divides Pitch, Yaw, Roll by a scalar. Returns a new FRotator.
---@operator div(number): FRotator

--- Negates Pitch, Yaw, Roll. Returns a new FRotator.
---@operator unm(): FRotator

--- Checks for *exact* equality of Pitch, Yaw, and Roll components.
--- Use the `:Equals(other, tolerance)` method for comparing orientation equivalence.
---@operator eq(FRotator): boolean


return FRotator