import docjson_lib as lib
import argparse
import os, time, json, re, sys
import webbrowser
from tkinter import *
from tkinter import filedialog
from tkinter import messagebox
from tkinter import simpledialog
from tkinter import ttk

# import traceback
# print(traceback.print_exc())

parser = argparse.ArgumentParser()
parser.add_argument('--in', dest='inputfile', default='zsdocs_main.json',metavar='FILE',
    help='.json to generate docs from')
parser.add_argument('--out', dest='outputfile', default='ZScript_Docs.html',metavar='FILE',
    help='HTML webpage output')
parser.add_argument('--db','-d', action='store_true', help='Print debug info')
parser.add_argument('--cli','-c', dest='edit', action='store_false', help='Just cli, don\'t open editor')
args = parser.parse_args()

lib.debug_out = args.db
if args.edit:
    #If opened by double-click, starts in system32??
    os.chdir(os.path.dirname(__file__))
cur_directory = ''
cur_file = ''
file_loaded = False
needs_save = False
needs_autosave = False
needs_edit_save = False
cursheet = 0
cursec = 0
curentry = 0
curjump = 0
root = None
mainframe = None
copiedentry = None
# For refreshing
refr_entry = None
#Colors
def theme(ind):
    global BGC,FGC,DIS_FGC,FLD_BGC,FLD_FGC,FLD_DIS_BGC,FLD_DIS_FGC,ACT_BGC,ACT_FLD_BGC
    if ind == 1: #dark
        BGC = '#283C55'
        ACT_BGC = '#40546D'
        FGC = '#D2E2FF'
        DIS_FGC = '#6C7088'
        FLD_BGC = '#1C2038'
        ACT_FLD_BGC = '#0078D7'
        FLD_FGC = '#699195'
        FLD_DIS_BGC = '#283C55'
        FLD_DIS_FGC = '#6C7088'
    else:
        BGC = '#DDD'
        ACT_BGC = '#BBB'
        FGC = '#000'
        DIS_FGC = '#BBB'
        FLD_BGC = '#FFF'
        ACT_FLD_BGC = '#DDF'
        FLD_FGC = '#000'
        FLD_DIS_BGC = '#F0F0F0'
        FLD_DIS_FGC = '#888'
    if mainframe:
        mainframe.refresh()
theme(0)
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
        update_root_title()
def update_root_title():
    global cur_file, needs_save, needs_edit_save
    _title = 'ZS Docs Editor'
    if cur_file:
        _saved = '*' if needs_save else ''
        _saved2 = '**' if needs_edit_save else _saved
        if navi.depth() == 4: #Entry editor
            _title += f' \'{cur_file}\' {_saved2}'
        else:
            _title += f' \'{cur_file}\' {_saved}'
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
    return lib._dict_get(config,key,defval)
def read_config():
    global config
    try:
        with open('docjson_config.json', 'r') as file:
            config = json.loads(file.read())
    except:
        config = {}
    _def_config('autosave_minutes',5)
    _def_config('theme',0)
    theme(config['theme'])
def write_config():
    global config
    try:
        with open('docjson_config.json', 'w') as file:
            file.write(json.dumps(config, indent=4))
    except Exception as e:
        print(e, file=sys.stderr)

config = None
read_config()

def openlink(url):
    webbrowser.open_new(url)
def openhtml(fpath):
    url = 'file://'+fpath
    openlink(url)
## GUI functions
def loader_json():
    global json_obj, root, needs_save, file_loaded, curpage, cursheet, needs_autosave, cur_file, cur_directory
    if not save_warn('Load file'):
        return
    fname = filedialog.askopenfilename(parent = root, title = 'Load', initialdir = cur_directory, initialfile = cur_file, filetypes = (('Json','*.json'),))
    if len(fname) < 1:
        return
    cursheet = 0
    file_loaded = False
    mark_saved()
    mark_autosaved()
    update_root_title()
    try:
        json_obj = lib.loadjson(fname)
        update_file(trim_auto(fname))
        file_loaded = True
        navi.clear()
    except Exception as e:
        json_obj = None
        update_file('')
        switch(InfoPage)
        popError(f"Error '{str(e)}' occurred loading input file:\n{fname}")
def saver_json():
    global json_obj, needs_save, file_loaded
    if not needs_save or not file_loaded:
        return
    if len(args.inputfile) < 1:
        saver_json_as()
        return
    try:
        lib.savejson(args.inputfile,json_obj)
        mark_saved()
    except:
        popError(f'Error occurred saving file:\n{args.inputfile}')
def saver_json_as():
    global json_obj, root, file_loaded, needs_save, cur_directory, cur_file
    if not file_loaded:
        return
    fname = filedialog.asksaveasfilename(parent = root, title = 'Save As', initialdir = cur_directory, initialfile = cur_file, filetypes = (('Json','*.json'),),defaultextension = '.json')
    if len(fname) < 1:
        return
    try:
        lib.savejson(fname,json_obj)
        update_file(trim_auto(fname))
        mark_saved()
    except:
        popError(f'Error occurred saving file:\n{fname}')
def saver_html(filename=None):
    global json_obj, root, file_loaded
    if not file_loaded:
        return
    _dir = os.path.dirname(args.outputfile)
    if filename:
        fname = filename
    else:
        fname = os.path.basename(args.outputfile)
        fname = filedialog.asksaveasfilename(parent = root, title = 'Export HTML', initialdir = _dir, initialfile = fname, filetypes = (('HTML','*.html'),),defaultextension = '.html')
    if len(fname) < 1:
        return
    try:
        lib.savehtml(fname,json_obj)
        args.outputfile = fname
    except Exception as e:
        popError(f"Error '{str(e)}'\nOccurred saving file:\n{fname}")
        return None
def preview_html():
    tout = args.outputfile
    saver_html('_preview.html')
    openhtml(os.getcwd()+'\\'+'_preview.html')
    args.outputfile = tout
def save_warn(_msg):
    global root, needs_save, needs_edit_save, warned
    warned = False
    if needs_edit_save:
        warned = True
        if not messagebox.askyesno(parent=root, title = f'{_msg} without saving?', message = 'Edits to this entry have not been saved!'):
            return False
    if needs_save:
        warned = True
        if not messagebox.askyesno(parent=root, title = f'{_msg} without saving?', message = 'Changes to the .json have not been saved!'):
            return False
    return True
def quitter(skipwarn=True):
    global root, warned
    if not save_warn('Exit'):
        return
    if not skipwarn and not warned:
        if not messagebox.askyesno(parent=root, title = 'Exit?', message = 'Would you like to exit?'):
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
    optmenu.entryconfig(0, label = f"Autosave ({asmin} min)" if asmin else "Autosave (off)")
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
def schedule_autosave(i=1):
    global root
    root.after(time_minute - timer() % time_minute, update_autosave, i)
def update_autosave(i=1):
    global config, needs_autosave
    if not needs_autosave or config['autosave_minutes'] == 0: #Reset, nothing to autosave
        schedule_autosave()
    elif i >= config['autosave_minutes']:
        fname = add_auto(args.inputfile)
        lib.savejson(fname,json_obj)
        mark_autosaved()
        #Reset the autosave timer
        schedule_autosave()
    else:
        # Wait another minute
        schedule_autosave(i+1)
def mark_edited():
    global needs_save, needs_autosave
    needs_autosave = True
    if needs_save:
        return
    needs_save = True
    update_root_title()
def mark_saved():
    global needs_save
    if not needs_save:
        return
    needs_save = False
    update_root_title()
def mark_autosaved():
    global needs_autosave
    if not needs_autosave:
        return
    needs_autosave = False
def local_edited(val:bool=True):
    global needs_edit_save, mainframe
    if needs_edit_save == val:
        return
    needs_edit_save = val
    update_root_title()
    if isinstance(mainframe, EditEntryPage):
        for b in mainframe.save_btns:
            disable_widg(b,not val)
def get_sel(listbox):
    sel = listbox.curselection()
    if len(sel) < 1:
        return -1
    return sel[0]

def del_conf():
    global root
    return messagebox.askokcancel(parent=root, title = 'Delete Confirmation', message = 'Delete current selection?')

