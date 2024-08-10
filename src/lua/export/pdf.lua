--[[--
File              : pdf.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 01.01.2024
Last Modified Date: 10.08.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--

local NextCharInWord = wg.nextcharinword

local PdfNew = wg.pdf_new
local PdfAddPage = wg.pdf_add_page
local PdfClose = wg.pdf_close
local PdfWriteText = wg.pdf_write_text
local PdfLoadFont = wg.pdf_load_font
local PdfStartParagraph = wg.pdf_start_paragraph
local PdfEndParagraph = wg.pdf_end_paragraph
local PdfStartLine = wg.pdf_start_line
local PdfEndLine = wg.pdf_end_line
local PdfJustyfyRight = wg.pdf_justify_right
local PdfJustyfyCenter = wg.pdf_justify_center
local PdfJustyfyBoth = wg.pdf_justify_both
local PdfMakeIndent = wg.pdf_make_indent

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

local function callback(document)
	local config = DocumentSet.addons.pageconfig
	local npage = 1 -- page number

	return ExportFileUsingCallbacks(document,
	{
		prologue = function()
			-- start PDF and create new page
			PdfNew()
			PdfAddPage(
				npage, 
				config.pagesize, 
				config.Landscape, 
				config.left, 
				config.right, 
				config.top, 
				config.bottom)	
			PdfLoadFont(CONFIGDIR .. "/1.ttf", config.fontsize)
		end,
		
		rawtext = function(s)
		end,
		
		text = function(s)
			PdfWriteText(s)
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
			PdfStartParagraph()	
		end,		
		
		paragraph_end = function(para)
			PdfEndParagraph()	
		end,
		
		line_start = function(ln, para)
			PdfStartLine()

			if DocumentStyles[para.style].indent
			then
				PdfMakeIndent(DocumentStyles[para.style].indent)
			end

			if ln == 1 then
				local firstindent = 
					DocumentStyles[para.style].firstindent or 0
				if firstindent then
					PdfMakeIndent(firstindent)
				end
			end
			
			if 
				para.style == "RIGHT" or
				para.style == "CENTER" or
				para.style == "BOTH"
			then
				-- get text from line to handle it's width
				local nlines = 0
				local text = {}
				for n, wn in ipairs(para.lines[ln]) do
					if n ~= 1 then 
						text[#text + 1] = " "
					end
					local word = para[wn]
					text[#text + 1] = word
					nlines = n
				end

				if para.style == "RIGHT" then
					PdfJustyfyRight(table_concat(text))
				elseif para.style == "CENTER" then
					PdfJustyfyCenter(table_concat(text))
				elseif para.style == "BOTH" then
					if ln < nlines - 1 then
						PdfJustyfyBoth(table_concat(text))
					end
				end
			end

		end,
		
		line_end = function(ln, para)
			PdfEndLine()
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
	
	callback(Document)

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
