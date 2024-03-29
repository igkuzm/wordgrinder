--[[--
File              : benchmark.lua
Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
Date              : 01.01.2024
Last Modified Date: 01.01.2024
Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
--]]--
#!/usr/bin/env -S wordgrinder --lua

-- © 2015 David Given.
-- WordGrinder is licensed under the MIT open source license. See the COPYING
-- file in this distribution for the full text.

-- This user script is used to do quick and dirty benchmarks of the file
-- load/save code. It generates a ludicrously big (million word) file,
-- then does stuff to it and measures the result.

local text = [[Sed ut perspiciatis unde omnis iste natus error sit voluptatem
accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo
inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo
enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia
consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque
porro quisquam est, qui dolorem ipsum quia dolor sit amet, consectetur,
adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et
dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis
nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex
ea commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea
voluptate velit esse quam nihil molestiae consequatur, vel illum qui dolorem
eum fugiat quo voluptas nulla pariatur?]]

local words = {}
for w in text:gmatch("%S+") do
	words[#words+1] = w
end
print(#words.." words of source text")

-- Generate some source text.

math.randomseed(0) -- predictable pseudorandom numbers
while ((Document.wordcount or 0) < 10000) do
	local a = math.random(#words)
	local b = math.random(#words)
	if (b < a) then
		a, b = b, a
	end

	Cmd.InsertStringIntoWord(words[a])
	for i=a+1, b do
		Cmd.SplitCurrentWord()
		Cmd.InsertStringIntoWord(words[i])
	end
	Cmd.SplitCurrentParagraph()
	FireEvent(Event.Changed)
end

-- Now duplicate it a hundred times (way faster than generating a million words).

Cmd.GotoBeginningOfDocument()
Cmd.ToggleMark()
Cmd.GotoEndOfDocument()
Cmd.Copy()

for i = 1, 100 do
	Cmd.Paste()
end

FireEvent(Event.Changed)
print(Document.wordcount.." words generated")

-- Now the benchmarks!

local function time(name, cb)
	local before = os.clock()
	cb()
	local after = os.clock()
	print(name..": "..math.floor((after-before)*1000).."ms")
end

local function getfilesize(filename)
	local fp = io.open(filename, "r")
	local size = fp:seek("end")
	fp:close()
	return size
end

time("Save .wg file", function() Cmd.SaveCurrentDocumentAs("/tmp/temp.wg") end)
print("(size of file: "..getfilesize("/tmp/temp.wg")..")")
time("Save .html file", function() Cmd.ExportHTMLFile("/tmp/temp.html") end)
time("Save .odt file", function() Cmd.ExportODTFile("/tmp/temp.odt") end)
time("Save .txt file", function() Cmd.ExportTextFile("/tmp/temp.txt") end)
time("Save .rtf file", function() Cmd.ExportRTFFile("/tmp/temp.rtf") end)

print("Performing save/load test...")
Cmd.LoadDocumentSet("/tmp/temp.wg")
Cmd.SaveCurrentDocumentAs("/tmp/temq.wg")
if (getfilesize("/tmp/temp.wg") ~= getfilesize("/tmp/temq.wg")) then
	print("*** File sizes do not match in save/load test!")
end