def edit_named():
    navi.visit_new((-1,0))
def edit_sheet(ind):
    if ind < 0:
        return
    navi.visit_new((ind,0))
def add_sheet(ind):
    global json_obj
    if ind < 0:
        ind = 0
    name = 'New Sheet'
    sheets = json_obj['sheets']
    newsheet = [{'name':name,'tabs':[]}]
    if not len(json_obj['sheets']):
        newind = 0
        json_obj['sheets'] = newsheet
    else:
        newind = ind+1
        json_obj['sheets'] = sheets[:newind] + newsheet + sheets[newind:]
    mark_edited()
    mainframe.reload_sheets(newind)
def del_sheet(ind):
    global json_obj
    if not del_conf():
        return
    if ind < 0:
        return
    sheets = json_obj['sheets']
    json_obj['sheets'] = sheets[:ind] + sheets[ind+1:]
    mark_edited()
    mainframe.reload_sheets(ind-1)
def ren_sheet(ind):
    global json_obj, root
    if ind < 0:
        return
    
    name = simpledialog.askstring('Input', f"Rename '{json_obj['sheets'][ind]['name']}' to?", parent=root)
    if not name:
        return
    mark_edited()
    json_obj['sheets'][ind]['name'] = name
    mainframe.reload_sheets(ind)

def edit_sec(ind):
    global cursheet
    if ind < 0:
        return
    navi.visit_new((cursheet,ind,0))
def add_sec(ind):
    global json_obj, cursheet
    if ind < 0:
        ind = 0
    name = 'New Section'
    sheet = _getsheet(cursheet)
    newsec = [{'name':name,'lines':[]}]
    if not len(sheet['tabs']):
        newind = 0
        sheet['tabs'] = newsec
    else:
        newind = ind+1
        sheet['tabs'] = sheet['tabs'][:newind] + newsec + sheet['tabs'][newind:]
    _setsheet(cursheet,sheet)
    mark_edited()
    mainframe.reload_sections(newind)
def del_sec(ind):
    global json_obj, cursheet
    if not del_conf():
        return
    if ind < 0:
        return
    sheet = _getsheet(cursheet)
    sheet['tabs'] = sheet['tabs'][:ind] + sheet['tabs'][ind+1:]
    _setsheet(cursheet,sheet)
    mark_edited()
    mainframe.reload_sections(ind-1)
def ren_sec(ind):
    global json_obj, cursheet, root
    if ind < 0:
        return
    
    sheet = _getsheet(cursheet)
    name = simpledialog.askstring('Input', f"Rename '{sheet['tabs'][ind]['name']}' to?", parent=root)
    if not name:
        return
    mark_edited()
    sheet['tabs'][ind]['name'] = name
    _setsheet(cursheet,sheet)
    mainframe.reload_sections(ind)

def edit_entry(ind):
    global cursheet, cursec
    if ind < 0:
        return
    navi.visit_new((cursheet,cursec,ind,0))
def add_entry(ind):
    global json_obj, cursheet, cursec
    if ind < 0:
        ind = 0
    name = 'New Entry'
    sheet = _getsheet(cursheet)
    lines = sheet['tabs'][cursec]['lines']
    newl = [{'name':name,'val':''}]
    if not len(lines):
        newind = 0
        lines = newl
    else:
        newind = ind+1
        lines = lines[:newind] + newl + lines[newind:]
    sheet['tabs'][cursec]['lines'] = lines
    _setsheet(cursheet,sheet)
    mark_edited()
    mainframe.reload_entry(newind)
def del_entry(ind):
    global json_obj, cursheet, cursec
    if not del_conf():
        return
    if ind < 0:
        return
    sheet = _getsheet(cursheet)
    lines = sheet['tabs'][cursec]['lines']
    lines = lines[:ind] + lines[ind+1:]
    sheet['tabs'][cursec]['lines'] = lines
    _setsheet(cursheet,sheet)
    mark_edited()
    mainframe.reload_entry(ind-1)
def get_entry_link(ind):
    global json_obj, cursheet, cursec
    if ind < 0:
        return
    sheet = _getsheet(cursheet)
    try:
        entry = sheet['tabs'][cursec]['lines'][ind]
        val = entry['val']
        if not val or val[0] != '$':
            return
        _s,*_ = val[1:].split('$')
        sheetnum = -1
        try:
            sheetnum = int(_s)
            if sheetnum == -1:
                sheetnum = cursheet
        except:
            sheetnum = get_sheetind_named(_s)
        if sheetnum < 0:
            return
        secnum = 0
        if _:
            try:
                secnum = int(_[0])
            except:
                secnum = 0
        return (sheetnum,secnum)
    except:
        return
def link_entry(ind):
    global cursheet, cursec
    val = get_entry_link(ind)
    if val:
        navi.visit_new(val+(0,))
        return True
    return False

def edit_jump(ind):
    global mainframe
    if ind < 0 or ind >= len(mainframe.list_jumps):
        return
    string = simpledialog.askstring('Input', f"Change '{mainframe.list_jumps[ind]}' to?", parent=root)
    if not string:
        return
    string = string.strip()
    if not string:
        return
    mainframe.list_jumps[ind] = string
    local_edited()
    mainframe.reload_jump(ind)
def add_jump(ind):
    global mainframe
    string = simpledialog.askstring('Input', f"New jump string?", parent=root)
    if not string:
        return
    string = string.strip()
    if not string:
        return
    mainframe.list_jumps = mainframe.list_jumps[:ind+1] + [string] + mainframe.list_jumps[ind+1:]
    local_edited()
    mainframe.reload_jump(ind+1)
def del_jump(ind):
    global mainframe
    if not del_conf():
        return
    if ind < 0:
        return
    mainframe.list_jumps = mainframe.list_jumps[:ind] + mainframe.list_jumps[ind+1:]
    local_edited()
    mainframe.reload_jump(ind-1)

def swap_sheets(s1,s2):
    global json_obj
    json_obj['sheets'][s1],json_obj['sheets'][s2] = json_obj['sheets'][s2],json_obj['sheets'][s1]
    mark_edited()
def swap_secs(s1,s2):
    global json_obj, cursheet
    sheet = _getsheet(cursheet)
    sheet['tabs'][s1],sheet['tabs'][s2] = sheet['tabs'][s2],sheet['tabs'][s1]
    _setsheet(cursheet,sheet)
    mark_edited()
def swap_entry(s1,s2):
    global json_obj, cursheet, cursec
    sheet = _getsheet(cursheet)
    sheet['tabs'][cursec]['lines'][s1],sheet['tabs'][cursec]['lines'][s2] = sheet['tabs'][cursec]['lines'][s2],sheet['tabs'][cursec]['lines'][s1]
    _setsheet(cursheet,sheet)
    mark_edited()
def swap_jump(s1,s2):
    global mainframe
    mainframe.list_jumps[s1],mainframe.list_jumps[s2] = mainframe.list_jumps[s2],mainframe.list_jumps[s1]
    local_edited()
def sheetshift(shift):
    global mainframe, cursheet, sheetlistbox
    ind = cursheet
    ind2 = ind+shift
    if ind <= 0 or ind2 <= 0 or ind2 >= sheetlistbox.size():
        return
    swap_sheets(ind,ind2)
    mainframe.reload_sheets(ind2)
def secshift(shift):
    global mainframe, cursec, seclistbox
    ind = cursec
    ind2 = ind+shift
    if ind < 0 or ind2 < 0 or ind2 >= seclistbox.size():
        return
    swap_secs(ind,ind2)
    mainframe.reload_sections(ind2)
def entryshift(shift):
    global mainframe, curentry, entrylistbox
    ind = curentry
    ind2 = ind+shift
    if ind < 0 or ind2 < 0 or ind2 >= entrylistbox.size():
        return
    swap_entry(ind,ind2)
    mainframe.reload_entry(ind2)
def jumpshift(shift):
    global mainframe, curjump, jumplistbox
    ind = curjump
    ind2 = ind+shift
    if ind < 0 or ind2 < 0 or ind2 >= jumplistbox.size():
        return
    swap_jump(ind,ind2)
    mainframe.reload_jump(ind2)

