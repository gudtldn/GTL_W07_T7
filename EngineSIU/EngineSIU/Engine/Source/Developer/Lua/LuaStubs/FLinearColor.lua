---@meta

--[[!
@module FLinearColor
@description Lua stub for the C++ FLinearColor struct, adapted to use the .new() factory pattern.
             Intended for Sol3 bindings or similar frameworks.
             Provides type hints for autocompletion and static analysis.
]]

---@class FLinearColor
--- Represents a linear color using float components (Red, Green, Blue, Alpha), typically 0.0 to 1.0.
--- Use `FLinearColor.new(...)` to create new instances.
---@field R number The Red component (read/write).
---@field G number The Green component (read/write).
---@field B number The Blue component (read/write).
---@field A number The Alpha component (read/write).
local FLinearColor = {}

--[[ Type Aliases for dependent types. Adjust if necessary ]]
---@alias FVector4 { X: number, Y: number, Z: number, W: number } -- Placeholder definition


--- Internal helper class defining constructor overloads. Access via `FLinearColor.new`.
---@class FLinearColor.Constructor
local Constructor = {}

--- Creates a new FLinearColor instance.
---@overload fun():FLinearColor Creates a black color (R=0, G=0, B=0, A=0). Note: C++ default is (0,0,0,0), not (0,0,0,1).
---@overload fun(InR: number, InG: number, InB: number, InA?: number):FLinearColor Creates a color from RGBA float components. Alpha defaults to 1.0 if omitted.
---@overload fun(SourceString: string):FLinearColor Creates a color by parsing a string like "(R=1,G=0.5,B=0,A=1)". Assumes FString maps to Lua string.
---@overload fun(InVector: FVector):FLinearColor Creates a color from an FVector (X=R, Y=G, Z=B, A=1.0). Depends on binding configuration for explicit C++ constructors.
---@overload fun(InVector4: FVector4):FLinearColor Creates a color from an FVector4 (X=R, Y=G, Z=B, W=A). Depends on binding configuration for explicit C++ constructors.
---@overload fun(InColor: FColor):FLinearColor Creates a linear color from an FColor (dividing integer components 0-255 by 255.0).
---@return FLinearColor
function Constructor:new(...) end -- Stub function definition

--- Factory function to create new FLinearColor objects. Refer to overloads for usage.
FLinearColor.new = Constructor.new


--[[ Static Default Colors (Bound as read-only properties) ]]

---@type FLinearColor @A predefined white color (1,1,1,1). (Read Only)
FLinearColor.White = nil

---@type FLinearColor @A predefined gray color (0.5, 0.5, 0.5, 1). (Read Only)
FLinearColor.Gray = nil

---@type FLinearColor @A predefined black color (0,0,0,1). (Read Only) Note: Different from default constructor's (0,0,0,0).
FLinearColor.Black = nil

---@type FLinearColor @A predefined transparent color (0,0,0,0). (Read Only)
FLinearColor.Transparent = nil

---@type FLinearColor @A predefined red color (1,0,0,1). (Read Only)
FLinearColor.Red = nil

---@type FLinearColor @A predefined green color (0,1,0,1). (Read Only)
FLinearColor.Green = nil

---@type FLinearColor @A predefined blue color (0,0,1,1). (Read Only)
FLinearColor.Blue = nil

---@type FLinearColor @A predefined yellow color (1,1,0,1). (Read Only)
FLinearColor.Yellow = nil


--[[ Static Methods (Bound as functions on the type table) ]]

--- Creates an FLinearColor from an FColor (uint8 components).
---@param InColor FColor The FColor to convert.
---@return FLinearColor The resulting FLinearColor.
function FLinearColor.FromColor(InColor) end

--- Linearly interpolates between two colors, returning a new color instance.
--- This is the non-mutating version, often preferred in scripting.
---@param Start FLinearColor The starting color.
---@param End FLinearColor The ending color.
---@param Alpha number The interpolation factor (usually clamped 0.0 to 1.0).
---@return FLinearColor The interpolated color.
function FLinearColor.Lerp(Start, End, Alpha) end


--[[ Member Methods (Called using colon syntax: `myColor:Method()`) ]]

--- Returns a new color with components clamped to the specified range.
---@param self FLinearColor
---@param InMin? number The minimum value for each component (default 0.0).
---@param InMax? number The maximum value for each component (default 1.0).
---@return FLinearColor A new FLinearColor with clamped values.
function FLinearColor:GetClamp(InMin, InMax) end

--- Checks if this color is approximately equal to another within a given tolerance.
---@param self FLinearColor
---@param Other FLinearColor The color to compare against.
---@param Tolerance? number The maximum difference allowed for each component (defaults to a small engine value like KINDA_SMALL_NUMBER).
---@return boolean True if colors are close enough, false otherwise.
function FLinearColor:Equals(Other, Tolerance) end

--- **[Warning: Modifies self]** Performs linear interpolation between Start and End based on Alpha and assigns the result *to this color instance*.
--- Use the static `FLinearColor.Lerp(Start, End, Alpha)` for a non-mutating version.
--- Only include this if your binding explicitly exposes the mutating C++ member function.
---@param self FLinearColor
---@param Start FLinearColor The conceptual start color for the interpolation range.
---@param End FLinearColor The conceptual end color for the interpolation range.
---@param Alpha number The interpolation factor.
function FLinearColor:Lerp(Start, End, Alpha) end -- Optional: Check if bound

--- Gets the maximum value among the R, G, B, and A components.
---@param self FLinearColor
---@return number The maximum component value.
function FLinearColor:GetMax() end

--- Gets the minimum value among the R, G, B, and A components.
---@param self FLinearColor
---@return number The minimum component value.
function FLinearColor:GetMin() end

--- Returns a string representation of the color (e.g., "(R=1.0,G=0.5,B=0.0,A=1.0)").
--- Exact format depends on C++ implementation.
---@param self FLinearColor
---@return string The string representation.
function FLinearColor:ToString() end

--- **[Modifies self]** Initializes this color by parsing a string representation.
---@param self FLinearColor
---@param SourceString string The string to parse, typically like "(R=...,G=...,B=...,A=...)".
---@return boolean True if parsing succeeded, false otherwise (reflects C++ return).
function FLinearColor:InitFromString(SourceString) end


--[[ Operators (Bound via Sol3 using metamethods: __add, __sub, __mul, __div, __eq) ]]

--- Adds two FLinearColors component-wise. Returns a new FLinearColor.
---@operator add(FLinearColor): FLinearColor

--- Subtracts another FLinearColor component-wise. Returns a new FLinearColor.
---@operator sub(FLinearColor): FLinearColor

--- Multiplies two FLinearColors component-wise (Hadamard product). Returns a new FLinearColor.
---@operator mul(FLinearColor): FLinearColor
--- Multiplies each component by a scalar. Returns a new FLinearColor.
---@operator mul(number): FLinearColor

--- Divides this FLinearColor by another component-wise. Returns a new FLinearColor.
---@operator div(FLinearColor): FLinearColor
--- Divides each component by a scalar. Returns a new FLinearColor.
---@operator div(number): FLinearColor

--- Checks for *exact* equality (all components must match precisely).
--- Use the `:Equals(other, tolerance)` method for approximate comparisons.
---@operator eq(FLinearColor): boolean

return FLinearColor