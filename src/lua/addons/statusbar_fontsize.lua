-- © 2015 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

local string_format = string.format

-----------------------------------------------------------------------------
-- Build the status bar.

do
	local function cb(event, token, terms)
		terms[#terms+1] = 
			{
				priority=70,
				value=string_format("%dpt",
					DocumentSet.addons.pageconfig.fontsize or 12)
			}
	end
	
	AddEventListener(Event.BuildStatusBar, cb)
end

