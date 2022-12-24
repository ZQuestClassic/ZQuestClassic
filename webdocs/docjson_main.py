import docjson_lib as lib
import argparse
import os
from tkinter import *
from tkinter import filedialog
from tkinter import messagebox
from tkinter import simpledialog
from tkinter import ttk

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
cursheet = 0
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
    global json_obj, root, needs_save, file_loaded, curpage, cursheet
    fname = filedialog.askopenfilename(parent = root, title = 'Load', initialfile = args.inputfile, filetypes = (('Json','*.json'),))
    if len(fname) < 1:
        return
    switch(LoadPage)
    cursheet = 0
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
def get_sel(listbox):
    sel = listbox.curselection()
    if len(sel) < 1:
        return -1
    return sel[0]
def edit_sheet(ind):
    global cursheet
    if ind < 0:
        return
    cursheet = ind
    switch(EditShPage)
def edit_named():
    switch(EditNamedPage)
def add_sheet(ind):
    global json_obj, cursheet, needs_save
    if ind < 0:
        ind = 0
    name = 'New Sheet'
    sheets = json_obj['sheets']
    json_obj['sheets'] = sheets[:ind+1] + [{'name':name,'tabs':[]}] + sheets[ind+1:]
    needs_save = True
    cursheet = ind+1
    switch(SheetsPage)

def ren_sheet(ind):
    global json_obj, cursheet, root, needs_save
    if ind < 0:
        return
    
    name = simpledialog.askstring('Input', f"Rename '{json_obj['sheets'][ind]['name']}' to?", parent=root)
    if not name:
        return
    needs_save = True
    json_obj['sheets'][ind]['name'] = name
    cursheet = ind
    switch(SheetsPage)
    
def swap_sheets(s1,s2):
    global json_obj, needs_save
    json_obj['sheets'][s1],json_obj['sheets'][s2] = json_obj['sheets'][s2],json_obj['sheets'][s1]
    needs_save = True
def sheetshift(listbox,shift):
    global sheetlistbox, cursheet
    ind = get_sel(listbox)
    ind2 = ind+shift
    if ind <= 0 or ind2 <= 0 or ind2 >= listbox.size():
        return
    swap_sheets(ind,ind2)
    cursheet = ind2
    switch(SheetsPage)
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
        global json_obj, sheetlistbox, cursheet
        Frame.__init__(frame,root)
        
        f1 = Frame(frame)
        Label(f1, text = 'Sheets').pack()
        sheetlistbox = Listbox(f1)
        for ind,sheet in enumerate(json_obj['sheets']):
            sheetlistbox.insert(ind,sheet['name'])
        sheetlistbox.pack()
        sheetlistbox.selection_set(cursheet)
        f1.pack(side='left')
        
        f2 = Frame(frame)
        Button(f2, text = '↑', command = lambda:sheetshift(sheetlistbox,-1)).pack()
        Button(f2, text = '↓', command = lambda:sheetshift(sheetlistbox,1)).pack()
        f2.pack(side='left')
        
        f2 = Frame(frame)
        wid = 10
        Button(f2, bd = 3, width = wid, text = 'Edit', command = lambda:edit_sheet(get_sel(sheetlistbox))).pack()
        Button(f2, bd = 3, width = wid, text = 'Add', command = lambda:add_sheet(get_sel(sheetlistbox))).pack()
        Button(f2, bd = 3, width = wid, text = 'Rename', command = lambda:ren_sheet(get_sel(sheetlistbox))).pack()
        Button(f2, bd = 3, width = wid, text = 'Named Data', command = edit_named).pack()
        f2.pack(side='left')
        
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
class EditShPage(Frame):
    def __init__(frame, root):
        global json_obj
        Frame.__init__(frame,root)
        Label(frame, text = f"TODO Edit Sheet {json_obj['sheets'][cursheet]['name']}").pack()
        Button(frame, text = 'Exit', command = lambda: switch(SheetsPage)).pack()
class EditNamedPage(Frame):
    def __init__(frame, root):
        Frame.__init__(frame,root)
        Label(frame, text = 'TODO Edit Named').pack()
        Button(frame, text = 'Exit', command = lambda: switch(SheetsPage)).pack()

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
