-- © 2008 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

local function callback(writer, document)
	return ExportFileUsingCallbacks(document,
	{
		prologue = function()
		end,
		
		rawtext = function(s)
			writer(s)
		end,
		
		text = function(s)
			writer(s)
		end,
		
		notext = function(s)
		end,
		
		italic_on = function()
		end,
		
		italic_off = function()
		end,
		
		underline_on = function()
		end,
		
		underline_off = function()
		end,
		
		bold_on = function()
		end,
		
		bold_off = function()
		end,
		
		list_start = function()
		end,
		
		list_end = function()
		end,
		
		paragraph_start = function(para)
		end,		
		
		paragraph_end = function(para)
			writer('\n')
		end,
	
		table_start = function(para)
			writer('+')
			for cn, cell in ipairs(para.cells) do
				writer(string.rep("─", para.cellWidth[cn]))
				writer('+')
			end
			writer('\n')
		end,
		
		table_end = function(para)
			writer('\n')
		end,

		tablerow_start = function(para)
			writer('\n')
			writer('+')
			for cn, cell in ipairs(para.cells) do
				writer(string.rep("─", para.cellWidth[cn]))
				writer('+')
			end
			writer('\n')
		end,
		
		tablerow_end = function(para)
			writer('\n')
			writer('+')
			for cn, cell in ipairs(para.cells) do
				writer(string.rep("─", para.cellWidth[cn]))
				writer('+')
			end
			writer('\n')
		end,

		tablecell_start = function(para)
		end,
		
		tablecell_end = function(para)
			writer('|')
		end,
		
		image_start = function(para)
			writer(para.imagename)
			writer(' ')
			for _, wn in ipairs(para.imagetitle) do
					writer(para[wn])
					writer(' ')
			end
		end,
		image_end = function(para)
			writer('\n')
		end,
		
		epilogue = function()
		end
	})
end

function Cmd.ExportTextFile(filename)
	return ExportFileWithUI(filename, "Export Text File", ".txt",
		callback)
end

function Cmd.ExportToTextString()
	return ExportToString(Document, callback)
end
