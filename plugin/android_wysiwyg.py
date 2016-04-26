import sublime, sublime_plugin
import os

wysiwyg_module_num = 0
wysiwyg_file_id = 0
wysiwyg_so_location = '/ssd/BrainBridge/BrainBridge/data/lib/'
wysiwyg_script_location = '/ssd/BrainBridge/BrainBridge/data/script/'
wysiwyg_target_location = '/data/WYSIWYG/'
wysiwyg_ticket = 1

class UpdateCommand(sublime_plugin.TextCommand):
	def run(self, edit):
		global wysiwyg_module_num
		global wysiwyg_file_id
		global wysiwyg_ticket
		global wysiwyg_target_location
		global wysiwyg_so_location
		#self.view.insert(edit, 0, "Hello, World!")
		print('Target File ',self.view.file_name())
		pos = self.view.sel()[0]

		found = False
		moduleID = -1

		for i in range(wysiwyg_module_num):
			tmp = self.view.get_regions(str(i))[0]
			if tmp.contains(pos) :
				mRegion = tmp;
				found = True
				moduleID = i
		if found :
			print("Updating... Module"+str(moduleID) + '\n')
			newCode = self.view.substr(mRegion)
			f = open(self.view.file_name() + ".cpp.cpp", 'r')
			code = f.readlines()
			f.close()


			#print(code)
			f = open(self.view.file_name() + ".cpp.cpp", 'w')

			chop = False

			for i in range(len(code)) :
				if code[i] == ("/*WYSIWYG_FILE" + str(wysiwyg_file_id) + "_FUNC" + str(moduleID) + "_START*/\n") :
					chop = True
					f.write(code[i])
					f.write(newCode)
					f.write("\n")
				if code[i] == ("/*WYSIWYG_FILE" + str(wysiwyg_file_id) + "_FUNC" + str(moduleID) + "_END*/\n") :
					chop = False
				
				if chop == False and code[i] != "WYSIWYG_Init();\n" :
					f.write(code[i])

			f.close()

			#Compile Module
			mCmd = wysiwyg_script_location +str(wysiwyg_file_id)+'.sh'
			os.system(mCmd)
			print(mCmd+'\n')


			#Send to Phone	
			mCmd = 'adb push ' +wysiwyg_so_location+str(wysiwyg_file_id)+'.so '+wysiwyg_target_location
			mCmd = mCmd + str(wysiwyg_file_id)+'_'+str(wysiwyg_ticket)+'.so '
			os.system(mCmd)
			print(mCmd+'\n')


			#Apply
			mCmd = 'adb shell \"echo '
			mCmd = mCmd + str(wysiwyg_ticket) +' ' + str(wysiwyg_file_id) + ' '
			mCmd = mCmd + wysiwyg_target_location + str(wysiwyg_file_id)+'_'+str(wysiwyg_ticket)+'.so '
			mCmd = mCmd + 'FILE'+str(wysiwyg_file_id)+'_FUNC'+str(moduleID)
			mCmd = mCmd + '>'
			mCmd = mCmd + wysiwyg_target_location + 'cmd\"'
			print(mCmd + '\n')

			os.system(mCmd)

			wysiwyg_ticket = wysiwyg_ticket + 1


class EventDump(sublime_plugin.EventListener):  
	def on_load(self, view):  
		global wysiwyg_module_num
		global wysiwyg_file_id
		print('Target File ',view.file_name())
		sourcelist = [line.rstrip('\n') for line in open('/ssd/BrainBridge/BrainBridge/data/sourcelist','r')]
		print(sourcelist)

		fileID = -1
		for i in range(len(sourcelist)):
			if sourcelist[i] == view.file_name():
				fileID = i


		print(fileID)
		wysiwyg_file_id = fileID 

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
				#moduleStart = view.text_point(modulelist[i][2]-1, modulelist[i][3]-1)
				#moduleEnd = view.text_point(modulelist[i][4]+1, modulelist[i][5])
				mScope = "string"
				if modulelist[i][1] % 3 == 0:
					mScope = "entity.name.function"
				if modulelist[i][1] % 3 == 1:
					#mScope = "keyword"
					mScope = "comment"
				if modulelist[i][1] > wysiwyg_module_num:
					wysiwyg_module_num = modulelist[i][1]

				view.add_regions(str(modulelist[i][1]),[sublime.Region(moduleStart,moduleEnd)], mScope,"dot",sublime.DRAW_NO_FILL)
