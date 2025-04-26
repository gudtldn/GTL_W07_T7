---@meta 

--[[!
@module FVector
@description Lua stub for the C++ FVector struct, using the .new() factory pattern.
             Represents a 3D vector with X, Y, Z components.
             Provides type hints for Sol3 bindings or similar frameworks.
]]

---@class FVector
--- Represents a 3D vector with float components (X, Y, Z).
--- Use `FVector.new(...)` to create new instances.
---@field X number The X component (read/write).
---@field Y number The Y component (read/write).
---@field Z number The Z component (read/write).
local FVector = {}

--[[ Type Aliases for dependent types. Adjust if necessary ]]


--- Internal helper class defining constructor overloads. Access via `FVector.new`.
---@class FVector.Constructor
local Constructor = {}

--- Creates a new FVector instance.
---@overload fun():FVector Creates a zero vector (0, 0, 0).
---@overload fun(X: number, Y: number, Z: number):FVector Creates a vector from individual float components.
---@overload fun(Scalar: number):FVector Creates a vector with all components set to Scalar (e.g., FVector.new(1.0) -> (1, 1, 1)). Depends on binding configuration for explicit C++ constructor.
---@overload fun(InRotator: FRotator):FVector Creates a vector from an FRotator (typically converting angles to direction/vector components). Depends on binding configuration for explicit C++ constructor.
---@return FVector
function Constructor:new(...) end -- Stub function definition

--- Factory function to create new FVector objects. Refer to overloads for usage.
FVector.new = Constructor.new


--[[ Static Constants (Bound as read-only properties) ]]

---@type FVector @Vector (0, 0, 0). (Read Only)
FVector.ZeroVector = nil
---@type FVector @Vector (1, 1, 1). (Read Only)
FVector.OneVector = nil
---@type FVector @Vector (0, 0, 1). World up direction. (Read Only)
FVector.UpVector = nil
---@type FVector @Vector (0, 0, -1). World down direction. (Read Only)
FVector.DownVector = nil
---@type FVector @Vector (1, 0, 0). Forward direction (X-axis). (Read Only)
FVector.ForwardVector = nil
---@type FVector @Vector (-1, 0, 0). Backward direction (-X-axis). (Read Only)
FVector.BackwardVector = nil
---@type FVector @Vector (0, 1, 0). Right direction (Y-axis). (Read Only)
FVector.RightVector = nil
---@type FVector @Vector (0, -1, 0). Left direction (-Y-axis). (Read Only)
FVector.LeftVector = nil
---@type FVector @Vector (1, 0, 0). Unit X Axis. (Read Only)
FVector.XAxisVector = nil
---@type FVector @Vector (0, 1, 0). Unit Y Axis. (Read Only)
FVector.YAxisVector = nil
---@type FVector @Vector (0, 0, 1). Unit Z Axis. (Read Only)
FVector.ZAxisVector = nil


--[[ Static Methods (Bound as functions on the type table) ]]

--- Returns the zero vector (0,0,0). Equivalent to `FVector.ZeroVector`.
---@return FVector
function FVector.Zero() end

--- Returns the one vector (1,1,1). Equivalent to `FVector.OneVector`.
---@return FVector
function FVector.One() end

--- Returns the unit X vector (1,0,0). Equivalent to `FVector.XAxisVector`.
---@return FVector
function FVector.UnitX() end

--- Returns the unit Y vector (0,1,0). Equivalent to `FVector.YAxisVector`.
---@return FVector
function FVector.UnitY() end

--- Returns the unit Z vector (0,0,1). Equivalent to `FVector.ZAxisVector`.
---@return FVector
function FVector.UnitZ() end

--- Calculates the Euclidean distance between two points (vectors).
---@param V1 FVector The first point.
---@param V2 FVector The second point.
---@return number The distance between V1 and V2.
function FVector.Distance(V1, V2) end

--- Calculates the dot product of two vectors.
---@param A FVector The first vector.
---@param B FVector The second vector.
---@return number The dot product (A | B).
function FVector.DotProduct(A, B) end

--- Calculates the cross product of two vectors.
---@param A FVector The first vector.
---@param B FVector The second vector.
---@return FVector The cross product (A ^ B).
function FVector.CrossProduct(A, B) end

--- Returns a vector with the absolute value of each component of the input vector.
---@param v FVector The input vector.
---@return FVector A new vector with absolute components (|X|, |Y|, |Z|).
function FVector.GetAbs(v) end


--[[ Member Methods (Called using colon syntax: `myVector:Method()`) ]]

--- Calculates the dot product between this vector and another.
---@param self FVector
---@param Other FVector The other vector.
---@return number The dot product (self | Other).
function FVector:Dot(Other) end
-- Note: The C++ operator| is typically bound to this Dot method in Lua.

--- Calculates the cross product between this vector and another.
---@param self FVector
---@param Other FVector The other vector.
---@return FVector The cross product (self ^ Other).
function FVector:Cross(Other) end
-- Note: The C++ operator^ is typically bound to this Cross method in Lua.

--- Checks if this vector is approximately equal to another within a given tolerance.
---@param self FVector
---@param V FVector The vector to compare against.
---@param Tolerance? number The maximum difference allowed for each component (defaults to a small engine value like KINDA_SMALL_NUMBER).
---@return boolean True if vectors are close enough, false otherwise.
function FVector:Equals(V, Tolerance) end

