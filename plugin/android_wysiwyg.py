import sublime, sublime_plugin

class ExampleCommand(sublime_plugin.TextCommand):
	def run(self, edit):
		#self.view.insert(edit, 0, "Hello, World!")
		print('Target File ',self.view.file_name())
		sourcelist = [line.rstrip('\n') for line in open('/ssd/BrainBridge/BrainBridge/data/sourcelist','r')]
		print(sourcelist)

		fileID = -1
		for i in range(len(sourcelist)):
			if sourcelist[i] == self.view.file_name():
				fileID = i

		print(fileID)

		if fileID == -1:
			return;

		with open('/ssd/BrainBridge/BrainBridge/data/modulelist','r') as f:
			modulelist = [[int(x) for x in line.split()] for line in f]

		print(modulelist)

		for i in range(len(modulelist)):
			if modulelist[i][0] == fileID :
				#insert regions
				moduleStart = self.view.text_point(modulelist[i][2], modulelist[i][3]-1)
				moduleEnd = self.view.text_point(modulelist[i][4], modulelist[i][5])
				self.view.add_regions(str(modulelist[i][1]),[sublime.Region(moduleStart,moduleEnd)], "somescope","dot",sublime.DRAW_SOLID_UNDERLINE|sublime.DRAW_NO_FILL|sublime.DRAW_NO_OUTLINE)

		#v = sublime.active_window().active_view()
		#v.add_regions("hello", [sublime.Region(0,v.size())], "somescope","", sublime.DRAW_SOLID_UNDERLINE|sublime.DRAW_NO_FILL|sublime.DRAW_NO_OUTLINE)
