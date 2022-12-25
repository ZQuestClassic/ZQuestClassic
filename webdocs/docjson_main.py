import docjson_lib as lib
import argparse
import os, time, json
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

cur_directory = ''
cur_file = ''
file_loaded = False
needs_save = False
needs_autosave = False
needs_edit_save = False
cursheet = 0
cursec = 0
curentry = 0
root = None

def popError(s):
    messagebox.showinfo(title = 'Error!', message = s)
def update_file(fname):
    global cur_directory, cur_file
    args.inputfile = fname
    cur_directory = os.path.dirname(fname)
    cur_file = os.path.basename(fname)
    if not cur_file:
        args.inputfile = ''
    if root:
        _title = 'ZS Docs Editor'
        if cur_file:
            _title += f' ({cur_file})'
        root.title(_title)

if os.path.exists(args.inputfile):
    if not args.edit:
        json_obj = lib.loadjson(args.inputfile)
        lib.savehtml(args.outputfile,json_obj)
        exit(0)
    
    try:
        json_obj = lib.loadjson(args.inputfile)
        file_loaded = True
    except Exception as e:
        popError(f"Error '{str(e)}' occurred loading input file:\n{args.inputfile}")
else:
    if args.edit:
        update_file(args.inputfile)
        cur_file = ''
    else:
        lib.parse_fail(f"Input file '{args.inputfile}' does not exist!")

## Configs
def _def_config(key,defval):
    global config
    try:
        return config[key]
    except:
        config[key] = defval
        return defval
def read_config():
    global config
    try:
        with open('docjson_config.json', 'r') as file:
            config = json.loads(file.read())
    except:
        config = {}
    _def_config('autosave_minutes',5)
def write_config():
    global config
    with open('docjson_config.json', 'w') as file:
        file.write(json.dumps(config, indent=4))

config = None
read_config()

## GUI functions
def loader_json():
    global json_obj, root, needs_save, file_loaded, curpage, cursheet, needs_autosave, cur_file, cur_directory
    fname = filedialog.askopenfilename(parent = root, title = 'Load', initialdir = cur_directory, initialfile = cur_file, filetypes = (('Json','*.json'),))
    if len(fname) < 1:
        return
    cursheet = 0
    try:
        json_obj = lib.loadjson(fname)
        update_file(trim_auto(fname))
        needs_save = False
        needs_autosave = False
        file_loaded = True
        switch(SheetsPage)
    except Exception as e:
        json_obj = None
        update_file('')
        needs_save = False
        needs_autosave = False
        file_loaded = False
        switch(InfoPage)
        popError(f"Error '{str(e)}' occurred loading input file:\n{fname}")
def saver_json():
    global json_obj, needs_save
    if len(args.inputfile) < 1:
        saver_json_as()
        return
    try:
        lib.savejson(args.inputfile,json_obj)
        needs_save = False
    except:
        popError(f'Error occurred saving file:\n{args.inputfile}')
def saver_json_as():
    global json_obj, root, needs_save, cur_directory, cur_file
    fname = filedialog.asksaveasfilename(parent = root, title = 'Save As', initialdir = cur_directory, initialfile = cur_file, filetypes = (('Json','*.json'),),defaultextension = '.json')
    if len(fname) < 1:
        return
    try:
        lib.savejson(fname,json_obj)
        update_file(trim_auto(fname))
        needs_save = False
    except:
        popError(f'Error occurred saving file:\n{fname}')
def saver_html():
    global json_obj, root
    _dir = os.path.dirname(args.outputfile)
    fname = os.path.basename(args.outputfile)
    fname = filedialog.asksaveasfilename(parent = root, title = 'Export HTML', initialdir = _dir, initialfile = fname, filetypes = (('HTML','*.html'),),defaultextension = '.html')
    if len(fname) < 1:
        return
    try:
        lib.savehtml(fname,json_obj)
        args.outputfile = fname
    except:
        popError(f'Error occurred saving file:\n{fname}')
def quitter():
    global root, needs_save, needs_edit_save
    if needs_edit_save:
        if not messagebox.askyesno(parent=root, title = 'Exit without saving?', message = 'Edits to this entry have not been saved!'):
            return
    if needs_save:
        if not messagebox.askyesno(parent=root, title = 'Exit without saving?', message = 'Changes to the .json have not been saved!'):
            return
    write_config()
    root.destroy()
    exit(0)