--- Checks if all components (X, Y, Z) of this vector are approximately equal to each other.
---@param self FVector
---@param Tolerance? number The maximum difference allowed between components (defaults to KINDA_SMALL_NUMBER).
---@return boolean True if X, Y, and Z are nearly equal, false otherwise.
function FVector:AllComponentsEqual(Tolerance) end

--- Calculates the magnitude (length) of this vector.
---@param self FVector
---@return number The vector's length |V|.
function FVector:Length() end

--- Calculates the squared magnitude (length) of this vector. Often faster than Length() as it avoids a square root.
---@param self FVector
---@return number The vector's length squared |V|^2.
function FVector:SquaredLength() end

--- Calculates the squared magnitude (length) of this vector. Alias for SquaredLength().
---@param self FVector
---@return number The vector's length squared |V|^2.
function FVector:SizeSquared() end -- Alias

--- **[Modifies self]** Normalizes this vector in-place, making its length 1.
--- Returns false if the vector length is too small (close to zero) to normalize safely.
---@param self FVector
---@param Tolerance? number The minimum squared length required to normalize (defaults to SMALL_NUMBER).
---@return boolean True if normalization was successful, false otherwise.
function FVector:Normalize(Tolerance) end

--- Returns a normalized copy of this vector without checking for zero length.
--- Potentially results in NaN/Inf components if the original vector is zero. Use GetSafeNormal for safety.
---@param self FVector
---@return FVector A new vector with length 1 (or NaN/Inf if input is zero).
function FVector:GetUnsafeNormal() end

--- Returns a normalized copy of this vector, safely handling non-zero length checks.
--- Returns a zero vector if the original vector's length is too small.
---@param self FVector
---@param Tolerance? number The minimum squared length required for normalization (defaults to SMALL_NUMBER).
---@return FVector A new vector with length 1, or (0,0,0) if the input length is too small.
function FVector:GetSafeNormal(Tolerance) end

--- Returns a vector containing the minimum of each component between this vector and another.
---@param self FVector
---@param Other FVector The vector to compare against.
---@return FVector A new vector (min(X1, X2), min(Y1, Y2), min(Z1, Z2)).
function FVector:ComponentMin(Other) end

--- Returns a vector containing the maximum of each component between this vector and another.
---@param self FVector
---@param Other FVector The vector to compare against.
---@return FVector A new vector (max(X1, X2), max(Y1, Y2), max(Z1, Z2)).
function FVector:ComponentMax(Other) end

--- Checks if the vector is close to the zero vector within a tolerance.
---@param self FVector
---@param Tolerance? number Maximum absolute value allowed for each component (defaults to SMALL_NUMBER).
---@return boolean True if all components are close to zero, false otherwise.
function FVector:IsNearlyZero(Tolerance) end

--- Checks if the vector is exactly the zero vector (0,0,0).
---@param self FVector
---@return boolean True if X, Y, and Z are all exactly 0.0.
function FVector:IsZero() end

--- Checks if the vector is normalized (its length is close to 1).
---@param self FVector
---@return boolean True if the vector's length is approximately 1.0.
function FVector:IsNormalized() end -- Note: Tolerance used internally depends on C++ implementation

--- Returns a string representation of the vector (e.g., "X=1.0 Y=2.0 Z=3.0").
--- Exact format depends on C++ implementation.
---@param self FVector
---@return string The string representation.
function FVector:ToString() end

--- **[Modifies self]** Initializes this vector by parsing a string representation.
---@param self FVector
---@param InSourceString string The string to parse, format depends on C++ implementation (e.g., "X=1,Y=2,Z=3"). Assumes FString maps to Lua string.
---@return boolean True if parsing succeeded, false otherwise (reflects C++ return).
function FVector:InitFromString(InSourceString) end


--[[ Operators (Bound via Sol3 using metamethods: __add, __sub, __mul, __div, __unm, __eq) ]]
-- Note: Compound assignment operators (+=, -=, *=, /=) from C++ are usually not directly bound as operators in Lua.
--       Use `vec = vec + other` etc. instead.
--       Indexing (`[]`) might be bound via __index/__newindex metamethods; consult your binding documentation.

--- Adds two FVectors component-wise. Returns a new FVector.
---@operator add(FVector): FVector

--- Subtracts another FVector component-wise. Returns a new FVector.
---@operator sub(FVector): FVector

--- Multiplies two FVectors component-wise (Hadamard product). Returns a new FVector.
---@operator mul(FVector): FVector
--- Multiplies each component by a scalar. Returns a new FVector.
---@operator mul(number): FVector

--- Divides this FVector by another component-wise. Returns a new FVector.
---@operator div(FVector): FVector
--- Divides each component by a scalar. Returns a new FVector.
---@operator div(number): FVector

--- Negates the vector component-wise. Returns a new FVector (-X, -Y, -Z).
---@operator unm(): FVector

--- Checks for *exact* equality (all components must match precisely).
--- Use the `:Equals(other, tolerance)` method for approximate comparisons.
---@operator eq(FVector): boolean


return FVector