--[[--
File              : pdf.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 01.01.2024
Last Modified Date: 09.08.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--

local NextCharInWord = wg.nextcharinword

local PdfNew = wg.pdf_new
local PdfAddPage = wg.pdf_add_page
local PdfClose = wg.pdf_close
local PdfWriteText = wg.pdf_write_text
local PdfLoadFont = wg.pdf_load_font

local getimagesize = wg.getimagesize
local string_len = string.len
local string_char = string.char
local string_format = string.format
local string_gsub = string.gsub
local table_concat = table.concat

-----------------------------------------------------------------------------
-- The exporter itself.
function pdf_error_handler(msg)
	NonmodalMessage(msg)
end

HOME = os.getenv("HOME") or os.getenv("USERPROFILE")
CONFIGDIR = HOME .. "/.wordgrinder"

local function PdfParse(document)

	local npage = 1 -- page number

	-- start PDF and create new page
	PdfNew()
	PdfAddPage(npage, "A4", false, 2, 2, 2, 2)	
	PdfLoadFont(CONFIGDIR .. "/1.ttf", 12)
		
	-- parse paragraph
	for pn, p in ipairs(document) do
		if 
			p.style == "BOTH"
		then
			p.wrapBoth(p, document.wrapwidth)
			-- parse earch line
			for ln, line in ipairs(p.lines) do
				-- concat words
				local text = {}
				for _, wn in ipairs(line) do
					local word = p[wn]
					text[#text + 1] = word
					text[#text + 1] = " "
				end
				PdfWriteText(table_concat(text))
			end
		end
		
	end
	
end

local function export_pdf_with_ui(filename, title, extension)
	if not filename then
		filename = Document.name
		if filename then
			if not filename:find("%..-$") then
				filename = filename .. extension
			else
				filename = filename:gsub("%..-$", extension)
			end
		else
			filename = "(unnamed)"
		end
			
		filename = FileBrowser(title, "Export as:", true,
			filename)
		if not filename then
			return false
		end
		if filename:find("/[^.]*$") then
			filename = filename .. extension
		end
	end
	
	ImmediateMessage("Exporting...")
	
	PdfParse(Document)

	PdfClose(filename)
	
	QueueRedraw()
	return true
end
	
function Cmd.ExportPDFFile(filename)
	return export_pdf_with_ui(filename, "Export PDF File", ".pdf",
		callback)
end

function Cmd.ExportToRTFString()
	return ExportToString(Document, callback)
end
