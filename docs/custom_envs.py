from docutils import nodes
from docutils.parsers.rst import Directive, directives
from docutils.statemachine import ViewList  # Add this import
import re
class JSLClassDirective(Directive):
	has_content = True
	required_arguments = 1
	option_spec = {
		'file': directives.unchanged,
		'namespace': directives.unchanged,
	}

	def run(self):
		file = self.options.get('file', None)
		nsp = self.options.get('namespace', None)
		class_name = self.arguments[0]
		body_text = "\n".join(self.content)
		raw_lines = [body_text,"",]

		if nsp is None:
			attempt = str(class_name).split("::")[0:-1]
			if (len(attempt) > 0):
				nsp = "::".join(attempt)

		defs = []
		if file is not None:
			defs += [
				f"    :file: {file}",
			] 
		if nsp is not None:
			defs += [
				f"    :namespace: {nsp}",
			] 
		if len(defs) > 0:
			raw_lines += ["",
				f".. jsl-meta::",
			] + defs
		
		
		
		raw_lines += [
			f"",
			f".. doxygenclass:: {class_name}",
			f"   :members:",
			f"",
			f".. dropdown:: Internal Implementation",
			f"   :color: secondary",
			f"   :icon: gear",
			f"",
			f"   .. doxygenclass:: {class_name}",
			f"      :no-link:",
			f"      :members-only:",
			f"      :private-members:",
			f"      :protected-members:",
			f"      :members: False",
			f"",
			f""
		]


		# 2. Convert to a ViewList so Sphinx can track it
		vl = ViewList()
		for line in raw_lines:
			# We use 'manual-jsl-directive' as the "source" for error reporting
			vl.append(line, 'manual-jsl-directive')

		# 3. Create a container node and parse the ViewList into it
		node = nodes.container()
		self.state.nested_parse(vl, 0, node)
		
		return node.children


class JSLMetaDirective(Directive):
	has_content = True
	option_spec = {
		'file': directives.unchanged,
		'namespace': directives.unchanged,
	}

	def run(self):
		file = self.options.get('file', None)
		nsp = self.options.get('namespace', None)

		
		defs = []
		if file is not None:
			defs += [
				f"   :Defined In: ``include/JSL/{file}``",
			] 
		if nsp is not None:
			defs += [
				f"   :Namespace: ``{nsp}``",
			] 
		if len(defs) > 0:
			defs = ["",
				f".. dropdown:: Metadata",
				f"   :color: secondary",
				f"   :icon: gear","",
			] + defs

		vl = ViewList()
		for line in defs:
			# We use 'manual-jsl-directive' as the "source" for error reporting
			vl.append(line, 'manual-jsl-directive')

		# 3. Create a container node and parse the ViewList into it
		node = nodes.container()
		self.state.nested_parse(vl, 0, node)
		
		return node.children
	
class JSLHandover(Directive):
	option_spec = {
		'file': directives.unchanged,
		'namespace': directives.unchanged,
		'command' : directives.unchanged,
	}
	required_arguments = 1
	def run(self):
		file = self.options.get('file', None)
		nsp = self.options.get('namespace', None)
		command = self.options.get('command')
		arg = self.arguments[0]

		if nsp is None:
			attempt = str(arg).split("::")[0:-1]
			if (len(attempt) > 0):
				nsp = "::".join(attempt)

		if command is None or nsp is None or file is None:
			raise ValueError("A command, a namespace and file must be passed to a jsl object")
		
		lines = [
			f".. jsl-meta::",
			f"    :file: {file}", 
			f"    :namespace: {nsp}",
			"",
			f".. {command}:: {arg}",
			"",
		]
		vl = ViewList()
		for line in lines:
			# We use 'manual-jsl-directive' as the "source" for error reporting
			vl.append(line, 'manual-jsl-directive')

		# 3. Create a container node and parse the ViewList into it
		node = nodes.container()
		self.state.nested_parse(vl, 0, node)
		
		return node.children


class IndexGroup:
	def __init__(self,name):
		self.Name = name
		self.Subgroups = {}
		self.Files = []
	def Add(self,namespaces,links):
		front = namespaces[0]
		if len(namespaces) == 1:
			self.Files.append([front,links])
		else:
			if front not in self.Subgroups:
				self.Subgroups[front] = IndexGroup(front)
			self.Subgroups[front].Add(namespaces[1:],links)
	def Render(self,indent=0):
		print(f"{"  "*indent}{self.Name}::")
		indent += 1
		for file in self.Files:
			print(f"{"  "*indent}-{file[0]}")
		for mod in self.Subgroups.values():
			mod.Render(indent)
	def count(self):
		s = len(self.Files)
		for f in self.Subgroups.values():
			s += f.count()
		return s
	def CollateEntries(self,parent,group,depth):
		for file,link in self.Files:
			if depth == 0:
				group.append((parent+file,link))
			else:
				group.append((parent+file,link[0]))
		for subgroup in self.Subgroups.values():
			if depth > 0 or subgroup.count() < 2: 
				subgroup.CollateEntries(parent+subgroup.Name+"::",group,depth+1)
			else:
				sub = []
				subgroup.CollateEntries("",sub,depth+1)
				group.append((parent+subgroup.Name+"::",([],sub,None)))

	def SphinxRender(self):
		entries = []
		self.CollateEntries("",entries,0)
		return (self.Name,entries)

bracket_re = re.compile(r"(.*)\s\((.*)\)")
def cleanIndex(app,pagename,templatename,context,doctree):
	if pagename == "genindex":
		print("\n\tIntercepting html for modification")
		group = {}

		def testGroup(group,val):
			if val not in group.keys():
				group[val] = IndexGroup(val)

		for letter, entries in context.get("genindexentries",[]):
			for entry in entries:
				grab = bracket_re.match(entry[0])
				if (grab):
					name = grab.group(1)
					explode = name.split("::")
					root = explode[0]
					if len(explode) > 1:
						testGroup(group,root)
						group[root].Add(explode[1:],entry[1])
					else:
						n = "Macros & Global Namespace"
						testGroup(group,n)
						group[n].Add([root],entry[1])


		newentries = []
		for g in group.values():
			newentries.append(g.SphinxRender())
		# print(newentries[0][1][0])
		context['genindexentries'] = newentries