def sel_sheet(evt):
    global cursheet
    cursheet = get_sel(evt.widget)
def sel_sec(evt):
    global cursec
    cursec = get_sel(evt.widget)
def sel_entry(evt):
    global curentry, mainframe
    curentry = get_sel(evt.widget)
    mainframe.reload_entry_link()
def sel_jump(evt):
    global curjump
    curjump = get_sel(evt.widget)

def _getsheet(ind):
    global json_obj
    return json_obj['sheets'][ind] if ind > -1 else json_obj['named']
def _setsheet(ind,sh):
    global json_obj
    if ind > -1:
        json_obj['sheets'][ind] = sh
    else:
        json_obj['named'] = sh

def get_sheetind_named(name,default=-1):
    global json_obj
    for ind,sheet in enumerate(json_obj['sheets']):
        if sheet['name'] == name:
            return ind
    return default

def get_sheetnames():
    global json_obj
    return tuple(sheet['name'] for sheet in json_obj['sheets'])
def get_sheetnums():
    global json_obj
    return tuple(x for x in range(len(json_obj['sheets'])))
def get_secnums(sheetind):
    global json_obj, cursheet
    if sheetind < 0:
        sheetind = cursheet
    return tuple(x for x in range(len(json_obj['sheets'][sheetind]['tabs'])))

def toggle_todo(d:dict):
    global mainframe
    if lib._dict_get(d,'todo',False):
        lib._dict_del(d,'todo')
    else:
        d['todo'] = True
    mainframe.reload_lists()
    mark_edited()
## Info
def info_program():
    messagebox.showinfo('Info','''Working files are saved in '.json' format. Saving to HTML exports a preview of the docs with your changes, but does not actually save your changes.

Keybinds:
F1 - Display this help message
Enter - Confirm a page.
Esc - Exit up, to the containing page. If in the root page, attempts to quit the program (with confirmation)
Alt+Left Arrow - Same effect as clicking the <- button
Alt+Right Arrow - Same effect as clicking the -> button
Ctrl+S - Same effect as 'File->Save'. Note that this will save the json, but NOT current entry changes.
Ctrl+L or Ctrl+O - Same effect as 'File->Load'

Listboxes:
Double-Click acts the same as Enter, confirming the page. In most cases, this will 'Edit' the clicked object.
In the Section editor, double-clicking a [LINK] entry will follow the link instead.
Right-Click will 'Edit' any entry in the entry list, or will 'Rename' things in other lists.
''')
def info_sheets():
    messagebox.showinfo('Info','''Double-clicked sheets will be edited.
Pressing 'Enter' has the same effect as double-clicking the current selection.
Right-clicked sheets will be renamed.
Sheet names are used to link to sections in sheets.
The top sheet cannot be re-ordered, it contains the root of the document as its' first entry.''')
def info_editsheet():
    messagebox.showinfo('Info','''Double-clicked sections will be edited.
Pressing 'Enter' has the same effect as double-clicking the current selection.
Right-clicked sections will be renamed.
Section names are not displayed in the html, and have no meaning except in Named_Data (where they are used for search priority)''')
def info_editsec():
    messagebox.showinfo('Info','''Entries named exactly '--' will be hidden from the list.
The first Page type Entry will be displayed by default for the section, even if named '--'.
Making a '--' first entry can work if there are no other pages to display a body alongside a section full of links.

A double-clicked entry is edited unless it is a link, in which case it is followed.
Pressing 'Enter' has the same effect as double-clicking the current selection.
Right-clicking an entry will edit it, even if it is a link.''')
def info_editentry():
    messagebox.showinfo('Info','''Entry Types:
Empty- An empty entry placeholder.
Link- A link to another section.
Page- An html page to display content.

If an entry has no jumps, it will use its' name, split at '/' characters, as jumps.
Double-clicking or right-clicking a jump will edit it.''')
## Pages
def switch(pageclass):
    global root, mainframe, curpage
    if mainframe is not None:
        mainframe.destroy()
    mainframe = pageclass(root)
    mainframe._postinit()
    mainframe.grid()
    curpage = pageclass
    root.update()

def hover_scroll(scr,hv):
    if hv and not DISABLED in scr.state():
        scr.config(cursor='double_arrow',style='Hov.Vertical.TScrollbar')
    else:
        scr.config(cursor='arrow',style='TScrollbar')
def pack_scrollable_widg(widg):
    scroll = ttk.Scrollbar(widg.master, cursor='double_arrow')
    scroll.bind('<Enter>',lambda _: hover_scroll(scroll,True))
    scroll.bind('<Leave>',lambda _: hover_scroll(scroll,False))
    widg.config(yscrollcommand = scroll.set)
    scroll.config(command = widg.yview)
    widg.pack(side='left', fill=X, expand=True)
    scroll.pack(side='left', fill=Y)
    
def pack_scrollable_listbox(listbox):
    pack_scrollable_widg(listbox)
def pack_scrollable_text(txt):
    pack_scrollable_widg(txt)
def _set_field(field,text):
    field.delete(0,END)
    field.insert(0,text)
def _load_list(listbox, selind, _list, loadfunc):
    listbox.delete(0,END)
    for ind,val in enumerate(_list):
        listbox.insert(ind,loadfunc(val))
    if selind < 0:
        selind = 0
    if selind >= len(_list):
        selind = len(_list)-1
    listbox.selection_set(selind)
    listbox.activate(selind)
    listbox.see(selind)

def sel_clicked_lb(evt):
    sel = evt.widget.nearest(evt.y)
    
    try:
        _, yoffset, _, height = evt.widget.bbox(sel)
    except:
        return -1 #error
    if evt.y > height + yoffset + 5:
        return -1 #not on widget
    
    evt.widget.selection_clear(0,END)
    evt.widget.selection_set(sel)
    evt.widget.activate(sel)
    evt.widget.event_generate('<<ListboxSelect>>',when='tail')
    return sel
def disable_widg(widg,dis):
    if dis:
        widg.config(state=DISABLED)
    else:
        widg.config(state=NORMAL)
    if issubclass(type(widg),Button):
        disable_btn(widg,dis)
    elif issubclass(type(widg),Text):
        disable_txt(widg,dis)
    elif issubclass(type(widg),ttk.Combobox):
        if not dis:
            widg.config(state='readonly')
def disable_widgs(widglist,dis):
    for w in widglist:
        disable_widg(w,dis)
def style_label(label):
    label.config(bg=BGC, fg=FGC, disabledforeground=DIS_FGC)
def style_lb(lb,rclick_cb=None):
    lb.config(bg=FLD_BGC, fg=FLD_FGC, disabledforeground=FLD_DIS_FGC)
    lb.bind('<B1-Motion>', sel_clicked_lb)
    if not rclick_cb:
        rclick_cb = sel_clicked_lb
    lb.bind('<B3-Motion>', sel_clicked_lb)
    lb.bind('<Button-3>', sel_clicked_lb)
    lb.bind('<ButtonRelease-3>', rclick_cb)
def style_cb(cb):
    cb.bind('<Enter>',func=lambda _: hover_cb(cb,True))
    cb.bind('<Leave>',func=lambda _: hover_cb(cb,False))
    pass
def hover_cb(cb,hv):
    if hv and not DISABLED in cb.state():
        cb.config(style='Hov.TCombobox')
    else:
        cb.config(style='TCombobox')
def style_rad(rad):
    pass#rad.config(bg=BGC,activebackground=BGC,fg=FGC,fieldbackground=FLD_BGC,disabledforeground=DIS_FGC)
def style_entry(ent):
    ent.config(bg=FLD_BGC, fg=FLD_FGC, disabledforeground=FLD_DIS_FGC, insertbackground=FLD_FGC)
def style_btn(btn):
    btn.config(bd=2,bg=BGC,fg=FGC,disabledforeground=DIS_FGC,
        activebackground=BGC,activeforeground=FGC,
        highlightcolor=FGC)
    btn.bind('<Enter>',func=lambda _: btn.config(background=BGC if btn['state']==DISABLED else ACT_BGC,activebackground=BGC if btn['state']==DISABLED else ACT_BGC))
    btn.bind('<Leave>',func=lambda _: btn.config(background=BGC,activebackground=BGC))
    disable_btn(btn,False)
