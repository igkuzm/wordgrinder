--[[--
File              : opendocument.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 01.01.2024
Last Modified Date: 16.01.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--
-- © 2008 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

local table_concat = table.concat
local writezip = wg.writezip
local string_format = string.format
local addimagetozip = wg.addimagetozip
local getimagesize = wg.getimagesize

-----------------------------------------------------------------------------
-- The exporter itself.

local content = {}
local images = {}
local imageid = 1
local tableid = 1

local function add_style(s)
	styles[#styles+1] = s
end

local function unhtml(s)
	s = s:gsub("&", "&amp;")
	s = s:gsub("<", "&lt;")
	s = s:gsub(">", "&gt;")
	--s = s:gsub("%s", "<text:span/>")
	return s
end

local function emit(s)
	return function(para) return s end
end

local function header()
	return function(para)
		return string_format('<text:h text:style-name="%s" text:outline-level="1">', para.style)
	end
end
		
local function simple()
	return function(para)
		return string_format('<text:p text:style-name="%s">', para.style)
	end
end
		
local function list()
	return function(para)
		return string_format('<text:list text:style-name="%s"><text:list-item%s><text:p text:style-name="P">',
			para.style,
			para.number and string_format(' text:start-value="%d"', para.number) or '')
	end
end

local style_tab =
{
	["H1"]     = {false, header(), emit('</text:h>') },
	["H2"]     = {false, header(), emit('</text:h>') },
	["H3"]     = {false, header(), emit('</text:h>') },
	["H4"]     = {false, header(), emit('</text:h>') },
	["P"]      = {false, emit('<text:p text:style-name="P">'), emit('</text:p>') },
	["L"]      = {false, list(), emit('</text:p></text:list-item></text:list>') },
	["LB"]     = {false, list(), emit('</text:p></text:list-item></text:list>') },
	["LN"]     = {false, list(), emit('</text:p></text:list-item></text:list>') },
	["Q"]      = {false, simple(), emit('</text:p>') },
	["V"]      = {false, simple(), emit('</text:p>') },
	["RAW"]    = {false, emit(''), emit('') },
	["PRE"]    = {false, simple(), emit('</text:p>') },
	["CENTER"] = {false, emit('<text:p text:style-name="CENTER">'), emit('</text:p>') },
	["RIGHT"]  = {false, emit('<text:p text:style-name="RIGHT">'), emit('</text:p>') },
	["LEFT"]   = {false, emit('<text:p text:style-name="LEFT">'), emit('</text:p>') },
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
		
	return ExportFileUsingCallbacks(document,
	{
		prologue = function()
			writer(
				[[<?xml version="1.0" encoding="UTF-8"?>
					<office:document-content office:version="1.0"
					xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
					xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
					xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
					xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
					xmlns:xlink="http://www.w3.org/1999/xlink" 
					xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" 
					xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0" 
					xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0"> 
					<office:automatic-styles>
									<style:style style:name="TR"
                		style:family="table-cell">
										<style:table-cell-properties 
											fo:border="none"/>
                	</style:style>

									<style:style style:name="TRB"
                		style:family="table-cell">
										<style:table-cell-properties 
											fo:border="0.5pt solid #000000"/>
                	</style:style>  

									 <style:style style:name="IMG" 
										style:family="graphic" 
										style:parent-style-name="Graphics">
										<style:graphic-properties 
											fo:margin-left="0cm" 
											fo:margin-right="0cm" 
											fo:margin-top="0cm" 
											fo:margin-bottom="0cm" 
											style:vertical-pos="top" 
											style:vertical-rel="baseline" 
											fo:background-color="transparent" 
											draw:fill="none" 
											draw:fill-color="#ffffff" 
											fo:padding="0cm" 
											fo:border="none" 
											style:mirror="none" 
											fo:clip="rect(0cm, 0cm, 0cm, 0cm)" 
											draw:luminance="0%" 
											draw:contrast="0%" 
											draw:red="0%" 
											draw:green="0%" 
											draw:blue="0%" 
											draw:gamma="100%" 
											draw:color-inversion="false" 
											draw:image-opacity="100%" 
											draw:color-mode="standard"/>
									 </style:style>
				]])

				-- leave space here to insert rows - number 2
				writer('')

				writer(
				[[	
				</office:automatic-styles>
					<office:body><office:text>
				]])
		end,
		
		epilogue = function()
			changepara(nil)
			writer('</office:text></office:body></office:document-content>\n')	
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
			writer('<text:span text:style-name="I">')
		end,
		
		italic_off = function()
			writer("</text:span>")
		end,
		
		bold_on = function()
			writer('<text:span text:style-name="B">')
		end,
		
		bold_off = function()
			writer("</text:span>")
		end,
		
		underline_on = function()
			writer('<text:span text:style-name="UL">')
		end,
		
		underline_off = function()
			writer("</text:span>")
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

		table_start = function(para)
			changepara(para)
			writer('<table:table>')
			--writer(string_format('<table:table-column table:number-columns-repeated="%s"/>', para.cn))
			for cn, cell in ipairs(para.cells) do
				local w = para.cellWidth[cn] / 7

				table.insert(content, 2, string_format('<style:style style:name="Table%d.Column%d" style:family="table-column">\n', tableid, cn))
				table.insert(content, 3, string_format('<style:table-column-properties style:column-width="%dpt"/>\n', w*5))
				table.insert(content, 4, ('</style:style>'))

				writer(string_format('<table:table-column table:style-name="Table%d.Column%d"/>', tableid, cn))
			end
		end,
		
		table_end = function(para)
			writer('</table:table>')
			tableid = tableid + 1
		end,
		
		tablerow_start = function(para)
			writer('<table:table-row>')
		end,
		
		tablerow_end = function(para)
			writer('</table:table-row>')
		end,
		
		tablecell_start = function(para)
			writer(string_format('<table:table-cell table:style-name="%s" office:value-type="string">', para.style))
			writer('<text:p text:style-name="P">')
		end,

		tablecell_end = function(para)
			writer('</text:p>')
			writer('</table:table-cell>')
		end,
		
		image_start = function(para)
			changepara(para)
			writer('<text:p text:style-name="CENTER">')
		end,
		
		image_end = function(para)
			local X = 0
			local Y = 0
			local imagesize = function(x, y)
				X = x
				Y = y
			end
			getimagesize(para.imagename, imagesize)
			local image = {imageid, para.imagename}
			images[#images+1] = image
			
			writer(string_format([[
				<draw:frame draw:style-name="IMG" draw:name="Image%d" text:anchor-type="as-char" svg:width="16.85cm" svg:height="%dcm" draw:z-index="0">
					<draw:image xlink:href="Pictures/Image%d.jpg" xlink:type="simple" xlink:show="embed" xlink:actuate="onLoad" draw:mime-type="image/jpeg"/>
				</draw:frame>
			]], imageid, Y/X*16.85, imageid))
			writer('</text:p>')
			imageid = imageid + 1
		end,
	})
end

local function export_odt_with_ui(filename, title, extension)
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
	
	local writer = function(s)
		content[#content+1] = s
	end

	callback(writer, Document)
	content = table_concat(content)

	local xml =
	{
		["mimetype"] = "application/vnd.oasis.opendocument.text",

		["META-INF/manifest.xml"] = [[<?xml version="1.0" encoding="UTF-8"?>
			<manifest:manifest xmlns:manifest="urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"> 
				<manifest:file-entry
					manifest:media-type="application/vnd.oasis.opendocument.text"
					manifest:full-path="/"/> 
				<manifest:file-entry
					manifest:media-type="text/xml"
					manifest:full-path="content.xml"/> 
				<manifest:file-entry
					manifest:media-type="text/xml"
					manifest:full-path="meta.xml"/> 
				<manifest:file-entry
					manifest:media-type="text/xml"
					manifest:full-path="settings.xml"/> 
				<manifest:file-entry
					manifest:media-type="text/xml"
					manifest:full-path="styles.xml"/> 
			</manifest:manifest>
		]],
		
		["styles.xml"] = [[<?xml version="1.0" encoding="UTF-8"?>
			<office:document-styles office:version="1.0"
				xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
				xmlns:fo="urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"
				xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
				xmlns:table="urn:oasis:names:tc:opendocument:xmlns:table:1.0" 
				xmlns:draw="urn:oasis:names:tc:opendocument:xmlns:drawing:1.0" 
				xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
				xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0">
				
				<office:font-face-decls>
				  <style:font-face style:name="monospace" svg:font-family="monospace" style:font-family-generic="modern" style:font-pitch="fixed"/>
				  <style:font-face style:name="serif" svg:font-family="serif" style:font-family-generic="roman" style:font-pitch="variable"/>
				</office:font-face-decls>
				 
				<office:styles>
<style:default-style style:family="graphic">
      <style:graphic-properties svg:stroke-color="#3465a4" draw:fill-color="#729fcf" fo:wrap-option="no-wrap" draw:shadow-offset-x="0.3cm" draw:shadow-offset-y="0.3cm" draw:start-line-spacing-horizontal="0.283cm" draw:start-line-spacing-vertical="0.283cm" draw:end-line-spacing-horizontal="0.283cm" draw:end-line-spacing-vertical="0.283cm" style:flow-with-text="false"/>
      <style:paragraph-properties style:text-autospace="ideograph-alpha" style:line-break="strict" style:writing-mode="lr-tb" style:font-independent-line-spacing="false">
        <style:tab-stops/>
      </style:paragraph-properties>
      <style:text-properties style:use-window-font-color="true" style:font-name="Nimbus Roman" fo:font-size="12pt" fo:language="ru" fo:country="RU" style:letter-kerning="true" style:font-name-asian="Nimbus Sans1" style:font-size-asian="12pt" style:language-asian="zh" style:country-asian="CN" style:font-name-complex="FreeSans" style:font-size-complex="12pt" style:language-complex="hi" style:country-complex="IN"/>
    </style:default-style>
					<style:style style:name="B" style:family="text">
	              		<style:text-properties fo:font-weight="bold"
	              			style:font-weight-complex="bold"
	                		style:font-weight-asian="bold"/>
	            	</style:style>
	            	
					<style:style style:name="I" style:family="text">
	              		<style:text-properties fo:font-style="italic"
	              			style:font-style-asian="italic"
	                		style:font-style-complex="italic"/>
	            	</style:style>
	            	
					<style:style style:name="UL" style:family="text">
						<style:text-properties style:text-underline-style="solid"
							style:text-underline-width="auto"
							style:text-underline-color="font-color"/>
	            	</style:style>
                	
                	<style:style style:name="H1"
                		style:family="paragraph" style:class="text">
                		<style:paragraph-properties
                			fo:margin-top="5mm"
                			fo:margin-bottom="2mm"/>
                		<style:text-properties
                			fo:font-size="150%"
                			style:font-name="serif"
                			fo:font-weight="bold"/>
                	</style:style>
                	
                	<style:style style:name="H2"
                		style:family="paragraph" style:class="text">
                		<style:paragraph-properties
                			fo:margin-top="5mm"
                			fo:margin-bottom="2mm"/>
                		<style:text-properties
                			fo:font-size="130%"
                			style:font-name="serif"
                			fo:font-weight="bold"/>
                	</style:style>
                	
                	<style:style style:name="H3"
                		style:family="paragraph" style:class="text">
                		<style:paragraph-properties
                			fo:margin-top="5mm"
                			fo:margin-bottom="2mm"/>
                		<style:text-properties
                			fo:font-size="110%"
                			style:font-name="serif"
                			fo:font-weight="bold"/>
                	</style:style>
                	
                	<style:style style:name="H4"
                		style:family="paragraph" style:class="text">
                		<style:paragraph-properties
                			fo:margin-top="5mm"
                			fo:margin-bottom="2mm"/>
                		<style:text-properties
                			style:font-name="serif"
                			fo:font-weight="bold"/>
                	</style:style>
                	
                	<style:style style:name="P"
                		style:family="paragraph" style:class="text">
                		<style:paragraph-properties
                			fo:margin-top="1.5mm"
											fo:text-align="justify"
											fo:text-indent="1.00cm"
                			fo:margin-bottom="1.5mm"/>
                		<style:text-properties style:font-name="serif"/>
                	</style:style>
                	
									<style:style style:name="CENTER"
                		style:family="paragraph" style:class="text">
                		<style:paragraph-properties
                			fo:margin-top="1.5mm"
											fo:text-align="center"
                			fo:margin-bottom="1.5mm"/>
                		<style:text-properties style:font-name="serif"/>
                	</style:style>
                	
									<style:style style:name="LEFT"
                		style:family="paragraph" style:class="text">
                		<style:paragraph-properties
                			fo:margin-top="1.5mm"
											fo:text-align="left"
                			fo:margin-bottom="1.5mm"/>
                		<style:text-properties style:font-name="serif"/>
                	</style:style>
        
									<style:style style:name="RIGHT"
                		style:family="paragraph" style:class="text">
                		<style:paragraph-properties
                			fo:margin-top="1.5mm"
											fo:text-align="right"
                			fo:margin-bottom="1.5mm"/>
                		<style:text-properties style:font-name="serif"/>
                	</style:style>
        
									<style:style style:name="Q"
                		style:family="paragraph" style:class="text">
                		<style:paragraph-properties
							fo:margin-top="1.5mm"
                			fo:margin-bottom="1.5mm"
                			fo:margin-left="10mm"/>
                		<style:text-properties style:font-name="serif"/>
                	</style:style>
                	
                	<style:style style:name="V"
                		style:family="paragraph" style:class="text">
                		<style:paragraph-properties
                			fo:margin-left="10mm"
                			fo:margin-top="0mm"
                			fo:margin-bottom="0mm"/>
                		<style:text-properties style:font-name="serif"/>
                	</style:style>
                	
                	<style:style style:name="PRE"
                		style:family="paragraph" style:class="text">
                		<style:paragraph-properties
                			fo:margin-top="0mm"
                			fo:margin-bottom="0mm"/>
                		<style:text-properties style:font-name="monospace"/>
                	</style:style>
                	
                	<text:list-style style:name="LB">
                		<text:list-level-style-bullet text:level="1" text:bullet-char="•">
							<style:list-level-properties
								text:space-before="5mm"
								text:min-label-width="5mm"/>
						</text:list-level-style-bullet>
                	</text:list-style>
                	
                	<text:list-style style:name="LN">
                		<text:list-level-style-number text:level="1">
							<style:list-level-properties
								text:space-before="5mm"
								text:min-label-width="5mm"/>
						</text:list-level-style-number>
                	</text:list-style>
                	
                	<text:list-style style:name="L">
                		<text:list-level-style-bullet text:level="1" text:bullet-char=" ">
							<style:list-level-properties
								text:space-before="5mm"
								text:min-label-width="5mm"/>
						</text:list-level-style-bullet>
                	</text:list-style>
						</office:styles></office:document-styles>
		]],
		
		["settings.xml"] = [[<?xml version="1.0" encoding="UTF-8"?>
			<office:document-settings office:version="1.0"
				xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"/>
		]],
		
		["meta.xml"] = [[<?xml version="1.0" encoding="UTF-8"?>
			<office:document-meta office:version="1.0"
				xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"/>
		]],
		
		["content.xml"] = content
	}
	
	if not writezip(filename, xml) then
		ModalMessage(nil, "Unable to open the output file "..e..".")
		QueueRedraw()
		return false
	end
		
	for _, image in ipairs(images) do
		local file = image[2]
		local key = string_format('Pictures/Image%d.jpg', image[1])
		addimagetozip(filename, key, file)
	end
	
	QueueRedraw()
	return true
end

function Cmd.ExportODTFile(filename)
	return export_odt_with_ui(filename, "Export ODT File", ".odt")
end

-- Note: just the content.xml.
function Cmd.ExportToODTString()
	return ExportToString(Document, callback)
end

