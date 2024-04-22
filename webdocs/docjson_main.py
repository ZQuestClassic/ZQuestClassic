import docjson_lib as lib
import argparse
import os

parser = argparse.ArgumentParser()
parser.add_argument('--in', dest='inputfile', default='zsdocs_main.json',metavar='FILE',
    help='.json to generate docs from')
parser.add_argument('--out', dest='outputfile', default='ZScript_Docs.html',metavar='FILE',
    help='HTML webpage output')
parser.add_argument('--db','-d', action='store_true', help='Print debug info')
parser.add_argument('--cli','-c', dest='edit', action='store_false', help='Just cli, don\'t open editor')
args = parser.parse_args()

lib.debug_out = args.db

if not args.edit:
    if os.path.exists(args.inputfile):
        if not args.edit:
            json_obj = lib.loadjson(args.inputfile)
            lib.savehtml(args.outputfile,json_obj)
            exit(0)
    lib.parse_fail(f"Input file '{args.inputfile}' does not exist!")
    #Exception raised
## END CLI
import traceback, json, re, sys, copy
import time, webbrowser, pyperclip
from tkinter import *
from tkinter import filedialog
from tkinter import messagebox
from tkinter import simpledialog
from tkinter import ttk

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
copiedsec = None
copiedsheet = None
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

def popInfo(message,title='Info'):
    messagebox.showinfo(title = title, message = message)
def popError(message,title='Error!'):
    messagebox.showerror(title = title, message = message)
def popWarn(message,title='Warning!'):
    messagebox.showwarning(title = title, message = message)
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
    try:
        json_obj = lib.loadjson(args.inputfile)
        file_loaded = True
    except Exception as e:
        popError(f"Error '{str(e)}' occurred loading input file:\n{args.inputfile}")
else:
    update_file(args.inputfile)
    cur_file = ''

## Configs
bookmarks = []
def _def_config(key,defval):
    global config
    return lib._dict_get(config,key,defval)
def read_config():
    global config, bookmarks
    try:
        with open('docjson_config.json', 'r') as file:
            config = json.loads(file.read())
    except:
        config = {}
    _def_config('autosave_minutes',5)
    _def_config('theme',0)
    _def_config('finddupe',0)
    _def_config('findmode',0)
    for q in range(10):
        _def_config(f'bookmark{q}','')
    bookmarks = [Bookmark(indx=q) for q in range(10)]
    theme(config['theme'])
def write_config():
    global config
    try:
        with open('docjson_config.json', 'w') as file:
            file.write(json.dumps(config, indent=4))
    except Exception as e:
        print(e, file=sys.stderr)

config = None

def clipboard_copy(s:str):
    pyperclip.copy(s)
def clipboard_paste()->str:
    return pyperclip.paste()
def jsoncopy(obj):
    return json.loads(json.dumps(obj))
# Take what is on the system clipboard, as a json object, and try to copy it
def copy_from_sys_clipboard():
    s = clipboard_paste()
    try:
        jobj = json.loads(s)
    except:
        jobj = None
    if jobj:
        if validate_sheet(jobj):
            copy_sheet(jobj)
            popInfo(f"Imported sheet '{sheet_disp_name(jobj)}' from clipboard!")
            return
        if validate_sec(jobj):
            copy_sec(jobj)
            popInfo(f"Imported section '{sec_disp_name(jobj)}' from clipboard!")
            return
        if validate_entry(jobj):
            copy_entry(jobj)
            popInfo(f"Imported entry '{entry_disp_name(jobj)}' from clipboard!")
            return
    errstr = s
    if len(errstr) > 40:
        errstr = errstr[:40]+'...'
    errstr = re.sub('\r?\n','\\n',errstr)
    popError(f"Clipboard string '{errstr}'\nNot recognized as valid Sheet, Section, or Entry object!")
def onedit():
    global editmenu
    #editmenu.entryconfig('Copy From Clipboard', state = 'normal' if file_loaded else 'disabled')
    pass

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
        return False
    if len(args.inputfile) < 1:
        saver_json_as()
        return False
    try:
        lib.savejson(args.inputfile,json_obj)
        mark_saved()
        return True
    except Exception as e:
        popError(f"Error:\n{str(e)}\nOccurred saving file:\n{args.inputfile}")
        print(traceback.print_exc())
        return False
def saver_json_as():
    global json_obj, root, file_loaded, needs_save, cur_directory, cur_file
    if not file_loaded:
        return False
    fname = filedialog.asksaveasfilename(parent = root, title = 'Save As', initialdir = cur_directory, initialfile = cur_file, filetypes = (('Json','*.json'),),defaultextension = '.json')
    if len(fname) < 1:
        return False
    try:
        lib.savejson(fname,json_obj)
        update_file(trim_auto(fname))
        mark_saved()
        return True
    except Exception as e:
        popError(f"Error:\n{str(e)}\nOccurred saving file:\n{fname}")
        print(traceback.print_exc())
        return False
def new_json():
    global json_obj, file_loaded, cur_file
    if not save_warn('Create file'):
        return
    json_obj = {'key':'zs_docjson_py',
        'ver':1,
        'pagetitle':'JsonDocPage',
        'header':'Page Header Here',
        'url':'',
        'urltxt':'Latest',
        'sheets':[{'name':'MainData',
            'tabs':[
                {'name':'ROOT (HTML home)',
                    'lines':[
                        {'name':'--','val':'<todo>Edit this!</todo>'},
                        {'name':'Other Page ;; pg2','val':'$-1$1'}
                    ]
                },
                {'name':'Section 2',
                    'lines':[
                        {'name':'&lt;-- Back ;; root','val':'$-1'},
                        {'name':'--','val':'<todo>Add some page content</todo>'}
                    ]
                }]
            }],
        'named':{'name':'Named_Data',
            'tabs':[
                {'name':'MISC','lines':[]},
                {'name':'TTIPS','lines':[]}
            ]}
    }
    mark_saved()
    mark_autosaved()
    local_edited(False)
    cur_file = 'Untitled.json'
    update_root_title()
    args.inputfile = ''
    file_loaded = True
    navi.clear()

def get_save_html_name(filename=None):
    if filename:
        return filename
    _dir = os.path.dirname(args.outputfile)
    fname = os.path.basename(args.outputfile)
    fname = filedialog.asksaveasfilename(parent = root, title = 'Export HTML', initialdir = _dir, initialfile = fname, filetypes = (('HTML','*.html'),),defaultextension = '.html')
    return fname
def saver_html(filename=None,skipwarn=False,load_messager=None):
    global json_obj, root, file_loaded, htmlwarn_fname
    if not file_loaded:
        return False
    fname = get_save_html_name(filename)
    if fname == '':
        return False
    try:
        lib.savehtml(fname,json_obj,load_messager=load_messager)
        args.outputfile = fname
        return True
    except Exception as e:
        popError(f"Error:\n{str(e)}\nOccurred saving file:\n{fname}")
        print(traceback.print_exc())
        return False
    finally:
        if not skipwarn and lib.parse_warnings:
            popWarn('Warnings:\n'+'\n'.join(lib.parse_warnings)+f"\nOccurred saving file:\n{htmlwarn_fname}")

centered_popup_fr = None
centered_popup_labels = None
centered_popup_prog = None
def centered_popup(fr:Frame):
    global centered_popup_fr
    centered_popup_fr = fr
    
    fr.config(highlightthickness=2,highlightbackground=FGC)
    fr.grid()
    
    update_popup_size()
    return fr
def update_popup_size():
    global centered_popup_fr, centered_popup_prog
    root.update_idletasks()
    cx = mainframe.winfo_x()
    cy = mainframe.winfo_y()
    frw = mainframe.winfo_width()
    frh = mainframe.winfo_height()
    cx += frw/2
    cy += frh/2
    w=centered_popup_fr.winfo_width()
    h=centered_popup_fr.winfo_height()
    centered_popup_fr.place(x=cx-w/2,y=cy-h/2)
    if centered_popup_prog:
        t1,prog = centered_popup_prog
        pb,label,s = t1
        pb.config(length=w-10)
        pb['value'] = round((float(prog[0])*100)/prog[1])
        if label:
            label.config(text = s.format(prog[0],prog[1]))
def build_labels(strs:list,parent=None,waitstr=None,prog=None)->Frame:
    global root, centered_popup_labels
    if not parent:
        parent = root
    if centered_popup_labels:
        if parent == centered_popup_labels.master:
            for widg in centered_popup_labels.winfo_children():
                widg.destroy()
            fr = centered_popup_labels
        else:
            centered_popup_labels.destroy()
            centered_popup_labels = None
    if not centered_popup_labels:
        fr = Frame(parent,bg=BGC)
        centered_popup_labels = fr
        
    for s in strs:
        lb=Label(fr, text = s)
        style_label(lb)
        lb.pack()
    return fr
