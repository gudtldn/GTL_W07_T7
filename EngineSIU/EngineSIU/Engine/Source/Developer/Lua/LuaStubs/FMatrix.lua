---@meta

--[[!
@module FMatrix
@description Lua stub for the C++ FMatrix struct, using the .new() factory pattern.
             Represents a 4x4 matrix, typically used for transformations in 3D space.
             Provides type hints for Sol3 bindings or similar frameworks.
]]

---@class FMatrix
--- Represents a 4x4 transformation matrix. Elements M[row][col].
--- Use `FMatrix.new()` for identity or specific factory functions like `FMatrix.CreateRotationMatrix(...)`.
--- Note: Direct access to the M[4][4] array might be handled differently by Lua bindings (e.g., getter/setter methods or specific indexing logic).
local FMatrix = {}

--[[ Type Aliases for dependent types. Adjust if necessary ]]


--- Internal helper class defining constructor overloads. Access via `FMatrix.new`.
---@class FMatrix.Constructor
local Constructor = {}

--- Creates a new FMatrix instance.
---@overload fun():FMatrix Creates an identity matrix.
---@overload fun(Other: FMatrix):FMatrix Creates a copy of another FMatrix. (Reflects potential copy constructor binding)
---@return FMatrix
function Constructor:new(...) end -- Stub function definition

--- Factory function to create new FMatrix objects (typically identity).
--- Use static `Create...` functions for specific matrix types.
FMatrix.new = Constructor.new


--[[ Static Constants (Bound as read-only properties) ]]

---@type FMatrix @The 4x4 Identity Matrix. (Read Only)
FMatrix.Identity = nil


--[[ Static Methods (Bound as functions on the type table) ]]

--- Returns the transpose of the given matrix.
---@param Mat FMatrix The matrix to transpose.
---@return FMatrix The transposed matrix.
function FMatrix.Transpose(Mat) end

--- Returns the inverse of the given matrix. May fail or return identity if the matrix is singular.
---@param Mat FMatrix The matrix to invert.
---@return FMatrix The inverted matrix.
function FMatrix.Inverse(Mat) end

--- Creates a rotation matrix from Euler angles (Roll, Pitch, Yaw).
--- Angle units (degrees/radians) depend on C++ implementation (typically degrees).
---@param roll number Rotation around the X axis.
---@param pitch number Rotation around the Y axis.
---@param yaw number Rotation around the Z axis.
---@return FMatrix The resulting rotation matrix.
function FMatrix.CreateRotationMatrix(roll, pitch, yaw) end

--- Creates a scale matrix.
---@param scaleX number Scale factor along the X axis.
---@param scaleY number Scale factor along the Y axis.
---@param scaleZ number Scale factor along the Z axis.
---@return FMatrix The resulting scale matrix.
function FMatrix.CreateScaleMatrix(scaleX, scaleY, scaleZ) end

--- Transforms a 3D vector (assuming W=0, a direction) by the matrix.
---@param v FVector The direction vector to transform.
---@param m FMatrix The transformation matrix.
---@return FVector The transformed direction vector.
function FMatrix.TransformVector(v, m) end

--- Transforms a 4D vector by the matrix.
--- Note: Same name as the FVector version in C++. Lua binding might rename this (e.g., `TransformVector4`) or use argument checking.
---@param v FVector4 The 4D vector to transform.
---@param m FMatrix The transformation matrix.
---@return FVector4 The transformed 4D vector.
function FMatrix.TransformVector4(v, m) end -- Renamed for clarity, check binding

--- Creates a translation matrix.
---@param position FVector The translation offset.
---@return FMatrix The resulting translation matrix.
function FMatrix.CreateTranslationMatrix(position) end

--- Creates a scale matrix from a scale vector.
---@param InScale FVector Scale factors for X, Y, Z.
---@return FMatrix The resulting scale matrix.
function FMatrix.GetScaleMatrix(InScale) end

--- Creates a translation matrix from a position vector.
---@param InPosition FVector The translation offset.
---@return FMatrix The resulting translation matrix.
function FMatrix.GetTranslationMatrix(InPosition) end

--- Creates a rotation matrix from an FRotator (Pitch, Yaw, Roll).
---@param InRotation FRotator The rotation angles (typically degrees).
---@return FMatrix The resulting rotation matrix.
function FMatrix.GetRotationMatrix(InRotation) end

--- Creates a rotation matrix from an FQuat quaternion.
---@param InRotation FQuat The rotation quaternion.
---@return FMatrix The resulting rotation matrix.
function FMatrix.GetRotationMatrixFromQuat(InRotation) end -- Renamed for clarity, check binding


--[[ Member Methods (Called using colon syntax: `myMatrix:Method()`) ]]

--- Transforms a 4D vector by this matrix. Equivalent to `FMatrix.TransformVector4(vector, self)`.
---@param self FMatrix
---@param vector FVector4 The 4D vector to transform.
---@return FVector4 The transformed 4D vector.
function FMatrix:TransformFVector4(vector) end

--- Transforms a 3D vector (assuming W=1, a position) by this matrix. Includes translation.
---@param self FMatrix
---@param vector FVector The position vector to transform.
---@return FVector The transformed position vector.
function FMatrix:TransformPosition(vector) end

--- Converts this matrix (or potentially another matrix passed as M?) to a quaternion.
--- Note: The C++ signature `ToQuat(const FMatrix& M)` is unusual for a member function.
--- Assuming it's intended to convert `self` to a quaternion. Check binding implementation.
---@param self FMatrix
---@return FQuat The equivalent quaternion representation.
function FMatrix:ToQuat() end -- Corrected based on common usage, verify binding


--[[ Operators (Bound via Sol3 using metamethods: __add, __sub, __mul, __div) ]]
-- Note: Matrix multiplication is non-commutative (A * B != B * A).
-- Note: `operator[]` for row access might be bound differently in Lua (e.g., `matrix:GetRow(i)` or specific index metamethods).

--- Adds two matrices component-wise. Returns a new FMatrix.
---@operator add(FMatrix): FMatrix

--- Subtracts another matrix component-wise. Returns a new FMatrix.
---@operator sub(FMatrix): FMatrix

--- Multiplies this matrix by another matrix (standard matrix multiplication). Returns a new FMatrix. Order matters: A * B applies B then A.
---@operator mul(FMatrix): FMatrix
--- Multiplies each element of the matrix by a scalar. Returns a new FMatrix.
---@operator mul(number): FMatrix

--- Divides each element of the matrix by a scalar. Returns a new FMatrix.
---@operator div(number): FMatrix

--[[ Operator[] comment:
-- C++ allows `matrix[row]` to access rows. Lua bindings might expose this via:
-- - Metamethods: `__index` / `__newindex` allowing `matrix[rowIndex]` or `matrix[rowIndex][colIndex]` (less common for matrices).
-- - Specific functions: `matrix:GetRow(rowIndex)` -> table, `matrix:GetElement(rowIndex, colIndex)` -> number.
-- Consult your specific Lua binding documentation for matrix element access.
]]


return FMatrix