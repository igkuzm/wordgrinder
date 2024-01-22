--[[--
File              : opendocument.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 03.01.2024
Last Modified Date: 22.01.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--
-- © 2008-2013 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

local ITALIC = wg.ITALIC
local UNDERLINE = wg.UNDERLINE
local BOLD = wg.BOLD
local ParseWord = wg.parseword
local WriteU8 = wg.writeu8
local ReadFromZip = wg.readfromzip
local UnzipFile = wg.unzipfile
local bitand = bit32.band
local bitor = bit32.bor
local bitxor = bit32.bxor
local bit = bit32.btest
local string_char = string.char
local string_find = string.find
local string_sub = string.sub
local string_gmatch = string.gmatch
local table_concat = table.concat

local OFFICE_NS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0"
local STYLE_NS  = "urn:oasis:names:tc:opendocument:xmlns:style:1.0"
local FO_NS     = "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
local TEXT_NS   = "urn:oasis:names:tc:opendocument:xmlns:text:1.0"
local TABLE_NS  = "urn:oasis:names:tc:opendocument:xmlns:table:1.0"
local DRAW_NS   = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"
local XLINK_NS  = "http://www.w3.org/1999/xlink"

local zipfile
-----------------------------------------------------------------------------
-- The importer itself.

local function parse_style(styles, xml)
	local NAME = STYLE_NS .. " name"
	local FAMILY = STYLE_NS .. " family"
	local PARENT_NAME = STYLE_NS .. " parent-style-name"
	local TEXT_PROPERTIES = STYLE_NS .. " text-properties"
	local PARAGRAPH_PROPERTIES = STYLE_NS .. " paragraph-properties"
	local TABLE_PROPERTIES = STYLE_NS .. " table-properties"
	local TABLE_COLUMN_PROPERTIES = STYLE_NS .. " table-column-properties"
	local TABLE_CELL_PROPERTIES = STYLE_NS .. " table-cell-properties"
	local FONT_STYLE = FO_NS .. " font-style"
	local FONT_WEIGHT = FO_NS .. " font-weight"
	local UNDERLINE_STYLE = STYLE_NS .. " text-underline-style"
	local MARGIN_LEFT = FO_NS .. " margin-left"
	local TEXT_ALIGN = FO_NS .. " text-align"
	local LIST_LEVEL_STYLE_NUMBER = TEXT_NS .. " list-level-style-number"
	
	local name = xml[NAME]
	local style =
	{
		parent = xml[PARENT_NAME]
	}

	for _, element in ipairs(xml) do
		if (element._name == TEXT_PROPERTIES) then
			if (element[FONT_STYLE] == "italic") then
				style.italic = true
			end
			if (element[FONT_WEIGHT] == "bold") then
				style.bold = true
			end
			if (element[UNDERLINE_STYLE] == "solid") then
				style.underline = true
			end
		elseif (element._name == PARAGRAPH_PROPERTIES) then
			if 
				element[MARGIN_LEFT] and
				element[MARGIN_LEFT] ~= "0cm"
			then
				style.indented = true
			end
			if (element[TEXT_ALIGN]) then
				if 
					element[TEXT_ALIGN] == "right" or
					element[TEXT_ALIGN] == "end"
				then
					style.right = true
				elseif (element[TEXT_ALIGN] == "center") then
					style.center = true
				elseif 
					element[TEXT_ALIGN] == "left" or
					element[TEXT_ALIGN] == "start"
				then
					style.left = true
				elseif 
					element[TEXT_ALIGN] == "justify" or
					element[TEXT_ALIGN] == "both"
				then
					style.justify = true
				end
			end
		elseif (element._name == TABLE_COLUMN_PROPERTIES) then
				local w = element[STYLE_NS .. " column-width"]
				if w then
					style.width = w
				end
		elseif (element._name == TABLE_CELL_PROPERTIES) then
				if 
					element[FO_NS .. " border"] and
					element[FO_NS .. " border"] ~= "none"
				then
					style.trb = true
				end

				if 
					element[FO_NS .. " border-left"] and
					element[FO_NS .. " border-left"] ~= "none"
				then
					style.trb = true
				end
				if 
					element[FO_NS .. " border-right"] and
					element[FO_NS .. " border-right"] ~= "none"
				then
					style.trb = true
				end
				if 
					element[FO_NS .. " border-top"] and
					element[FO_NS .. " border-top"] ~= "none"
				then
					style.trb = true
				end
				if 
					element[FO_NS .. " border-bottom"] and
					element[FO_NS .. " border-bottom"] ~= "none"
				then
					style.trb = true
				end
		elseif (element._name == LIST_LEVEL_STYLE_NUMBER) then
			if element[STYLE_NS .. " num-format"] == "1" then
				style.numbers = true
			end
		end
	end

	styles[name] = style
end

local function resolve_parent_styles(styles)
	local function recursively_fetch(name, attr)
		local style = styles[name]
		if style[attr] then
			return true
		end
		if style.parent then
			return recursively_fetch(style.parent, attr)
		end
		return nil
	end

	for k, v in pairs(styles) do
		v.italic = recursively_fetch(k, "italic")
		v.bold = recursively_fetch(k, "bold")
		v.underline = recursively_fetch(k, "underline")
		v.indented = recursively_fetch(k, "indented")
	end
end

local function collect_styles(styles, xml)
	local STYLES = OFFICE_NS .. " styles"
	local AUTOMATIC_STYLES = OFFICE_NS .. " automatic-styles"
	local STYLE = STYLE_NS .. " style"

	for _, element in ipairs(xml) do
		if (element._name == STYLES) or (element._name == AUTOMATIC_STYLES) then
			for _, element in ipairs(element) do
				if (element._name == STYLE) then
					parse_style(styles, element)
				end
				if (element._name == TEXT_NS .. " list-style") then
					parse_style(styles, element)
				end
			end
		end
	end
end

local textwidth = 0;

local function add_text(styles, importer, xml)
	local SPACE = TEXT_NS .. " s"
	local SPACECOUNT = TEXT_NS .. " c"
	local SPAN = TEXT_NS .. " span"
	local STYLENAME = TEXT_NS .. " style-name"
	
	for _, element in ipairs(xml) do
		if (type(element) == "string") then
			local needsflush = false
			if string_find(element, "^ ") then
				needsflush = true
			end
			for word in string_gmatch(element, "%S+") do
				if needsflush then
					importer:flushword(false)
				end
				textwidth = textwidth + string.len(word) + 1
				importer:text(word)
				needsflush = true
			end
			if string_find(element, " $") then
				importer:flushword(false)
			end
		elseif (element._name == SPACE) then
			local count = tonumber(element[SPACECOUNT] or 0) + 1
			for i = 1, count do
				importer:flushword(false)
			end
		elseif (element._name == SPAN) then
			local stylename = element[STYLENAME] or ""
			local style = styles[stylename] or {}
			
			if style.italic then
				importer:style_on(ITALIC)
			end
			if style.bold then
				importer:style_on(BOLD)
			end
			if style.underline then
				importer:style_on(UNDERLINE)
			end
			add_text(styles, importer, element)
			if style.underline then
				importer:style_off(UNDERLINE)
			end
			if style.bold then
				importer:style_off(BOLD)
			end
			if style.italic then
				importer:style_off(ITALIC)
			end
		else
			add_text(styles, importer, element)
		end
	end
end

local function import_paragraphs(styles, importer, xml, defaultstyle)
	local PARAGRAPH = TEXT_NS .. " p"
	local HEADER = TEXT_NS .. " h"
	local LIST = TEXT_NS .. " list"
	local OUTLINELEVEL = TEXT_NS .. " outline-level"
	local STYLENAME = TEXT_NS .. " style-name"
	local STARTVALUE = TEXT_NS .. " start-value"
	
	for _, element in ipairs(xml) do
		if (element._name == TABLE_NS .. " table") then
			local wgstyle = "TR"
			local width = {}
			for en, element in ipairs(element) do
				if (element._name == TABLE_NS .. " table-column") then
					local stylename = element[TABLE_NS .. " style-name"] or ""
					local style = styles[stylename] or {}
					width[en] = style.width
				end
				if (element._name == TABLE_NS .. " table-row") then
					local firstcell = true
					for en, element in ipairs(element) do
						if (element._name == TABLE_NS .. " table-cell") then
							textwidth = 0
							if not firstcell then
								importer:text(" ; ")
							end
							firstcell = false
							local stylename = element[TABLE_NS .. " style-name"] or ""
							local style = styles[stylename] or {}
							if style.trb then
								wgstyle = "TRB"
							end
							for _, element in ipairs(element) do
								if (element._name == PARAGRAPH) then
									add_text(styles, importer, element)
									local w = width[en];
									if w then
										local width = tonumber(string.match(w, "%d+"))
										if string.match(w, "cm") then
											width = width * 5
										end
										if string.match(w, "in") then
											width = width * 5 * 2.5
										end
										if string.match(w, "dxa") then
											width = width/20/72 * 12
										end
										print(width)
										local i 
										for i=textwidth,width, 1 do
											importer:text(" ")
										end
									end
								end
							end
						end
					end
					importer:flushparagraph(wgstyle)
				end
			end
		end
		if (element._name == PARAGRAPH) then
			local stylename = element[STYLENAME] or ""
			local style  = styles[stylename] or {}
			local parent = styles[style.parent] or {}
			local wgstyle = defaultstyle
			
			if style.indented or parent.indented then
				wgstyle = "Q"
			end
			
			if style.right or parent.right then
				wgstyle = "RIGHT"
			end
			
			if style.left or parent.left then
				wgstyle = "LEFT"
			end
			
			if style.center or parent.center then
				wgstyle = "CENTER"
			end
			
			if style.justify or parent.justify then
				wgstyle = "P"
			end

			-- find drawing
			for _, element in ipairs(element) do
				if (element._name == DRAW_NS .. " frame") then
					for _, element in ipairs(element) do
						if (element._name == DRAW_NS .. " image") then
							local image = element[XLINK_NS .. " href"]
							if image then
								local tmpname = os.tmpname()
								UnzipFile(zipfile, image, tmpname)
								-- add image to WG
								importer:text(tmpname)
								importer:flushword(false)
								importer:flushparagraph("IMG")
							end
						end
					end
				end
			end
			
			add_text(styles, importer, element)
			importer:flushparagraph(wgstyle)
		elseif (element._name == HEADER) then
			local level = tonumber(element[OUTLINELEVEL] or 1)
			if (level > 4) then
				level = 4
			end
			
			add_text(styles, importer, element)
			importer:flushparagraph("H"..level)
		elseif (element._name == LIST) then
			local stylename = element[STYLENAME] or ""
			local style = styles[stylename] or {}
			for _, element in ipairs(element) do
				local hasnumber = element[STARTVALUE] ~= nil
				if (hasnumber or style.numbers) then
					wgstyle = "LN"
				else
					wgstyle = "LB"
				end
				import_paragraphs(styles, importer, element, wgstyle)
			end
		end
	end
end

function Cmd.ImportODTFile(filename)
	if not filename then
		filename = FileBrowser("Import ODT File", "Import from:", false)
		if not filename then
			return false
		end
	end
	
	ImmediateMessage("Importing...")	

	-- Load the styles and content subdocuments.
	
	zipfile = filename

	local stylesxml = ReadFromZip(filename, "styles.xml")
	local contentxml = ReadFromZip(filename, "content.xml")
	if not stylesxml or not contentxml then
		ModalMessage(nil, "The import failed, probably because the file could not be found.")
		QueueRedraw()
		return false
	end
		
	stylesxml = ParseXML(stylesxml)
	contentxml = ParseXML(contentxml)

	-- Find out what text styles the document creates (so we can identify
	-- italic and underlined text).
	
	local styles = {}
	collect_styles(styles, stylesxml)
	collect_styles(styles, contentxml)
	resolve_parent_styles(styles)

	-- Actually import the content.
	
	local document = CreateDocument()
	local importer = CreateImporter(document)
	importer:reset()

	local BODY = OFFICE_NS .. " body"
	local TEXT = OFFICE_NS .. " text"
	for _, element in ipairs(contentxml) do
		if (element._name == BODY) then
			for _, element in ipairs(element) do
				if (element._name == TEXT) then
					import_paragraphs(styles, importer, element, "P")
				end
			end
		end 
	end

	-- All the importers produce a blank line at the beginning of the
	-- document (the default content made by CreateDocument()). Remove it.
	
	if (#document > 1) then
		document:deleteParagraphAt(1)
	end
	
	-- Add the document to the document set.
	
	local docname = Leafname(filename)

	if DocumentSet.documents[docname] then
		local id = 1
		while true do
			local f = docname.."-"..id
			if not DocumentSet.documents[f] then
				docname = f
				break
			end
		end
	end
	
	DocumentSet:addDocument(document, docname)
	DocumentSet:setCurrent(docname)

	QueueRedraw()
	return true
end

