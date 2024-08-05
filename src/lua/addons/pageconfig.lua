--[[--
File              : pageconfig.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 03.08.2024
Last Modified Date: 05.08.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--

-----------------------------------------------------------------------------
-- Addon registration. Create the default settings in the DocumentSet.

do
	local function cb()
		DocumentSet.addons.pageconfig = DocumentSet.addons.pageconfig or {
			landscape = false,
			pagesize = "A4",
			top = 2.0,
			left = 2.0,
			bottom = 2.0,
			right = 2.0,
			fontsize = 12,
		}
	end
	
	AddEventListener(Event.RegisterAddons, cb)
end

-----------------------------------------------------------------------------
-- Configuration user interface.

function Cmd.ConfigurePage()
	local settings = DocumentSet.addons.pageconfig or {}
	
	local landscape_checkbox =
		Form.Checkbox {
			x1 = 1, y1 = 1,
			x2 = 33, y2 = 1,
			label = "Enable landscape layout",
			value = settings.landscape
		}

	local pagesize_textfield =
		Form.TextField {
			x1 = 33, y1 = 3,
			x2 = 43, y2 = 3,
			value = tostring(settings.pagesize)
		}
		
	local top_textfield =
		Form.TextField {
			x1 = 33, y1 = 5,
			x2 = 43, y2 = 5,
			value = tostring(settings.top)
		}

	local left_textfield =
		Form.TextField {
			x1 = 33, y1 = 7,
			x2 = 43, y2 = 7,
			value = tostring(settings.left)
		}

	local bottom_textfield =
		Form.TextField {
			x1 = 33, y1 = 9,
			x2 = 43, y2 = 9,
			value = tostring(settings.bottom)
		}

	local right_textfield =
		Form.TextField {
			x1 = 33, y1 = 11,
			x2 = 43, y2 = 11,
			value = tostring(settings.right)
		}

	local fontsize_textfield =
		Form.TextField {
			x1 = 33, y1 = 13,
			x2 = 43, y2 = 13,
			value = tostring(settings.fontsize)
		}

	local dialogue =
	{
		title = "Page Layout Config",
		width = Form.Large,
		height = 15,
		stretchy = false,

		["KEY_^C"] = "cancel",
		["KEY_RETURN"] = "confirm",
		["KEY_ENTER"] = "confirm",
		
		landscape_checkbox,
		
		Form.Label {
			x1 = 1, y1 = 3,
			x2 = 32, y2 = 3,
			align = Form.Left,
			value = "Page size (a4, letter):"
		},
		pagesize_textfield,
			
		Form.Label {
			x1 = 1, y1 = 5,
			x2 = 32, y2 = 5,
			align = Form.Left,
			value = "Page top margin (cm):"
		},
		top_textfield,
		
		Form.Label {
			x1 = 1, y1 = 7,
			x2 = 32, y2 = 7,
			align = Form.Left,
			value = "Page left margin (cm):"
		},
		left_textfield,
		
		Form.Label {
			x1 = 1, y1 = 9,
			x2 = 32, y2 = 9,
			align = Form.Left,
			value = "Page bottom margin (cm):"
		},
		bottom_textfield,
		
		Form.Label {
			x1 = 1, y1 = 11,
			x2 = 32, y2 = 11,
			align = Form.Left,
			value = "Page right margin (cm):"
		},
		right_textfield,

		Form.Label {
			x1 = 1, y1 = 11,
			x2 = 32, y2 = 11,
			align = Form.Left,
			value = "Default font size (12, 14):"
		},
		fontsize_textfield,
	}
	
	while true do
		local result = Form.Run(dialogue, RedrawScreen,
			"SPACE to toggle, RETURN to confirm, CTRL+C to cancel")
		if not result then
			return false
		end
		
		local landscape = landscape_checkbox.value
		local pagesize = pagesize_textfield.value
		
		local top = tonumber(top_textfield.value)
		local left = tonumber(left_textfield.value)
		local bottom = tonumber(bottom_textfield.value)
		local right = tonumber(right_textfield.value)
		
		if not pagesize then
			ModalMessage("Parameter error", "Pagesize should be A4, A5 or letter.")
		elseif pagesize ~= "a4" and 
					 pagesize ~= "A4" and
					 pagesize ~= "a5" and
					 pagesize ~= "A5" and
					 pagesize ~= "letter" and
					 pagesize ~= "Letter" and
					 pagesize ~= "LETTER"
		then
			ModalMessage("Parameter error", "Pagesize should be A4, A5, or letter.")
		
		elseif pagesize ~= "12" and 
					 pagesize ~= "14"
		then
			ModalMessage("Parameter error", "Default font size should be 12 or 14")
		
		else
			settings.landscape = landscape
			settings.pagesize = pagesize
			settings.top = top
			settings.left = left
			settings.bottom = bottom
			settings.right = right
			settings.fontsize = fontsize
			DocumentSet:touch()

			return true
		end
	end
		
	return false
end