def onfile():
    global filemenu
    filemenu.entryconfig('Save', state = 'normal' if needs_save and file_loaded else 'disabled')
    filemenu.entryconfig('Save As', state = 'normal' if file_loaded else 'disabled')
    filemenu.entryconfig('Save HTML', state = 'normal' if file_loaded else 'disabled')
def onoption():
    global optmenu
    asmin = config['autosave_minutes']
    optmenu.entryconfig(1, label = f"Autosave ({asmin} min)" if asmin else "Autosave (off)")
def opt_autosave():
    global config
    val = simpledialog.askinteger('Input', f"Autosave every how many minutes? (current: {config['autosave_minutes']})", minvalue=0, parent=root)
    if val is None:
        return
    config['autosave_minutes'] = val
def timer():
    return round(time.perf_counter() * 1000)
def trim_auto(s):
    s1,ext = os.path.splitext(s)
    s2 = os.path.splitext(s1)
    if len(s2) > 1 and s2[1] == '.auto':
        s1 = s2[0]
    return s1 + ext
def add_auto(s):
    s1,ext = os.path.splitext(s)
    s2 = os.path.splitext(s1)
    if len(s2) > 1 and s2[1] == '.auto':
        return s
    return f'{s1}.auto{ext}'

time_minute = 1000*60
def update_autosave(i=1):
    global config, needs_autosave
    if not needs_autosave: #Reset, nothing to autosave
        root.after(time_minute - timer() % time_minute, update_autosave)
    if i >= config['autosave_minutes']:
        fname = add_auto(args.inputfile)
        lib.savejson(fname,json_obj)
        needs_autosave = False
        #Reset the autosave timer
        root.after(time_minute - timer() % time_minute, update_autosave)
    else:
        # Wait another minute
        root.after(time_minute - timer() % time_minute, update_autosave, i+1)
def mark_edited():
    global needs_save, needs_autosave
    needs_save = True
    needs_autosave = True
def local_edited():
    global needs_edit_save
    needs_edit_save = True
def get_sel(listbox):
    sel = listbox.curselection()
    if len(sel) < 1:
        return -1
    return sel[0]

def edit_named():
    global cursheet, cursec
    cursheet = -1
    cursec = 0
    switch(EditShPage)
def edit_sheet(ind):
    global cursheet, cursec
    if ind < 0:
        return
    cursheet = ind
    cursec = 0
    switch(EditShPage)
def add_sheet(ind):
    global json_obj, cursheet, needs_save
    if ind < 0:
        ind = 0
    name = 'New Sheet'
    sheets = json_obj['sheets']
    json_obj['sheets'] = sheets[:ind+1] + [{'name':name,'tabs':[]}] + sheets[ind+1:]
    mark_edited()
    cursheet = ind+1
    switch(SheetsPage)
def ren_sheet(ind):
    global json_obj, cursheet, root, needs_save
    if ind < 0:
        return
    
    name = simpledialog.askstring('Input', f"Rename '{json_obj['sheets'][ind]['name']}' to?", parent=root)
    if not name:
        return
    mark_edited()
    json_obj['sheets'][ind]['name'] = name
    cursheet = ind
    switch(SheetsPage)

def edit_sec(ind):
    global cursec
    if ind < 0:
        return
    cursec = ind
    switch(EditSecPage)
def add_sec(ind):
    global json_obj, cursheet, needs_save, cursec
    if ind < 0:
        ind = 0
    name = 'New Section'
    sheet = _getsheet(cursheet)
    sheet['tabs'] = sheet['tabs'][:ind+1] + [{'name':name,'lines':[]}] + sheet['tabs'][ind+1:]
    _setsheet(cursheet,sheet)
    mark_edited()
    cursec = ind+1
    switch(EditShPage)
def ren_sec(ind):
    global json_obj, cursheet, root, needs_save, cursec
    if ind < 0:
        return
    
    sheet = _getsheet(cursheet)
    name = simpledialog.askstring('Input', f"Rename '{sheet['tabs'][ind]['name']}' to?", parent=root)
    if not name:
        return
    mark_edited()
    sheet['tabs'][ind]['name'] = name
    _setsheet(cursheet,sheet)
    cursec = ind
    switch(EditShPage)

def edit_entry(ind):
    global curentry
    if ind < 0:
        return
    curentry = ind
    switch(EditEntryPage)