def disable_btn(btn,dis):
    if dis:
        btn.config(relief=GROOVE)
    else:
        btn.config(relief=RAISED)
def style_txt(txt):
    txt.config(relief=SUNKEN,wrap='char')
    disable_txt(txt,False)
def disable_txt(txt,dis):
    if dis:
        txt.config(bg = FLD_DIS_BGC, fg = FLD_DIS_FGC, insertbackground=FLD_DIS_BGC)
    else:
        txt.config(bg = FLD_BGC, fg = FLD_FGC, insertbackground=FLD_FGC)

def stylize():
    global style
    root.option_clear()
    style.theme_use('clam')
    # ttk.Combobox
    style.configure('TCombobox',fieldbackground=FLD_BGC,background=BGC,foreground=FGC,
        selectbackground=FLD_BGC,selectforeground=FGC,
        lightcolor=BGC,bordercolor=FGC,borderwidth=1,
        darkcolor=DIS_FGC,
        arrowsize=16)
    style.map('TCombobox',
        fieldbackground=[('disabled',FLD_DIS_BGC),('readonly',FLD_BGC)],
        background=[('disabled',BGC),('readonly',BGC)],
        foreground=[('disabled',DIS_FGC),('readonly',FGC)],
        arrowcolor=[('disabled',DIS_FGC),('readonly',FGC)],
        bordercolor=[('disabled',DIS_FGC)])
    style.configure('Hov.TCombobox',background=ACT_BGC, fieldbackground=ACT_FLD_BGC,
        lightcolor=ACT_FLD_BGC,selectbackground=ACT_FLD_BGC)
    style.map('Hov.TCombobox',
        fieldbackground=[('disabled',FLD_DIS_BGC),('readonly',ACT_FLD_BGC)],
        background=[('disabled',BGC),('readonly',ACT_BGC)],
        selectbackground=[('disabled',FLD_DIS_BGC),('readonly',ACT_FLD_BGC)])
    root.option_add('*TCombobox*Listbox*Background',FLD_BGC)
    root.option_add('*TCombobox*Listbox*Foreground',FLD_FGC)
    # print(style.lookup('Vertical.Scrollbar.thumb','lightcolor'))
    
    # ttk.Radiobutton
    style.configure('TRadiobutton',padding=1,background=BGC,
        foreground=FGC,indicatorbackground=FLD_BGC,indicatorforeground=FLD_FGC,
        indicatorrelief=RAISED,indicatorsize=12,focuscolor=FGC)
    style.map('TRadiobutton',
        background=[('disabled',BGC),('pressed',BGC),('active',ACT_BGC)],
        foreground=[('disabled',DIS_FGC),('pressed',FGC),('active',FGC)],
        indicatorbackground=[('disabled',FLD_DIS_BGC)],
        indicatorforeground=[('disabled',FLD_DIS_FGC)],
        indicatorrelief=[('disabled',FLAT)])
    
    # ttk.Scrollbar
    # print(style.layout('Vertical.TScrollbar'))
    # print(style.element_options('Vertical.Scrollbar.thumb'))
    # print(style.lookup('Vertical.Scrollbar.thumb','lightcolor'))
    style.configure('TScrollbar',foreground=FGC,background=BGC,
        troughcolor=FLD_BGC,relief=RAISED,borderwidth=0,
        lightcolor=FGC,darkcolor=BGC,bordercolor=FLD_BGC,
        gripcount=5)
    style.map('TScrollbar',
        background=[('disabled',FLD_DIS_BGC)],
        troughcolor=[('disabled',BGC)],
        lightcolor=[('disabled',DIS_FGC)],
        darkcolor=[('disabled',DIS_FGC)],
        arrowcolor=[('disabled',DIS_FGC)],
        relief=[('disabled',FLAT)],
        gripcount=[('disabled',0)])
    style.configure('Hov.Vertical.TScrollbar',background=ACT_BGC,troughcolor=BGC)
    style.map('Hov.Vertical.TScrollbar',
        background=[('disabled',FLD_BGC)])

class Page(Frame):
    def __init__(self,root):
        Frame.__init__(self,root)
    def style(self):
        self.config(bg=BGC)
    def _postinit(self):
        self.style()
        self.postinit()
        navi.update()
    def postinit(self):
        pass
    def refresh(self):
        global root
        root.config(bg=BGC)
        stylize()
        self.reload()
    def reload(self):
        switch(type(self))
    def confirm(self):
        pass
    def reload_lists(self):
        pass
    def copy(self):
        pass
    def paste(self):
        pass

class InfoPage(Page):
    def __init__(self, root):
        Page.__init__(self,root)
        lb=Label(self, text = '''This GUI allows editing the web documentation.
        Docs are stored in .json format, which can be saved/loaded in the 'File' menu.
        You can also export the generated .html file.
        
        You must load a .json file to continue.''')
        style_label(lb)
        lb.pack()
    def postinit(self):
        pass

def sheet_disp_name(sheet)->str:
    name = sheet['name']
    if lib._dict_get(sheet,'todo',False):
        name = f'[TODO] {name}'
    return name
def sh_rclick(evt):
    sel = sel_clicked_lb(evt) #Selects the clicked entry
    if sel > -1: #could be off of elements
        ren_sheet(sel)
class SheetsPage(Page):
    def __init__(self, root):
        global json_obj, sheetlistbox, cursheet
        Page.__init__(self,root)
        
        if cursheet < 0:
            cursheet = 0
        
        f1 = Frame(self, bg=BGC)
        f2 = Frame(f1, bg=BGC)
        wid = 5
        self.backbtn = Button(f2, width=wid, text='←', command=lambda:navi.back())
        self.backbtn.pack(side='left')
        self.fwdbtn = Button(f2, width=wid, text='→', command=lambda:navi.fwd())
        self.fwdbtn.pack(side='left')
        style_btn(self.backbtn)
        style_btn(self.fwdbtn)
        f2.pack()
        lb=Label(f1, text = 'Sheets')
        style_label(lb)
        lb.pack()
        f2 = Frame(f1, bg=BGC)
        sheetlistbox = Listbox(f2)
        style_lb(sheetlistbox, sh_rclick)
        sheetlistbox.bind('<<ListboxSelect>>', sel_sheet)
        sheetlistbox.bind('<Double-1>', lambda _:self.confirm())
        self.reload_sheets(cursheet)
        pack_scrollable_listbox(sheetlistbox)
        sheetlistbox.focus_set()
        f2.pack(fill=X,expand=True)
        f_space = Frame(f1, bg=BGC, width=250)
        f_space.pack()
        f1.pack(side='left')
        
        f2 = Frame(self, bg=BGC)
        _btn=Button(f2, text = '?', command=info_sheets)
        style_btn(_btn)
        _btn.pack(anchor=W)
        wid = 5
        _btn=Button(f2, width=wid, text='↑', command=lambda:sheetshift(-1))
        style_btn(_btn)
        _btn.pack(anchor=W)
        _btn=Button(f2, width=wid, text='↓', command=lambda:sheetshift(1))
        style_btn(_btn)
        _btn.pack(anchor=W)
        wid = 10
        _btn=Button(f2, width=wid, text='Edit', command=lambda:edit_sheet(cursheet))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Mark Todo', command=lambda:toggle_todo(json_obj['sheets'][cursheet]))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Add', command=lambda:add_sheet(cursheet))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Delete', command=lambda:del_sheet(cursheet))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Rename', command=lambda:ren_sheet(cursheet))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Named Data', command=edit_named)
        style_btn(_btn)
        _btn.pack()
        f2.pack(side='left')
    def reload_sheets(self,selind):
        global json_obj, cursheet, sheetlistbox
        cursheet = selind
        _load_list(sheetlistbox, selind, json_obj['sheets'], sheet_disp_name)
    def postinit(self):
        pass
    def reload(self):
        navi.refresh()
    def confirm(self):
        global cursheet
        edit_sheet(cursheet)
    def reload_lists(self):
        global cursheet
        self.reload_sheets(cursheet)

