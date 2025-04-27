--- This file provides Lua definitions (stubs) for the C++ FColor struct.
--- It helps with static analysis and autocompletion in Lua development environments.
--- Based on the typical binding patterns using sol2.

---@meta -- Indicates this is a metadata/stub file for static analysis.

---@class FColor Represents a color with Red, Green, Blue, and Alpha components.
---@field R number Integer value representing the Red component (0-255).
---@field G number Integer value representing the Green component (0-255).
---@field B number Integer value representing the Blue component (0-255).
---@field A number Integer value representing the Alpha component (0-255).
---@field Bits? number Read-only integer representation of the color (e.g., 0xAARRGGBB). (Availability depends on binding)
local FColor = {}

--- Creates a new FColor object. Use `FColor.new(...)`.
---@class FColor.Constructor
local Constructor = {}

--- Creates a new FColor object.
---@overload fun():FColor Creates a default color (usually R=0, G=0, B=0, A=255).
---@overload fun(r: number, g: number, b: number, a?: number):FColor Creates a color from RGBA components (uint8 0-255). Alpha defaults to 255 if omitted.
---@overload fun(colorValue: number):FColor Creates a color from a 32-bit integer value (e.g., 0xAARRGGBB).
---@overload fun(floatParams: {r: number, g: number, b: number, a?: number}):FColor [Optional] Creates a color from a table of float components (e.g., 0.0-1.0, depends on C++ factory binding).
---@return FColor
function Constructor:new(...) end -- Stub function

--- Access constructor methods via `FColor.new(...)`
FColor.new = Constructor.new

--- Adds another color to this color, clamping each component at 255.
--- Modifies the color instance directly (equivalent to C++ operator+=).
---@param self FColor The FColor instance to modify.
---@param Other FColor The color to add.
function FColor:add_assign(Other) end

--- Returns the color as a 32-bit integer value (e.g., 0xAARRGGBB).
--- (Availability and name depend on C++ binding).
---@param self FColor
---@return number
function FColor:DWColor() end -- Example name, might be different if bound

-- Note: Equality operator (==) is typically overloaded via the __eq metamethod.
-- FColor instances can be compared using `==` in Lua if the C++ operator== is bound
-- via `sol::meta_function::equal_to`.

-- Static Color Constants (Predefined colors)
---@type FColor Predefined White color (255, 255, 255, 255).
FColor.White = nil
---@type FColor Predefined Black color (0, 0, 0, 255).
FColor.Black = nil
---@type FColor Predefined Transparent color (0, 0, 0, 0).
FColor.Transparent = nil
---@type FColor Predefined Red color (255, 0, 0, 255).
FColor.Red = nil
---@type FColor Predefined Green color (0, 255, 0, 255).
FColor.Green = nil
---@type FColor Predefined Blue color (0, 0, 255, 255).
FColor.Blue = nil
---@type FColor Predefined Yellow color (255, 255, 0, 255).
FColor.Yellow = nil
---@type FColor Predefined Cyan color (0, 255, 255, 255).
FColor.Cyan = nil
---@type FColor Predefined Magenta color (255, 0, 255, 255).
FColor.Magenta = nil
---@type FColor Predefined Orange color. (Actual RGB values depend on C++ definition)
FColor.Orange = nil
---@type FColor Predefined Purple color. (Actual RGB values depend on C++ definition)
FColor.Purple = nil
---@type FColor Predefined Turquoise color. (Actual RGB values depend on C++ definition)
FColor.Turquoise = nil
---@type FColor Predefined Silver color. (Actual RGB values depend on C++ definition)
FColor.Silver = nil
---@type FColor Predefined Emerald color. (Actual RGB values depend on C++ definition)
FColor.Emerald = nil


return FColor -- Return the table representing the type (optional, depends on usage)