-- © 2008 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

local ITALIC = wg.ITALIC
local UNDERLINE = wg.UNDERLINE
local BOLD = wg.BOLD
local ParseWord = wg.parseword
local bitand = bit32.band
local bitor = bit32.bor
local bitxor = bit32.bxor
local bit = bit32.btest
local string_lower = string.lower
local time = wg.time
local GetStringWidth = wg.getstringwidth

-- Renders the document by calling the appropriate functions on the cb
-- table.

function ExportFileUsingCallbacks(document, cb)
	document:renumber()
	cb.prologue()

	local listmode = false
	local rawmode = false
	local italic, underline, bold
	local olditalic, oldunderline, oldbold
	local firstword
	local wordbreak
	local emptyword

	local wordwriter = function (style, text)
		italic = bit(style, ITALIC)
		underline = bit(style, UNDERLINE)
		bold = bit(style, BOLD)

		local writer
		if rawmode then
			writer = cb.rawtext
		else
			writer = cb.text
		end

		-- Underline is stopping, so do so *before* the space
		if wordbreak and not underline and oldunderline then
			cb.underline_off()
		end

		if wordbreak then
			writer(' ')
			wordbreak = false
		end

		if not wordbreak and oldunderline then
			cb.underline_off()
		end
		if oldbold then
			cb.bold_off()
		end
		if olditalic then
			cb.italic_off()
		end
		if italic then
			cb.italic_on()
		end
		if bold then
			cb.bold_on()
		end
		if underline then
			cb.underline_on()
		end
		writer(text)

		emptyword = false
		olditalic = italic
		oldunderline = underline
		oldbold = bold
	end

	local cl = 1 -- current line

	for _, paragraph in ipairs(Document) do
		local name = paragraph.style
		local style = DocumentStyles[name]

		if listmode and not style.list then
			cb.list_end(listmode)
			listmode = false
		end
		if not listmode and style.list then
			cb.list_start(name)
			listmode = true
		end

		rawmode = (name == "RAW")

		if 
			paragraph.style == "TR" or 
			paragraph.style == "TRB"
		then
			
			-- gets table cells
			paragraph:wrapTableRow()
			
			-- get previous paragraph
			local pn = 0 
			for n, par in ipairs(Document) do
				if par == paragraph then
					pn = n - 1
				end
			end
			local pp
			if pn then
				pp = Document[pn]
			end
			if 
				 pp and
				 (pp.style == "TR" or 
				  pp.style == "TRB")
			then
				-- 
			else
				-- this is first row
				cb.table_start(paragraph)
			end
			cb.tablerow_start(paragraph)

		elseif paragraph.style == "IMG" 
		then
		else
			cb.paragraph_start(paragraph)
		end

		if 
			paragraph.style == "TR" or 
			paragraph.style == "TRB"
		then
			for ln, line in ipairs(paragraph.lines) do
				cl = cl + 1
			end
				
			for cn, cell in ipairs(paragraph.cells) do
				cb.tablecell_start(paragraph, cn)
				
				firstword = true
				wordbreak = false
				olditalic = false
				oldunderline = false
				oldbold = false

				for _, wn in ipairs(cell) do
					local word = paragraph[wn]
					if 
						word:find(';') and  
						GetStringWidth(word) < 2 
					then
						wordwriter(0, "")
						-- skip ';'
					else
						if firstword then
							firstword = false
						else
							wordbreak = true
						end

						emptyword = true
						italic = false
						underline = false
						bold = false
						ParseWord(word, 0, wordwriter) -- FIXME
						if emptyword then
							wordwriter(0, "")
						end
					end
				end
				cb.tablecell_end(paragraph, cn)
			end

				if underline then
					cb.underline_off()
				end
				if bold then
					cb.bold_off()
				end
				if italic then
					cb.italic_off()
				end
				
		elseif paragraph.style == "IMG" then
			local imagetitle = {}
			for wn, word in ipairs(paragraph) do
				if wn == 1 then
					paragraph.imagename = word
				else
					imagetitle[#imagetitle + 1] = wn			
				end
			end
			paragraph.imagetitle = imagetitle
			cb.image_start(paragraph)

			firstword = true
			wordbreak = false
			olditalic = false
			oldunderline = false
			oldbold = false

			for _, wn in ipairs(imagetitle) do
				local word = paragraph[wn]
				if firstword then
					firstword = false
				else
					wordbreak = true
				end

				emptyword = true
				italic = false
				underline = false
				bold = false
				ParseWord(word, 0, wordwriter) -- FIXME
				if emptyword then
					wordwriter(0, "")
				end
			end
			cb.image_end(paragraph)

			paragraph:wrapImage()
			for ln, line in ipairs(paragraph.lines) do
				cl = cl + 1
			end
		
		else
			if (#paragraph == 1) and (#paragraph[1] == 0) then
				cl = cl + 1
				cb.notext()
			else
				firstword = true
				wordbreak = false
				olditalic = false
				oldunderline = false
				oldbold = false

				-- wrap paragraph to lines
				if paragraph.style == "BOTH" then
					paragraph.wrapBoth(paragraph)
				elseif paragraph.style == "RIGHT" then
					paragraph.wrapRight(paragraph)
				else
					paragraph.wrap(paragraph)
				end

				for ln, line in ipairs(paragraph.lines) do
					if cb.line_start then
						cb.line_start(ln, paragraph, cl)
					end
					--for wn, word in ipairs(paragraph) do
					for _, wn in ipairs(line) do
						local word = paragraph[wn]
						
						if firstword then
							firstword = false
						else
							wordbreak = true
						end

						emptyword = true
						italic = false
						underline = false
						bold = false
						ParseWord(word, 0, wordwriter) -- FIXME
						if emptyword then
							wordwriter(0, "")
						end
					end
					cl = cl + 1
					if cb.line_end then
						cb.line_end(ln, paragraph)
					end
				end

				if underline then
					cb.underline_off()
				end
				if bold then
					cb.bold_off()
				end
				if italic then
					cb.italic_off()
				end
			end
		end

		if 
			paragraph.style == "TR" or 
			paragraph.style == "TRB"
		then
			-- get next paragraph
			local pn = 0 
			for n, par in ipairs(Document) do
				if par == paragraph then
					pn = n + 1
				end
			end
			local pp
			if pn then
				pp = Document[pn]
			end
			if 
				 pp and
				 (pp.style == "TR"  or
				 pp.style == "TRB") 
			then
				cb.tablerow_end(paragraph)
			else
				-- this is last row
				cb.tablerow_end(paragraph)
				cb.table_end(paragraph)
			end
		elseif paragraph.style == "IMG" 
		then
		else
			cb.paragraph_end(paragraph)
		end

	end
	if listmode then
		cb.list_end()
	end
	cb.epilogue()
end

-- Prompts the user to export a document, and then calls
-- exportcb(writer, document) to actually do the work.

function ExportFileWithUI(filename, title, extension, callback)
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
	local fp, e = io.open(filename, "w")
	if not fp then
		ModalMessage(nil, "Unable to open the output file "..e..".")
		QueueRedraw()
		return false
	end

	local fpw = fp.write
	local writer = function(...)
		fpw(fp, ...)
	end

	callback(writer, Document)
	fp:close()

	QueueRedraw()
	return true
end

--- Converts a document into a local string.

function ExportToString(document, callback)
	local ss = {}
	local writer = function(...)
		for _, s in ipairs({...}) do
			ss[#ss+1] = s
		end
	end

	callback(writer, document)

	return table.concat(ss)
end

