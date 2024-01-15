--[[--
File              : docx.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 01.01.2024
Last Modified Date: 15.01.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--
-- © 2008 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

local table_concat = table.concat
local writezip = wg.writezip
local addimagetozip = wg.addimagetozip
local getimagesize = wg.getimagesize
local string_format = string.format

-----------------------------------------------------------------------------
-- The exporter itself.

local relations = {[[<?xml version="1.0" encoding="UTF-8"?>
		<Relationships 
			xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
			<Relationship 
				Id="rId1" 
				Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" 
				Target="styles.xml"/>
			<Relationship
				Id="rId2" 
				Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering" Target="numbering.xml"/>
			<Relationship 
				Id="rId3" 
				Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable" 
				Target="fontTable.xml"/>
			<Relationship 
				Id="rId4" 
				Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" 
				Target="settings.xml"/>
				]]}

local function add_relation(s)
	relations[#relations+1] = s
end

local images = {}

local function unhtml(s)
	s = s:gsub("&", "&amp;")
	s = s:gsub("<", "&lt;")
	s = s:gsub(">", "&gt;")
	return s
end

local function emit(s)
	return function(para) return s end
end

local function header()
	return function(para)
		return string_format('<w:p><w:pPr><w:pStyle w:val="%s"/></w:pPr><w:r><w:t>', para.style)
	end
end
	
local function simple()
	return function(para)
		return string_format('<w:pStyle w:val="%s">', para.style)
	end
end

local list_tab =
{
	["L"]  = 0,
	["LN"] = 1,
	["LB"] = 2,
}

local function list()
	return function(para)
		return string_format('<w:p><w:pPr><w:pStyle w:val="P"/><w:numPr><w:ilvl w:val="0"/><w:numId w:val="%s"/></w:numPr></w:pPr><w:r><w:t>', list_tab[para.style])
	end
end

local style_tab =
{
	["H1"]     = {false, header(), emit('</w:t></w:r></w:p>') },
	["H2"]     = {false, header(), emit('</w:t></w:r></w:p>') },
	["H3"]     = {false, header(), emit('</w:t></w:r></w:p>') },
	["H4"]     = {false, header(), emit('</w:t></w:r></w:p>') },
	["P"]      = {false, header(), emit('</w:t></w:r></w:p>') },
	["L"]      = {false, header(), emit('</w:t></w:r></w:p>') },
	["LB"]     = {false, header(), emit('</w:t></w:r></w:p>') },
	["LN"]     = {false, list(),   emit('</w:t></w:r></w:p>') },
	["Q"]      = {false, header(), emit('</w:t></w:r></w:p>') },
	["V"]      = {false, header(), emit('</w:t></w:r></w:p>') },
	["RAW"]    = {false, header(), emit('</w:t></w:r></w:p>') },
	["PRE"]    = {false, header(), emit('</w:t></w:r></w:p>') },
	["CENTER"] = {false, header(), emit('</w:t></w:r></w:p>') },
	["RIGHT"]  = {false, header(), emit('</w:t></w:r></w:p>') },
	["LEFT"]   = {false, header(), emit('</w:t></w:r></w:p>') },
	["TR"]     = {false, emit(''), emit('') },
	["TRB"]    = {false, emit(''), emit('') },
	["IMG"]    = {false, emit(''), emit('') },
}

local function callback(writer, document)
	local settings = DocumentSet.addons.htmlexport
	local currentstylename = nil
	
	function changepara(para)
		local newstylename = para and para.style
		local currentstyle = style_tab[currentstylename]
		local newstyle = style_tab[newstylename]
		
		if (newstylename ~= currentstylename) or
			not newstylename or
			not currentstyle[1] or
			not newstyle[1] 
		then
			if currentstyle then
				writer(currentstyle[3](para))
			end
			writer("\n")
			if newstyle then
				writer(newstyle[2](para))
			end
			currentstylename = newstylename
		else
			writer("\n")
		end
	end

	local rId = 5 
	local imageid = 1
		
	return ExportFileUsingCallbacks(document,
	{
		prologue = function()
			writer(
				[[<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
				<w:document 
					xmlns:o="urn:schemas-microsoft-com:office:office" 
					xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" 
					xmlns:v="urn:schemas-microsoft-com:vml" 
					xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" 
					xmlns:w10="urn:schemas-microsoft-com:office:word" 
					xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing" 
					xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape" 
					xmlns:wpg="http://schemas.microsoft.com/office/word/2010/wordprocessingGroup" 
					xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" 
					xmlns:wp14="http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing" 
					xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" 
					xmlns:w15="http://schemas.microsoft.com/office/word/2012/wordml" 
					mc:Ignorable="w14 wp14 w15">
					<w:body>
				]]
			)
			 
		end,
		
		epilogue = function()
			changepara(nil)
			writer('</w:body></w:document>\n')	
		end,
		
		rawtext = function(s)
			writer(s)
		end,
		
		text = function(s)
			writer(unhtml(s))
		end,
		
		notext = function(s)
		end,
		
		italic_on = function()
			writer('</w:t><w:rPr><w:rStyle w:val="B"/></w:rPr><w:t xml:space="preserve">')
		end,
		
		italic_off = function()
			writer('</w:t></w:r><w:r><w:t>')
		end,
		
		bold_on = function()
			writer('</w:t><w:rPr><w:rStyle w:val="B"/></w:rPr><w:t xml:space="preserve">')
		end,
		
		bold_off = function()
			writer('</w:t></w:r><w:r><w:t>')
		end,
		
		underline_on = function()
			writer('</w:t><w:rPr><w:rStyle w:val="B"/></w:rPr><w:t xml:space="preserve">')
		end,
		
		underline_off = function()
			writer('</w:t></w:r><w:r><w:t>')
		end,
		
		list_start = function()
		end,
		
		list_end = function()
		end,
		
		paragraph_start = function(para)
			changepara(para)
		end,		
		
		paragraph_end = function(para)
		end,
		
		image_start = function(para)
			changepara(nil)
		end,
		
		image_end = function(para)
			local X = 0
			local Y = 0
			local imagesize = function(x, y)
				X = x
				Y = y
			end
			if getimagesize(para.imagename, imagesize) then
			
				add_relation(string_format('<Relationship Id="rId%d" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/image%d.jpeg"\n/>', rId, imageid))
				local image = {rId, imageid, para.imagename}
				images[#images+1] = image

				writer('<w:p><w:pPr><w:pStyle w:val="CENTER"/></w:pPr><w:r><w:t>')
				for _, wn in ipairs(para.imagetitle) do
					writer(para[wn])
					writer(' ')
				end
				writer('</w:t></w:r></w:p>')

				writer(string_format([[
				<w:p>
					<w:pPr>
						<w:pStyle w:val="IMG"/>
						<w:bidi w:val="0"/>
						<w:spacing w:before="0" w:after="0"/>
						<w:rPr>
							<w:rFonts w:ascii="Times New Roman" w:hAnsi="Times New Roman"/>
							<w:sz w:val="24"/>
						</w:rPr>
					</w:pPr>
					<w:r>
						<w:rPr/>
						<w:drawing>
							<wp:inline distT="0" distB="0" distL="0" distR="0">
								<wp:docPr id="%s" name="Image%s" descr="" title="%s"/>
								<wp:cNvGraphicFramePr>
									<a:graphicFrameLocks xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" noChangeAspect="1"/>
								</wp:cNvGraphicFramePr>
								<a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">
									<a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">
										<pic:pic xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture">
											<pic:nvPicPr>
												<pic:cNvPr id="%s" name="Image%s" descr="" title="%s"/>
												<pic:cNvPicPr>
													<a:picLocks noChangeAspect="1" noChangeArrowheads="1"/>
												</pic:cNvPicPr>
											</pic:nvPicPr>
											<pic:blipFill>
												<a:blip r:embed="rId%s"/>
												<a:stretch>
													<a:fillRect/>
												</a:stretch>
											</pic:blipFill>
											<pic:spPr bwMode="auto">
												<a:xfrm>
													<a:off x="0" y="0"/>
													<a:ext cx="5878830" cy="%d"/>
												</a:xfrm>
												<a:prstGeom prst="rect">
													<a:avLst/>
												</a:prstGeom>
											</pic:spPr>
										</pic:pic>
									</a:graphicData>
								</a:graphic>
							</wp:inline>
						</w:drawing>
					</w:r>
				</w:p>
				]], imageid, imageid, para.imagename, imageid, imageid, para.imagename, rId, Y/X*5878830))
				rId = rId + 1
				imageid = imageid + 1
			end
		end,
		
		table_start = function(para)
			changepara(para)
			writer([[
			<w:tbl>
			<w:tblPr>
			</w:tblPr>
			<w:tblGrid>	
			]])
			local cn = 1
			for _, cell in ipairs(para.cells) do
				local width = para.cellWidth[cn] * 12 *72
				writer(string_format('<w:gridCol w:w="%s"/>', width))	
				cn = cn + 1
			end
			writer('</w:tblGrid>')	
		end,		
		
		table_end = function(para)
			writer('</w:tbl>')
		end,
		
		tablerow_start = function(para)
			writer('<w:tr>')
		end,		
		
		tablerow_end = function(para)
			writer('</w:tr>')
		end,

		tablecell_start = function(para)
			if para.style == "TRB" then
				writer([[
				<w:tc><w:tcPr><w:tcBorders>
        <w:top w:val="single" w:sz="4" w:space="0" w:color="000000"/>
        <w:bottom w:val="single" w:sz="4" w:space="0" w:color="000000"/>
        <w:left w:val="single" w:sz="4" w:space="0" w:color="000000"/>
        <w:right w:val="single" w:sz="4" w:space="0" w:color="000000"/>
				</w:tcBorders></w:tcPr>
				<w:p><w:r><w:t>
				]])
			else
				writer('<w:tc><w:r><w:t>')
			end
		end,		
		
		tablecell_end = function(para)
			writer('</w:t></w:r></w:p></w:tc>')
		end,
	})
end

local function export_docx_with_ui(filename, title, extension)
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
	
	local content = {}
	local writer = function(s)
		content[#content+1] = s
	end
	callback(writer, Document)
	content = table_concat(content)
	
	add_relation('</Relationships>\n')
	relations = table_concat(relations)
	
	local xml =
	{
		["_rels/.rels"] = [[<?xml version="1.0" encoding="UTF-8"?>
		<Relationships 
			xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
			<Relationship 
				Id="rId1" 
				Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" 
				Target="docProps/core.xml"/>
			<Relationship 
				Id="rId2" 
				Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" 
				Target="docProps/app.xml"/>
			<Relationship 
				Id="rId3" 
				Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" 
				Target="word/document.xml"/>
		</Relationships>]],

		["docProps/app.xml"] = [[<?xml version="1.0" encoding="UTF-8"?>
		<Properties 
			xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties" 
			xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">
			<Template/>
			<TotalTime>1</TotalTime>
			<Application>WordGrinder</Application>
			<AppVersion>0.8</AppVersion>
			<Pages>1</Pages>
			<Words>178</Words>
			<Characters>1061</Characters>
			<CharactersWithSpaces>1299</CharactersWithSpaces>
			<Paragraphs>14</Paragraphs>
		</Properties>]],

		["docProps/core.xml"] = [[<?xml version="1.0" encoding="UTF-8"?>
		<cp:coreProperties 
			xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" 
			xmlns:dc="http://purl.org/dc/elements/1.1/" 
			xmlns:dcterms="http://purl.org/dc/terms/" 
			xmlns:dcmitype="http://purl.org/dc/dcmitype/" 
			xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
			<dc:creator/>
			<dc:description/>
			<dc:language>ru-RU</dc:language>
			<cp:lastModifiedBy/>
			<dcterms:modified xsi:type="dcterms:W3CDTF">2024-01-02T07:31:29Z</dcterms:modified>
			<cp:revision>1</cp:revision>
			<dc:subject/>
			<dc:title/>
		</cp:coreProperties>]],

		["word/_rels/document.xml.rels"] = relations,
		
		["word/fontTable.xml"] = [[<?xml version="1.0" encoding="UTF-8"?>
		<w:fonts 
			xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" 
			xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
			<w:font w:name="Times New Roman">
				<w:charset w:val="00"/>
				<w:family w:val="roman"/>
				<w:pitch w:val="variable"/>
			</w:font>
			<w:font w:name="Symbol">
				<w:charset w:val="02"/>
				<w:family w:val="roman"/>
				<w:pitch w:val="variable"/>
			</w:font>
			<w:font w:name="Arial">
				<w:charset w:val="00"/>
				<w:family w:val="swiss"/>
				<w:pitch w:val="variable"/>
			</w:font>
			<w:font w:name="Nimbus Roman">
				<w:charset w:val="01"/>
				<w:family w:val="roman"/>
				<w:pitch w:val="variable"/>
			</w:font>
			<w:font w:name="serif">
				<w:charset w:val="01"/>
				<w:family w:val="roman"/>
				<w:pitch w:val="variable"/>
			</w:font>
			<w:font w:name="monospace">
				<w:charset w:val="01"/>
				<w:family w:val="modern"/>
				<w:pitch w:val="fixed"/>
			</w:font>
		</w:fonts>]],

		["word/settings.xml"] = [[<?xml version="1.0" encoding="UTF-8"?>
		<w:settings 
			xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
			<w:zoom w:percent="100"/>
			<w:defaultTabStop w:val="1134"/>
			<w:autoHyphenation w:val="true"/>
			<w:compat>
				<w:compatSetting
					w:name="compatibilityMode" 
					w:uri="http://schemas.microsoft.com/office/word" 
					w:val="15"/>
			</w:compat>
		</w:settings>]],

		["word/numbering.xml"] = [[<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
		<w:numbering 
			xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" 
			xmlns:o="urn:schemas-microsoft-com:office:office" 
			xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" 
			xmlns:v="urn:schemas-microsoft-com:vml" 
			xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" 
			xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" mc:Ignorable="w14">
			<w:abstractNum w:abstractNumId="0">
				<w:lvl w:ilvl="0">
					<w:start w:val="1"/>
					<w:numFmt w:val="none"/>
					<w:suff w:val="nothing"/>
					<w:lvlText w:val=""/>
					<w:lvlJc w:val="left"/>
					<w:pPr>
						<w:tabs>
							<w:tab w:val="num" w:pos="567"/>
						</w:tabs>
						<w:ind w:left="567" w:hanging="283"/>
					</w:pPr>
				</w:lvl>
			</w:abstractNum>
			<w:abstractNum w:abstractNumId="1">
				<w:lvl w:ilvl="0">
					<w:start w:val="1"/>
					<w:numFmt w:val="decimal"/>
					<w:lvlText w:val="%1"/>
					<w:lvlJc w:val="left"/>
					<w:pPr>
						<w:tabs>
							<w:tab w:val="num" w:pos="567"/>
						</w:tabs>
						<w:ind w:left="567" w:hanging="283"/>
					</w:pPr>
				</w:lvl>
			</w:abstractNum>
			<w:abstractNum w:abstractNumId="2">
				<w:lvl w:ilvl="0">
					<w:start w:val="1"/>
					<w:numFmt w:val="upperLetter"/>
					<w:lvlText w:val="-"/>
					<w:lvlJc w:val="left"/>
					<w:pPr>
						<w:tabs>
							<w:tab w:val="num" w:pos="567"/>
						</w:tabs>
						<w:ind w:left="567" w:hanging="283"/>
					</w:pPr>
				</w:lvl>
			</w:abstractNum>
			<w:num w:numId="0">
				<w:abstractNumId w:val="0"/>
			</w:num>
			<w:num w:numId="1">
				<w:abstractNumId w:val="1"/>
			</w:num>
			<w:num w:numId="2">
				<w:abstractNumId w:val="2"/>
			</w:num>
		</w:numbering>]],

		["[Content_Types].xml"] = [[<?xml version="1.0" encoding="UTF-8"?>
		<Types 
			xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
			<Default Extension="xml" ContentType="application/xml"/>
			<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
			<Default Extension="png" ContentType="image/png"/>
			<Default Extension="jpeg" ContentType="image/jpeg"/>
			<Override PartName="/_rels/.rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
			<Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>
			<Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>
			<Override PartName="/word/_rels/document.xml.rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
			<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
			<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
			<Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>
			<Override PartName="/word/fontTable.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml"/>
			<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
		</Types>]],

		["word/styles.xml"] = [[<?xml version="1.0" encoding="utf-8"?>
		<w:styles 
			xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" 
			xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml" 
			xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" 
			mc:Ignorable="w14">
			<w:docDefaults>
				<w:rPrDefault>
					<w:rPr>
						<w:rFonts w:ascii="Nimbus Roman" w:hAnsi="Nimbus Roman" w:eastAsia="Cantarell" w:cs="FreeSerif"/>
						<w:color w:val="000000"/>
						<w:sz w:val="24"/>
						<w:szCs w:val="24"/>
						<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
					</w:rPr>
				</w:rPrDefault>
				<w:pPrDefault>
					<w:pPr>
						<w:suppressAutoHyphens w:val="true"/>
					</w:pPr>
				</w:pPrDefault>
			</w:docDefaults>
			<w:style w:type="paragraph" w:styleId="Normal">
				<w:name w:val="Normal"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
				</w:pPr>
				<w:rPr>
					<w:sz w:val="24"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="character" w:styleId="B">
				<w:name w:val="B"/>
				<w:qFormat/>
				<w:rPr>
					<w:b/>
					<w:bCs/>
				</w:rPr>
			</w:style>
			<w:style w:type="character" w:styleId="I">
				<w:name w:val="I"/>
				<w:qFormat/>
				<w:rPr>
					<w:i/>
					<w:iCs/>
				</w:rPr>
			</w:style>
			<w:style w:type="character" w:styleId="UL">
				<w:name w:val="UL"/>
				<w:qFormat/>
				<w:rPr>
					<w:u w:val="single"/>
				</w:rPr>
			</w:style>
			<w:style w:type="paragraph" w:styleId="H1">
				<w:name w:val="H1"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
					<w:spacing w:before="283" w:after="113"/>
				</w:pPr>
				<w:rPr>
					<w:rFonts w:ascii="serif" w:hAnsi="serif"/>
					<w:b/>
					<w:sz w:val="36"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="paragraph" w:styleId="H2">
				<w:name w:val="H2"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
					<w:spacing w:before="283" w:after="113"/>
				</w:pPr>
				<w:rPr>
					<w:rFonts w:ascii="serif" w:hAnsi="serif"/>
					<w:b/>
					<w:sz w:val="31"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="paragraph" w:styleId="H3">
				<w:name w:val="H3"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
					<w:spacing w:before="283" w:after="113"/>
				</w:pPr>
				<w:rPr>
					<w:rFonts w:ascii="serif" w:hAnsi="serif"/>
					<w:b/>
					<w:sz w:val="26"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="paragraph" w:styleId="H4">
				<w:name w:val="H4"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
					<w:spacing w:before="283" w:after="113"/>
				</w:pPr>
				<w:rPr>
					<w:rFonts w:ascii="serif" w:hAnsi="serif"/>
					<w:b/>
					<w:sz w:val="24"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="paragraph" w:styleId="P">
				<w:name w:val="P"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
					<w:spacing w:before="85" w:after="85"/>
					<w:ind w:firstLine="567"/>
					<w:jc w:val="both"/>
				</w:pPr>
				<w:rPr>
					<w:rFonts w:ascii="serif" w:hAnsi="serif"/>
					<w:sz w:val="24"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="paragraph" w:styleId="CENTER">
				<w:name w:val="CENTER"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
					<w:spacing w:before="85" w:after="85"/>
					<w:jc w:val="center"/>
				</w:pPr>
				<w:rPr>
					<w:rFonts w:ascii="serif" w:hAnsi="serif"/>
					<w:sz w:val="24"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="paragraph" w:styleId="LEFT">
				<w:name w:val="LEFT"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
					<w:spacing w:before="85" w:after="85"/>
					<w:jc w:val="left"/>
				</w:pPr>
				<w:rPr>
					<w:rFonts w:ascii="serif" w:hAnsi="serif"/>
					<w:sz w:val="24"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="paragraph" w:styleId="RIGHT">
				<w:name w:val="RIGHT"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
					<w:spacing w:before="85" w:after="85"/>
					<w:jc w:val="right"/>
				</w:pPr>
				<w:rPr>
					<w:rFonts w:ascii="serif" w:hAnsi="serif"/>
					<w:sz w:val="24"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="paragraph" w:styleId="Q">
				<w:name w:val="Q"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
					<w:spacing w:before="85" w:after="85"/>
					<w:ind w:left="567" w:hanging="0"/>
				</w:pPr>
				<w:rPr>
					<w:rFonts w:ascii="serif" w:hAnsi="serif"/>
					<w:sz w:val="24"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="paragraph" w:styleId="V">
				<w:name w:val="V"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
					<w:spacing w:before="0" w:after="0"/>
					<w:ind w:left="567" w:hanging="0"/>
				</w:pPr>
				<w:rPr>
					<w:rFonts w:ascii="serif" w:hAnsi="serif"/>
					<w:sz w:val="24"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="paragraph" w:styleId="PRE">
				<w:name w:val="PRE"/>
				<w:qFormat/>
				<w:pPr>
					<w:widowControl w:val="false"/>
					<w:bidi w:val="0"/>
					<w:spacing w:before="0" w:after="0"/>
				</w:pPr>
				<w:rPr>
					<w:rFonts w:ascii="monospace" w:hAnsi="monospace"/>
					<w:sz w:val="24"/>
					<w:lang w:val="ru-RU" w:eastAsia="zh-CN" w:bidi="hi-IN"/>
				</w:rPr>
			</w:style>
			<w:style w:type="numbering" w:styleId="LB">
				<w:name w:val="LB"/>
				<w:qFormat/>
			</w:style>
			<w:style w:type="numbering" w:styleId="LN">
				<w:name w:val="LN"/>
				<w:qFormat/>
			</w:style>
			<w:style w:type="numbering" w:styleId="L">
				<w:name w:val="L"/>
				<w:qFormat/>
			</w:style>
		</w:styles>]],

		 ["word/document.xml"] = content
	}


	if not writezip(filename, xml) then
		ModalMessage(nil, "Unable to open the output file "..e..".")
		QueueRedraw()
		return false
	end
	
	for _, image in ipairs(images) do
		local file = image[3]
		local key = string_format('word/media/image%d.jpeg', image[2])
		addimagetozip(filename, key, file)
	end
		
	QueueRedraw()
	return true
end

function Cmd.ExportDOCXFile(filename)
	return export_docx_with_ui(filename, "Export DOCX File", ".docx")
end

-- Note: just the content.xml.
function Cmd.ExportToDOCXString()
	return ExportToString(Document, callback)
end

