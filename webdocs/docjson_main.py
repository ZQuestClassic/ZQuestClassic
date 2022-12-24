import docjson_lib as lib
import argparse
import os
from tkinter import *
from tkinter import filedialog
from tkinter import messagebox

parser = argparse.ArgumentParser()
parser.add_argument('--in', dest='inputfile', default='',metavar='FILE',
    help='.json to generate docs from')
parser.add_argument('--out', dest='outputfile', default='ZScript_Docs.html',metavar='FILE',
    help='HTML webpage output')
parser.add_argument('--db','-d', action='store_true', help='Print debug info')
parser.add_argument('--cli','-c', dest='edit', action='store_false', help='Just cli, don\'t open editor')
args = parser.parse_args()

lib.debug_out = args.db

file_loaded = False
needs_save = False
if os.path.exists(args.inputfile):
    json_obj = lib.loadjson(args.inputfile)
    file_loaded = True
    if not args.edit:
        lib.savehtml(args.outputfile,json_obj)
        exit(0)
else:
    if args.edit:
        args.inputfile = os.path.dirname(args.inputfile)
    else:
        lib.parse_fail(f"Input file '{args.inputfile}' does not exist!")

## GUI functions
def loader_json():
    global json_obj, root, needs_save, file_loaded, curpage
    fname = filedialog.askopenfilename(parent = root, title = 'Load', initialfile = args.inputfile, filetypes = (('Json','*.json'),))
    if len(fname) < 1:
        return
    switch(LoadPage)
    try:
        json_obj = lib.loadjson(fname)
        args.inputfile = fname
        needs_save = False
        file_loaded = True
        switch(SheetsPage)
    except:
        json_obj = None
        args.inputfile = ''
        needs_save = False
        file_loaded = False
        switch(InfoPage)
        popError(f'Error occurred loading input file:\n{fname}')
def saver_json():
    global json_obj, needs_save
    if len(args.inputfile) < 1:
        saver_json_as()
        return
    oldpage = curpage
    switch(SavePage)
    try:
        lib.savejson(args.inputfile,json_obj)
        needs_save = False
    except:
        popError(f'Error occurred saving file:\n{args.inputfile}')
    switch(oldpage)
def saver_json_as():
    global json_obj, root, needs_save
    fname = filedialog.asksaveasfilename(parent = root, title = 'Save As', initialfile = args.inputfile, filetypes = (('Json','*.json'),),defaultextension = '.json')
    if len(fname) < 1:
        return
    oldpage = curpage
    switch(SavePage)
    try:
        lib.savejson(fname,json_obj)
        args.inputfile = fname
        needs_save = False
    except:
        popError(f'Error occurred saving file:\n{fname}')
    switch(oldpage)
def saver_html():
    global json_obj, root
    fname = filedialog.asksaveasfilename(parent = root, title = 'Export HTML', initialfile = args.outputfile, filetypes = (('HTML','*.html'),),defaultextension = '.html')
    if len(fname) < 1:
        return
    oldpage = curpage
    switch(SavePage)
    try:
        lib.savehtml(fname,json_obj)
        args.outputfile = fname
    except:
        popError(f'Error occurred saving file:\n{fname}')
    switch(oldpage)
def quitter():
    global root, needs_save
    if needs_save:
        if not messagebox.askyesno(parent=root, title = 'Exit without saving?', message = 'Changes to the .json have not been saved!'):
            return
    root.destroy()
    exit(0)
def edited():
    global needs_save
    needs_save = True
def onfile():
    global filemenu
    filemenu.entryconfig('Save', state = 'normal' if needs_save and file_loaded else 'disabled')
    filemenu.entryconfig('Save As', state = 'normal' if file_loaded else 'disabled')
    filemenu.entryconfig('Save HTML', state = 'normal' if file_loaded else 'disabled')
def popError(s):
    messagebox.showinfo(title = 'Error!', message = s)
## Pages
def switch(pageclass):
    global root, mainframe, curpage
    if mainframe is not None:
        mainframe.destroy()
    mainframe = pageclass(root)
    mainframe.grid()
    curpage = pageclass
    root.update()

class InfoPage(Frame):
    def __init__(frame, root):
        Frame.__init__(frame,root)
        Label(frame, text = '''This GUI allows editing the web documentation.
        Docs are stored in .json format, which can be saved/loaded in the 'File' menu.
        You can also export the generated .html file.
        
        You must load a .json file to continue.''').pack()
class SheetsPage(Frame):
    def __init__(frame, root):
        Frame.__init__(frame,root)
        Label(frame, text = 'Insert list of sheets').pack()
class TabsPage(Frame):
    def __init__(frame, root):
        Frame.__init__(frame,root)
        Label(frame, text = 'Insert list of tabs').pack()
class LoadPage(Frame):
    def __init__(frame, root):
        Frame.__init__(frame,root)
        Label(frame, text = 'Loading... Please Wait...').pack()
class SavePage(Frame):
    def __init__(frame, root):
        Frame.__init__(frame,root)
        Label(frame, text = 'Saving... Please Wait...').pack()

root = Tk()
root.title('ZS Docs Editor')
root.geometry('640x480')
menubar = Menu(root)
filemenu = Menu(menubar, tearoff=0, postcommand=onfile)
filemenu.add_command(label = 'Load', command = loader_json)
filemenu.add_command(label = 'Save', command = saver_json)
filemenu.add_command(label = 'Save As', command = saver_json_as)
filemenu.add_command(label = 'Save HTML', command = saver_html)
filemenu.add_separator()
filemenu.add_command(label = 'Exit', command = quitter)
menubar.add_cascade(label='File', menu=filemenu)
root.config(menu=menubar)
root.protocol("WM_DELETE_WINDOW", quitter)
root.grid_rowconfigure(0, weight=1)
root.grid_columnconfigure(0, weight=1)

mainframe = None
if file_loaded:
    switch(SheetsPage)
else:
    switch(InfoPage)

root.mainloop()
