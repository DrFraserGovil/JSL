#!/bin/python3
import sys
import re
import colorbrewer
pattern = re.compile(r'node\s+\[(.*)\]\s+"(\S*)"\s*\[label = "(.*)"\]')
edge = re.compile(r'\"(\S*)\"\s+->\s+\"(.*)\"')
def extractData(file):
	rawlines = []
	data = []
	troubleFiles = [[2,"JSL/Concepts/ranges.h","Concepts.h","[weight=0.3]"],
				 	[2,"include/JSL/Concepts/ranges.h","Concepts.h","[weight=0.3]"],
					[2,"include/JSL/Display/Log.h","Display.h",None]]
	trouble = []
	for id,line in enumerate(file):
		rawlines.append(line.rstrip())
		match = pattern.search(line)
		if match:
			# group(1) extracts the content inside the capturing parentheses
			data.append([id,match.group(2).strip()])
		match = edge.search(line)
		if match:
			for i in range(len(troubleFiles)):
				g = troubleFiles[i][0]
				f = troubleFiles[i][1]
				safe = troubleFiles[i][2]
				action = troubleFiles[i][3]
				if match.group(g).strip() == f and (safe is None or  safe not in line):
					trouble.append([id,g,match.group(1).strip(), match.group(2).strip(),action])
	print(f"Extracted {len(rawlines)} lines, identified {len(data)} recolour points and {len(trouble)} node-modifications")
	return data,rawlines,trouble
def collateGroups(data):
	cols = dict()
	reassign = []
	for d in data:
		explode = d[1].split("/")
		q = "/".join(explode[:-1])
		slimname = "/".join(explode[1:])
		if (len(q) == 0):
			q = "-root-"
			slimname = "Library Entry"
		cols[q] = "#494949"
		reassign.append([d[0],q,slimname])
	
	g = max(3,min(12,len(cols)))
	palette = colorbrewer.Set3[g]
	for i,c in enumerate(cols.keys()):
		cols[c] = palette[i % g]
	for d in reassign:
		if ".cpp" in d[2]:	
			d[1] = "fillcolor = \"#8B8B8B\",style=\"\", shape=\"note\", color=\"#8c8c8c\", fontcolor=\"#4b4b4b\""
		else:
			merger = "JSL/" + d[2].split(".")[0]
			if merger in cols.keys():
				d[1] =  "fillcolor = \"" + ('#%02x%02x%02x' % cols[merger]) + "\", shape=\"folder\", style = \"filled\""
			elif d[2] == "Library Entry":
				d[1] =  "fillcolor = \"" + ('#%02x%02x%02x' % cols[d[1]]) + "\",shape=\"tripleoctagon\", style = \"filled\""
			else:
				d[1] =  "fillcolor = \"" + ('#%02x%02x%02x' % cols[d[1]]) + "\",shape=\"Mrecord\", style = \"filled\""

	return reassign
# def reweight(data):

# 	for d in data:


def interweave(file,reassign,trouble):
	rdx = 0
	tdx = 0
	for i,line in enumerate(file):
		if rdx < len(reassign) and i == reassign[rdx][0]:
			match = pattern.search(line)
			file[i] = line.replace(match.group(1),reassign[rdx][1]).replace(" = \"" + match.group(3)," = \"" + reassign[rdx][2])

			rdx += 1
		else:
			
			if tdx < len(trouble) and i == trouble[tdx][0]:
				target = trouble[tdx]
				p = [f"\"{target[2]}\"",f"\"{target[3]}\""]
				
				
				if target[4] is not None:
					p[target[1]-1] += target[4]
					
					space = " "*(42-len(p[0]))
					file[i] = f"    {p[0]}{space}-> {p[1]}"
				else:
					file[i] = ""
				tdx += 1
			dot = "[style=dotted, label=\"?\"]"
			if str(line).count(dot) > 0:
				file[i] = line.replace(dot,"")
			scaler = "scalexy"
			if str(line).count(scaler) > 0:
				file[i] = line.replace(scaler,"false") 
	return file
	
def colourise(file_path):

	try:
		with open(file_path, 'r') as file:
			data,lines,trouble = extractData(file)				
			reassign = collateGroups(data)
			newdata = interweave(lines,reassign,trouble)	
		with open(file_path,'w') as file:
			for line in newdata:
				file.write(line+"\n")	
	except FileNotFoundError:
		print(f"Error: File '{file_path}' not found.")
	except Exception as e:
		print(f"An error occurred: {e}")

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print("Usage: python3 extract.py <filename>")
	else:
		colourise(sys.argv[1])