def sec_disp_name(sec)->str:
    name = sec['name']
    if lib._dict_get(sec,'todo',False):
        name = f'[TODO] {name}'
    return name
def sec_rclick(evt):
    sel = sel_clicked_lb(evt) #Selects the clicked entry
    if sel > -1: #could be off of elements
        ren_sec(sel)
class EditShPage(Page):
    def __init__(self, root):
        global json_obj, seclistbox, cursheet, cursec
        Page.__init__(self,root)
        sheet = _getsheet(cursheet)
        
        f1 = Frame(self, bg=BGC)
        f2 = Frame(f1, bg=BGC)
        wid = 5
        self.backbtn = Button(f2, width=wid, text='←', command=lambda:navi.back())
        self.backbtn.pack(side='left')
        self.fwdbtn = Button(f2, width=wid, text='→', command=lambda:navi.fwd())
        self.fwdbtn.pack(side='left')
        style_btn(self.backbtn)
        style_btn(self.fwdbtn)
        f2.pack()
        lb=Label(f1, text = f"Sections in '{sheet['name']}'")
        style_label(lb)
        lb.pack()
        f2 = Frame(f1, bg=BGC)
        seclistbox = Listbox(f2)
        style_lb(seclistbox, sec_rclick)
        seclistbox.bind('<<ListboxSelect>>', sel_sec)
        seclistbox.bind('<Double-1>', lambda _:self.confirm())
        self.reload_sections(cursec)
        pack_scrollable_listbox(seclistbox)
        seclistbox.focus_set()
        f2.pack(fill=X,expand=True)
        f_space = Frame(f1, bg=BGC, width=250)
        f_space.pack()
        f1.pack(side='left')
        
        f2 = Frame(self, bg=BGC)
        _btn=Button(f2, text = '?', command=info_editsheet)
        style_btn(_btn)
        _btn.pack(anchor=W)
        wid = 5
        _btn=Button(f2, width=wid, text='↑', command=lambda:secshift(-1))
        style_btn(_btn)
        _btn.pack(anchor=W)
        _btn=Button(f2, width=wid, text='↓', command=lambda:secshift(1))
        style_btn(_btn)
        _btn.pack(anchor=W)
        wid = 10
        _btn=Button(f2, width=wid, text='Edit', command=lambda:edit_sec(cursec))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Mark Todo', command=lambda:toggle_todo(json_obj['sheets'][cursheet]['tabs'][cursec]))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Add', command=lambda:add_sec(cursec))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Delete', command=lambda:del_sec(cursec))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Rename', command=lambda:ren_sec(cursec))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Exit (Up)', command=lambda:navi.up())
        style_btn(_btn)
        _btn.pack()
        f2.pack(side='left')
    def reload_sections(self,selind):
        global cursheet, cursec, seclistbox
        cursec = selind
        sheet = _getsheet(cursheet)
        _load_list(seclistbox, selind, sheet['tabs'], sec_disp_name)
    def postinit(self):
        pass
    def reload(self):
        navi.refresh()
    def confirm(self):
        global cursec
        edit_sec(cursec)
    def reload_lists(self):
        global cursec
        self.reload_sections(cursec)

def entry_disp_name(entry)->str:
    name = lib.get_line_display(entry)
    pref = ''
    if lib._dict_get(entry,'todo',False):
        pref = '[TODO]'
    try:
        if entry['val'][0] == '$':
            pref += '[LINK]'
        else:
            pref += '[PAGE]'
    except:
        pref += '[NULL]'
    if pref:
        return f'{pref} {name}'
    return name
def nil_entry(entry)->bool:
    try:
        if entry['val'] != '':
            return False
        if entry['name'] != '' and entry['name'] != 'New Entry':
            return False
        if lib._dict_get(entry,'todo',False):
            return False
    except:
        pass
    return True
def copy_entry():
    global cursheet, cursec, curentry, copiedentry
    try:
        copiedentry = json_obj['sheets'][cursheet]['tabs'][cursec]['lines'][curentry]
    except:
        copiedentry = None
    try:
        disable_widg(mainframe.pastebtn,not copiedentry)
    except:
        pass
def paste_entry():
    global cursheet, cursec, curentry, copiedentry
    if not copiedentry:
        return
    e = json_obj['sheets'][cursheet]['tabs'][cursec]['lines'][curentry]
    if e == copiedentry: #Identical object
        return
    if not nil_entry(e):
        name = entry_disp_name(e)
        cname = entry_disp_name(copiedentry)
        if not messagebox.askyesno(parent=root, title = f'Paste over {name}?', message = f"The entry '{name}' will be replaced with the copied entry '{cname}'."):
            return
    json_obj['sheets'][cursheet]['tabs'][cursec]['lines'][curentry] = copiedentry
    mainframe.reload_lists()
    mark_edited()
def entry_rclick(evt):
    sel = sel_clicked_lb(evt) #Selects the clicked entry
    if sel > -1: #could be off of elements
        edit_entry(sel)
class EditSecPage(Page):
    def __init__(self, root):
        global json_obj, entrylistbox, cursheet, cursec, curentry, copiedentry
        Page.__init__(self,root)
        sheet = _getsheet(cursheet)
        
        shname = sheet['name']
        secname = sheet['tabs'][cursec]['name']
        
        f1 = Frame(self, bg=BGC)
        f2 = Frame(f1, bg=BGC)
        wid = 5
        self.backbtn = Button(f2, width=wid, text='←', command=lambda:navi.back())
        self.backbtn.pack(side='left')
        self.fwdbtn = Button(f2, width=wid, text='→', command=lambda:navi.fwd())
        self.fwdbtn.pack(side='left')
        style_btn(self.backbtn)
        style_btn(self.fwdbtn)
        f2.pack()
        lb=Label(f1, text = f"Entries in '{shname}->{secname}'")
        style_label(lb)
        lb.pack()
        f2 = Frame(f1, bg=BGC)
        entrylistbox = Listbox(f2)
        style_lb(entrylistbox, entry_rclick)
        entrylistbox.bind('<<ListboxSelect>>', sel_entry)
        entrylistbox.bind('<Double-1>', lambda _:self.confirm())
        pack_scrollable_listbox(entrylistbox)
        entrylistbox.focus_set()
        f2.pack(fill=X,expand=True)
        f_space = Frame(f1, bg=BGC, width=250)
        f_space.pack()
        f1.pack(side='left')
        
        gr = Frame(self, bg=BGC)
        f2 = Frame(gr, bg=BGC)
        _btn=Button(f2, text = '?', command=info_editsec)
        style_btn(_btn)
        _btn.pack(anchor=W)
        wid = 5
        _btn=Button(f2, width=wid, text='↑', command=lambda:entryshift(-1))
        style_btn(_btn)
        _btn.pack(anchor=W)
        _btn=Button(f2, width=wid, text='↓', command=lambda:entryshift(1))
        style_btn(_btn)
        _btn.pack(anchor=W)
        f2.grid(row=0,column=0,sticky=SW)
        f2 = Frame(gr, bg=BGC)
        wid = 10
        _btn=Button(f2, width=wid, text='Edit', command=lambda:edit_entry(curentry))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Mark Todo', command=lambda:toggle_todo(json_obj['sheets'][cursheet]['tabs'][cursec]['lines'][curentry]))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Add', command=lambda:add_entry(curentry))
        style_btn(_btn)
        _btn.pack()
        _btn=Button(f2, width=wid, text='Delete', command=lambda:del_entry(curentry))
        style_btn(_btn)
        _btn.pack()
        f2.grid(row=1,column=0,sticky=NW)
        f2 = Frame(gr, bg=BGC)
        _btn=Button(f2, width=wid, text='Copy', command=lambda:copy_entry())
        style_btn(_btn)
        _btn.pack()
        self.pastebtn=Button(f2, width=wid, text='Paste', command=lambda:paste_entry())
        style_btn(self.pastebtn)
        disable_widg(self.pastebtn,not copiedentry)
        self.pastebtn.pack()
        self.linkbtn = Button(f2, width=wid, text='Follow Link', command=lambda:link_entry(curentry))
        style_btn(self.linkbtn)
        self.linkbtn.pack()
        _btn=Button(f2, width=wid, text='Exit (Up)', command=lambda:navi.up())
        style_btn(_btn)
        _btn.pack()
        f2.grid(row=1,column=1,sticky=NW)
        gr.pack(side='left')
    def reload_entry_link(self):
        global curentry
        link = get_entry_link(curentry)
        disable_widg(self.linkbtn,not link)
    def reload_entry(self,selind):
        global cursheet, curentry, cursec, entrylistbox
        curentry = selind
        self.reload_entry_link()
        sheet = _getsheet(cursheet)
        _load_list(entrylistbox, selind, sheet['tabs'][cursec]['lines'], entry_disp_name)
    def postinit(self):
        self.reload_entry(curentry)
    def reload(self):
        navi.refresh()
    def confirm(self):
        global curentry
        if link_entry(curentry):
            return #Followed a valid link
        #No link
        edit_entry(curentry)
    def reload_lists(self):
        global curentry
        self.reload_entry(curentry)
    def copy(self):
        copy_entry()
    def paste(self):
        paste_entry()