def build_prog(prog,waitstr=None,parent=None):
    global centered_popup_prog, root
    if not parent:
        parent = root
    fr = Frame(parent,bg=BGC)
    centered_popup_prog = None
    if prog and len(prog) == 2:
        #prog is tuple(current, expected) representing progress
        lb = None
        if waitstr:
            lb=Label(fr, text=waitstr.format(prog[0],prog[1]))
            style_label(lb)
            lb.pack()
        pb = ttk.Progressbar(fr,orient='horizontal',mode='determinate',length=10)
        pb.pack()
        centered_popup_prog = ((pb,lb,waitstr),prog)
    return fr
def destroy_popup():
    global centered_popup_fr, centered_popup_prog, centered_popup_labels
    if centered_popup_fr:
        centered_popup_fr.destroy()
        centered_popup_fr = None
    centered_popup_prog = None
    if centered_popup_labels:
        centered_popup_labels.destroy()
        centered_popup_labels = None
    root.update()
def repop_centered_labels(l,fr=None,prog=None,waitstr=None):
    global centered_popup_labels
    if not fr:
        fr = centered_popup_labels
    par = None
    if fr:
        par = fr.master
    fr = build_labels(l,parent=par,prog=prog,waitstr=waitstr)
    root.update()
    return fr
class ProgressTracker:
    def __init__(self,emptystr='',modcallback=None):
        self.labelstrs = []
        self.emptystr = emptystr
        self.modcb = modcallback
    def callback(self,tpl:tuple):
        ind,s = tpl
        if ind >= 0:
            if ind+1 < len(self.labelstrs):
                self.labelstrs = self.labelstrs[:ind+1]
            else:
                while len(self.labelstrs) <= ind:
                    self.labelstrs.append(self.emptystr)
            self.labelstrs[ind] = s
        if self.modcb:
            self.modcb(self.labelstrs)
progpop_inst = None
class ProgressPopup:
    def __init__(self,titlestrs=[],waitstr='Please Wait {}/{}',ccount=0,cmax=0,parent=None,tracker=None):
        global root
        self.titlestrs = titlestrs
        self.waitstr = waitstr
        self.popup = None
        self.callcount = ccount
        self.callmax = cmax
        self.tracker = tracker
        if not parent:
            parent = root
        self.parent = parent
    def pop(self):
        global root
        if self.popup:
            return #already popped
        self.popup = centered_popup(Frame(root,bg=BGC))
        
        f1 = build_labels(self.titlestrs,parent=self.popup)
        f1.pack()
        
        f2 = build_prog(parent=self.popup,prog=(self.callcount,self.callmax))
        f2.pack()
        update_popup_size()
        if not self.tracker:
            self.tracker = ProgressTracker()
        self.tracker.modcb = lambda x: self.update(x)
    def kill(self):
        if self.popup:
            destroy_popup()
            self.popup = None
    def update(self, labelstrs):
        global centered_popup_prog, root
        repop_centered_labels(self.titlestrs+labelstrs)
        centered_popup_prog = (centered_popup_prog[0],(self.callcount,self.callmax))
        update_popup_size()
        root.update()
        self.callcount += 1
    @staticmethod
    def open(titlestrs=[],waitstr='Please Wait {}/{}',ccount=0,cmax=0,parent=None):
        global progpop_inst, root
        if not parent:
            parent = root
        progpop_inst = ProgressPopup(titlestrs,waitstr,ccount,cmax,parent)
        progpop_inst.pop()
    @staticmethod
    def close():
        global progpop_inst
        if progpop_inst:
            progpop_inst.kill()
        progpop_inst = None
    @staticmethod
    def callback(tpl:tuple):
        global progpop_inst
        if progpop_inst.popup:
            progpop_inst.tracker.callback(tpl)
def save_html(filename=None,skipwarn=False):
    global mainframe
    filename = get_save_html_name(filename)
    if filename == '':
        return
    
    tout = args.outputfile
    lib.poll_html_callback()
    ProgressPopup.open(titlestrs=['Saving HTML...'],cmax=lib.NUM_HTML_CALLBACK-1)
    
    saver_html(filename,skipwarn=skipwarn,load_messager=ProgressPopup.callback)
    
    ProgressPopup.close()
    args.outputfile = tout
def preview_html():
    global mainframe
    tout = args.outputfile
    lib.poll_html_callback()
    ProgressPopup.open(titlestrs=['Parsing preview HTML...'],cmax=lib.NUM_HTML_CALLBACK-1)
    
    if saver_html('_preview.html',skipwarn=True,load_messager=ProgressPopup.callback):
        _go = True
        if lib.parse_warnings:
            _go = messagebox.askokcancel(parent=root,title='Warning!',message='Warnings:\n'+'\n'.join(lib.parse_warnings)+'\n\nPreview anyway?')
        if _go:
            openhtml(os.getcwd()+'\\'+'_preview.html')
    
    ProgressPopup.close()
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
    filemenu.entryconfig('Export HTML', state = 'normal' if file_loaded else 'disabled')
def onoption():
    global optmenu
    asmin = config['autosave_minutes']
    optmenu.entryconfig(0, label = f"Autosave ({asmin} min)" if asmin else "Autosave (off)")
def ontool():
    global toolmenu
    toolmenu.entryconfig('Broken Links', state = 'normal' if file_loaded else 'disabled')
    toolmenu.entryconfig('Todos', state = 'normal' if file_loaded else 'disabled')
def onpage():
    global pagemenu
    pagemenu.entryconfig('Settings', state = 'normal' if file_loaded else 'disabled')
    pagemenu.entryconfig('Named Data', state = 'normal' if file_loaded else 'disabled')
def onbkmrk():
    global bkmrkmenu
    curnode = navi.current() if file_loaded else None
    cansave = curnode and navi.validate(curnode)
    bkmrkmenu.entryconfig(0, state = 'normal' if cansave else 'disabled')
    validbk = False
    if file_loaded:
        for b in bookmarks:
            if b.valid():
                validbk = True
                break
    bkmrkmenu.entryconfig(1, state = 'normal' if validbk else 'disabled')
    bkmrkmenu.entryconfig(2, state = 'normal' if validbk else 'disabled')
def onbkmrk_save():
    global bkmrkmenu_save, bookmarks
    curnode = navi.current()
    cansave = curnode and navi.validate(curnode)
    for q in range(10):
        bkmrkmenu_save.entryconfig(q, label = bookmarks[q].save_str(),
            state = 'normal' if cansave else 'disabled')
def onbkmrk_load():
    global bkmrkmenu_load, bookmarks
    for q in range(10):
        bkmrkmenu_load.entryconfig(q, label = bookmarks[q].load_str(),
            state = 'normal' if bookmarks[q].valid() else 'disabled')
def onbkmrk_clr():
    global bkmrkmenu_clear, bookmarks
    for q in range(10):
        bkmrkmenu_clear.entryconfig(q, label = bookmarks[q].clear_str(),
            state = 'normal' if bookmarks[q].valid() else 'disabled')
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
def check_brlink():
    global mainframe, brlinks
    tout = args.outputfile
    lib.poll_html_callback()
    ProgressPopup.open(titlestrs=['Parsing HTML...'],cmax=lib.NUM_HTML_CALLBACK-1)
    
    brlinks = []
    if saver_html('_preview.html',skipwarn=True,load_messager=ProgressPopup.callback):
        brlinks = lib.broken_links
    
    ProgressPopup.close()
    args.outputfile = tout
    
    if brlinks:
        popup_brlink()
def check_todo():
    global mainframe
    todos = search_todo()
    if todos:
        popup_todo(todos)
    else:
        popInfo('No todos found!')

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
    navi.go_named()
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
    
    oldname = json_obj['sheets'][ind]['name']
    name = popup_editstring('Sheet Name',oldname,'Rename Sheet',f"Rename the sheet '{oldname}'?")
    
    if name is None:
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
    oldname = sheet['tabs'][ind]['name']
    name = popup_editstring('Tab Name',oldname,'Rename Tab',f"Rename the tab '{oldname}'?")
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
    oldname = mainframe.list_jumps[ind]
    string = popup_editstring('Jump Name',oldname,'Rename Jump',f"Rename the jump '{oldname}'?")
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
    string = popup_editstring('Add Jump String','','New Jump',f"New jump string?")
    if not string:
        return
    string = string.strip()
    if not string:
        return
    mainframe.list_jumps = mainframe.list_jumps[:ind+1] + [string] + mainframe.list_jumps[ind+1:]
    local_edited()
    mainframe.reload_jump(ind+1)
