import json
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--in', dest='inputfile', default='zsdocs.json',metavar='FILE',
    help='Old-style .json to convert')
parser.add_argument('--out', dest='outputfile', default='zsdocs_new.json',metavar='FILE',
    help='New-style .json to output')
parser.add_argument('--min', action='store_true', help='Minified output')
args = parser.parse_args()

with open(args.inputfile, 'r') as file:
    jstring = file.read();

#Old-style doc object from the JS
#'vals': list (of list (of obj{name:str,val:str}))
#'sheet_sz': list (of int, number of objects for each sheet)
#'sheet_nm': list (of str, name of each sheet)
jsonobj = json.loads(jstring);

newobj = {
    'key': 'zs_docjson_py',
    'ver': 0,
    'sheets': [],
    'named': {'name':'Named_Data', 'tabs':[{'name':v['name'],'lines':v['val']} for v in jsonobj['named']]}
}

sheetind = 0;
for i in range(len(jsonobj['sheet_nm'])):
    sname = jsonobj['sheet_nm'][i];
    ssz = jsonobj['sheet_sz'][i];
    if ssz < 1: #Skip empty sheets
        continue;
    slist = [];
    for q in range(ssz):
        slist.append({'name':str(q), 'lines':jsonobj['vals'][sheetind+q]});
    sheetind += ssz;
    newobj['sheets'].append({'name': sname, 'tabs':slist});

#for sheet in newobj['sheets']:
#    for tab in sheet['tabs']:
#        for line in tab:
#            print(line['name']);

with open(args.outputfile, 'w') as file:
    if args.min:
        file.write(json.dumps(newobj));
    else:
        file.write(json.dumps(newobj, indent=4));