def get_ttip(hld=None):
    return ('[[',hld if hld else 'TITLE','|TIP TEXT]]')
def get_named(hld=None):
    return ('$[',hld if hld else 'DATA_NAME',']')
def get_spoil(hld=None):
    return ('#{TITLE|',hld if hld else 'HIDDEN TEXT','}')
def get_link(hld=None):
    return ('${KEY|',hld if hld else 'DISPLAY TEXT','}')
def get_code1(hld=None):
    return ('`',hld if hld else 'CODE','`')
def get_code2(hld=None):
    return ('```',hld if hld else 'CODE','```')
def get_block(hld=None):
    return ('<block>',hld if hld else 'TEXT','</block>')
def get_iblock(hld=None):
    return ('<iblock>',hld if hld else 'TEXT','</iblock>')
def get_todo(hld=None):
    return ('<todo>',hld if hld else 'TEXT','</todo>')
def get_header(hld=None):
    return ('<h3>',hld if hld else 'HEADER TEXT','</h3>')
def get_bold(hld=None):
    return ('<strong>',hld if hld else 'TEXT','</strong>')
def get_italic(hld=None):
    return ('<em>',hld if hld else 'TEXT','</em>')

def txt_insert(txt,getter):
    sel = None
    has_sel = bool(txt.tag_ranges(SEL))
    if has_sel:
        sel = txt.get(SEL_FIRST, SEL_LAST)
    vls = getter(sel)
    if not vls: #Cancelled?
        return;
    s = vls[0]+vls[1]+vls[2]
    
    if has_sel:
        tmp = txt.count('1.0',SEL_FIRST)
        txt.delete(SEL_FIRST, SEL_LAST)
    else:
        tmp = txt.count('1.0',INSERT)
    first = tmp[0] if tmp else 0
    first += len(vls[0])
    last = first + len(vls[1])
    txt.insert(INSERT, s)
    txt.tag_add('sel',f'1.0+{first}c',f'1.0+{last}c')
    txt.mark_set(INSERT,f'1.0+{last}c')


def jump_rclick(evt):
    sel = sel_clicked_lb(evt) #Selects the clicked entry
    if sel > -1: #could be off of elements
        edit_jump(sel)