def repl_jump():
    global mainframe
    tpl = popup_findrepl('Replace a given string in all these jumps?',candupe=True)
    if not tpl:
        return
    findstr,replstr,mode,dupl = tpl
    escstr = findstr
    if mode == 0:
        escstr = re.escape(findstr)
    updated = 0
    oldjumps = mainframe.list_jumps
    newjumps = []
    endjumps = []
    for q in range(len(oldjumps)):
        oldstr = oldjumps[q]
        if re.search(escstr,oldstr):
            newstr = re.sub(escstr, replstr, oldstr)
            if dupl > 0:
                newjumps.append(oldstr)
            if newstr:
                match dupl:
                    case 1: #Add inbetween
                        newjumps.append(newstr)
                    case 2: #Add at end
                        endjumps.append(newstr)
                    case 0: #Replace
                        newjumps.append(newstr)
                updated += 1
            elif not dupl:
                updated += 1
        else:
            newjumps.append(oldstr)
    if updated > 0:
        mainframe.list_jumps = newjumps+endjumps
        mainframe.reload_lists()
        popInfo(f'{"Duplicated" if dupl else "Replaced"} {updated} instances!')
        local_edited(True)
    else:
        popInfo(f'Found no instances!')
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
    if lib._dict_get(d,'todo'):
        lib._dict_del(d,'todo')
    else:
        d['todo'] = True
    mainframe.reload_lists()
    mark_edited()
## Info
def info_program():
    popInfo('''Working files are saved in '.json' format. Saving to HTML exports a preview of the docs with your changes, but does not actually save your changes.

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
    popInfo('''Double-clicked sheets will be edited.
Pressing 'Enter' has the same effect as double-clicking the current selection.
Right-clicked sheets will be renamed.
Sheet names are used to link to sections in sheets.
The top sheet cannot be re-ordered, it contains the root of the document as its' first entry.''')
def info_editsheet():
    popInfo('''Double-clicked sections will be edited.
Pressing 'Enter' has the same effect as double-clicking the current selection.
Right-clicked sections will be renamed.
Section names are not displayed in the html, and have no meaning except in Named_Data (where they are used for search priority)''')
def info_editsec():
    popInfo('''Entries named exactly '--' will be hidden from the list.
The first Page type Entry will be displayed by default for the section, even if named '--'.
Making a '--' first entry can work if there are no other pages to display a body alongside a section full of links.

A double-clicked entry is edited unless it is a link, in which case it is followed.
Pressing 'Enter' has the same effect as double-clicking the current selection.
Right-clicking an entry will edit it, even if it is a link.''')
def info_editentry():
    popInfo('''Entry Types:
Empty- An empty entry placeholder.
Link- A link to another section.
Page- An html page to display content.

If an entry has no jumps, it will use its' name, split at '/' characters, as jumps.
Jumps beginning with a '*' are 'Search Jumps'. These... I'm not sure what do differently anymore? Generally they are shorter/more recognizable.
Double-clicking or right-clicking a jump will edit it.''')
## Pages
def switch(pageclass):
    global root, mainframe, curpage
    if mainframe is not None:
        mainframe.destroy()
    mainframe = pageclass(root)
    curpage = pageclass
    mainframe._postinit()
    mainframe.grid()
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
    elif issubclass(type(widg),Checkbutton):
        disable_check(widg,dis)
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
    pass
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
        btn.config(relief=GROOVE,background=BGC)
    else:
        btn.config(relief=RAISED)
def style_check(chkbtn):
    chkbtn.config(bd=2,bg=BGC,fg=FLD_FGC,disabledforeground=DIS_FGC,
        activebackground=ACT_BGC,activeforeground=FLD_FGC,highlightcolor=FLD_FGC)
    # style.configure('TRadiobutton',padding=1,background=BGC,
        # foreground=FGC,indicatorbackground=FLD_BGC,indicatorforeground=FLD_FGC,
        # indicatorrelief=RAISED,indicatorsize=12,focuscolor=FGC)
    disable_check(chkbtn,False)
def disable_check(chkbtn,dis):
    if dis:
        chkbtn.config(relief=GROOVE,background=BGC)
    else:
        chkbtn.config(relief=RAISED)
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
    
    # ttk.Progressbar
    style.configure('TProgressbar',troughcolor=FLD_BGC,background=FGC)
    
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
    def copytext(self,txt):
        try:
            self.copylabel.config(text=txt)
        except:
            pass

class InfoPage(Page):
    def __init__(self, root):
        Page.__init__(self,root)
        lb=Label(self, text = '''This GUI allows editing the web documentation.
        Docs are stored in .json format, which can be saved/loaded in the 'File' menu.
        You can also export the generated .html file.
        
        You must use 'Load' or 'New' to continue.''')
        style_label(lb)
        lb.pack()
    def postinit(self):
        pass

curpage = InfoPage
def sheet_disp_name(sheet)->str:
    name = sheet['name']
    if lib._dict_get(sheet,'todo'):
        name = f'[TODO] {name}'
    return name
def nil_sheet(sheet)->bool:
    try:
        if sheet['tabs'] != []:
            return False
        if sheet['name'] != '' and sheet['name'] != 'New Sheet':
            return False
        if lib._dict_get(sheet,'todo'):
            return False
    except:
        pass
    return True
def validate_sheet(sheet)->bool:
    try:
        if type(sheet['name']) is not str:
            return False
        if type(sheet['tabs']) is not list:
            return False
    except:
        return False
    return True
def copy_sheet(force=False):
    global cursheet, copiedsheet, mainframe
    if force is not False:
        if not force:
            force = None
        elif not validate_sheet(force):
            return
        copiedsheet = force
    else:
        try:
            copiedsheet = json_obj['named'] if cursheet == -1 else json_obj['sheets'][cursheet]
            try:
                clipboard_copy(json.dumps(copiedsheet,indent=4))
            except:
                pass
        except:
            copiedsheet = None
    if curpage != SheetsPage:
        return
    try:
        if copiedsheet:
            dispname = sheet_disp_name(copiedsheet)
            ctext = dispname
            if dispname[0] == '[':
                ind = 0
                while dispname[ind] == '[':
                    while dispname[ind] != ']':
                        ind += 1
                    ind +=1
                ctext = ' '+dispname[:ind]+'\n'
                while dispname[ind] == ' ':
                    ind += 1
                ctext += dispname[ind:]
            else:
                ctext = '\n'+ctext
            mainframe.copytext(f'Copied:{ctext}')
        else:
            mainframe.copytext('')
    except:
        pass
    try:
        disable_widg(mainframe.pastebtn,not copiedsheet)
    except:
        pass
def paste_sheet():
    global cursheet, copiedsheet
    if not copiedsheet:
        return
    e = json_obj['named'] if cursheet == -1 else json_obj['sheets'][cursheet]
    if e == copiedsheet: #Identical object
        return
    if not nil_sheet(e):
        name = sheet_disp_name(e)
        cname = sheet_disp_name(copiedsheet)
        if not messagebox.askyesno(parent=root, title = f'Paste over {name}?', message = f"The sheet '{name}' will be replaced with the copied sheet '{cname}'."):
            return
    if cursheet == -1:
        json_obj['named'] = jsoncopy(copiedsheet)
    else:
        json_obj['sheets'][cursheet] = jsoncopy(copiedsheet)
    mainframe.reload_lists()
    mark_edited()
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
        
        cmd_inf = info_sheets
        cmd_up = lambda:sheetshift(-1)
        cmd_dn = lambda:sheetshift(1)
        cmd_edit = lambda:edit_sheet(cursheet)
        cmd_todo = lambda:toggle_todo(json_obj['sheets'][cursheet])
        cmd_add = lambda:add_sheet(cursheet)
        cmd_del = lambda:del_sheet(cursheet)
        exbtn_txt = 'Rename'
        exbtn_cmd = lambda:ren_sheet(cursheet)
        exit_txt = 'Exit Program'
        makeButtonGrid(self,cmd_inf,cmd_up,cmd_dn,cmd_edit,cmd_todo,cmd_add,cmd_del,exbtn_txt,exbtn_cmd,exit_txt)
    def reload_sheets(self,selind):
        global json_obj, cursheet, sheetlistbox
        cursheet = selind
        _load_list(sheetlistbox, selind, json_obj['sheets'], sheet_disp_name)
    def postinit(self):
        global copiedsheet
        copy_sheet(copiedsheet)
    def reload(self):
        navi.refresh()
    def confirm(self):
        global cursheet
        edit_sheet(cursheet)
    def reload_lists(self):
        global cursheet
        self.reload_sheets(cursheet)
    def copy(self):
        copy_sheet()
    def paste(self):
        paste_sheet()

def sec_disp_name(sec)->str:
    name = sec['name']
    if lib._dict_get(sec,'todo'):
        name = f'[TODO] {name}'
    return name
def nil_sec(sec)->bool:
    try:
        if sec['lines'] != []:
            return False
        if sec['name'] != '' and sec['name'] != 'New Section':
            return False
        if lib._dict_get(sec,'todo'):
            return False
    except:
        pass
    return True
def validate_sec(sec)->bool:
    try:
        if type(sec['name']) is not str:
            return False
        if type(sec['lines']) is not list:
            return False
    except:
        return False
    return True
