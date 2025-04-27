---@meta

--[[!
@module FQuat
@description Lua stub for the C++ FQuat struct, using the .new() factory pattern.
             Represents a rotation in 3D space using a Quaternion (W, X, Y, Z).
             Provides type hints for Sol3 bindings or similar frameworks.
]]

---@class FQuat
--- Represents a rotation using a Quaternion (W, X, Y, Z components).
--- Use `FQuat.new(...)` to create new instances. The identity quaternion is (W=1, X=0, Y=0, Z=0).
---@field W number The scalar component.
---@field X number The vector component along the X axis.
---@field Y number The vector component along the Y axis.
---@field Z number The vector component along the Z axis.
local FQuat = {}

--[[ Type Aliases for dependent types. Adjust if necessary ]]


--- Internal helper class defining constructor overloads. Access via `FQuat.new`.
---@class FQuat.Constructor
local Constructor = {}

--- Creates a new FQuat instance.
---@overload fun():FQuat Creates an identity quaternion (W=1, X=0, Y=0, Z=0), representing no rotation.
---@overload fun(Axis: FVector, Angle: number):FQuat Creates a quaternion representing a rotation of `Angle` (likely radians, check binding) around the given `Axis`.
---@overload fun(InW: number, InX: number, InY: number, InZ: number):FQuat Creates a quaternion from individual W, X, Y, Z components.
---@overload fun(InMatrix: FMatrix):FQuat Creates a quaternion from a rotation matrix.
---@return FQuat
function Constructor:new(...) end -- Stub function definition

--- Factory function to create new FQuat objects. Refer to overloads for usage.
FQuat.new = Constructor.new


--[[ Static Methods (Bound as functions on the type table) ]]

--- Creates a quaternion representing a rotation from an axis and an angle.
---@param Axis FVector The normalized axis of rotation.
---@param Angle number The angle of rotation (likely in radians, confirm with binding implementation).
---@return FQuat A new FQuat representing the axis-angle rotation.
function FQuat.FromAxisAngle(Axis, Angle) end

--- Creates a quaternion from Euler angles (Roll, Pitch, Yaw).
--- Order of application and angle units (degrees/radians) depend on the C++ implementation. Assumed degrees typical for Roll/Pitch/Yaw.
---@param roll number Rotation around the X axis (degrees).
---@param pitch number Rotation around the Y axis (degrees).
---@param yaw number Rotation around the Z axis (degrees).
---@return FQuat A new FQuat representing the combined rotation.
function FQuat.CreateRotation(roll, pitch, yaw) end


--[[ Member Methods (Called using colon syntax: `myQuat:Method()`) ]]

--- Rotates a given vector by this quaternion.
---@param self FQuat
---@param Vec FVector The vector to rotate.
---@return FVector The rotated vector.
function FQuat:RotateVector(Vec) end

--- Checks if this quaternion is normalized (its magnitude is close to 1).
---@param self FQuat
---@return boolean True if the quaternion's length is approximately 1.0.
function FQuat:IsNormalized() end

--- Returns a normalized copy of this quaternion (unit length).
--- Does not modify the original quaternion. Handles zero-length quaternions safely (usually returns identity).
---@param self FQuat
---@return FQuat A new FQuat with length 1.
function FQuat:Normalize() end -- Note: C++ is 'const', returns a copy.

--- Converts this quaternion into an equivalent 4x4 rotation matrix.
---@param self FQuat
---@return FMatrix The equivalent rotation matrix.
function FQuat:ToMatrix() end


--[[ Operators (Bound via Sol3 using metamethods: __mul) ]]
-- Note: Quaternion multiplication is non-commutative (A * B != B * A).
--       It represents combining rotations: applying rotation B then rotation A.

--- Multiplies this quaternion by another. Combines the rotations (Other is applied first, then self).
---@operator mul(FQuat): FQuat


--[[ Missing C++ Features (Not in snippet, but common for Quaternions):
-- - FQuat:Inverse() -> FQuat: Returns the inverse rotation.
-- - FQuat.Slerp(Q1, Q2, Alpha) -> FQuat: Spherical Linear Interpolation.
-- - FQuat:ToAxisAndAngle(OutAxis, OutAngle): Extracts axis and angle.
-- - FQuat:ToEuler() -> FVector or {Pitch, Yaw, Roll}: Converts to Euler angles.
-- - operator*(FVector): Often overloaded to mean RotateVector.
-- - operator==, operator!=: Equality checks (exact or with tolerance).
-- Add these if they are present in your full FQuat implementation and bound to Lua.
]]

return FQuat