-- © 2015 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

local string_format = string.format

-----------------------------------------------------------------------------
-- Build the status bar.

do
	local function cb(event, token, terms)
		local pageconf = DocumentSet.addons.pageconfig or {}
		local orientation = "port"
		if pageconf.landscape then
			orientation = "land"
		end
		terms[#terms+1] = 
			{
				priority=60,
				value=string_format("%s %s",
					DocumentSet.addons.pageconfig.pagesize,
					orientation)
			}
	end
	
	AddEventListener(Event.BuildStatusBar, cb)
end

