--[[--
File              : rtf.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 01.01.2024
Last Modified Date: 19.08.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--
-- © 2011 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

local NextCharInWord = wg.nextcharinword
local ReadU8 = wg.readu8
local ImageToRTF = wg.imagetortf
local getimagesize = wg.getimagesize
local string_len = string.len
local string_char = string.char
local string_format = string.format
local string_gsub = string.gsub
local table_concat = table.concat

-----------------------------------------------------------------------------
-- The exporter itself.

local function unrtf(s)
	local ss = {}
	local o = 1
	local n = string_len(s)
	
	while (o <= n) do
		local c = ReadU8(s, o)
		o = NextCharInWord(s, o)
		if (c == 92) then
			ss[#ss+1] = "\\"
		elseif (c == 123) then
			ss[#ss+1] = "\\{"
		elseif (c == 125) then
			ss[#ss+1] = "\\}"
		elseif (c < 127) then
			ss[#ss+1] = string_char(c)
		elseif (c < 0x10000) then
			ss[#ss+1] = string_format('\\u%d ', c)
		else
			c = c - 0x10000
			ss[#ss+1] = string_format('\\u%d', 0xd800 + (c / 0x400)) 
			ss[#ss+1] = string_format('\\u%d ', 0xdc00 + (c % 0x400)) 
		end
	end
	
	return table_concat(ss)
end

local style_tab = {}

local function style_tab_init()
	local fontsize = DocumentSet.addons.pageconfig.fontsize
	style_tab =
{
	["H1"]     = {15,  string_format('\\fs%d\\sb400\\b\\sbasedon0 H1', fontsize * 3)},
	["H2"]     = {16,  string_format('\\fs%d\\sb360\\b\\sbasedon0 H2', fontsize * 2.5)},
	["H3"]     = {17,  string_format('\\fs%d\\sb320\\b\\sbasedon0 H3', fontsize * 2.2)},
	["H4"]     = {18,  string_format('\\fs%d\\sb280\\b\\sbasedon0 H4', fontsize * 2)},
	["P"]      = {19,  string_format('\\fs%d\\sb140\\sbasedon0 P', fontsize * 2)},
	["L"]      = {20,  string_format('\\fs%d\\sb140\\sbasedon5 L', fontsize * 2)},
	["LB"]     = {21,  string_format('\\fs%d\\sb140\\sbasedon5 LB', fontsize * 2)},
	["LN"]     = {22,  string_format('\\fs%d\\sb140\\sbasedon5 LN', fontsize * 2)},
	["Q"]      = {23,  string_format('\\fs%d\\sb140\\li500\\sbasedon5 Q', fontsize * 2)},
	["V"]      = {24, string_format('\\fs%d\\sb140\\li500\\sbasedon5 V', fontsize * 2)},
	["RAW"]    = {25, string_format('\\fs%d\\sb140\\sbasedon5 RAW', fontsize * 2)},
	["PRE"]    = {26, string_format('\\fs%d\\sb140\\sbasedon5 PRE', fontsize * 2)},
	["LEFT"]   = {27, string_format('\\fs%d\\sb140\\sbasedon5\\ql LEFT', fontsize * 2)},
	["RIGHT"]  = {28, string_format('\\fs%d\\sb140\\sbasedon5\\qr RIGHT', fontsize * 2)},
	["BOTH"]   = {29, string_format('\\fs%d\\sb140\\sbasedon5\\qj BOTH', fontsize * 2)},
	["CENTER"] = {30, string_format('\\fs%d\\sb140\\sbasedon5\\qc CENTER', fontsize * 2)},
	["IMG"]    = {31, string_format('\\fs%d\\sb140\\sbasedon5\\qc IMG', fontsize * 2)},
}
end

local function callback(writer, document)
	local settings = DocumentSet.addons.htmlexport

	style_tab_init()
	
	return ExportFileUsingCallbacks(document,
	{
		prologue = function()
			writer('{\\rtf1\\ansi\\deff0')
			writer('{\\fonttbl{\\f0 Times New Roman}}')
			writer('\\deflang1033\\widowctrl')
			writer('\\uc0\n')

			-- set page size
			local settings = DocumentSet.addons.pageconfig
			local h = 0
			local w = 0
			local x = 0
			local y = 0
			
			if settings.pagesize == "A4" or 
				 settings.pagesize == "a4" then 

				 x = 11906
				 y = 16838
			end

			if settings.pagesize == "A5" or 
				 settings.pagesize == "a5" then 

				 x = 8391
				 y = 11906
			end

			if settings.pagesize == "letter" or 
				 settings.pagesize == "Letter" or 
				 settings.pagesize == "LETTER" then 

				 x = 12240
				 y = 15840
			end

			if settings.language then 
				w = y
				h = x
			else
				w = x
				h = y
			end

			local str = string_format('\\paperw%d\\paperh%d\\margl%d\\margr%d\\margt%d\\margb%d\n', w, h, settings.left*567, settings.right*567, settings.top*567, settings.bottom*567)
			
			writer(str)
			
			writer('{\\*\\listtable\n')
			writer('{\\list\\listtemplateid2\\listsimple')
			writer('{\\listlevel\\levelnfc0\\fi-300\\li500}')
			writer('\\listid2\\listname LN;}\n')
			writer('{\\list\\listtemplateid1\\listsimple')
			writer('{\\listlevel\\levelnfc23\\fi-300\\li500}')
			writer('\\listid1\\listname LB;}\n')
			writer('}\n')
			
			writer('{\\listoverridetable\n')
			writer('{\\listoverride\\listid1\\listoverridecount0\\ls1}\n')
			--writer('{\\listoverride\\listid2\\listoverridecount0\\ls2}\n')
			writer('}\n')
			
			writer('{\\stylesheet\n')
			writer(string_format('{\\s0\\fs%d Normal;}\n', settings.fontsize * 2))
			for _, s in pairs(style_tab) do
				writer('{\\s', s[1], ' ', s[2], ';}\n')
			end
			writer('}\n')
			writer('\n')
		end,
		
		rawtext = function(s)
			writer(s)
		end,
		
		text = function(s)
			writer(unrtf(s))
		end,
		
		notext = function(s)
		end,
		
		bold_on = function()
			writer('\\b ')
		end,
		
		bold_off = function()
			writer('\\b0 ')
		end,
		
		italic_on = function()
			writer('\\i ')
		end,
		
		italic_off = function()
			writer('\\i0 ')
		end,
		
		underline_on = function()
			writer('\\ul ')
		end,
		
		underline_off = function()
			writer('\\ul0 ')
		end,
		
		list_start = function()
		end,
		
		list_end = function()
			writer('{\\listoverride\\listid1\\listoverridecount0\\ls1}\n')
		end,
		
		paragraph_start = function(para)
			local settings = DocumentSet.addons.pageconfig
			local n = 2
			if para.style == "H1" then
				n = 3
			elseif para.style == "H2" then
				n = 2.5
			elseif para.style == "H3" then
				n = 2.2
			end

			writer('\\pard\\s', style_tab[para.style][1], string_format("\\fs%d", settings.fontsize*n))
			
			if (para.style == "LN") then
				writer('\\ls1')
			end
			
			if (para.style == "LB") then
				writer('\\ls2 \\bullet ')
			end
			
			if (para.style == "L") then
				writer('\\ls2 ')
			end
			
			--first line indent
			if (para.style == "P") then
				writer('\\fi600')
			end
			writer(' ')
			bold = false
			italic = false
			underline = false
			--changepara(style)
		end,		
		
		paragraph_end = function(para)
			writer('\\par\n')
		end,
		
		table_start = function(para)
			local settings = DocumentSet.addons.pageconfig
			writer(string_format('\\pard\\trowd\\fs%d\n', settings.fontsize*2))
			local width = 0
			for cn, cell in ipairs(para.cells) do
				if (para.style == "TRB") then
					writer('\\clbrdrt\\brdrs\\clbrdrl\\brdrs\\clbrdrb\\brdrs\\clbrdrr\\brdrs')
				end
				width = width + para.cellWidth[cn]
				writer(string_format('\\cellx%d\n', width*117))
			end
			writer('\n')
		end,
		
		table_end = function(para)
			writer('\\lastrow')
		end,

		tablerow_start = function(para)
		end,
		
		tablerow_end = function(para)
			writer('\\row\n')
		end,

		tablecell_start = function(para)
			writer('\\intbl ')
		end,
		
		tablecell_end = function(para)
			writer('\\cell\n')
		end,

		image_start = function(para)
			local settings = DocumentSet.addons.pageconfig
			writer('\\pard\\s', style_tab[para.style][1], string_format("\\fs%d", settings.fontsize * 2))
		end,
		
		image_end = function(para)
			
			-- set page size
			local settings = DocumentSet.addons.pageconfig
			local h = 0
			local w = 0
			local x1 = 0
			local y1 = 0
			
			if settings.pagesize == "A4" or 
				 settings.pagesize == "a4" then 

				 x1 = 11906
				 y1 = 16838
			end

			if settings.pagesize == "A5" or 
				 settings.pagesize == "a5" then 

				 x1 = 8391
				 y1 = 11906
			end

			if settings.pagesize == "letter" or 
				 settings.pagesize == "Letter" or 
				 settings.pagesize == "LETTER" then 

				 x1 = 12240
				 y1 = 15840
			end

			if settings.landscape then 
				w = y1
				h = x1
			else
				w = x1
				h = y1
			end

			local pagewidth = w

			pagewidth = pagewidth - settings.left*567 - settings.right*567
			

			local X = 0
			local Y = 0
			
			local imagesize = function(x, y)
				X = x
				Y = y
			end
			
			local rtfimage = function(rtf) 
				writer(rtf)
				writer('}\n')
			end

			getimagesize(para.imagename, imagesize)
			writer(string_format('{\\pict\\picwgoal%d\\pichgoal%d\\jpegblip\n', pagewidth, Y/X*pagewidth))
			ImageToRTF(para.imagename, rtfimage)
			
			writer('\\par\n')
		end,

		epilogue = function()
			writer('}')
		end
	})
end

function Cmd.ExportRTFFile(filename)
	return ExportFileWithUI(filename, "Export RTF File", ".rtf",
		callback)
end

function Cmd.ExportToRTFString()
	return ExportToString(Document, callback)
end
