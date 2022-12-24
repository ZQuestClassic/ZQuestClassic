import docjson_lib as lib
import argparse
from tkinter import *
from tkinter import filedialog
from tkinter.messagebox import askyesno

parser = argparse.ArgumentParser()
parser.add_argument('--in', dest='inputfile', default='zsdocs_new.json',metavar='FILE',
    help='.json to generate docs from')
parser.add_argument('--out', dest='outputfile', default='ZScript_Docs.html',metavar='FILE',
    help='HTML webpage output')
parser.add_argument('--db','-d', action='store_true', help='Print debug info')
parser.add_argument('--edit','-e', action='store_true', help='Open Editor')
args = parser.parse_args()

lib.debug_out = args.db

json_obj = lib.loadjson(args.inputfile)
if not args.edit:
    lib.savehtml(args.outputfile,json_obj)
    exit(0)

needs_save = False

## GUI functions
def loader_json():
    global json_obj, root, needs_save
    fname = filedialog.askopenfilename(parent = root, title = 'Load', initialfile = args.inputfile, filetypes = (('Json','*.json'),))
    if len(fname) < 1:
        return
    args.inputfile = fname
    json_obj = lib.loadjson(args.inputfile)
    needs_save = False
def saver_json():
    global json_obj, needs_save
    if len(args.inputfile) < 1:
        saver_json_as()
        return
    lib.savejson(args.inputfile,json_obj)
    needs_save = False
def saver_json_as():
    global json_obj, root, needs_save
    fname = filedialog.asksaveasfilename(parent = root, title = 'Save As', initialfile = args.inputfile, filetypes = (('Json','*.json'),),defaultextension = '.json')
    if len(fname) < 1:
        return
    args.inputfile = fname
    lib.savejson(args.inputfile,json_obj)
    needs_save = False
def saver_html():
    global json_obj, root
    fname = filedialog.asksaveasfilename(parent = root, title = 'Export HTML', initialfile = args.outputfile, filetypes = (('HTML','*.html'),),defaultextension = '.html')
    if len(fname) < 1:
        return
    args.outputfile = fname
    lib.savehtml(args.outputfile,json_obj)
def quitter():
    global root, needs_save
    if needs_save:
        if not askyesno(parent=root, title = 'Exit without saving?', message = 'Changes to the .json have not been saved!'):
            return
    root.destroy()
    exit(0)
def edited():
    global needs_save
    needs_save = True
## End GUI functions

root = Tk()
root.title('ZS Docs Editor')
root.geometry('640x480')
# Button(root, text = "Load", command = loader_json).grid(row=0,column=0,sticky=N)
# Button(root, text = "Save", command = saver_json).grid(row=1,column=0,sticky=N)
# Button(root, text = "Save As", command = saver_json_as).grid(row=2,column=0,sticky=N)
# Button(root, text = "Save HTML", command = saver_html).grid(row=3,column=0,sticky=N)
menubar = Menu(root)
filemenu = Menu(menubar, tearoff=0)
filemenu.add_command(label = 'Load', command = loader_json)
filemenu.add_command(label = 'Save', command = saver_json)
filemenu.add_command(label = 'Save As', command = saver_json_as)
filemenu.add_command(label = 'Save HTML', command = saver_html)
filemenu.add_separator()
filemenu.add_command(label = 'Exit', command = quitter)
menubar.add_cascade(label='File', menu=filemenu)
root.config(menu=menubar)
root.protocol("WM_DELETE_WINDOW", quitter)
root.mainloop()
