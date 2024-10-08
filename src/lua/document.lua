--[[--
File              : document.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 01.01.2024
Last Modified Date: 20.08.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--
-- © 2008 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

local table_remove = table.remove
local table_insert = table.insert
local table_concat = table.concat
local Write = wg.write
local WriteStyled = wg.writestyled
local ParseImage = wg.parseimage
local ClearToEOL = wg.cleartoeol
local SetNormal = wg.setnormal
local SetBold = wg.setbold
local SetUnderline = wg.setunderline
local SetReverse = wg.setreverse
local SetDim = wg.setdim
local GetStringWidth = wg.getstringwidth
local GetBytesOfCharacter = wg.getbytesofcharacter
local GetWordText = wg.getwordtext
local BOLD = wg.BOLD
local ITALIC = wg.ITALIC
local UNDERLINE = wg.UNDERLINE
local REVERSE = wg.REVERSE
local BRIGHT = wg.BRIGHT
local DIM = wg.DIM

local stylemarkup =
{
	["H1"] = ITALIC + BRIGHT + BOLD + UNDERLINE,
	["H2"] = BRIGHT + BOLD + UNDERLINE,
	["H3"] = ITALIC + BRIGHT + BOLD,
	["H4"] = BRIGHT + BOLD
}

DocumentSetClass =
{
	-- remove any cached data prior to saving
	purge = function(self)
		for _, l in ipairs(self.documents) do
			l:purge()
		end
	end,

	touch = function(self)
		self.changed = true
		self.justchanged = true
		Document:touch()
	end,

	clean = function(self)
		self.changed = nil
		self.justchanged = nil
	end,

	getDocumentList = function(self)
		return self.documents
	end,

	_findDocument = function(self, name)
		for i, d in ipairs(self.documents) do
			if (d.name == name) then
				return i
			end
		end
		return nil
	end,

	findDocument = function(self, name)
		local document = self.documents[name]
		if not document then
			document = self.documents[self:_findDocument(name)]
			if document then
				ModalMessage("Document index inconsistency corrected",
					"Something freaky happened to '"..name.."'.")
				self.documents[name] = document
			end
		end
		return document
	end,

	addDocument = function(self, document, name, index)
		document.name = name

		local n = self:_findDocument(name) or (#self.documents + 1)
		self.documents[n] = document
		self.documents[name] = document
		if not self.current or (self.current.name == name) then
			self:setCurrent(name)
		end

		self:touch()
		RebuildDocumentsMenu(self.documents)
		return document
	end,

	moveDocumentIndexTo = function(self, name, targetIndex)
		local n = self:_findDocument(name)
		if not n then
			return
		end
		local document = self.documents[n]

		table_remove(self.documents, n)
		table_insert(self.documents, targetIndex, document)
		self:touch()
		RebuildDocumentsMenu(self.documents)
	end,

	deleteDocument = function(self, name)
		if (#self.documents == 1) then
			return false
		end

		local n = self:_findDocument(name)
		if not n then
			return
		end
		local document = self.documents[n]

		table_remove(self.documents, n)
		self.documents[name] = nil

		self:touch()
		RebuildDocumentsMenu(self.documents)

		if (Document == document) then
			document = self.documents[n]
			if not document then
				document = self.documents[#self.documents]
			end

			self:setCurrent(document.name)
		end

		return true
	end,

	setCurrent = function(self, name)
		-- Ensure any housekeeping on the current document gets done.

		if Document.changed then
			FireEvent(Event.Changed)
		end

		Document = self.documents[name]
		if not Document then
			Document = self.documents[1]
		end

		self.current = Document
		Document:renumber()
		ResizeScreen()
	end,

	renameDocument = function(self, oldname, newname)
		if self.documents[newname] then
			return false
		end

		local d = self.documents[oldname]
		self.documents[oldname] = nil
		self.documents[newname] = d
		d.name = newname

		self:touch()
		RebuildDocumentsMenu(self.documents)
		return true
	end,

	setClipboard = function(self, clipboard)
		self.clipboard = clipboard
	end,

	getClipboard = function(self)
		return self.clipboard
	end,
}

DocumentClass =
{
	appendParagraph = function(self, p)
		self[#self+1] = p
	end,

	insertParagraphBefore = function(self, paragraph, pn)
		table.insert(self, pn, paragraph)
	end,

	deleteParagraphAt = function(self, pn)
		table.remove(self, pn)
	end,

	wrap = function(self, width)
		self.wrapwidth = width
	end,

	getMarks = function(self)
		if not self.mp then
			return
		end

		local mp1 = self.mp
		local mw1 = self.mw
		local mo1 = self.mo
		local mp2 = self.cp
		local mw2 = self.cw
		local mo2 = self.co

		if (mp1 > mp2) or
		   ((mp1 == mp2) and
		       ((mw1 > mw2) or ((mw1 == mw2) and (mo1 > mo2)))
		   ) then
			return mp2, mw2, mo2, mp1, mw1, mo1
		end

		return mp1, mw1, mo1, mp2, mw2, mo2
	end,

	-- remove any cached data prior to saving
	purge = function(self)
		for _, paragraph in ipairs(self) do
			paragraph:touch()
		end

		self.topp = nil
		self.topw = nil
		self.botp = nil
		self.botw = nil
		self.wrapwidth = nil

		-- These should no longer exist; this dates from a previous attempt
		-- at undo with file version 6. We're not storing the undo buffer
		-- in files any more.
		self.undostack = nil
		self.redostack = nil
	end,

	-- calculate space above this paragraph
	spaceAbove = function(self, pn)
		local paragraph = self[pn]
		local paragraphabove = self[pn - 1]

		local sa = DocumentStyles[paragraph.style].above or 0 -- FIXME
		local sb = 0
		if paragraphabove then
			sb = DocumentStyles[paragraphabove.style].below or 0 -- FIXME
		end

		if (sa > sb) then
			return sa
		else
			return sb
		end
	end,

	-- calculate space below this paragraph
	spaceBelow = function(self, pn)
		local paragraph = self[pn]
		local paragraphbelow = self[pn + 1]

		local sb = DocumentStyles[paragraph.style].below or 0 -- FIXME
		local sa = 0
		if paragraphbelow then
			sa = DocumentStyles[paragraphbelow.style].above or 0 -- FIXME
		end

		if (sa > sb) then
			return sa
		else
			return sb
		end
	end,

	touch = function(self)
		FireEvent(Event.DocumentModified, self)
	end,

	renumber = function(self)
		local wc = 0
		local pn = 1

		for _, p in ipairs(self) do
			wc = wc + #p

			local style = DocumentStyles[p.style]
			if style.numbered then
				p.number = pn
				pn = pn + 1
			elseif not style.list then
				pn = 1
			end
		end

		self.wordcount = wc
	end
}

ParagraphClass =
{
	copy = function(self)
		local words = {}
		for _, w in ipairs(self) do
			words[#words+1] = w
		end

		return CreateParagraph(self.style, words)
	end,

	touch = function(self)
		self.lines = nil
		self.wrapwidth = nil
		self.xs = nil
		self.sentences = nil
	end,

	wrapTableRow = function(self, width)

		width = width or Document.wrapwidth

		if with == nil then
			width = 80
		end
		
		local sentences = self.sentences
		if (sentences == nil) then
			local issentence = true
			sentences = {}
			for wn, word in ipairs(self) do
				if issentence then
					sentences[wn] = true
					issentence = false
				end

				if word:find("%.$") then
					issentence = true
				end
			end
			sentences[#self] = true
			self.sentences = sentences
		end
		
		local fullstopspaces = WantFullStopSpaces()

		-- get previous paragraph
		local pn = 0 
		for n, par in ipairs(Document) do
			if par == self then
				pn = n - 1
			end
		end
		local pp = nil
		if pn > 0 then
			pp = Document[pn]
		end

		-- count cells
		local cells = {}
		local cell = {}
		local cellWidth = {}
		local cn = 1
		local w = 0
		local allcellswidth = 0
		for wn, word in ipairs(self) do
			-- get width of word (including space)
			local ww = GetStringWidth(word) + 1

			-- add an extra space if the user asked for it
			if fullstopspaces and word:find("%.$") then
				ww = ww + 1
			end

			w = w + ww
			cell[#cell+1] = wn
			
			if 
				word:find(';') and GetStringWidth(word) < 2
			then
				cells[#cells+1] = cell
				cellWidth[cn] = w + 1
				allcellswidth = allcellswidth + cellWidth[cn]
				w = 0
				cn = cn + 1
				cell = {}
			end
		end
		cellWidth[cn] = width - allcellswidth 
		if cellWidth[cn] <= 0 then 
			cellWidth[cn] = cellWidth[cn-1]
		end
		cells[#cells+1] = cell
		
		self.isFirstRow = true
		self.cn = cn
		self.cells = cells
		self.cellWidth = cellWidth
		if pp ~= nil then 
			if pp.style == "TR"  or 
				 pp.style == "TRB" 
			then
				isFirstRow = false
				if pp.cn and pp.cn >= self.cn then
					self.cn = pp.cn -- cell numbers
					self.cellWidth = pp.cellWidth
				end
			end
		end
		
		-- wrap each cell
		self.rowheight = 1

		for cn, cell in ipairs(cells) do
			local i = 0;
			local lines = {}
			local line = {}
			local w = 0
			for _, wn in ipairs(cell) do
				local word = self[wn]	
				
				-- get width of word (including space)
				local ww = GetStringWidth(word) + 1

				-- add an extra space if the user asked for it
				if fullstopspaces and word:find("%.$") then
					ww = ww + 1
				end

				w = w + ww
				local cell_width = self.cellWidth[cn] or width
				if (w >= cell_width) then
					lines[#lines+1] = line
					i = i+1
					line = {wn = wn}
					w = ww
				end

				line[#line+1] = wn
			end

			if (#line > 0) then
				lines[#lines+1] = line
				i = i + 1
			end

			if i > self.rowheight then
				self.rowheight = i
			end

			cell.lines = lines
		end

		-- concat cell lines
		local wordp = {}
		local lines = {}
		local l 
		local xs = {}
		for l=1,self.rowheight,1 do
			local newline = {wn = 1}
			local hasstart = false
			local start = 0
			for cn, cell in ipairs(cells) do
				if cn > 1 then
					start = start + self.cellWidth[cn-1]
				end
				local w = start
				for ln, line in ipairs(cell.lines) do
					if ln == l then
						for n, wn in ipairs(line) do
							if not hasstart then
								newline.wn = wn
								hasstart = true
							end
							local word = self[wn]	
							-- get width of word (including space)
							local ww = GetStringWidth(word) + 1
							
							xs[wn] = w
							w = w + ww
							
							newline[#newline+1] = wn 
							wordp[#wordp+1] = wn
						end	
					end
				end
			end
			lines[#lines+1] = newline
		end
		self.xs = xs
		self.wordp = wordp
		self.lines = lines
		return self.lines
	end,

	wrapImage = function(self, width)
		local sentences = self.sentences
		if (sentences == nil) then
			local issentence = true
			sentences = {}
			for wn, word in ipairs(self) do
				if issentence then
					sentences[wn] = true
					issentence = false
				end

				if word:find("%.$") then
					issentence = true
				end
			end
			sentences[#self] = true
			self.sentences = sentences
		end

		local imagedata = {}
		local lines = {}
		local xs = {}
		self.xs = xs
		
		local rows = 1
		imagedata[#imagedata+1] = ""
		imagedata[#imagedata+1] = ""
		local writerow = function(row)
			imagedata[#imagedata+1] = row
			rows = rows + 1
		end
		self.imagedata = imagedata
		
		width = width or Document.wrapwidth
		ParseImage(self[1], width, writerow)

		local i
		for i=1, rows+1, 1 do
			local line = {wn=1}
			if i == 1 then
				local w = 0
				for wn, word in ipairs(self) do
					local ww = GetStringWidth(word) + 1
					xs[wn] = w
					w = w + ww
					line[#line+1] = wn
				end
			end
			lines[#lines+1] = line
		end
		
		self.lines = lines
		return self.lines
	end,

	wrap = function(self, width)
		local sentences = self.sentences
		if (sentences == nil) then
			local issentence = true
			sentences = {}
			for wn, word in ipairs(self) do
				if issentence then
					sentences[wn] = true
					issentence = false
				end

				if word:find("%.$") then
					issentence = true
				end
			end
			sentences[#self] = true
			self.sentences = sentences
		end

		width = width or Document.wrapwidth
		if (self.wrapwidth ~= width) then
			local lines = {}
			local line = {wn = 1}
			local w = 0
			local xs = {}
			local fullstopspaces = WantFullStopSpaces()
			self.xs = xs

			width = width - self:getIndentOfLine(1)

			for wn, word in ipairs(self) do
				-- get width of word (including space)
				local ww = GetStringWidth(word) + 1

				-- add an extra space if the user asked for it
				if fullstopspaces and word:find("%.$") then
					ww = ww + 1
				end

				xs[wn] = w
				w = w + ww
				if (w >= width) then
					lines[#lines+1] = line
					if #lines == 1 then
						width = width + self:getIndentOfLine(1) - self:getIndentOfLine(2)
					end
					line = {wn = wn}
					w = ww
					xs[wn] = 0
				end

				line[#line+1] = wn
			end

			if (#line > 0) then
				lines[#lines+1] = line
			end

			self.lines = lines
		end

		return self.lines
	end,

	wrapBoth = function(self, width)
		local sentences = self.sentences
		if (sentences == nil) then
			local issentence = true
			sentences = {}
			for wn, word in ipairs(self) do
				if issentence then
					sentences[wn] = true
					issentence = false
				end

				if word:find("%.$") then
					issentence = true
				end
			end
			sentences[#self] = true
			self.sentences = sentences
		end

		width = width or Document.wrapwidth
		if (self.wrapwidth ~= width) then
			local lines = {}
			local line = {wn = 1}
			local nlines = 0  -- number of lines
			local linesw = {} -- width of line
			local w = 0
			local xs = {}
			local fullstopspaces = WantFullStopSpaces()
			self.xs = xs

			width = width - self:getIndentOfLine(1)

			for wn, word in ipairs(self) do
				-- get width of word (including space)
				local ww = GetStringWidth(word) + 1

				-- add an extra space if the user asked for it
				if fullstopspaces and word:find("%.$") then
					ww = ww + 1
				end

				xs[wn] = w
				w = w + ww
				if (w >= width) then
					nlines = nlines + 1
					linesw[#linesw+1] = w - ww
					lines[#lines+1] = line
					if #lines == 1 then
						width = width + self:getIndentOfLine(1) - self:getIndentOfLine(2)
					end
					line = {wn = wn}
					w = ww
					xs[wn] = 0
				end

				line[#line+1] = wn
			end

			if (#line > 0) then
				nlines = nlines + 1
				linesw[#linesw+1] = w
				lines[#lines+1] = line
			end

			-- Justify lines
			for ln, line in ipairs(lines) do
				if ln < nlines then
					local w = linesw[ln]
					local dw = width
					if ln == 1 then
						dw = width - DocumentStyles[self.style].firstindent or 0
					end
					local n = 1
					while w < dw do
						if not line[n] then
							n = 1
						end
						for i, wn in ipairs(line) do
							if i > n then
								xs[wn] = xs[wn] + 1
							end
						end
						w = w + 1
						n = n + 1
					end
				end
			end

			self.lines = lines
		end

		return self.lines
	end,

	wrapRight = function(self, width)
		local sentences = self.sentences
		if (sentences == nil) then
			local issentence = true
			sentences = {}
			for wn, word in ipairs(self) do
				if issentence then
					sentences[wn] = true
					issentence = false
				end

				if word:find("%.$") then
					issentence = true
				end
			end
			sentences[#self] = true
			self.sentences = sentences
		end

		width = width or Document.wrapwidth
		if (self.wrapwidth ~= width) then
			local linesw = {} -- width of line
			local wordsw = {} -- width of word
			local lines = {}
			local line = {wn = 1}
			local w = 0
			local xs = {}
			local fullstopspaces = WantFullStopSpaces()
			self.xs = xs

			for wn, word in ipairs(self) do
				-- get width of word (including space)
				local ww = GetStringWidth(word) + 1

				-- add an extra space if the user asked for it
				if fullstopspaces and word:find("%.$") then
					ww = ww + 1
				end

				wordsw[#wordsw+1] = ww
				
				xs[wn] = w
				w = w + ww
				if (w >= width) then
					linesw[#linesw+1] = w - ww
					lines[#lines+1] = line
					line = {wn = wn}
					w = ww
					xs[wn] = 0
				end

				line[#line+1] = wn
			end

			if (#line > 0) then
				linesw[#linesw+1] = w
				lines[#lines+1] = line
			end
		
			-- set lines to right
			local n = 1
			for ln, line in ipairs(lines) do
				local x = width - linesw[ln]
				for _, wn in ipairs(line) do
					self.xs[wn] = x
					x = x + wordsw[n]
					n = n + 1
				end
			end

			self.lines = lines
		end

		return self.lines
	end,

	wrapCenter = function(self, width)
		local sentences = self.sentences
		if (sentences == nil) then
			local issentence = true
			sentences = {}
			for wn, word in ipairs(self) do
				if issentence then
					sentences[wn] = true
					issentence = false
				end

				if word:find("%.$") then
					issentence = true
				end
			end
			sentences[#self] = true
			self.sentences = sentences
		end


		width = width or Document.wrapwidth
		if (self.wrapwidth ~= width) then
			local linesw = {} -- width of line
			local wordsw = {} -- width of word
			local lines = {}
			local line = {wn = 1}
			local w = 0
			local xs = {}
			local fullstopspaces = WantFullStopSpaces()
			self.xs = xs

			for wn, word in ipairs(self) do
				-- get width of word (including space)
				local ww = GetStringWidth(word) + 1

				-- add an extra space if the user asked for it
				if fullstopspaces and word:find("%.$") then
					ww = ww + 1
				end

				wordsw[#wordsw+1] = ww

				xs[wn] = w
				w = w + ww
				if (w >= width) then
					linesw[#linesw+1] = w - ww
					lines[#lines+1] = line
					line = {wn = wn}
					w = ww
					xs[wn] = 0
				end

				line[#line+1] = wn
			end

			if (#line > 0) then
				linesw[#linesw+1] = w
				lines[#lines+1] = line
			end

			-- center lines
			local n = 1
			for ln, line in ipairs(lines) do
				local x = width / 2 - linesw[ln] / 2
				for _, wn in ipairs(line) do
					self.xs[wn] = x
					x = x + wordsw[n]
					n = n + 1
				end
			end

			self.lines = lines
		end

		return self.lines
	end,

	renderLine = function(self, line, x, y)
		local istable = 0; 
		if self.style == "TR"  or 
			 self.style == "TRB" 
		then
			istable = 1
		end

		local cstyle = stylemarkup[self.style] or 0
		local ostyle = 0
		local xs = self.xs
		for _, wn in ipairs(line) do
			local w = self[wn]

			local payload = {
				word = w,
				ostyle = ostyle,
				cstyle = cstyle,
				firstword = self.sentences[wn]
			}
			FireEvent(Event.DrawWord, payload)

			ostyle = WriteStyled(x+xs[wn], y, payload.word,
				payload.ostyle, nil, nil, payload.cstyle, istable)
		end
	end,

	renderMarkedLine = function(self, line, x, y, width, pn)
		local istable = 0; 
		if self.style == "TR"  or 
			 self.style == "TRB" 
		then
			istable = 1
		end
		
		width = width or (ScreenWidth - x)

		local lwn = line.wn
		local mp1, mw1, mo1, mp2, mw2, mo2 = Document:getMarks()

		local cstyle = stylemarkup[self.style] or 0
		local ostyle = 0
		for wn, w in ipairs(line) do
			local s, e

			wn = lwn + wn - 1

			if (pn < mp1) or (pn > mp2) then
				s = nil
			elseif (pn > mp1) and (pn < mp2) then
				s = 1
			else
				if (pn == mp1) and (pn == mp2) then
					if (wn == mw1) and (wn == mw2) then
						s = mo1
						e = mo2
					elseif (wn == mw1) then
						s = mo1
					elseif (wn == mw2) then
						s = 1
						e = mo2
					elseif (wn > mw1) and (wn < mw2) then
						s = 1
					end
				elseif (pn == mp1) then
					if (wn > mw1) then
						s = 1
					elseif (wn == mw1) then
						s = mo1
					end
				else
					s = 1
					if (wn > mw2) then
						s = nil
					elseif (wn == mw2) then
						e = mo2
					end
				end
			end

			local payload = {
				word = self[w],
				ostyle = ostyle,
				cstyle = cstyle,
				firstword = self.sentences[wn]
			}
			FireEvent(Event.DrawWord, payload)

			ostyle = WriteStyled(x+self.xs[w], y, payload.word,
				payload.ostyle, s, e, payload.cstyle, istable)
		end
	end,

	-- returns: line number, word number in line
	getLineOfWord = function(self, wn)
		local lines
		if self.style == "TR" or self.style == "TRB" 
		then
			lines = self:wrapTableRow()
			for ln, l in ipairs(lines) do
				for _, wnn in ipairs(l) do
					if (wn == wnn) then
						return ln, wn
					end
				end
			end
			return nil, nil
		elseif self.style == "BOTH" then
				lines = self:wrapBoth()
		elseif self.style == "CENTER" then
				lines = self:wrapCenter()
		elseif self.style == "RIGHT" then
				lines = self:wrapRight()
		else
				lines = self:wrap()
		end
		
		for ln, l in ipairs(lines) do
			if (wn <= #l) then
				return ln, wn
			end

			wn = wn - #l
		end

		return nil, nil
	end,

	-- returns: number of characters
	getIndentOfLine = function(self, ln)
		local indent
		if (ln == 1) then
			indent = DocumentStyles[self.style].firstindent
		end
		indent = indent or DocumentStyles[self.style].indent or 0
		return indent
	end,

	-- returns: word number
	getWordOfLine = function(self, ln)
		local lines
		if self.style == "TR" or self.style == "TRB" 
		then
			lines = self:wrapTableRow()
		elseif self.style == "BOTH" then
			lines = self:wrapBoth()
		elseif self.style == "CENTER" then
			lines = self:wrapCenter()
		elseif self.style == "RIGHT" then
			lines = self:wrapRight()
		else
			lines = self:wrap()
		end 
		
		return lines[ln].wn
	end,

	-- returns: X offset, line number, word number in line
	getXOffsetOfWord = function(self, wn)
		local lines
		if self.style == "TR" or self.style == "TRB" 
		then
			lines = self:wrapTableRow()
		elseif self.style == "BOTH" then
			lines = self:wrapBoth()
		elseif self.style == "CENTER" then
			lines = self:wrapCenter()
		elseif self.style == "RIGHT" then
			lines = self:wrapRight()
		else
			lines = self:wrap()
		end
		local x = self.xs[wn]
		local ln, wn = self:getLineOfWord(wn)
		return x, ln, wn
	end,

	sub = function(self, start, count)
		if not count then
			count = #self - start + 1
		else
			count = min(count, #self - start + 1)
		end

		local t = {}
		for i = start, start+count-1 do
			t[#t+1] = self[i]
		end
		return t
	end,

	-- return an unstyled string containing the contents of the paragraph.
	asString = function(self)
		local s = {}
		for _, w in ipairs(self) do
			s[#s+1] = GetWordText(w)
		end

		return table_concat(s, " ")
	end
}

function CreateParagraph(style, ...)
	words = {}

	for _, t in ipairs({...}) do
		if (type(t) == "table") then
			for _, w in ipairs(t) do
				words[#words+1] = w
			end
		else
			words[#words+1] = t
		end
	end

	if type(style) ~= "string" then
		error("paragraph style is not a string")
	end
	words.style = style

	setmetatable(words, {__index = ParagraphClass})
	return words
end

-- Returns how many screen spaces a portion of a string takes up.
function GetWidthFromOffset(s, o)
	return GetStringWidth(s:sub(1, o-1))
end

-- Returns the offset into a string needed for a screen width.
function GetOffsetFromWidth(s, x)
		local len = #s
		local o = 1
		while (o <= len) do
			if (x == 0) then
				return o
			end

			local charlen = GetBytesOfCharacter(string.byte(s, o))
			local char = s:sub(o, o+charlen-1)
			local ww = GetStringWidth(char)
			if (ww > x) then
				return o
			end

			x = x - ww
			o = o + charlen
		end

		return len + 1
end

function GetWordSimpleText(s)
	s = GetWordText(s)
	s = UnSmartquotify(s)
	s = s:gsub('[~#&^$"<>]+', "")
	s = s:gsub("^[.'([{]+", "")
	s = s:gsub("[',.!?:;)%]}]+$", "")
	return s
end

function OnlyFirstCharIsUppercase(s)
    -- Return true if only first character is uppercase
    local first_char = s:sub(0, 1)
    if first_char:upper() == first_char then
        local remaining_chars = s:sub(2, s:len())
        if remaining_chars:lower() == remaining_chars then
            return true
        end
    end
    return false
end

function UpdateDocumentStyles()
	local plaintext =
	{
		desc = "Plain text",
		name = "P"
	}

	if WantDenseParagraphLayout() then
		plaintext.above = 0
		plaintext.below = 0
		plaintext.firstindent = 4
	else
		plaintext.above = 1
		plaintext.below = 1
		plaintext.firstindent = 0
	end
	
	local both =
	{
		desc = "Justify text to both",
		name = "BOTH"
	}

	if WantDenseParagraphLayout() then
		both.above = 0
		both.below = 0
		both.firstindent = 4
	else
		both.above = 1
		both.below = 1
		both.firstindent = 0
	end


	local styles =
	{
		plaintext,
		{
			desc = "Heading #1",
			name = "H1",
			above = 3,
			below = 1,
		},
		{
			desc = "Heading #2",
			name = "H2",
			above = 2,
			below = 1,
		},
		{
			desc = "Heading #3",
			name = "H3",
			above = 1,
			below = 1,
		},
		{
			desc = "Heading #4",
			name = "H4",
			above = 1,
			below = 1,
		},
		{
			desc = "Indented text",
			name = "Q",
			indent = 4,
			above = 1,
			below = 1,
		},
		{
			desc = "List item with bullet",
			name = "LB",
			above = 1,
			below = 1,
			indent = 4,
			bullet = "-",
			list = true,
		},
		{
			desc = "List item with number",
			name = "LN",
			above = 1,
			below = 1,
			indent = 4,
			numbered = true,
			list = true,
		},
		{
			desc = "List item without bullet",
			name = "L",
			above = 1,
			below = 1,
			indent = 4,
			list = true,
		},
		{
			desc = "Indented text, run together",
			name = "V",
			indent = 4,
			above = 0,
			below = 0
		},
		{
			desc = "Preformatted text",
			name = "PRE",
			indent = 4,
			above = 0,
			below = 0
		},
		{
			desc = "Raw data exported to output file",
			name = "RAW",
			indent = 0,
			above = 0,
			below = 0
		},
		{
			desc = "Justify text to left",
			name = "LEFT",
			indent = 0,
			above = 0,
			below = 0
		},
		{
			desc = "Justify text to right",
			name = "RIGHT",
			indent = 0,
			above = 0,
			below = 0
		},
		{
			desc = "Justify text to center",
			name = "CENTER",
			indent = 0,
			above = 0,
			below = 0
		},
		both,
		{
			desc = "Table Row with ';' separeted cells",
			name = "TR",
			indent = 1,
			above = 1,
			below = 1
		},
		{
			desc = "Table Row with borders and ';' separeted cells",
			name = "TRB",
			indent = 1,
			above = 1,
			below = 1
		},
		{
			desc = "Image. First word is image path - then title",
			name = "IMG",
			indent = 1,
			above = 1,
			below = 1
		}
	}

	for _, s in ipairs(styles) do
		styles[s.name] = s
	end

	DocumentStyles = styles
end

function CreateDocumentSet()
	UpdateDocumentStyles()
	local ds =
	{
		fileformat = FILEFORMAT,
		statusbar = true,
		documents = {},
		addons = {},
	}

	setmetatable(ds, {__index = DocumentSetClass})
	return ds
end

function CreateDocument()
	local d =
	{
		wrapwidth = nil,
		viewmode = 1,
		margin = 0,
		cp = 1,
		cw = 1,
		co = 1,
	}

	setmetatable(d, {__index = DocumentClass})

	local p = CreateParagraph("P", {""})
	d:appendParagraph(p)
	return d
end
