--[[--
File              : rtf.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 12.01.2024
Last Modified Date: 12.01.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--

local ITALIC = wg.ITALIC
local UNDERLINE = wg.UNDERLINE
local BOLD = wg.BOLD
local ParseWord = wg.parseword
local UnRTF = wg.unrtf
local WriteU8 = wg.writeu8
local bitand = bit32.band
local bitor = bit32.bor
local bitxor = bit32.bxor
local bit = bit32.btest
local string_char = string.char
local string_find = string.find
local string_sub = string.sub
local string_gmatch = string.gmatch
local table_concat = table.concat

-----------------------------------------------------------------------------
local function add_text(importer, str)
    local needsflush = false
    if string_find(str, "^ ") then
        needsflush = true
    end
    for word in string_gmatch(str, "%S+") do
        if needsflush then
            importer:flushword(false)
        end
        importer:text(word)
        needsflush = true
    end
    if string_find(str, " $") then
        importer:flushword(false)
    end
end


-- The importer itself.
function Cmd.ImportRTFFile(filename)
	if not filename then
		filename = FileBrowser("Import DOCX File", "Import from:", false)
		if not filename then
			return false
		end
	end
	
	ImmediateMessage("Importing...")	

    local document = CreateDocument()
	local importer = CreateImporter(document)
	importer:reset()

    local current_style = "P"

    local tablerows = 0

    local paragraph_start = function()
    end
        
    local paragraph_end = function()
	  importer:flushparagraph(current_style)
      current_style = "P"
    end

    local bold_start = function()
	  importer:style_on(BOLD)
    end
    
    local bold_end = function()
	  importer:style_off(BOLD)
    end
	
    local italic_start = function()
	  importer:style_on(ITALIC)
    end
    
    local italic_end = function()
	  importer:style_off(ITALIC)
    end
	
    local underline_start = function()
	  importer:style_on(UNDERLINE)
    end
    
    local underline_end = function()
	  importer:style_off(UNDERLINE)
    end
	
    local table_start = function()
      tablerows = 0
    end
    
    local table_end = function()
    end
    
    local tablerow_width = function(i, w)
      tablerows = tablerows + 1
    end

    local tablerow_start = function(n)
    end
    
    local tablerow_end = function(n)
	  importer:flushparagraph("TRB")
    end
    
    local tablecell_start = function(n)
      if n ~= 0 then
        importer:text(" ; ")
      end
    end
    
    local tablecell_end = function(n)
    end
    
    local style = function(stl)
      current_style = stl
    end
    
    local text = function(txt)
      add_text(importer, txt)
    end

    UnRTF(
      filename,
      paragraph_start,
      paragraph_end,
      bold_start,
      bold_end,
      italic_start,
      italic_end,
      underline_start,
      underline_end,
      table_start,
      table_end,
      tablerow_width,
      tablerow_start,
      tablerow_end,
      tablecell_start,
      tablecell_end,
      style,
      text
    )
    
	if (#document > 1) then
		document:deleteParagraphAt(1)
	end

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
-- vim: sw=4 ts=4 et