def copy_sec(force=False):
    global cursheet, cursec, copiedsec, mainframe
    if force is not False:
        if not force:
            force = None
        elif not validate_sec(force):
            return
        copiedsec = force
    else:
        try:
            sh = json_obj['named'] if cursheet == -1 else json_obj['sheets'][cursheet]
            copiedsec = sh['tabs'][cursec]
            try:
                clipboard_copy(json.dumps(copiedsec,indent=4))
            except:
                pass
        except:
            copiedsec = None
    if curpage != EditShPage:
        return
    try:
        if copiedsec:
            dispname = sec_disp_name(copiedsec)
            ctext = dispname
            if dispname[0] == '[':
                ind = 0
                while dispname[ind] == '[':
                    while dispname[ind] != ']':
                        ind += 1
                    ind +=1
                ctext = ' '+dispname[:ind]+'\n'
                while dispname[ind] == ' ':
                    ind += 1
                ctext += dispname[ind:]
            else:
                ctext = '\n'+ctext
            mainframe.copytext(f'Copied:{ctext}')
        else:
            mainframe.copytext('')
    except:
        pass
    try:
        disable_widg(mainframe.pastebtn,not copiedsec)
    except:
        pass
def paste_sec():
    global cursheet, cursec, copiedsec
    if not copiedsec:
        return
    sh = json_obj['named'] if cursheet == -1 else json_obj['sheets'][cursheet]
    e = sh['tabs'][cursec]
    if e == copiedsec: #Identical object
        return
    if not nil_sec(e):
        name = sec_disp_name(e)
        cname = sec_disp_name(copiedsec)
        if not messagebox.askyesno(parent=root, title = f'Paste over {name}?', message = f"The section '{name}' will be replaced with the copied section '{cname}'."):
            return
    if cursheet == -1:
        json_obj['named']['tabs'][cursec] = jsoncopy(copiedsec)
    else:
        json_obj['sheets'][cursheet]['tabs'][cursec] = jsoncopy(copiedsec)
    mainframe.reload_lists()
    mark_edited()
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
        
        cmd_inf = info_editsheet
        cmd_up = lambda:secshift(-1)
        cmd_dn = lambda:secshift(1)
        cmd_edit = lambda:edit_sec(cursec)
        cmd_todo = lambda:toggle_todo(json_obj['sheets'][cursheet]['tabs'][cursec])
        cmd_add = lambda:add_sec(cursec)
        cmd_del = lambda:del_sec(cursec)
        exbtn_txt = 'Rename'
        exbtn_cmd = lambda:ren_sec(cursec)
        makeButtonGrid(self,cmd_inf,cmd_up,cmd_dn,cmd_edit,cmd_todo,cmd_add,cmd_del,exbtn_txt,exbtn_cmd)
        
    def reload_sections(self,selind):
        global cursheet, cursec, seclistbox
        cursec = selind
        sheet = _getsheet(cursheet)
        _load_list(seclistbox, selind, sheet['tabs'], sec_disp_name)
    def postinit(self):
        global copiedsec
        copy_sec(copiedsec)
    def reload(self):
        navi.refresh()
    def confirm(self):
        global cursec
        edit_sec(cursec)
    def reload_lists(self):
        global cursec
        self.reload_sections(cursec)
    def copy(self):
        copy_sec()
    def paste(self):
        paste_sec()

def entry_disp_name(entry)->str:
    name = lib.get_line_display(entry)
    pref = ''
    if lib._dict_get(entry,'todo'):
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
        if lib._dict_get(entry,'todo'):
            return False
    except:
        pass
    return True
def validate_entry(entry)->bool:
    try:
        if type(entry['name']) is not str:
            return False
        if type(entry['val']) is not str:
            return False
    except:
        return False
    return True
def copy_entry(force=False):
    global cursheet, cursec, curentry, copiedentry, mainframe
    if force is not False:
        if not force:
            force = None
        elif not validate_entry(force):
            return
        copiedentry = force
    else:
        try:
            sh = json_obj['named'] if cursheet == -1 else json_obj['sheets'][cursheet]
            copiedentry = sh['tabs'][cursec]['lines'][curentry]
            try:
                clipboard_copy(json.dumps(copiedentry,indent=4))
            except:
                pass
        except:
            copiedentry = None
    if curpage != EditSecPage:
        return
    try:
        if copiedentry:
            dispname = entry_disp_name(copiedentry)
            ctext = dispname
            if dispname[0] == '[':
                ind = 0
                while dispname[ind] == '[':
                    while dispname[ind] != ']':
                        ind += 1
                    ind +=1
                ctext = ' '+dispname[:ind]+'\n'
                while dispname[ind] == ' ':
                    ind += 1
                ctext += dispname[ind:]
            else:
                ctext = '\n'+ctext
            mainframe.copytext(f'Copied:{ctext}')
        else:
            mainframe.copytext('')
    except:
        pass
    try:
        disable_widg(mainframe.pastebtn,not copiedentry)
    except:
        pass
def paste_entry():
    global cursheet, cursec, curentry, copiedentry
    if not copiedentry:
        return
    sh = json_obj['named'] if cursheet == -1 else json_obj['sheets'][cursheet]
    e = sh['tabs'][cursec]['lines'][curentry]
    if e == copiedentry: #Identical object
        return
    if not nil_entry(e):
        name = entry_disp_name(e)
        cname = entry_disp_name(copiedentry)
        if not messagebox.askyesno(parent=root, title = f'Paste over {name}?', message = f"The entry '{name}' will be replaced with the copied entry '{cname}'."):
            return
    if cursheet == -1:
        json_obj['named']['tabs'][cursec]['lines'][curentry] = jsoncopy(copiedentry)
    else:
        json_obj['sheets'][cursheet]['tabs'][cursec]['lines'][curentry] = jsoncopy(copiedentry)
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
        
        cmd_inf = info_editsec
        cmd_up = lambda:entryshift(-1)
        cmd_dn = lambda:entryshift(1)
        cmd_edit = lambda:edit_entry(curentry)
        cmd_todo = lambda:toggle_todo(json_obj['sheets'][cursheet]['tabs'][cursec]['lines'][curentry])
        cmd_add = lambda:add_entry(curentry)
        cmd_del = lambda:del_entry(curentry)
        exbtn_txt = 'Follow Link'
        exbtn_cmd = lambda:link_entry(curentry)
        makeButtonGrid(self,cmd_inf,cmd_up,cmd_dn,cmd_edit,cmd_todo,cmd_add,cmd_del,exbtn_txt,exbtn_cmd)
    def reload_entry_link(self):
        global curentry
        link = get_entry_link(curentry)
        disable_widg(self.exbtn,not link)
    def reload_entry(self,selind):
        global cursheet, curentry, cursec, entrylistbox
        curentry = selind
        self.reload_entry_link()
        sheet = _getsheet(cursheet)
        _load_list(entrylistbox, selind, sheet['tabs'][cursec]['lines'], entry_disp_name)
    def postinit(self):
        global curentry, copiedentry
        self.reload_entry(curentry)
        copy_entry(copiedentry)
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

def text_pack(before='',sel='',after='',replall=False):
    return (before,sel,after,replall)
def get_ttip(before:str,after:str,sel:str=None):
    return text_pack('[[',sel if sel else 'TITLE','|TIP TEXT]]')
def get_named(before:str,after:str,sel:str=None):
    return text_pack('$[',sel if sel else 'DATA_NAME',']')
def get_spoil(before:str,after:str,sel:str=None):
    return text_pack('#{TITLE|',sel if sel else 'HIDDEN TEXT','}')
def get_link(before:str,after:str,sel:str=None):
    return text_pack('${KEY|',sel if sel else 'DISPLAY TEXT','}')
def get_code1(before:str,after:str,sel:str=None):
    return text_pack('`',sel if sel else 'CODE','`')
def get_code2(before:str,after:str,sel:str=None):
    return text_pack('```',sel if sel else 'CODE','```')
def get_block(before:str,after:str,sel:str=None):
    return text_pack('<block>',sel if sel else 'TEXT','</block>')
def get_iblock(before:str,after:str,sel:str=None):
    return text_pack('<iblock>',sel if sel else 'TEXT','</iblock>')
def get_monospace(before:str,after:str,sel:str=None):
    return text_pack('<mono>',sel if sel else 'TEXT','</mono>')
def get_lt(before:str,after:str,sel:str=None):
    return text_pack('&lt;','','')
def get_gt(before:str,after:str,sel:str=None):
    return text_pack('&gt;','','')
def get_amp(before:str,after:str,sel:str=None):
    return text_pack('&amp;','','')
def get_todo(before:str,after:str,sel:str=None):
    return text_pack('<todo>',sel if sel else 'TEXT','</todo>')
def get_header(before:str,after:str,sel:str=None):
    return text_pack('<h3>',sel if sel else 'HEADER TEXT','</h3>')
def get_bold(before:str,after:str,sel:str=None):
    return text_pack('<strong>',sel if sel else 'TEXT','</strong>')