class EditEntryPage(Page):
    def __init__(self, root):
        global json_obj, cursheet, cursec, curentry, jumplistbox, curjump, refr_entry
        Page.__init__(self,root)
        sheet = _getsheet(cursheet)
        entry = refr_entry if refr_entry else sheet['tabs'][cursec]['lines'][curentry]
        try:
            self.todo = entry['todo']
        except:
            self.todo = False
        _spl = entry['name'].split(';;',2)
        name = _spl[0].strip()
        jumps = []
        if len(_spl) > 1:
            jumps = [s.strip() for s in re.split('/|;;',_spl[1])]
        
        val = entry['val']
        is_link = val and val[0] == '$'
        is_empty = not val
        typestr = 'Page'
        if is_empty:
            typestr = 'Empty'
        elif is_link:
            typestr = 'Link'
        linkrad = 0
        linkname = ''
        linknum = 0
        self.secnum = 0
        if is_link:
            _s,*_ = val[1:].split('$')
            try:
                linknum = int(_s)
                linkrad = 2 if linknum == -1 else 1
            except:
                linkname = _s
            if _:
                try:
                    self.secnum = int(_[0])
                except:
                    self.secnum = 0
        if linkrad == 2: #current sheet
            linknum = cursheet
            linkname = sheet['name']
        elif linkrad == 1: #numbered
            linkname = _getsheet(linknum)['name']
        else: #named
            linknum = get_sheetind_named(linkname)
        
        pageval = '' if is_link else val
        
        sheetnames = get_sheetnames()
        sheetnums = get_sheetnums()
        
        if linkname not in sheetnames:
            linkname = sheetnames[0]
        if linknum not in sheetnums:
            linknum = 0
        
        toprow = Frame(self, bg=BGC)
        col1 = Frame(toprow, bg=BGC)
        # This entry's 'Name'
        lb=Label(col1, text = 'Name:')
        style_label(lb)
        lb.grid(row=0,column=0,sticky=E)
        self.field_name = StringVar(self, name)
        self.field_name.trace('w', lambda *_:local_edited())
        ent=Entry(col1, textvariable=self.field_name)
        style_entry(ent)
        ent.grid(row=0,column=1,sticky=W)
        # The jump search labels
        lb=Label(col1, text = 'Jumps:')
        style_label(lb)
        lb.grid(row=1,column=0,sticky=NE)
        self.list_jumps = jumps
        
        fr = Frame(col1, bg=BGC)
        f2 = Frame(fr, bg=BGC, width=20)
        jumplistbox = Listbox(f2)
        style_lb(jumplistbox, jump_rclick)
        self.reload_jump(curjump)
        jumplistbox.bind('<<ListboxSelect>>', sel_jump)
        jumplistbox.bind('<Double-1>', jump_rclick)
        pack_scrollable_listbox(jumplistbox)
        f2.pack(fill=X,expand=True)
        fr.grid(row=1,column=1,sticky=NW)
        
        fr = Frame(col1, bg=BGC)
        wid = 6
        _btn=Button(fr, width=wid, text = '?', command=info_editentry)
        style_btn(_btn)
        _btn.pack(anchor=W)
        _btn=Button(fr, width=wid, text='↑', command=lambda:jumpshift(-1))
        style_btn(_btn)
        _btn.pack(anchor=W)
        _btn=Button(fr, width=wid, text='↓', command=lambda:jumpshift(1))
        style_btn(_btn)
        _btn.pack(anchor=W)
        _btn=Button(fr, width=wid, text='Edit', command=lambda:edit_jump(curjump))
        style_btn(_btn)
        _btn.pack(anchor=W)
        _btn=Button(fr, width=wid, text='Add', command=lambda:add_jump(curjump))
        style_btn(_btn)
        _btn.pack(anchor=W)
        _btn=Button(fr, width=wid, text='Delete', command=lambda:del_jump(curjump))
        style_btn(_btn)
        _btn.pack(anchor=W)
        fr.grid(row=1,column=2,sticky=NW)
        
        self.field_ty = StringVar(self, typestr)
        self.field_ty.trace('w', lambda *_:self.update_type())
        cb = ttk.Combobox(col1, width=wid, textvariable=self.field_ty, state='readonly', values=('Empty', 'Page', 'Link'))
        style_cb(cb)
        cb.grid(row=0,column=2,sticky=W)
        
        fr = Frame(col1, bg=BGC)
        lb=Label(fr, text = 'Link:')
        style_label(lb)
        lb.pack(side='left',anchor=N)
        linkfr = Frame(fr, bg=BGC)
        self.radio_link = IntVar(self,linkrad)
        self.radio_link.trace('w', lambda *_:self.update_radlink())
        self.rad_link_cursh = ttk.Radiobutton(linkfr, text='Current Sheet', value=2, variable=self.radio_link)
        style_rad(self.rad_link_cursh)
        self.rad_link_cursh.pack(fill=BOTH)
        self.rad_link_sname = ttk.Radiobutton(linkfr, text='Sheet Name', value=0, variable=self.radio_link)
        style_rad(self.rad_link_sname)
        self.rad_link_sname.pack(fill=BOTH)
        
        wid = 13
        self.field_linkname = StringVar(self, linkname)
        self.field_linkname.trace('w', lambda *_:self.update_radlink())
        self.cb_link_sname = ttk.Combobox(linkfr, width=wid, textvariable=self.field_linkname, state='readonly', values=sheetnames)
        style_cb(self.cb_link_sname)
        self.cb_link_sname.pack()
        
        self.rad_link_snum = ttk.Radiobutton(linkfr, text='Sheet Number', value=1, variable=self.radio_link)
        style_rad(self.rad_link_snum)
        self.rad_link_snum.pack(fill=BOTH)
        self.field_linknum = StringVar(self, str(linknum))
        self.field_linknum.trace('w', lambda *_:self.update_radlink())
        self.cb_link_shnum = ttk.Combobox(linkfr, width=wid, textvariable=self.field_linknum, state='readonly', values=sheetnums)
        style_cb(self.cb_link_shnum)
        self.cb_link_shnum.pack()
        
        self.labl_secnum=Label(linkfr, text='Section Number')
        style_label(self.labl_secnum)
        self.labl_secnum.pack()
        self.field_secnum = StringVar(self, str(self.secnum))
        self.field_secnum.trace('w', lambda *_:self.update_secnum())
        self.cb_link_secnum = ttk.Combobox(linkfr, width=wid, textvariable=self.field_secnum, state='readonly')
        style_cb(self.cb_link_secnum)
        self.cb_link_secnum.pack()
        
        linkfr.pack(side='left',anchor=NW)
        fr.grid(row=2,column=1,columnspan=2,sticky=NE)
        lb=Label(col1, text = 'Page Content:')
        style_label(lb)
        lb.grid(row=0,column=4,sticky=SW)
        fr = Frame(col1, bg=BGC)
        self.txt_body = Text(fr, bd=5, height=20, width=6)
        style_txt(self.txt_body)
        self.ignore_modify = 2
        self.txt_body.insert(END,pageval)
        self.txt_body.config(undo=1)
        self.txt_body.bind_all('<<Modified>>', lambda *_:self.update_val()) 
        pack_scrollable_text(self.txt_body)
        fr.grid(row=1,column=4,sticky=NW,rowspan=2)
        fr = Frame(col1, bg=BGC)
        btns = []
        wid=12
        b=Button(fr, width=wid, text='Spoiler', command=lambda:txt_insert(self.txt_body,get_spoil))
        btns.append(b)
        b=Button(fr, width=wid, text='Tooltip', command=lambda:txt_insert(self.txt_body,get_ttip))
        btns.append(b)
        b=Button(fr, width=wid, text='Named Data', command=lambda:txt_insert(self.txt_body,get_named))
        btns.append(b)
        b=Button(fr, width=wid, text='Doc Link', command=lambda:txt_insert(self.txt_body,get_link))
        btns.append(b)
        b=Button(fr, width=wid, text='Code Inline', command=lambda:txt_insert(self.txt_body,get_code1))
        btns.append(b)
        b=Button(fr, width=wid, text='Code Block', command=lambda:txt_insert(self.txt_body,get_code2))
        btns.append(b)
        b=Button(fr, width=wid, text='TODO', command=lambda:txt_insert(self.txt_body,get_todo))
        btns.append(b)
        b=Button(fr, width=wid, text='Header (h3)', command=lambda:txt_insert(self.txt_body,get_header))
        btns.append(b)
        b=Button(fr, width=wid, text='Bold', command=lambda:txt_insert(self.txt_body,get_bold))
        btns.append(b)
        b=Button(fr, width=wid, text='Italic', command=lambda:txt_insert(self.txt_body,get_italic))
        btns.append(b)
        b=Button(fr, width=wid, text='Block', command=lambda:txt_insert(self.txt_body,get_block))
        btns.append(b)
        b=Button(fr, width=wid, text='Inline-Block', command=lambda:txt_insert(self.txt_body,get_iblock))
        btns.append(b)
        for btn in btns:
            style_btn(btn)
            btn.pack()
        self.insert_btns = btns
        fr.grid(row=1,column=5,sticky=NW,rowspan=2)
        
        col1.pack(side = 'left')
        toprow.pack()
        butrow = Frame(self, bg=BGC)
        wid = 5
        btns = []
        wid = 12    
        self.backbtn = Button(butrow, width=wid, text='←', command=lambda:navi.back())
        btns.append(self.backbtn)
        self.prevbtn=Button(butrow, width=wid, text='Preview HTML', command=preview_entry)
        btns.append(self.prevbtn)
        b=Button(butrow, width=wid, text='Save', command=save_entry)
        btns.append(b)
        b=Button(butrow, width=wid, text='Reset', command=reset_entry)
        btns.append(b)
        for btn in btns:
            style_btn(btn)
            btn.pack(side='left')
        self.save_btns = btns[-2:] # 'Save' and 'Reset', disabled if no changes.
        butrow.pack()
    def update_type(self):
        ty = self.field_ty.get()
        ispage = ty=='Page'
        islink = ty=='Link'
        disable_widgs([self.labl_secnum,self.cb_link_sname,self.cb_link_shnum,
            self.cb_link_secnum,self.rad_link_sname,self.rad_link_snum,
            self.rad_link_cursh],not islink)
        if islink:
            self.update_radlink()
        disable_widgs(self.insert_btns+[self.txt_body,self.prevbtn],not ispage)
        if ispage:
            self.txt_body.config(height=20, width=75)
        local_edited()
    def update_radlink(self):
        ty = self.radio_link.get()
        disable_widg(self.cb_link_sname,ty!=0)
        disable_widg(self.cb_link_shnum,ty!=1)
        sheetind = -1
        if ty == 0:
            sheetind = get_sheetind_named(self.field_linkname.get(),0)
        elif ty == 1:
            sheetind = int(self.field_linknum.get())
        self.cb_link_secnum['values'] = get_secnums(sheetind)
        if len(self.cb_link_secnum['values']) < 2:
            disable_widg(self.cb_link_secnum,True)
            cursec = self.secnum
            self.field_secnum.set('0')
            self.secnum = cursec
        else:
            disable_widg(self.cb_link_secnum,False)
            self.field_secnum.set(str(self.secnum))
        local_edited()
    def update_secnum(self):
        self.secnum = int(self.field_secnum.get())
        local_edited()
    def update_val(self):
        self.txt_body.tk.call(self.txt_body._w, 'edit', 'modified', 0)
        if self.ignore_modify:
            self.ignore_modify -= 1
            return
        local_edited()
    def get_val(self):
        ty = self.field_ty.get()
        match ty:
            case 'Empty':
                return ''
            case 'Link':
                sheet_ty = self.radio_link.get()
                sheet = '-1'
                if sheet_ty == 0: #Name
                    sheet = self.field_linkname.get()
                elif sheet_ty == 1:
                    sheet = self.field_linknum.get()
                sec = int(self.field_secnum.get())
                if str(sec) not in self.cb_link_secnum['values']:
                    sec = 0
                if sec:
                    return f'${sheet}${sec}'
                return f'${sheet}'
            case 'Page':
                return self.txt_body.get("1.0", "end-1c")
    def reload_jump(self,selind):
        global jumplistbox, curjump
        curjump = selind
        _load_list(jumplistbox, selind, self.list_jumps, lambda a:a)
    def postinit(self):
        global needs_edit_save, refr_entry
        self.update_type()
        needs_edit_save = True #Ensure it counts as 'changed' for local_edited()
        refr_entry = None
        local_edited(False)
    def reload(self):
        navi.refresh()
    def reload_lists(self):
        global curjump
        self.reload_jump(curjump)
