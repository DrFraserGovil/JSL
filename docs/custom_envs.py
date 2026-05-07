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

		defs = []
		if file is not None:
			defs += [
				f"    #. Defined in: :file:`{file}`",
			] 
		if nsp is not None:
			defs += [
				f"    #. Belongs to ``namespace {nsp}``",
			] 
		if len(defs) > 0:
			raw_lines += ["",
				f".. dropdown:: Metadata",
				f"   :color: secondary",
				f"   :icon: gear",""
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

