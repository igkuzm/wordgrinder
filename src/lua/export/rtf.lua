--[[--
File              : rtf.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 01.01.2024
Last Modified Date: 12.01.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--
-- © 2011 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

local NextCharInWord = wg.nextcharinword
local ReadU8 = wg.readu8
local ImageToRTF = wg.imagetortf
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

local style_tab =
{
	["H1"]     = {1, '\\fs40\\sb400\\b\\sbasedon0 H1'},
	["H2"]     = {2, '\\fs36\\sb360\\b\\sbasedon0 H2'},
	["H3"]     = {3, '\\fs32\\sb320\\b\\sbasedon0 H3'},
	["H4"]     = {4, '\\fs28\\sb280\\b\\sbasedon0 H4'},
	["P"]      = {5, '\\fs28\\sb140\\sbasedon0\\qj P'},
	["L"]      = {6, '\\fs28\\sb140\\sbasedon5 L'},
	["LB"]     = {7, '\\fs28\\sb140\\sbasedon5 LB'},
	["LN"]     = {8, '\\fs28\\sb140\\sbasedon5 LN'},
	["Q"]      = {9, '\\fs28\\sb140\\li500\\sbasedon5 Q'},
	["V"]      = {10, '\\fs28\\sb140\\li500\\sbasedon5 V'},
	["RAW"]    = {11, '\\fs28\\sb140\\sbasedon5 RAW'},
	["PRE"]    = {12, '\\fs28\\sb140\\sbasedon5 PRE'},
	["LEFT"]   = {13, '\\fs28\\sb140\\sbasedon5\\ql LEFT'},
	["RIGHT"]  = {14, '\\fs28\\sb140\\sbasedon5\\qr RIGHT'},
	["CENTER"] = {15, '\\fs28\\sb140\\sbasedon5\\qc CENTER'},
	["IMG"]    = {16, '\\fs28\\sb140\\sbasedon5\\qc IMG'},
}

local function callback(writer, document)
	local settings = DocumentSet.addons.htmlexport
	
	return ExportFileUsingCallbacks(document,
	{
		prologue = function()
			writer('{\\rtf1\\ansi\\deff0')
			writer('{\\fonttbl{\\f0 Times New Roman}}')
			writer('\\deflang1033\\widowctrl')
			writer('\\uc0\\n')
			
			writer('{\\listtable\n')
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
			writer('{\\s0 Normal;}\n')
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
			writer('\\pard\\s', style_tab[para.style][1])
			
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
			writer('\\pard\\trowd\n')
			local width = 0
			for cn, cell in ipairs(para.cells) do
				if (para.style == "TRB") then
					writer('\\clbrdrt\\brdrs\\clbrdrl\\brdrs\\clbrdrb\\brdrs\\clbrdrr\\brdrs')
				end
				width = width + para.cellWidth[cn]
				writer(string_format('\\cellx%d\n', width*200))
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
			writer('\\pard\\s', style_tab[para.style][1])
		end,
		
		image_end = function(para)
			writer('\\par\n')
			local rtfimage = function(rtf) 
				writer(rtf)
			end
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
