--[[--
File              : pdf.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 01.01.2024
Last Modified Date: 19.08.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--

local NextCharInWord = wg.nextcharinword

local PdfNew = wg.pdf_new
local PdfAddPage = wg.pdf_add_page
local PdfClose = wg.pdf_close
local PdfWriteText = wg.pdf_write_text
local PdfLoadFont = wg.pdf_load_font
local PdfSetFont = wg.pdf_set_font
local PdfStartParagraph = wg.pdf_start_paragraph
local PdfEndParagraph = wg.pdf_end_paragraph
local PdfStartLine = wg.pdf_start_line
local PdfEndLine = wg.pdf_end_line
local PdfJustyfyRight = wg.pdf_justify_right
local PdfJustyfyCenter = wg.pdf_justify_center
local PdfJustyfyBoth = wg.pdf_justify_both
local PdfMakeIndent = wg.pdf_make_indent
local PdfSetUnderline = wg.pdf_set_underline

local LinixGetFontsPath = wg.linux_get_fonts_path
local MacOsGetFontsPath = wg.macos_get_fonts_path

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

local function find_fonts_dir()
end

local FONTSDIR = "" 
if (ARCH == "windows") then
	FONTSDIR = WINDOWS_INSTALL_DIR
else
	local function path(buf)
		FONTSDIR = buf
	end
	LinixGetFontsPath(path)
end

local FONTSANS           = FONTSDIR .. "/FreeSans.ttf"
local FONTSANSBOLD       = FONTSDIR .. "/FreeSansBold.ttf"
local FONTSANSITALIC     = FONTSDIR .. "/FreeSansOblique.ttf"
local FONTSANSBOLDITALIC = FONTSDIR .. "/FreeSansBoldOblique.ttf"
local FONTMONO           = FONTSDIR .. "/FreeMono.ttf"
local FONTMONOBOLD       = FONTSDIR .. "/FreeMonoBold.ttf"
local FONTMONOITALIC     = FONTSDIR .. "/FreeMonoOblique.ttf"
local FONTMONOBOLDITALIC = FONTSDIR .. "/FreeMonoBoldOblique.ttf"

local function callback(document)
	local config = DocumentSet.addons.pageconfig
	local npage = 1 -- page number
		
	local nlist = 1 -- list number
	local inlist = false

	local bold = false
	local italic = false
	local underline = false
	local font = "sans"

	local FONT = wg.FONTSANS

	local function SetFont()
		if font == "mono" then
			if not bold and not italic then
				FONT = wg.FONTMONO
			elseif bold and not italic then
				FONT = wg.FONTMONOBOLD
			elseif not bold and italic then
				FONT = wg.FONTMONOITALIC
			elseif bold and italic then
				FONT = wg.FONTMONOBOLDITALIC
			end
		elseif font == "sans" then
			if not bold and not italic then
				FONT = wg.FONTSANS
			elseif bold and not italic then
				FONT = wg.FONTSANSBOLD
			elseif not bold and italic then
				FONT = wg.FONTSANSITALIC
			elseif bold and italic then
				FONT = wg.FONTSANSBOLDITALIC
			end
		end
		
		PdfSetFont(FONT, config.fontsize or 12)
	end

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
				PdfLoadFont(FONTSANS, wg.FONTSANS)
				PdfLoadFont(FONTSANSBOLD, wg.FONTSANSBOLD)
				PdfLoadFont(FONTSANSITALIC, wg.FONTSANSITALIC)
				PdfLoadFont(FONTSANSBOLDITALIC, wg.FONTSANSBOLDITALIC)
				PdfLoadFont(FONTMONO, wg.FONTMONO)
				PdfLoadFont(FONTMONOBOLD, wg.FONTMONOBOLD)
				PdfLoadFont(FONTMONOITALIC, wg.FONTMONOITALIC)
				PdfLoadFont(FONTMONOBOLDITALIC, wg.FONTMONOBOLDITALIC)
				SetFont()
		end,
		
		rawtext = function(s)
			PdfWriteText(s)
		end,
		
		text = function(s)
			PdfWriteText(s)
		end,
		
		notext = function(s)
			PdfStartLine()
			PdfEndLine()
		end,
		
		bold_on = function()
			bold = true
			SetFont()
		end,
		
		bold_off = function()
			bold = false
			SetFont()
		end,
		
		italic_on = function()
			italic = true
			SetFont()
		end,
		
		italic_off = function()
			italic = false
			SetFont()
		end,
		
		underline_on = function()
			PdfSetUnderline(true)
		end,
		
		underline_off = function()
			PdfSetUnderline(false)
		end,
		
		list_start = function()
			nlist = 1
			inlist = true
		end,
		
		list_end = function()
			inlist = false
		end,
		
		paragraph_start = function(para)
			bold = false
			italic = false
			underline = false
			font = "sans"
			if para.type == "RAW" then
				font = "mono"
			end
			SetFont()
			PdfStartParagraph()	
		end,		
		
		paragraph_end = function(para)
			PdfEndParagraph()	
		end,
		
		line_start = function(ln, para, cp)
			-- check page size
			local lpp = LinesPerPage()
			if (cp > lpp * npage) then
				npage = npage + 1
				PdfAddPage(
				npage, 
				config.pagesize, 
				config.Landscape, 
				config.left, 
				config.right, 
				config.top, 
				config.bottom)
				PdfSetFont(FONT, config.fontsize or 12)
			end

			-- handle line
			PdfStartLine()

			if DocumentStyles[para.style].indent
			then
				PdfMakeIndent(DocumentStyles[para.style].indent)
			end

			if ln == 1 then
				if inlist then
					local str = string_format("%d. ", nlist)
					if para.style == "LB" then
						str = DocumentStyles[para.style].bullet .. " "
					end
					PdfWriteText(str)
					nlist = nlist + 1
				end
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
					nlines = nlines + 1
				end

				if para.style == "RIGHT" then
					PdfJustyfyRight(table_concat(text))
				elseif para.style == "CENTER" then
					PdfJustyfyCenter(table_concat(text))
				elseif para.style == "BOTH" then
					if nlines > 1 and ln < nlines - 1 then
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
