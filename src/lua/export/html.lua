--[[--
File              : html.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 01.01.2024
Last Modified Date: 31.07.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--
-- © 2008 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

local string_format = string.format

-----------------------------------------------------------------------------
-- The exporter itself.

local function unhtml(s)
	s = s:gsub("&", "&amp;")
	s = s:gsub("<", "&lt;")
	s = s:gsub(">", "&gt;")
	return s
end

local style_tab =
{
	["H1"] = {pre=false, list=false,
		on='<h1>', off='</h1>'},
	["H2"] = {pre=false, list=false,
		on='<h2>', off='</h2>'},
	["H3"] = {pre=false, list=false,
		on='<h3>', off='</h3>'},
	["H4"] = {pre=false, list=false,
		on='<h4>', off='</h4>'},
	["P"] =  {pre=false, list=false,
		on='<p>', off='</p>'},
	["L"] =  {pre=false, list=true,
		on='<li style="list-style-type: none;">', off='</li>'},
	["LB"] = {pre=false, list=true,
		on='<li>', off='</li>'},
	["LN"] = {pre=false, list=true,
		on=nil, off='</li>'},
	["Q"] =  {pre=false, list=false,
		on='<blockquote>', off='</blockquote>'},
	["V"] =  {pre=false, list=false,
		on='<blockquote>', off='</blockquote>'},
	["RAW"] = {pre=false, list=false,
		on='', off=''},
	["PRE"] = {pre=true, list=false,
		on='<pre>', off='</pre>'},
	["BOTH"] =  {pre=false, list=false,
		on='<p style="text-align:justify;">', off='</p>'},
	["CENTER"] =  {pre=false, list=false,
		on='<p style="text-align:center;">', off='</p>'},
	["LEFT"] =  {pre=false, list=false,
		on='<p style="text-align:left;">', off='</p>'},
	["RIGHT"] =  {pre=false, list=false,
		on='<p style="text-align:right;">', off='</p>'},
	["IMG"]   =  {pre=false, list=false,
		on='', off=''},
}


local function callback(writer, document)
	local settings = DocumentSet.addons.htmlexport
	local currentstylename = nil
	local islist = false
	
	function changepara(para)
		local newstylename = para and para.style
		local currentstyle = style_tab[currentstylename]
		local newstyle = style_tab[newstylename]
		
		if (newstylename ~= currentstylename) or
			not newstylename or
			not currentstyle.pre or
			not newstyle.pre
		then
			if currentstyle then
				writer(currentstyle.off)
			end
			writer("\n")

			if (not newstyle or not newstyle.list) and islist then
				writer("</ul>\n")
				islist = false
			end
			if (newstyle and newstyle.list) and not islist then
				islist = true
				writer("<ul>\n")
			end

			if newstyle then
				if newstylename == "LN" then
					writer(string.format('<li style="list-style-type: decimal;" value=%d>', para.number))
				else
					writer(newstyle.on)
				end
			end
			currentstylename = newstylename
		else
			writer("\n")
		end
	end
		
	return ExportFileUsingCallbacks(document,
	{
		prologue = function()
			writer('<html xmlns="http://www.w3.org/1999/xhtml"><head>\n')
			writer('<meta http-equiv="Content-Type" content="text/html;charset=utf-8"/>\n')
			writer('<meta name="generator" content="WordGrinder '..VERSION..'"/>\n')
			writer('<title>', unhtml(document.name), '</title>\n')
			writer('</head><body>\n')
		end,
		
		rawtext = function(s)
			writer(s)
		end,
		
		text = function(s)
			writer(unhtml(s))
		end,
		
		notext = function(s)
			if (currentstylename ~= "PRE") then
				writer('<br/>')
			end
		end,
		
		italic_on = function()
			writer(settings.italic_on)
		end,
		
		italic_off = function()
			writer(settings.italic_off)
		end,
		
		underline_on = function()
			writer(settings.underline_on)
		end,
		
		underline_off = function()
			writer(settings.underline_off)
		end,
		
		bold_on = function()
			writer(settings.bold_on)
		end,
		
		bold_off = function()
			writer(settings.bold_off)
		end,
		
		list_start = function()
		end,
		
		list_end = function()
		end,
		
		paragraph_start = function(para)
			changepara(para)
		end,		
		
		paragraph_end = function(para)
		end,
		
		table_start = function(para)
			changepara(para)
			writer('<table width="100%" cellpadding="4" cellspacing="0">')
		end,		
		
		table_end = function(para)
			writer('</table>')
		end,
		
		tablerow_start = function(para)
			writer('<tr valign="top">')
		end,
		
		tablerow_end = function(para)
			writer('</tr>')
		end,
		
		tablecell_start = function(para)
			if para.style == "TRB" then
				writer('<td style="border: 1px solid #000000; padding: 0.1cm">')
			else
				writer('<td>')
			end
			writer('<p align="justify">')
		end,		
		
		tablecell_end = function(para)
			writer('</p>')
			writer('</td>')
		end,
		
		image_start = function(para)
			changepara(nil, nil)
			writer('<p style="text-align:center;">\n')
		end,
		
		image_end = function(para)
			writer('<img\n')
			writer(string_format('src="%s" ', para.imagename))
			writer(string_format('alt="%s"/>\n', para.imagename))
			writer('</p>\n')
		end,
		
		epilogue = function()
			changepara(nil, nil)
			writer('</body>\n')	
			writer('</html>\n')
		end
	})
end

function Cmd.ExportHTMLFile(filename)
	return ExportFileWithUI(filename, "Export HTML File", ".html",
		callback)
end

function Cmd.ExportToHTMLString()
	return ExportToString(Document, callback)
end

-----------------------------------------------------------------------------
-- Addon registration. Set the HTML export settings.

do
	local function cb()
		DocumentSet.addons.htmlexport = DocumentSet.addons.htmlexport or {
			underline_on = "<u>",
			underline_off = "</u>",
			italic_on = "<i>",
			italic_off = "</i>"
		}

		local s = DocumentSet.addons.htmlexport
		s.bold_on = s.bold_on or "<b>"
		s.bold_off = s.bold_off or "</b>"
	end
	
	AddEventListener(Event.RegisterAddons, cb)
end

-----------------------------------------------------------------------------
-- Configuration user interface.

function Cmd.ConfigureHTMLExport()
	local settings = DocumentSet.addons.htmlexport

	local underline_on_textfield =
		Form.TextField {
			x1 = 16, y1 = 1,
			x2 = -1, y2 = 1,
			value = settings.underline_on
		}

	local underline_off_textfield =
		Form.TextField {
			x1 = 16, y1 = 3,
			x2 = -1, y2 = 3,
			value = settings.underline_off
		}

	local italic_on_textfield =
		Form.TextField {
			x1 = 16, y1 = 5,
			x2 = -1, y2 = 5,
			value = settings.italic_on
		}

	local italic_off_textfield =
		Form.TextField {
			x1 = 16, y1 = 7,
			x2 = -1, y2 = 7,
			value = settings.italic_off
		}

	local bold_on_textfield =
		Form.TextField {
			x1 = 16, y1 = 9,
			x2 = -1, y2 = 9,
			value = settings.bold_on
		}

	local bold_off_textfield =
		Form.TextField {
			x1 = 16, y1 = 11,
			x2 = -1, y2 = 11,
			value = settings.bold_off
		}

	local dialogue =
	{
		title = "Configure HTML Export",
		width = Form.Large,
		height = 13,
		stretchy = false,

		["KEY_^C"] = "cancel",
		["KEY_RETURN"] = "confirm",
		["KEY_ENTER"] = "confirm",

		Form.Label {
			x1 = 1, y1 = 1,
			x2 = 32, y2 = 1,
			align = Form.Left,
			value = "Underline on:"
		},
		underline_on_textfield,
		
		Form.Label {
			x1 = 1, y1 = 3,
			x2 = 32, y2 = 3,
			align = Form.Left,
			value = "Underline off:"
		},
		underline_off_textfield,
		
		Form.Label {
			x1 = 1, y1 = 5,
			x2 = 32, y2 = 5,
			align = Form.Left,
			value = "Italics on:"
		},
		italic_on_textfield,
		
		Form.Label {
			x1 = 1, y1 = 7,
			x2 = 32, y2 = 7,
			align = Form.Left,
			value = "Italics off:"
		},
		italic_off_textfield,
		
		Form.Label {
			x1 = 1, y1 = 9,
			x2 = 32, y2 = 9,
			align = Form.Left,
			value = "Bold on:"
		},
		bold_on_textfield,
		
		Form.Label {
			x1 = 1, y1 = 11,
			x2 = 32, y2 = 11,
			align = Form.Left,
			value = "Bold off:"
		},
		bold_off_textfield,
	}
	
	while true do
		local result = Form.Run(dialogue, RedrawScreen,
			"SPACE to toggle, RETURN to confirm, CTRL+C to cancel")
		if not result then
			return false
		end
		
		settings.underline_on = underline_on_textfield.value
		settings.underline_off = underline_off_textfield.value
		settings.italic_on = italic_on_textfield.value
		settings.italic_off = italic_off_textfield.value
		settings.bold_on = bold_on_textfield.value
		settings.bold_off = bold_off_textfield.value
		return true
	end
		
	return false
end