def get_italic(before:str,after:str,sel:str=None):
    return text_pack('<em>',sel if sel else 'TEXT','</em>')

def get_escapes(before:str,after:str,sel:str=None):
    if not sel:
        return None
    #Escape ampersands and gt/lt tags
    sel = re.sub('&', '&amp;', sel)
    sel = re.sub('<', '&lt;', sel)
    sel = re.sub('>', '&gt;', sel)
    #...but don't escape the ampersand of an escape code
    sel = re.sub('&amp;(amp|lt|gt);', lambda m:f'&{m.group(1)};', sel)
    return text_pack(sel=sel)
def get_unescapes(before:str,after:str,sel:str=None):
    if not sel:
        return None
    sel = re.sub('&amp;', '&', sel)
    sel = re.sub('&lt;', '<', sel)
    sel = re.sub('&gt;', '>', sel)
    return text_pack(sel=sel)
def get_findrepl(before:str,after:str,sel:str=None):
    global findstr, replstr
    if sel:
        findstr = sel
    tpl = popup_findrepl(f"Replace a given string in {'the selection' if sel else 'the page text'}?")
    if not tpl:
        return None
    findstr,replstr,mode,dupl = tpl
    escstr = findstr
    if mode == 0:
        escstr = re.escape(findstr)
    updated = 0
    s1,s2,s3 = '','',''
    if sel: #Search in selection
        if re.search(escstr,sel):
            updated = len(re.findall(escstr,sel))
            s2 = re.sub(escstr, replstr, sel)
        else:
            s2 = sel
    else: #Search in the total page
        if re.search(escstr,before):
            updated = len(re.findall(escstr,before))
            s1 = re.sub(escstr, replstr, before)
        else:
            s1 = before
        if re.search(escstr,after):
            updated += len(re.findall(escstr,after))
            s3 = re.sub(escstr, replstr, after)
        else:
            s3 = after
    if not updated:
        popInfo('Found no instances!')
        return None
    popInfo(f'Replaced {updated} instances!')
    return text_pack(s1,s2,s3,replall=not sel)

def txt_insert(txt,getter):
    has_sel = bool(txt.tag_ranges(SEL))
    if has_sel:
        sel = txt.get(SEL_FIRST, SEL_LAST)
        before = txt.get('1.0', SEL_FIRST)
        after = txt.get(SEL_LAST, END)
    else:
        sel = None
        before = txt.get('1.0', INSERT)
        after = txt.get(INSERT, END)
    tpl = getter(before,after,sel)
    if not tpl: #Cancelled?
        return
    str_bef,str_sel,str_aft,replall = tpl
    
    s = str_bef+str_sel+str_aft
    
    if replall:
        if before == str_bef and after == str_aft and sel == str_sel:
            return
    elif has_sel:
        if sel == str_sel and not str_bef and not str_aft:
            return
    elif not s:
        return
    txt.edit_separator()
    txt.config(autoseparator=False)
    ## In a textfield edit bubble
    if replall:
        tmp = 0
        txt.delete('1.0',END)
    elif has_sel:
        tmp = txt.count('1.0',SEL_FIRST)
        txt.delete(SEL_FIRST, SEL_LAST)
    else:
        tmp = txt.count('1.0',INSERT)
    first = tmp[0] if tmp else 0
    first += len(str_bef)
    last = first + len(str_sel)
    txt.insert(INSERT, s)
    txt.tag_add('sel',f'1.0+{first}c',f'1.0+{last}c')
    txt.mark_set(INSERT,f'1.0+{last}c')
    ## End textfield edit bubble
    txt.config(autoseparator=True)
    txt.edit_separator()

def makeButtonGrid(fr,cmd_inf=None,cmd_up=None,cmd_dn=None,cmd_edit=None,cmd_todo=None,
    cmd_add=None,cmd_del=None,exbtn_txt='',exbtn_cmd=None,exit_txt='Exit (Up)'):
    gr = Frame(fr, bg=BGC)
    f2 = Frame(gr, bg=BGC)
    _btn=Button(f2, text = '?', command=cmd_inf)
    style_btn(_btn)
    _btn.pack(anchor=W)
    wid = 5
    _btn=Button(f2, width=wid, text='↑', command=cmd_up)
    style_btn(_btn)
    _btn.pack(anchor=W)
    _btn=Button(f2, width=wid, text='↓', command=cmd_dn)
    style_btn(_btn)
    _btn.pack(anchor=W)
    f2.grid(row=0,column=0,sticky=SW)
    f2 = Frame(gr, bg=BGC)
    fr.copylabel = Label(f2, text='', wraplength=125)
    style_label(fr.copylabel)
    fr.copylabel.pack()
    f2.grid(row=0,column=1,columnspan=3,sticky=SW)
    f2 = Frame(gr, bg=BGC)
    wid = 10
    _btn=Button(f2, width=wid, text='Edit', command=cmd_edit)
    style_btn(_btn)
    _btn.pack()
    _btn=Button(f2, width=wid, text='Mark Todo', command=cmd_todo)
    style_btn(_btn)
    _btn.pack()
    _btn=Button(f2, width=wid, text='Add', command=cmd_add)
    style_btn(_btn)
    _btn.pack()
    _btn=Button(f2, width=wid, text='Delete', command=cmd_del)
    style_btn(_btn)
    _btn.pack()
    f2.grid(row=1,column=0,columnspan=2,sticky=NW)
    f2 = Frame(gr, bg=BGC)
    _btn=Button(f2, width=wid, text='Copy', command=lambda:fr.copy())
    style_btn(_btn)
    _btn.pack()
    fr.pastebtn=Button(f2, width=wid, text='Paste', command=lambda:fr.paste())
    style_btn(fr.pastebtn)
    fr.pastebtn.pack()
    fr.exbtn = Button(f2, width=wid, text=exbtn_txt, command=exbtn_cmd)
    style_btn(fr.exbtn)
    disable_widg(fr.exbtn,not exbtn_txt)
    fr.exbtn.pack()
    _btn=Button(f2, width=wid, text=exit_txt, command=lambda:navi.up())
    style_btn(_btn)
    _btn.pack()
    f2.grid(row=1,column=2,sticky=NW)
    f2 = Frame(gr, bg=BGC, width=25)
    f2.grid(row=1,column=3)
    gr.pack(side='left')

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
        _spl = entry['name'].split(';;',3)
        name = _spl[0].strip()
        jumps = []
        if len(_spl) > 1:
            jumps = [s.strip() for s in re.split('/',_spl[1])]
            if(len(_spl) > 2):
                jumps = jumps + ['*' + s.strip() for s in re.split('/',_spl[2])]
        
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
        pageval = ('\n'*2)+pageval.strip('\r\n \t')+('\n'*10)
        
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
        _btn=Button(fr, width=wid, text='Repl', command=lambda:repl_jump())
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
        insrow = Frame(col1, bg=BGC)
        fr = Frame(insrow, bg=BGC)
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
        fr.pack(side='left',anchor=N)
        self.insert_btns = btns
        btns = []
        fr = Frame(insrow, bg=BGC)
        b=Button(fr, width=wid, text='Monospaced', command=lambda:txt_insert(self.txt_body,get_monospace))
        btns.append(b)
        for btn in btns:
            style_btn(btn)
            btn.pack()
        self.insert_btns = self.insert_btns + btns
        btns = []
        br = Frame(fr, bg=BGC)
        wid = 3
        b=Button(br, width=wid, text='<', command=lambda:txt_insert(self.txt_body,get_lt))
        btns.append(b)
        b=Button(br, width=wid, text='>', command=lambda:txt_insert(self.txt_body,get_gt))
        btns.append(b)
        b=Button(br, width=wid, text='&', command=lambda:txt_insert(self.txt_body,get_amp))
        btns.append(b)
        for btn in btns:
            style_btn(btn)
            btn.pack(side='left')
        br.pack()
        self.insert_btns = self.insert_btns + btns
        wid = 12
        b=Button(fr, width=wid, text='Esc <>&', command=lambda:txt_insert(self.txt_body,get_escapes))
        btns.append(b)
        b=Button(fr, width=wid, text='Unesc <>&', command=lambda:txt_insert(self.txt_body,get_unescapes))
        btns.append(b)
        b=Button(fr, width=wid, text='FindRepl', command=lambda:txt_insert(self.txt_body,get_findrepl))
        btns.append(b)
        for btn in btns:
            style_btn(btn)
            btn.pack()
        self.insert_btns = self.insert_btns + btns
        fr.pack(side='left',anchor=N)
        insrow.grid(row=1,column=5,sticky=NW,rowspan=2)
        
        col1.pack(side = 'left')
        toprow.pack()
        butrow = Frame(self, bg=BGC)
        btns = []
        wid = 8
        self.upbtn=Button(butrow, width=wid, text='Exit (Up)', command=lambda:navi.up())
        btns.append(self.upbtn)
        self.backbtn = Button(butrow, width=wid, text='←', command=lambda:navi.back())
        btns.append(self.backbtn)
        self.fwdbtn = Button(butrow, width=wid, text='→', command=lambda:navi.fwd())
        btns.append(self.fwdbtn)
        wid = 12
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
    def get_ty(self):
        return self.field_ty.get()
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
                return self.txt_body.get('1.0', END)
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
        l = len(self.current())
        if l > 1: #Go up one page
            lastnode = None if self.index==0 else self.history[self.index-1]
            upnode = self.history[self.index][:-1]
            if upnode == lastnode:
                self.back() #Less clutter in the history
            else:
                self.visit_new(upnode)
        else: #Attempt to exit, but always ask for confirmation
            quitter(False)
    # Go to Named Data
    def go_named(self):
        node = self.history[self.index]
        if node[0] == -1: # In named data
            if len(node) == 2: #exactly
                return
            if len(node) > 2: #subpage
                self.visit_new(node[:2])
                return
        self.visit_new((-1,0)) #visit named data
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
        global json_obj, file_loaded
        if not file_loaded:
            return False
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
    def current(self,offs=0):
        global file_loaded
        if not file_loaded:
            return None
        newind = self.index+offs
        if newind < 0 or newind >= len(self.history):
            return None
        return self.history[newind]
    def depth(self,offs=0)->int:
        node = self.current(offs)
        if not node:
            return 0
        return len(node)
    def string(self,node:tuple)->str:
        global json_obj, file_loaded
        if not file_loaded:
            return '(Not Loaded)'
        if not self.validate(node):
            return '(Invalid)'
        l = len(node)-1
        if l < 0:
            return '(None)'
        if l < 1:
            return '(Root)'
        
        sh = node[0]
        s = '('+json_obj['sheets'][sh]['name']
        
        if l < 2:
            return s+')'
        sheet = _getsheet(sh)
        sec = node[1]
        s += ', '+sheet['tabs'][sec]['name']
        
        if l < 3:
            return s+')'
        entr = node[2]
        s += ', '+sheet['tabs'][sec]['lines'][entr]['name']
        
        return s+')'
        
