--[[--
File              : rtf.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 12.01.2024
Last Modified Date: 21.01.2024
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
    if str == " " then
      -- import empty paragraph
      importer:text(" ")
    else
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

    local cell = 1;
    local stringlen = 0;

    local paragraph = function(pstyle)
	  importer:flushparagraph(pstyle)
    end

    local style = function(cstyle, on)
      if on then
		importer:style_on(cstyle)
      else
		importer:style_off(cstyle)
      end
    end
    
    local tablerow = function(bordered)
      local pstyle = "TR"
      if bordered then
          pstyle = "TRB"
      end
	  importer:flushparagraph(pstyle)
      cell = 1;
    end
    
    local tablecell = function(ncells, len)
      if cell < ncells then
        local s
        for s=stringlen,len/1440*12,1 do
          importer:text(" ")
        end
        importer:text(" ; ")
      end
      cell = cell + 1
    end
    
    local text = function(txt)
      stringlen = string.len(txt)
      add_text(importer, txt)
    end

    local image = function(pstyle)
	  importer:flushparagraph(pstyle)
      local tmpname = os.tmpname()
      tmpname = string.format('%s.jpg', tmpname)
      importer:text(tmpname)
      importer:flushword(false)
	  importer:flushparagraph("IMG")
      return tmpname
    end

    UnRTF(
      filename,
      paragraph,
      style,
      tablerow,
      tablecell,
      text,
      image
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

