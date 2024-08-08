--[[--
File              : pdf.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 01.01.2024
Last Modified Date: 08.08.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--

local NextCharInWord = wg.nextcharinword
local PdfNew = wg.pdf_new
local PdfAddPage = wg.pdf_add_page
local PdfClose = wg.pdf_close
local PdfWriteText = wg.pdf_write_text
local getimagesize = wg.getimagesize
local string_len = string.len
local string_char = string.char
local string_format = string.format
local string_gsub = string.gsub
local table_concat = table.concat

-----------------------------------------------------------------------------
-- The exporter itself.
local function error_handler(msg)
end

local function callback(writer, document)

	local npage = 1 -- page number
	
	return ExportFileUsingCallbacks(document,
	{
		prologue = function()
			PdfNew()
			PdfAddPage(npage)	
			PdfWriteText("hello world!")
		end,
		
		rawtext = function(s)

		end,
		
		text = function(s)
			--PdfWriteText(s)
		end,
		
		notext = function(s)
		end,
		
		bold_on = function()

		end,
		
		bold_off = function()

		end,
		
		italic_on = function()

		end,
		
		italic_off = function()

		end,
		
		underline_on = function()

		end,
		
		underline_off = function()
		end,
		
		list_start = function()
		end,
		
		list_end = function()
		end,
		
		paragraph_start = function(para)
		end,		
		
		paragraph_end = function(para)
		end,
		
		table_start = function(para)
		end,
		
		table_end = function(para)
		end,

		tablerow_start = function(para)
		end,
		
		tablerow_end = function(para)
		end,

		tablecell_start = function(para)
		end,
		
		tablecell_end = function(para)
		end,

		image_start = function(para)
		end,
		
		image_end = function(para)
		end,

		epilogue = function()
		end
	})
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
	
	local writer = function(s)
	end
	callback(writer, Document)

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