navi = navigation()

class Bookmark:
    def __init__(self,dest=None,indx=None):
        self.dest = dest
        self.indx = indx
        if not dest:
            self.load_config_str()
    def valid(self):
        if not self.dest:
            return False
        return navi.validate(self.dest)
    def save(self):
        self.dest = navi.current()
        if self.indx is not None:
            config[f'bookmark{self.indx}'] = self.config_str()
    def load(self):
        navi.visit_new(self.dest)
    def clear(self):
        self.dest = None
        if self.indx is not None:
            config[f'bookmark{self.indx}'] = ''
    def __str__(self):
        if not self.dest:
            return '(None)'
        if not self.valid():
            return '(Invalid)'
        return f'{navi.string(self.dest)}'
    def save_str(self):
        if not self.valid():
            return f'Save {self.indx}: --'
        return f'Save {self.indx}: {str(self)}'
    def load_str(self):
        if not self.valid():
            return f'Load {self.indx}: --'
        return f'Load {self.indx}: {str(self)}'
    def clear_str(self):
        if not self.valid():
            return f'Clear {self.indx}: --'
        return f'Clear {self.indx}: {str(self)}'
    def config_str(self)->str:
        if not self.dest:
            return ''
        return ','.join([str(x) for x in self.dest])
    def load_config_str(self,s:str=None):
        if s is None and self.indx is not None:
            s = config[f'bookmark{self.indx}']
        if not s:
            self.dest = None
            return
        self.dest = tuple([int(x) for x in s.split(',')])
def addjump(obj, addjumps):
    newobj = copy.deepcopy(obj)
    if not addjumps:
        return newobj
    name = obj['name']
    if ';;' in name:
        newobj['name'] = name + ' / ' + ' / '.join(addjumps)
    else:
        newobj['name'] = name + ' ;; ' + name + ' / ' + ' / '.join(addjumps)
    return newobj
def filter_stars(jumps, allowlist=False):
    if allowlist:
        return [j[1:] for j in jumps if j[0] == '*']
    return [j for j in jumps if j[0] != '*']
def get_entry(addjumps=None):
    global mainframe
    
    name = mainframe.field_name.get()
    jumps = filter_stars(mainframe.list_jumps)
    sjumps = filter_stars(mainframe.list_jumps,True)
    print(jumps)
    print(sjumps)
    
    outname = name
    if (addjumps or sjumps) and not jumps:
        jumps = [s.strip() for s in re.split('/',name)]
    if addjumps:
        jumps = jumps + filter_stars(addjumps)
        sjumps = sjumps + filter_stars(addjumps,True)
    if jumps:
        outname += ' ;; ' + ' / '.join(jumps)
    if sjumps:
        outname += ' ;; ' + ' / '.join(sjumps)
    
    outval = mainframe.get_val().strip('\r\n \t')
    
    ret = {'name':outname,'val':outval}
    if mainframe.todo:
        ret['todo'] = True
    return ret
def entry_changed():
    global cursheet, cursec, curentry, needs_edit_save
    changed = _getsheet(cursheet)['tabs'][cursec]['lines'][curentry] != get_entry()
    local_edited(changed)
    return changed
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
def preview_entry(other=None):
    global cursheet, cursec, curentry
    if other and other[0] != -2:
        sheetnum,tabnum,linenum,*_ = other
    else:
        sheetnum = cursheet
        tabnum = cursec
        linenum = curentry
    sh = json_obj['named'] if sheetnum == -1 else json_obj['sheets'][sheetnum]
    old_e = sh['tabs'][tabnum]['lines'][linenum]
    if other:
        new_e = addjump(old_e, ['_PREVIEW_JUMP'])
    else:
        new_e = get_entry(['_PREVIEW_JUMP'])
    
    if sheetnum == -1: #named data
        json_obj['named']['tabs'][tabnum]['lines'][linenum] = new_e
        json_obj['sheets'].append(copy.deepcopy(json_obj['named']))
        json_obj['named']['tabs'][tabnum]['lines'][linenum] = get_entry()
        preview_html()
        json_obj['named']['tabs'][tabnum]['lines'][linenum] = old_e
        json_obj['sheets'] = json_obj['sheets'][:-1]
    else:
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
    global menubar,filemenu,optmenu,toolmenu,pagemenu,editmenu,root
    global bkmrkmenu,bkmrkmenu_save,bkmrkmenu_load,bkmrkmenu_clear
    menubar = Menu(root)
    filemenu = Menu(menubar, tearoff=0, postcommand=onfile)
    filemenu.add_command(label = 'New', command = new_json)
    filemenu.add_command(label = 'Load', command = loader_json)
    filemenu.add_separator()
    filemenu.add_command(label = 'Help [F1]', command = info_program)
    filemenu.add_separator()
    filemenu.add_command(label = 'Save', command = saver_json)
    filemenu.add_command(label = 'Save As', command = saver_json_as)
    filemenu.add_command(label = 'Export HTML', command = save_html)
    filemenu.add_separator()
    filemenu.add_command(label = 'Exit', command = quitter)
    menubar.add_cascade(label='File', menu=filemenu)
    editmenu = Menu(menubar, tearoff=0, postcommand=onedit)
    editmenu.add_command(label = 'Copy From Clipboard', command = copy_from_sys_clipboard)
    menubar.add_cascade(label='Edit', menu=editmenu)
    optmenu = Menu(menubar, tearoff=0, postcommand=onoption)
    optmenu.add_command(label = 'Autosave', command = opt_autosave)
    optmenu.add_checkbutton(label='Dark Theme', onvalue=1, offvalue=0, variable=_darktheme)
    menubar.add_cascade(label='Options', menu=optmenu)
    toolmenu = Menu(menubar, tearoff=0, postcommand=ontool)
    toolmenu.add_command(label = 'Broken Links', command = check_brlink)
    toolmenu.add_command(label = 'Todos', command = check_todo)
    #toolmenu.add_checkbutton(label='Dark Theme', onvalue=1, offvalue=0, variable=_darktheme)
    menubar.add_cascade(label='Tools', menu=toolmenu)
    pagemenu = Menu(menubar, tearoff=0, postcommand=onpage)
    pagemenu.add_command(label = 'Settings', command = popup_pagesetting)
    pagemenu.add_command(label = 'Named Data', command = edit_named)
    menubar.add_cascade(label='Page', menu=pagemenu)
    bkmrkmenu = Menu(menubar, tearoff=0, postcommand=onbkmrk)
    bkmrkmenu_save = Menu(bkmrkmenu, tearoff=0, postcommand=onbkmrk_save)
    bkmrkmenu_load = Menu(bkmrkmenu, tearoff=0, postcommand=onbkmrk_load)
    bkmrkmenu_clear = Menu(bkmrkmenu, tearoff=0, postcommand=onbkmrk_clr)
    for q in range(10):
        bk = bookmarks[q]
        bkmrkmenu_save.add_command(label=f'Save {q}', command = bk.save)
        bkmrkmenu_load.add_command(label=f'Load {q}', command = bk.load)
        bkmrkmenu_clear.add_command(label=f'Clear {q}', command = bk.clear)
    bkmrkmenu.add_cascade(label='Save', menu=bkmrkmenu_save)
    bkmrkmenu.add_cascade(label='Load', menu=bkmrkmenu_load)
    bkmrkmenu.add_cascade(label='Clear', menu=bkmrkmenu_clear)
    menubar.add_cascade(label='Bkmrk', menu=bkmrkmenu)
    root.config(menu=menubar)

