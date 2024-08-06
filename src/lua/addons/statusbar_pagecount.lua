-- © 2013 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

-----------------------------------------------------------------------------
-- Build the status bar.

do
	local function cb(event, token, terms)
		local lines = 0
		local cp, cw, co = Document.cp, Document.cw, Document.co
		local cw = Document.cw
		local paragraph = Document[cp]

		-- count lines of prev paragraphs
		local n = 1
		while n < cp do
			for _, l in ipairs(Document[n].lines) do
				lines = lines + 1
			end
			n = n + 1
		end
			
		-- get current line 
		local cl
		cl, cw = paragraph:getLineOfWord(cw)

		cl = lines + cl

		-- get current page
		local pageconf = DocumentSet.addons.pageconfig or {}
		-- if fontsize is 12 - then it should be 0,22 cm for 1
		-- symbol and 0,51 cm for 1 line
		-- if fontsize is 14 - then it should be 0,27 cm for 1
		-- symbol and 0,522 cm for 1 line
		local pageheight = 0;
		if not pageconf.landscape then
			if pageconf.pagesize == "A4" then
				pageheight = 29.7
			elseif pageconf.pagesize == "A5" then
				pageheight = 21.001
			elseif pageconf.pagesize == "letter" then
				pageheight = 27.94
			end
		else
			if pageconf.pagesize == "A4" then
				pageheight = 21.001
			elseif pageconf.pagesize == "A5" then
				pageheight = 14.801
			elseif pageconf.pagesize == "letter" then
				pageheight = 21.59
			end
		end

		pageheight = pageheight - pageconf.top - pageconf.bottom

		local linesperpage = 0
		if pageconf.fontsize == 12 then
			linesperpage = pageheight / 0.51
		else
			linesperpage = pageheight / 0.522
		end

		local page = math.floor(cl / linesperpage + 1)
 
		local settings = DocumentSet.addons.pagecount or {}
		if settings.enabled then
			--local pages = math.floor((Document.wordcount or 0) / settings.wordsperpage)
			terms[#terms+1] = {
				priority=80,
				value=string.format("%d %s", page,
					Pluralise(page, "page", "pages"))
			}
		end
	end
	
	AddEventListener(Event.BuildStatusBar, cb)
end

-----------------------------------------------------------------------------
-- Addon registration. Create the default settings in the DocumentSet.

do
	local function cb()
		DocumentSet.addons.pagecount = DocumentSet.addons.pagecount or {
			enabled = true,
			wordsperpage = 250,
		}
	end
	
	AddEventListener(Event.RegisterAddons, cb)
end

-----------------------------------------------------------------------------
-- Configuration user interface.

function Cmd.ConfigurePageCount()
	local settings = DocumentSet.addons.pagecount

	local enabled_checkbox =
		Form.Checkbox {
			x1 = 1, y1 = 1,
			x2 = 33, y2 = 1,
			label = "Show approximate page count",
			value = settings.enabled
		}

	local count_textfield =
		Form.TextField {
			x1 = 33, y1 = 3,
			x2 = 43, y2 = 3,
			value = tostring(settings.wordsperpage)
		}
		
	local dialogue =
	{
		title = "Configure Page Count",
		width = Form.Large,
		height = 5,
		stretchy = false,

		["KEY_^C"] = "cancel",
		["KEY_RETURN"] = "confirm",
		["KEY_ENTER"] = "confirm",
		
		enabled_checkbox,
		
		Form.Label {
			x1 = 1, y1 = 3,
			x2 = 32, y2 = 3,
			align = Form.Left,
			value = "Number of words per page:"
		},
		count_textfield,
	}
	
	while true do
		local result = Form.Run(dialogue, RedrawScreen,
			"SPACE to toggle, RETURN to confirm, CTRL+C to cancel")
		if not result then
			return false
		end
		
		local enabled = enabled_checkbox.value
		local wordsperpage = tonumber(count_textfield.value)
		
		if not wordsperpage then
			ModalMessage("Parameter error", "The number of words per page must be a valid number.")
		else
			settings.enabled = enabled
			settings.wordsperpage = wordsperpage
			DocumentSet:touch()

			return true
		end
	end
		
	return false
end
