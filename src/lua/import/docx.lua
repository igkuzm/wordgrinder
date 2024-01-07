--[[--
File              : docx.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 03.01.2024
Last Modified Date: 06.01.2024
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
local bitand = bit32.band
local bitor = bit32.bor
local bitxor = bit32.bxor
local bit = bit32.btest
local string_char = string.char
local string_find = string.find
local string_sub = string.sub
local string_gmatch = string.gmatch
local table_concat = table.concat

local W = "http://schemas.openxmlformats.org/wordprocessingml/2006/main"
local VAL = W .. " val"

-----------------------------------------------------------------------------
-- The importer itself.

-- number/bullet lists are in numbering.xml
-- first get types for abstractNumId
local function parse_abstract(lists, xml)
	local abstractNumId = xml[W .. " abstractNumId"] or "";
	local list = {}
	list.style = ""
	for _, element in ipairs(xml) do
		if (element._name == W .. " lvl") then
			if (element[W .. " ilvl"] == "0") then
				local numFmt  = ""
				local lvlText = ""
				for _, element in ipairs(element) do
					if (element._name == W .. " numFmt") then
						numFmt = element[VAL]
					end
					if (element._name == W .. " lvlText") then
						lvlText = element[VAL]
					end
				end
				if (numFmt == "decimal") then
					list.style = "LN"
				elseif (lvlText ~= "") then
					list.style = "LB"
				else
					list.style = "L"
				end
				lists[abstractNumId] = list
			end
		end
	end
end

-- second get abstractNumId for numId 
-- (to matche them in document.xml)
local function parse_num(lists, xml)
	local numId = xml[W .. " numId"] or "";
	for _, element in ipairs(xml) do
		if (element._name == W .. " abstractNumId") then
			local abstractNumId = element[VAL] or ""
			local list = lists[abstractNumId]	
			list.numId = numId
		end
	end
end
	
-- get styles from styles.xml
local function parse_style(styles, xml)

	local name = "";
	local style = {}

	for _, element in ipairs(xml) do
		-- style name
		if (element._name == W .. " name") then
			name = element[VAL] or ""
			-- check header
			if (type(name) == "string") then
				if (name == "H1" or name == "Heading1") then
					style.header = true
					style.iheader = "H1"
				elseif (name == "H2" or name == "Heading2") then
					style.header = true
					style.iheader = "H2"
				elseif (name == "H3" or name == "Heading3") then
					style.header = true
					style.iheader = "H3"
				elseif (name == "H4" or name == "Heading4") then
					style.header = true
					style.iheader = "H4"
				end
			end
		end
		
		-- style run properties
		if (element._name == W .. " rPr") then
			for _, element in ipairs(element) do
				if (element._name == W .. " b") then
					style.bold = true
				end
				if (element._name == W .. " i") then
					style.italic = true
				end
				if (element._name == W .. " u") then
					style.underline = true
				end
			end
		end

		-- style paragraph properties
		if (element._name == W .. " pPr") then
			for _, element in ipairs(element) do
				if (element._name == W .. " ind") then
					for _, element in ipairs(element) do
						if (element._name == W .. " left") then
							style.indented = true
						end
					end
				end
				if (element._name == W .. " jc") then
					if (element[VAL] == "left") or  (element[VAL] == "start") then
						style.left = true
					end
					if (element[VAL] == "right") or  (element[VAL] == "end") then
						style.right = true
					end
					if (element[VAL] == "center") then
						style.center = true
					end
				end
			end
		end
	end
	styles[name] = style
end

local function collect_styles(styles, xml)
	for _, element in ipairs(xml) do
		if (element._name == W .. " style") then
			parse_style(styles, element)
		end
	end
end

local function collect_lists(lists, xml)
	for _, element in ipairs(xml) do
		if (element._name == W .. " abstractNum") then
			parse_abstract(lists, element)
		end
		if (element._name == W .. " num") then
			parse_num(lists, element)
		end
	end
end

local function add_text(styles, importer, xml)
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
				importer:text(word)
				needsflush = true
			end
			if string_find(element, " $") then
				importer:flushword(false)
			end
		end
	end
end

local function import_run(styles, lists, importer, element, defaultstyle)
	-- set default values
	importer:style_off(ITALIC)
	importer:style_off(BOLD)
	importer:style_off(UNDERLINE)

	for _, element in ipairs(element) do
		--get run properties
		if (element._name == W .. " rPr") then
			for _, element in ipairs(element) do
				if (element._name == W .. " rStyle") then
					local stylename = element[VAL] or ""
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
				end
				if (element._name == W .. " b") then
					importer:style_on(BOLD)
				end
				if (element._name == W .. " i") then
					importer:style_on(ITALIC)
				end
				if (element._name == W .. " ul") then
					importer:style_on(UNDERLINE)
				end
			end
		end
		--get run text
		if (element._name == W .. " t") then
			add_text(styles, importer, element)
		end
	end
end

local function import_paragraphs(styles, lists, importer, element, defaultstyle)
	local wgstyle = defaultstyle
	-- get table 
	if (element._name == W .. " tbl") then
		for _, element in ipairs(element) do
			-- table row
			if (element._name == W .. " tr") then
				wgstyle = "TR"
				for _, element in ipairs(element) do
					-- table cell
					if (element._name == W .. " tc") then
						for _, element in ipairs(element) do
							-- paragraph
							if (element._name == W .. " p") then
								for _, element in ipairs(element) do
									-- run
									if (element._name == W .. " r") then
										import_run(styles, lists, importer, element, wgstyle)
									end
								end
							end
						end
						importer:text("; ")
					end
				end
				importer:flushparagraph(wgstyle)
			end
		end
	else
		for _, element in ipairs(element) do
			-- get paragraph properties
			if (element._name == W .. " pPr") then
				for _, element in ipairs(element) do
					--get paragraph style
					if (element._name == W .. " pStyle") then
						local stylename = element[VAL] or ""
						local style = styles[stylename] or {}
						if style.indented then
							wgstyle = "Q"
						end
						if style.right then
							wgstyle = "RIGHT"
						end
						if style.left then
							wgstyle = "LEFT"
						end
						if style.center then
							wgstyle = "CENTER"
						end
						if style.header then
							wgstyle = style.iheader
						end
					end
					--get numbering
					if (element._name == W .. " numPr") then
						for _, element in ipairs(element) do
							if (element._name == W .. " numId") then
								local numId = element[VAL]
								for _, list in pairs(lists) do
									if (numId == list.numId) then
										wgstyle = list.style
									end
								end
							end
						end
					end
				end
			end
			-- get run
			if (element._name == W .. " r") then
				import_run(styles, lists, importer, element, wgstyle)
			end
		end
	end
	importer:flushparagraph(wgstyle)
end

function Cmd.ImportDOCXFile(filename)
	if not filename then
		filename = FileBrowser("Import DOCX File", "Import from:", false)
		if not filename then
			return false
		end
	end
	
	ImmediateMessage("Importing...")	

	-- Load the styles and content subdocuments.
	
	local stylesxml     = ReadFromZip(filename, "word/styles.xml")
	local numberingxml  = ReadFromZip(filename, "word/numbering.xml")
	local contentxml    = ReadFromZip(filename, "word/document.xml")
	if not stylesxml or not contentxml then
		ModalMessage(nil, "The import failed, probably because the file could not be found.")
		QueueRedraw()
		return false
	end
		
	stylesxml      = ParseXML(stylesxml)
	contentxml     = ParseXML(contentxml)
	if (numberingxml) then
		numberingxml = ParseXML(numberingxml) 
	end

	-- Find out what text styles the document creates (so we can identify
	-- italic and underlined text).
	
	local styles = {}
	collect_styles(styles, stylesxml)

	local lists = {}
	if (numberingxml) then
		collect_lists(lists, numberingxml)
	end

	-- Actually import the content.
	
	local document = CreateDocument()
	local importer = CreateImporter(document)
	importer:reset()

	for _, element in ipairs(contentxml) do
		if (element._name == W .. " body") then
			for _, element in ipairs(element) do
				-- get paragraph
				if (element._name == W .. " p") or (element._name == W .. " tbl")then
					import_paragraphs(styles, lists, importer, element, "P")
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
