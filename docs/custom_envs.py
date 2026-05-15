from docutils import nodes
from docutils.parsers.rst import Directive, directives
from docutils.statemachine import ViewList  # Add this import

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