def add_entry(ind):
    global json_obj, cursheet, needs_save, cursec, curentry
    name = 'New Entry'
    sheet = _getsheet(cursheet)
    lines = sheet['tabs'][cursec]['lines']
    lines = lines[:ind+1] + [{'name':name,'val':'<todo>New Entry - fill content</todo>'}] + lines[ind+1:]
    sheet['tabs'][cursec]['lines'] = lines
    _setsheet(cursheet,sheet)
    mark_edited()
    curentry = ind+1
    switch(EditSecPage)

def swap_sheets(s1,s2):
    global json_obj, needs_save
    json_obj['sheets'][s1],json_obj['sheets'][s2] = json_obj['sheets'][s2],json_obj['sheets'][s1]
    mark_edited()
def swap_secs(s1,s2):
    global json_obj, needs_save, cursheet
    sheet = _getsheet(cursheet)
    sheet['tabs'][s1],sheet['tabs'][s2] = sheet['tabs'][s2],sheet['tabs'][s1]
    _setsheet(cursheet,sheet)
    mark_edited()
def swap_entry(s1,s2):
    global json_obj, needs_save, cursheet, cursec
    sheet = _getsheet(cursheet)
    sheet['tabs'][cursec]['lines'][s1],sheet['tabs'][cursec]['lines'][s2] = sheet['tabs'][cursec]['lines'][s2],sheet['tabs'][cursec]['lines'][s1]
    _setsheet(cursheet,sheet)
    mark_edited()
def sheetshift(listbox,shift):
    global cursheet
    ind = get_sel(listbox)
    ind2 = ind+shift
    if ind <= 0 or ind2 <= 0 or ind2 >= listbox.size():
        return
    swap_sheets(ind,ind2)
    cursheet = ind2
    switch(SheetsPage)
def secshift(listbox,shift):
    global cursec
    ind = get_sel(listbox)
    ind2 = ind+shift
    if ind < 0 or ind2 < 0 or ind2 >= listbox.size():
        return
    swap_secs(ind,ind2)
    cursec = ind2
    switch(EditShPage)
def entryshift(listbox,shift):
    global curentry
    ind = get_sel(listbox)
    ind2 = ind+shift
    if ind < 0 or ind2 < 0 or ind2 >= listbox.size():
        return
    swap_entry(ind,ind2)
    curentry = ind2
    switch(EditSecPage)

def _getsheet(ind):
    global json_obj
    return json_obj['sheets'][ind] if ind > -1 else json_obj['named']
def _setsheet(ind,sh):
    global json_obj
    if ind > -1:
        json_obj['sheets'][ind] = sh
    else:
        json_obj['named'] = sh
## Info
def info_sheets():
    pass
def info_editsheet():
    pass
def info_editsec():
    pass
def info_editentry():
    pass
## Pages
def switch(pageclass):
    global root, mainframe, curpage
    if mainframe is not None:
        mainframe.destroy()
    mainframe = pageclass(root)
    mainframe.grid()
    curpage = pageclass
    root.update()

def pack_scrollable_listbox(listbox):
    scroll = Scrollbar(listbox.master)
    listbox.config(yscrollcommand = scroll.set)
    scroll.config(command = listbox.yview)
    listbox.pack(side='left', fill=BOTH)
    scroll.pack(side='left', fill=BOTH)

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
        
        if cursheet < 0:
            cursheet = 0
        
        f1 = Frame(frame)
        Label(f1, text = 'Sheets').pack()
        sheetlistbox = Listbox(f1)
        for ind,sheet in enumerate(json_obj['sheets']):
            sheetlistbox.insert(ind,sheet['name'])
        sheetlistbox.selection_set(cursheet)
        pack_scrollable_listbox(sheetlistbox)
        f1.pack(side='left')
        
        f2 = Frame(frame)
        bor = 3
        Button(f2, bd=bor, text = '?', command=info_sheets).pack(anchor=W)
        wid = 5
        Button(f2, bd=bor, width=wid, text='↑', command=lambda:sheetshift(sheetlistbox,-1)).pack(anchor=W)
        Button(f2, bd=bor, width=wid, text='↓', command=lambda:sheetshift(sheetlistbox,1)).pack(anchor=W)
        wid = 10
        Button(f2, bd=bor, width=wid, text='Edit', command=lambda:edit_sheet(get_sel(sheetlistbox))).pack()
        Button(f2, bd=bor, width=wid, text='Add', command=lambda:add_sheet(get_sel(sheetlistbox))).pack()
        Button(f2, bd=bor, width=wid, text='Rename', command=lambda:ren_sheet(get_sel(sheetlistbox))).pack()
        Button(f2, bd=bor, width=wid, text='Named Data', command=edit_named).pack()
        f2.pack(side='left')