class widgTopLevel:
    def __init__(self,title=''):
        self.root = None
        self.closers = []
        self.savers = []
        self.index = -1
        self.retval = None
        self.confirmfunc = None
        self.cancelfunc = None
        self.saved = False
        self.focused = None
        tl = Toplevel(bg=BGC)
        tl.title(title)
        tl.grab_set()
        tl.protocol("WM_DELETE_WINDOW", lambda:self.clear(False))
        tl.grid_rowconfigure(0, weight=1)
        tl.grid_columnconfigure(0, weight=1)
        tl.confirm = None
        self.root = tl
    def clear(self,save=False):
        self.saved = save
        if save:
            for saver in self.savers:
                saver()
        for closer in self.closers:
            closer()
        destroy_popup()
        self.root.grab_release()
        self.root.destroy()
        popup_manager.close(self.index)
    def confirm(self):
        if self.confirmfunc:
            self.confirmfunc()
        elif self.okb:
            self.okb.invoke()
    def cancel(self):
        if self.cancelfunc:
            self.cancelfunc()
        elif self.cb:
            self.cb.invoke()
    def launch(self,frame,savers=[],closers=[],w=400,h=150,postinit=None):
        frame.grid()
        Frame(frame,bg=BGC,width=0,height=15).pack()
        btnfr = Frame(frame,bg=BGC)
        btns = []
        wid = 10
        okb = Button(btnfr,width=wid,text='OK',command=lambda:self.clear(True))
        btns.append(okb)
        cb = Button(btnfr,width=wid,text='Cancel',command=lambda:self.clear(False))
        btns.append(cb)
        for b in btns:
            style_btn(b)
            b.pack(side='left')
        btnfr.pack()
        self.okb = okb
        self.cb = cb
        if not self.focused:
            self.focused = okb
        self.focused.focus_set()
        self.root.protocol("WM_DELETE_WINDOW", lambda *_:cb.invoke())
        self.root.bind('<Return>',lambda *_:self.confirm())
        self.root.bind('<Escape>',lambda *_:cb.invoke())
        self.root.geometry(f'{w}x{h}')
        self.savers = savers
        self.closers = closers
        rx = root.winfo_x()
        ry = root.winfo_y()
        rw = root.winfo_width()
        rh = root.winfo_height()
        self.root.geometry("+%d+%d" %(rx+(rw-w)/2,ry+(rh-h)/2))
        if postinit:
            postinit(self.root)
        self.index = popup_manager.add(self)
        while popup_manager.update(self.index):
            pass
        if not self.saved:
            return None
        return self.retval
    def setret(self,val):
        self.retval = val
    def setret_ind(self,ind,val):
        self.retval[ind] = val
class tlManager:
    def __init__(self):
        self.tls = []
    def add(self,tl)->int:
        if tl in self.tls:
            return self.get(tl)
        self.tls.append(tl)
        return len(self.tls)-1
    def get(self,tl)->int:
        if tl in self.tls:
            for q in range(len(self.tls)):
                if self.tls[q] is tl:
                    return q
        return -1
    def update(self,ind):
        if not self.tls:
            return False
        if ind < 0:
            return False
        if ind < len(self.tls)-1:
            return True #not the current popup focused
        if ind > len(self.tls)-1:
            return False #closed
        tl = self.tls[-1]
        if not tl:
            return False
        tl.root.update_idletasks()
        tl.root.update()
        return True
    def close(self,ind):
        if not self.tls:
            return
        if ind > len(self.tls)-1:
            return
        if ind < 0:
            return
        tl = self.tls[-1]
        if not tl:
            return False
        self.tls[ind] = None
        lasthas = -1
        for q in range(len(self.tls)):
            if self.tls[q]:
                lasthas = q
        if lasthas < 0:
            return
        self.tls = self.tls[:lasthas]
popup_manager = tlManager()

def pack_entry_rows(tl,frame,l:list,wid=50):
    fr = Frame(frame, bg=BGC)
    r = 0
    evars = []
    savers = []
    for t in l:
        # t is tuple(title,key)
        _lbl = Label(fr, text=t[0])
        style_label(_lbl)
        _lbl.grid(row=r,column=0,sticky=E)
        
        default = t[2] if len(t) > 2 else None
        default = getkey(t[1],default)
        tvar = StringVar(tl, default)
        
        _ent = Entry(fr, width=wid, textvariable=tvar)
        style_entry(_ent)
        _ent.grid(row=r,column=1,sticky=W)
        
        r += 1
        evars.append(tvar)
        if len(t) > 3:
            savers.append(lambda: t[3](tvar.get()))
        else:
            savers.append(lambda: setkey(t[1],tvar.get()))
    fr.pack()
    return (savers,evars)
def pack_entry_rows_2(tl,frame,l:list,wid=50):
    fr = Frame(frame, bg=BGC)
    r = 0
    evars = []
    savers = []
    objs = []
    for t in l:
        # t is tuple(title,default,lambda)
        _lbl = Label(fr, text=t[0])
        style_label(_lbl)
        _lbl.grid(row=r,column=0,sticky=E)
        
        tvar = StringVar(tl, t[1])
        
        _ent = Entry(fr, width=wid, textvariable=tvar)
        style_entry(_ent)
        _ent.grid(row=r,column=1,sticky=W)
        
        evars.append(tvar)
        r += 1
        objs += [_lbl, _ent]
    for q in range(len(evars)):
        if l[q][2]:
            savers.append(lambda: l[q][2](evars[q].get()))
    fr.pack()
    return (objs,savers,evars)
def pack_checkboxes(tl,frame,l:list):
    fr = Frame(frame, bg=BGC)
    
    ivars = []
    savers = []
    objs = []
    for t in l:
        # t is tuple(title,default,lambda)
        
        ivar = IntVar(tl, 1 if t[1] else 0)
        
        _chk = Checkbutton(fr, text=t[0], variable=ivar, onvalue=1, offvalue=0)
        style_check(_chk)
        _chk.pack()
        
        objs.append(_chk)
        ivars.append(ivar)
        if t[2]:
            savers.append(lambda: t[2](True if ivar.get() == 1 else False))
    fr.pack()
    return (objs,savers,ivars)


def getkey(key:str,default=None):
    global json_obj
    return lib._dict_get(json_obj,key,default)
def setkey(key:str,val):
    global json_obj
    json_obj[key] = val

def sel_toplvl(evt):
    global curtop
    curtop = get_sel(evt.widget)

def popup_pagesetting():
    global json_obj, file_loaded
    if not file_loaded:
        return
    popup = centered_popup(Frame(root,bg=BGC))
    f1 = build_labels(['Page Settings open...','Please Close Popup'],parent=popup)
    f1.pack()
    update_popup_size()
    
    tl = widgTopLevel('Page Settings')
    topframe = Frame(tl.root, bg=BGC)
    savers,evars = pack_entry_rows(tl.root,topframe,[
        ('Page Title:','pagetitle'),
        ('Header:','header'),
        ('Host URL:','url'),
        ('URL Text:','urltxt')])
    savers.append(mark_edited)
    tl.launch(topframe,w=400,h=150,savers=savers)

def popup_editstring(strname:str='',oldstr:str='',title:str='',info:str=None,txtwid=25):
    popup = centered_popup(Frame(root,bg=BGC))
    f1 = build_labels(['String Popup open...','Please Close Popup'],parent=popup)
    f1.pack()
    update_popup_size()
    
    if not title:
        title = 'Rename?'
    if not strname:
        strname = 'Name'
    
    tl = widgTopLevel(title)
    topframe = Frame(tl.root, bg=BGC)
    if info:
        _lbl = Label(topframe, text = info)
        style_label(_lbl)
        _lbl.pack()
    objs1,savers1,vars1 = pack_entry_rows_2(tl.root,topframe,[
        (f'{strname}:',oldstr,lambda s:tl.setret(s))],wid=txtwid)
    tl.focused = objs1[1] #Textfield
    return tl.launch(topframe,w=450,h=100,savers=savers1)
