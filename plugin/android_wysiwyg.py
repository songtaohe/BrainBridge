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
				mScope = "string"
				if modulelist[i][1] % 3 == 0:
					mScope = "entity.name.function"
				if modulelist[i][1] % 3 == 1:
					mScope = "keyword"

				self.view.add_regions(str(modulelist[i][1]),[sublime.Region(moduleStart,moduleEnd)], mScope,"dot",sublime.DRAW_NO_FILL)


class EventDump(sublime_plugin.EventListener):  
	def on_load(self, view):  
		print('Target File ',view.file_name())
		sourcelist = [line.rstrip('\n') for line in open('/ssd/BrainBridge/BrainBridge/data/sourcelist','r')]
		print(sourcelist)

		fileID = -1
		for i in range(len(sourcelist)):
			if sourcelist[i] == view.file_name():
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
				moduleStart = view.text_point(modulelist[i][2], modulelist[i][3]-1)
				moduleEnd = view.text_point(modulelist[i][4], modulelist[i][5])
				mScope = "string"
				if modulelist[i][1] % 3 == 0:
					mScope = "entity.name.function"
				if modulelist[i][1] % 3 == 1:
					#mScope = "keyword"
					mScope = "comment"

				view.add_regions(str(modulelist[i][1]),[sublime.Region(moduleStart,moduleEnd)], mScope,"dot",sublime.DRAW_NO_FILL)