class EditShPage(Frame):
    def __init__(frame, root):
        global json_obj, seclistbox, cursheet, cursec
        Frame.__init__(frame,root)
        sheet = _getsheet(cursheet)
        
        f1 = Frame(frame)
        Label(f1, text = f"Sections in '{sheet['name']}'").pack()
        seclistbox = Listbox(f1)
        for ind,sec in enumerate(sheet['tabs']):
            seclistbox.insert(ind,sec['name'])
        seclistbox.selection_set(cursec)
        pack_scrollable_listbox(seclistbox)
        f1.pack(side='left')
        
        f2 = Frame(frame)
        bor = 3
        Button(f2, bd=bor, text = '?', command=info_editsheet).pack(anchor=W)
        wid = 5
        Button(f2, bd=bor, width=wid, text='↑', command=lambda:secshift(seclistbox,-1)).pack(anchor=W)
        Button(f2, bd=bor, width=wid, text='↓', command=lambda:secshift(seclistbox,1)).pack(anchor=W)
        wid = 10
        Button(f2, bd=bor, width=wid, text='Edit', command=lambda:edit_sec(get_sel(seclistbox))).pack()
        Button(f2, bd=bor, width=wid, text='Add', command=lambda:add_sec(get_sel(seclistbox))).pack()
        Button(f2, bd=bor, width=wid, text='Rename', command=lambda:ren_sec(get_sel(seclistbox))).pack()
        Button(f2, bd=bor, width=wid, text='Back', command=lambda:switch(SheetsPage)).pack()
        f2.pack(side='left')
class EditSecPage(Frame):
    def __init__(frame, root):
        global json_obj, entrylistbox, cursheet, cursec, curentry
        Frame.__init__(frame,root)
        sheet = _getsheet(cursheet)
        
        f1 = Frame(frame)
        Label(f1, text = f"Entries in '{sheet['tabs'][cursec]['name']}'").pack()
        entrylistbox = Listbox(f1)
        for ind,line in enumerate(sheet['tabs'][cursec]['lines']):
            entrylistbox.insert(ind,lib.get_line_display(line))
        entrylistbox.selection_set(curentry)
        entrylistbox.pack()
        pack_scrollable_listbox(entrylistbox)
        f1.pack(side='left')
        
        f2 = Frame(frame)
        bor = 3
        Button(f2, bd=bor, text = '?', command=info_editsec).pack(anchor=W)
        wid = 5
        Button(f2, bd=bor, width=wid, text='↑', command=lambda:entryshift(entrylistbox,-1)).pack(anchor=W)
        Button(f2, bd=bor, width=wid, text='↓', command=lambda:entryshift(entrylistbox,1)).pack(anchor=W)
        wid = 10
        Button(f2, bd=bor, width=wid, text='Edit', command=lambda:edit_entry(get_sel(entrylistbox))).pack()
        Button(f2, bd=bor, width=wid, text='Add', command=lambda:add_entry(get_sel(entrylistbox))).pack()
        Button(f2, bd=bor, width=wid, text='Back', command=lambda:switch(EditShPage)).pack()
        f2.pack(side='left')
class EditEntryPage(Frame):
    def __init__(frame, root):
        global json_obj, cursheet, cursec, curentry
        Frame.__init__(frame,root)
        Label(frame, text = f"TODO Edit Entry").pack()
        Button(frame, text = 'Exit', command = lambda: switch(EditSecPage)).pack()

root = Tk()
update_file(args.inputfile) #Update title to include loaded file
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
optmenu = Menu(menubar, tearoff=0, postcommand=onoption)
optmenu.add_command(label = 'Autosave', command = opt_autosave)
menubar.add_cascade(label='Options', menu=optmenu)
root.config(menu=menubar)
root.protocol("WM_DELETE_WINDOW", quitter)
root.grid_rowconfigure(0, weight=1)
root.grid_columnconfigure(0, weight=1)


mainframe = None
if file_loaded:
    switch(SheetsPage)
else:
    switch(InfoPage)

#Start autosave timer
root.after(time_minute - timer() % time_minute, update_autosave)

root.mainloop()
