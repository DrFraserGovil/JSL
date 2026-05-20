#!/bin/python3
import sys
import re
import colorbrewer
pattern = re.compile(r'node\s+\[fillcolor=\"([^,]+)\",\s*style=filled\]\s+"(\S*)"\s*\[label = "(.*)"\]')

def extractData(file):
	rawlines = []
	data = []
	for id,line in enumerate(file):
		rawlines.append(line.rstrip())
		match = pattern.search(line)
		if match:
			# group(1) extracts the content inside the capturing parentheses
			data.append([id,match.group(2).strip()])
	return data,rawlines
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
		cols[q] = "#000000"
		reassign.append([d[0],q,slimname])
	
	g = max(3,min(12,len(cols)))
	palette = colorbrewer.Set3[g]
	for i,c in enumerate(cols.keys()):
		cols[c] = palette[i % g]
	
	for d in reassign:
		d[1] =  '#%02x%02x%02x' % cols[d[1]]

	return reassign
	
def interweave(file,reassign):
	rdx = 0
	for i,line in enumerate(file):
		if rdx < len(reassign) and i == reassign[rdx][0]:
			match = pattern.search(line)
			file[i] = line.replace(match.group(1),reassign[rdx][1]).replace(" = \"" + match.group(3)," = \"" + reassign[rdx][2])

			rdx += 1
		else:
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
			data,lines = extractData(file)				
			reassign = collateGroups(data)
			newdata = interweave(lines,reassign)	
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