class navigation:
    def __init__(self):
        self.history = [(0,)]
        self.index = 0
    # Reset to blank navigation
    def clear(self):
        self.history = [(0,)]
        self.update(0)
        self.goto((0,))
        self.update(0)
    # Refresh the page
    def refresh(self):
        global refr_entry
        isentry = len(self.history[self.index])==4
        if isentry:
            refr_entry = get_entry()
        self.goto_unsafe(self.history[self.index])
    # Switch the active GUI window to the specified location
    def goto_unsafe(self,node:tuple):
        global cursheet, cursec, curentry, file_loaded
        if not file_loaded:
            switch(InfoPage)
            return False
        match len(node):
            case 1:
                cursheet = node[0]
                cursec = 0
                curentry = 0
                switch(SheetsPage)
            case 2:
                cursheet = node[0]
                cursec = node[1]
                curentry = 0
                switch(EditShPage)
            case 3:
                cursheet = node[0]
                cursec = node[1]
                curentry = node[2]
                switch(EditSecPage)
            case 4:
                cursheet = node[0]
                cursec = node[1]
                curentry = node[2]
                switch(EditEntryPage)
            case _:
                raise Exception('Bad Navigation: '+str(node))
        return True
    # Gives warning if exiting entry editor while needing to save
    def goto(self,node:tuple):
        isentry = len(self.history[self.index])==4
        if isentry and not check_exit_entry():
            return False
        return self.goto_unsafe(node)
    # Goto a new node, adding it to the history, and clearing any forward-history
    def visit_new(self,node:tuple):
        if not self.validate(node):
            return False
        if not self.goto(node):
            return False
        if not self.atEnd():
            self.history = self.history[:self.index+1]
        self.history.append(node)
        self.update(self.index + 1)
        return True
    # Go back a node
    def back(self):
        if self.atStart():
            return
        if self.goto(self.history[self.index-1]):
            self.update(self.index - 1)
    # Go forward a node
    def fwd(self):
        if self.atEnd():
            return
        if self.goto(self.history[self.index+1]):
            self.update(self.index + 1)
    # Go up a menu
    def up(self):
        l = len(self.history[self.index])
        if l == 4: #Nowhere to go but back one... don't clutter history
            self.back() 
        elif l > 1: #Go up one page
            self.visit_new(self.history[self.index][:-1])
        else: #Attempt to exit, but always ask for confirmation
            quitter(False)
    
    # Updates the state of the object
    def update(self,ind=-1):
        global mainframe
        if ind > -1:
            self.index = ind
        # Disable back/fwd buttons?
        try: #They might not exist?
            disable_widg(mainframe.backbtn, self.atStart())
        except AttributeError:
            pass
        try:
            disable_widg(mainframe.fwdbtn, self.atEnd())
        except AttributeError:
            pass
    def atEnd(self)->bool:
        return self.index == len(self.history)-1
    def atStart(self)->bool:
        return self.index == 0
    def validate(self,node:tuple)->bool:
        global json_obj
        l = len(node)-1
        if l < 1:
            return True
        
        sh = node[0]
        if sh < -1 or sh >= len(json_obj['sheets']):
            return False
        
        if l < 2:
            return True
        sheet = _getsheet(sh)
        sec = node[1]
        if sec < 0 or sec >= len(sheet['tabs']):
            return False
        
        if l < 3:
            return True
        entr = node[2]
        if entr < 0 or entr >= len(sheet['tabs'][sec]['lines']):
            return False
        
        return True
    def depth(self)->int:
        global file_loaded
        if not file_loaded:
            return 0
        return len(self.history[self.index])
navi = navigation()

def get_entry(addjumps=None):
    global mainframe
    
    name = mainframe.field_name.get()
    jumps = mainframe.list_jumps
    
    outname = name
    if addjumps:
        if not jumps:
            jumps = [s.strip() for s in re.split('/',name)]
        jumps += addjumps
    if jumps:
        outname += ' ;; ' + ' / '.join(jumps)
    
    outval = mainframe.get_val()
    
    ret = {'name':outname,'val':outval}
    if mainframe.todo:
        ret['todo'] = True
    return ret
def entry_changed():
    global cursheet, cursec, curentry
    return _getsheet(cursheet)['tabs'][cursec]['lines'][curentry] != get_entry()
def save_entry():
    global needs_edit_save, cursheet, cursec, curentry
    if not needs_edit_save:
        return
    local_edited(False)
    entry = get_entry()
    sheet = _getsheet(cursheet)
    if entry == sheet['tabs'][cursec]['lines'][curentry]:
        #Entry did not change
        return
    #Entry changed
    sheet['tabs'][cursec]['lines'][curentry] = entry
    _setsheet(cursheet, sheet)
    local_edited(False)
    mark_edited()
def preview_entry():
    global cursheet, cursec, curentry
    old_e = json_obj['sheets'][cursheet]['tabs'][cursec]['lines'][curentry]
    new_e = get_entry(['_PREVIEW_JUMP'])
    
    json_obj['sheets'][cursheet]['tabs'][cursec]['lines'][curentry] = new_e
    preview_html()
    json_obj['sheets'][cursheet]['tabs'][cursec]['lines'][curentry] = old_e

def save_entry_exit():
    save_entry()
    navi.up()
def check_exit_entry():
    global needs_edit_save, root
    if needs_edit_save and entry_changed():
        if not messagebox.askyesno(parent=root, title = 'Exit without saving?', message = 'Edits to this entry have not been saved!'):
            return False
    return True
def exit_entry():
    if check_exit_entry():
        local_edited(False)
        navi.up()
def reset_entry():
    global needs_edit_save, root
    if needs_edit_save and entry_changed():
        if not messagebox.askyesno(parent=root, title = 'Reset changes?', message = 'Edits to this entry will be discarded!'):
            return
        switch(EditEntryPage)
    local_edited(False)
def gen_menubar():
    global menubar,filemenu,optmenu,root
    menubar = Menu(root)
    filemenu = Menu(menubar, tearoff=0, postcommand=onfile)
    filemenu.add_command(label = 'Load', command = loader_json)
    filemenu.add_command(label = 'Save', command = saver_json)
    filemenu.add_command(label = 'Save As', command = saver_json_as)
    filemenu.add_command(label = 'Save HTML', command = saver_html)
    filemenu.add_separator()
    filemenu.add_command(label = 'Help [F1]', command = info_program)
    filemenu.add_separator()
    filemenu.add_command(label = 'Exit', command = quitter)
    menubar.add_cascade(label='File', menu=filemenu)
    optmenu = Menu(menubar, tearoff=0, postcommand=onoption)
    optmenu.add_command(label = 'Autosave', command = opt_autosave)
    optmenu.add_checkbutton(label='Dark Theme', onvalue=1, offvalue=0, variable=_darktheme)
    menubar.add_cascade(label='Options', menu=optmenu)
    root.config(menu=menubar)

root = Tk()
style = ttk.Style(root)
root.config(bg=BGC)
update_file(args.inputfile) #Update title to include loaded file
root.geometry('1000x400')
_darktheme = BooleanVar(root)
_darktheme.set(config['theme']==1)
def settheme(val):
    config['theme'] = 1 if val else 0
    theme(config['theme'])
_darktheme.trace('w', lambda *_:settheme(_darktheme.get()))
gen_menubar()
root.protocol("WM_DELETE_WINDOW", quitter)
root.grid_rowconfigure(0, weight=1)
root.grid_columnconfigure(0, weight=1)
root.bind('<Escape>', lambda _: navi.up())
root.bind('<Return>', lambda _: mainframe.confirm())
root.bind('<Alt-Left>', lambda _: navi.back())
root.bind('<Alt-Right>', lambda _: navi.fwd())
root.bind('<F1>', lambda _: info_program())
root.bind('<Control-Key-s>', lambda _: saver_json())
root.bind('<Control-Key-l>', lambda _: loader_json())
root.bind('<Control-Key-o>', lambda _: loader_json())
root.bind('<Control-Key-c>', lambda _: mainframe.copy())
root.bind('<Control-Key-v>', lambda _: mainframe.paste())

mainframe = None
theme(config['theme'])
if file_loaded:
    navi.clear()
else:
    switch(InfoPage)
theme(config['theme'])


#Start autosave timer
schedule_autosave()

root.mainloop()