findstr = ''
replstr = ''
def popup_findrepl(info:str=None,txtwid=25,candupe=False):
    global findstr, replstr
    findmode = config['findmode']
    finddupe = config['finddupe']
    popup = centered_popup(Frame(root,bg=BGC))
    f1 = build_labels(['String Popup open...','Please Close Popup'],parent=popup)
    f1.pack()
    update_popup_size()
    
    tl = widgTopLevel('Find+Replace')
    topframe = Frame(tl.root, bg=BGC)
    if info:
        _lbl = Label(topframe, text = info)
        style_label(_lbl)
        _lbl.pack()
    tl.setret(['','',0])
    objs1,_,vars1 = pack_entry_rows_2(tl.root,topframe,[
        ('Find:',findstr,None),
        ('Replace:',replstr,None)],wid=txtwid)
    objs2,_,vars2= pack_checkboxes(tl.root,topframe,[
        ('Use Regex Search',True if findmode else False,None)])
    if candupe:
        objs3,_,vars3= pack_checkboxes(tl.root,topframe,[
            ('Duplicate instead of Replace',True if finddupe else False,None),
            ('...Add dupes to end',True if finddupe == 2 else False,None)])
    
    tl.focused = objs1[1] #Top textfield
    tl.launch(topframe,w=450,h=200)
    if tl.saved:
        findstr = vars1[0].get()
        replstr = vars1[1].get()
        findmode = vars2[0].get()
        if candupe:
            finddupe = vars3[0].get()
            if finddupe and vars3[1].get():
                finddupe = 2
            config['finddupe'] = finddupe
        config['findmode'] = findmode
        r = (findstr,replstr,findmode,finddupe if candupe else 0)
        return r
    return None
def sub_brlink(br,s,txt):
    global _sub_count
    output = txt
    escbr = re.escape(br)
    pat1 = re.compile(r'\$\{'+escbr+r'\}')
    count = len(re.findall(pat1, output))
    if count:
        output = re.sub(pat1, '${'+s+'|'+br+'}', output)
        _sub_count += count
    pat2 = re.compile(r'\$\{'+escbr+r'\|')
    count = len(re.findall(pat2, output))
    if count:
        output = re.sub(pat2, '${'+s+'|', output)
        _sub_count += count
    return output
def repl_brlink(ind):
    global brlinks, _sub_count, toplevel, toplistbox, needs_edit_save
    br = brlinks[ind]
    s = popup_editstring('Replace Links',br,'New Link',f"Replace links to {br} with links to?")
    if not s:
        return
    _sub_count = 0
    for sh in range(len(json_obj['sheets'])):
        for tab in range(len(json_obj['sheets'][sh]['tabs'])):
            for line in range(len(json_obj['sheets'][sh]['tabs'][tab]['lines'])):
                json_obj['sheets'][sh]['tabs'][tab]['lines'][line]['val'] = sub_brlink(br,s,json_obj['sheets'][sh]['tabs'][tab]['lines'][line]['val'])
    for tab in range(len(json_obj['named']['tabs'])):
        for line in range(len(json_obj['named']['tabs'][tab]['lines'])):
            json_obj['named']['tabs'][tab]['lines'][line]['val'] = sub_brlink(br,s,json_obj['named']['tabs'][tab]['lines'][line]['val'])
    if _sub_count > 0:
        mark_edited()
    total = _sub_count
    _sub_count = 0
    if isinstance(mainframe, EditEntryPage):
        if mainframe.get_ty() == 'Page':
            txt = mainframe.get_val()
            txt = sub_brlink(br,s,txt)
            mainframe.txt_body.delete('1.0',END)
            mainframe.txt_body.insert(END,txt)
    if _sub_count > 0 and needs_edit_save:
        total += _sub_count
    _sub_count = 0
    s1 = "" if total == 1 else "s"
    s2 = "a link" if total == 1 else "links"
    popInfo(f'Replaced {total} instance{s1} of {s2} to "{br}" with {s2} to "{s}"')
    toplistbox.focus_set()
def search_todo():
    global needs_edit_save
    total = 0
    todos = []
    pat = re.compile('<todo>(.+)</todo>')
    for sh in range(len(json_obj['sheets'])):
        for tab in range(len(json_obj['sheets'][sh]['tabs'])):
            for line in range(len(json_obj['sheets'][sh]['tabs'][tab]['lines'])):
                val = json_obj['sheets'][sh]['tabs'][tab]['lines'][line]['val']
                count = len(re.findall(pat, val))
                if count:
                    re.sub(pat, lambda m:todos.append((sh,tab,line,m.group(1))), val)
                    total += count
    for tab in range(len(json_obj['named']['tabs'])):
        for line in range(len(json_obj['named']['tabs'][tab]['lines'])):
            val = json_obj['named']['tabs'][tab]['lines'][line]['val']
            count = len(re.findall(pat, val))
            if count:
                re.sub(pat, lambda m:todos.append((-1,tab,line,m.group(1))), val)
                total += count
    if isinstance(mainframe, EditEntryPage) and needs_edit_save:
        if mainframe.get_ty() == 'Page':
            val = mainframe.get_val()
            count = len(re.findall(pat, val))
            if count:
                re.sub(pat, lambda m:todos.append((-2,-2,-2,m.group(1))), val)
                total += count
    return todos
def reload_brlist():
    global toplistbox, toplevel
    toplevel.okb.invoke()
    check_brlink()
def popup_brlink():
    global json_obj, file_loaded, toplistbox, curtop, brlinks
    if not file_loaded:
        return
    popup = centered_popup(Frame(root,bg=BGC))
    f1 = build_labels(['Broken Link tool open...','Please Close Popup'],parent=popup)
    f1.pack()
    update_popup_size()
    
    tl = widgTopLevel('Broken Links')
    topframe = Frame(tl.root, bg=BGC)
    
    if not brlinks:
        _lbl = Label(topframe, text="No Broken Links Found!")
        style_label(_lbl)
        _lbl.pack()
    else:
        curtop = 0
        row = Frame(topframe, bg=BGC)
        fr = Frame(row, bg=BGC)
        toplistbox = Listbox(fr)
        style_lb(toplistbox)
        toplistbox.bind('<<ListboxSelect>>', sel_toplvl)
        toplistbox.bind('<Double-1>', lambda *_:repl_brlink(curtop))
        pack_scrollable_listbox(toplistbox)
        toplistbox.focus_set()
        _load_list(toplistbox, 0, brlinks, lambda a:a)
        fr.pack(side='left')
        fr = Frame(row, bg=BGC)
        wid = 10
        btn = Button(fr, width=wid, text='Refresh', command=reload_brlist)
        style_btn(btn)
        btn.pack()
        btn = Button(fr, width=wid, text='Replace', command=lambda *_:repl_brlink(curtop))
        style_btn(btn)
        btn.pack()
        fr.pack(side='left')
        row.pack()
    tl.launch(topframe,w=250,h=250,savers=[])
    destroy_popup()

def goto_todo(todo):
    global toplevel
    sh,tab,line,txt = todo
    toplevel.okb.invoke()
    if (sh,tab,line) == (-2,-2,-2):
        return #exit w/o navigation
    navi.visit_new((sh,tab,line,0))
def preview_todo(todo):
    global toplevel
    goto_todo(todo)
    preview_entry(todo)
def popup_todo(todos):
    global json_obj, file_loaded, toplistbox, curtop
    if not file_loaded:
        return
    popup = centered_popup(Frame(root,bg=BGC))
    f1 = build_labels(['Todos tool open...','Please Close Popup'],parent=popup)
    f1.pack()
    update_popup_size()
    
    tl = widgTopLevel('Todos')
    topframe = Frame(tl.root, bg=BGC)
    
    if not todos:
        _lbl = Label(topframe, text="No Todos Found!")
        style_label(_lbl)
        _lbl.pack()
    else:
        curtop = 0
        row = Frame(topframe, bg=BGC)
        fr = Frame(row, bg=BGC)
        toplistbox = Listbox(fr, width = 50)
        style_lb(toplistbox)
        toplistbox.bind('<<ListboxSelect>>', sel_toplvl)
        tl.confirmfunc = lambda *_:goto_todo(todos[curtop])
        toplistbox.bind('<Double-1>', tl.confirmfunc)
        pack_scrollable_listbox(toplistbox)
        toplistbox.focus_set()
        _load_list(toplistbox, 0, [x[3] for x in todos], lambda a:a)
        fr.pack(side='left')
        fr = Frame(row, bg=BGC)
        wid = 10
        btn = Button(fr, width=wid, text='Go To', command=tl.confirmfunc)
        style_btn(btn)
        btn.pack()
        btn = Button(fr, width=wid, text='Preview', command=lambda *_:preview_todo(todos[curtop]))
        style_btn(btn)
        btn.pack()
        fr.pack(side='left')
        row.pack()
    tl.launch(topframe,w=450,h=250,savers=[])

read_config()
root = Tk()
style = ttk.Style(root)
root.config(bg=BGC)
update_file(args.inputfile) #Update title to include loaded file
root.geometry('1100x400')
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
