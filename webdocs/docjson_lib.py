import json, re, math, sys
from datetime import date
from datetime import datetime
import pytz

def _dict_get(d:dict,s:str,default):
    try:
        return d[s]
    except:
        if default: #Set the default value
            d[s] = default
        return default
def _dict_del(d:dict,s:str):
    d.pop(s,None)

def loadjson(fname):
    global data_obj, debug_out
    parse_state('Loading Json...')
    with open(fname, 'r') as file:
        data_obj = json.loads(file.read())
    parse_state('Validating Json...')
    _key = '[MISSING]'
    try:
        _key = data_obj['key']
    except:
        pass
    if _key != 'zs_docjson_py':
        parse_state('FAIL!')
        raise Exception(f'Invalid .json key: {_key}')
    else:
        parse_state('Done!')
        return data_obj
def savejson(fname,obj):
    parse_state('Saving Json...')
    with open(fname, 'w') as file:
        file.write(json.dumps(obj, indent=4))
    parse_state('Done!')
def savehtml(fname,obj):
    outstr = generate_output(obj)
    parse_state('Writing output file...')
    with open(fname, 'w') as file:
        file.write(outstr)
    parse_state('Done!')
#data_obj has:
#'sheets': list of objects having:
#    'name': str, the sheet name
#    'tabs': list of objects having:
#        'name': str, the line name
#        'val': str, the content
#'named': list of objects having:
#    'title': str, the section title
#    'tabs': list of objects having:
#        'name': str, the name
#        'val': str, the content

#Called on program fatal error
def _getloc():
    global cur_body
    loc = ''
    if cur_body:
        loc = f"At '{cur_body.location()}':\n -"
    return loc
def parse_fail(msg):
    raise Exception(f'{_getloc()}FATAL: {msg}')
def parse_warn(msg):
    global parse_warnings, cur_body
    parse_warnings.append(_getloc()+msg)
#Called to print state information
def parse_state(msg):
    if debug_out:
        print(f'[ST] {msg}')

##Utility Functions
_pat_tags = re.compile('(<([^>]+)>)')
_pat_htmltag = re.compile('<\\/?[a-zA-Z0-9_]+ *((([a-zA-Z0-9_\\-]+)|(=)|(\'[^\\\']*\')|("[^\\"]*")) *)*\\/?>')
def _namestrip(s):
    return re.sub('$','',s).upper().strip()
def findNamed(title,name):
    global data_obj
    notitle = title is None
    if notitle:
        title = 'MISC'
    rep = f"'{name}'" if notitle else f"'{title},{name}'"
    _title = _namestrip(title)
    _name = _namestrip(name)
    for section in data_obj['named']['tabs']:
        if _namestrip(section['name']) != _title:
            continue
        #The specified section
        for line in section['lines']:
            if _namestrip(line['name']) == _name:
                return line['val']
        break
    else:
        if not notitle:
            parse_warn(f"Named data section '{title}' does not exist!")
    #Not found in specified section
    #Search all sections as a fallback
    for section in data_obj['named']['tabs']:
        for line in section['lines']:
            if _namestrip(line['name']) == _name:
                return line['val']
    #Not found anywhere, error
    parse_warn(f"Named data {rep} was not found!")
    return f"<todo>[NO DATA {rep}]</todo>"
def trimtags(string:str) -> str:
    return re.sub(_pat_tags, '', string)
def update_date():
    curdate = date.today().strftime('%B %d, %Y')
    curtime = datetime.now(pytz.timezone('America/New_York')).strftime('%H:%M')
    return f'{curdate} {curtime} EST'
def findBodyTab(name):
    global sz_tabs
    _tname = name.strip().lower()
    for curtab in range(sz_tabs):
        tab = get_tab_global(curtab)
        for line in tab['lines']:
            comp = trimtags(line['name']).lower()
            spl = re.split('/|;;',comp)
            for s in spl:
                if s.strip() == _tname:
                    return (line['linktab'],line['body'])
    return (-1,-1)
def get_line_display(line) -> str:
    name = line['name']
    try:
        s = trimtags(name).split(';;',2)[0].strip()
        return s if s else '[NONAME]'
    except:
        return '[??]'
def broken_link(id):
    global broken_links
    if id not in broken_links:
        broken_links.append(id)
def enquote(s):
    single = False
    double = False
    for c in s:
        if c == '"':
            double = True;
            if single:
                break
        elif c == "'":
            single = True;
            if double:
                break
    if single and double:
        _s = re.sub('"',"'", s)
        return f'"{_s}"'
    if single:
        return f'"{s}"'
    return f"'{s}'"
##Body Parsing Functions
class _tk_int_data:
    def __init__(self,ind,dat):
        self.ind = ind
        self.val = dat
class tkdata:
    def __init__(self):
        self.val = []
        self.token = 0
        self.tkind = 0
    
    def _fillto(self,index):
        while len(self.val) <= index:
            self.val.append(_tk_int_data(-1,''))
    
    def get(self,index):
        self._fillto(index)
        return self.val[index]
    
    def set(self,index,i:int,val:str):
        self._fillto(index)
        self.val[index] = _tk_int_data(i,val)
        return self.val[index]
    
    def getval(self,index):
        return self.val[index].val
    
    def replace(self,string,repl):
        return string[:self.val[0].ind] + repl + string[self.val[-1].ind+len(self.val[-1].val):]

class LeveledObj:
    def __init__(self,s:str,tok:list,insens:bool,nowarn:bool=False,htmlwarn:bool=False):
        self.string = s
        self.tokens = tok
        self.insens = insens
        self.html_lvl = 0
        self.nest_lvl = 0
        self.ind = 0
        self.token_data = []
        self.nowarn = nowarn
        self.htmlwarn = htmlwarn
        self.htmltags = []
        self.htmlindx = -1
        
    def comparestr(self,s1:str,s2:str):
        if self.insens:
            return s1.lower() == s2.lower()
        return s1 == s2
    
    def replacer(obj, replfunc) -> str:
        obj.db = cur_body.sheetid == 0 and cur_body.tabid == 0  and cur_body.lineid == 0 and obj.htmlwarn
        if obj.db:
            print(obj.string)
        while obj.ind < len(obj.string):
            if len(obj.token_data) < obj.html_lvl+1:
                obj.token_data.append(tkdata())
            tkobj = obj.token_data[obj.html_lvl]
            curtoken = obj.tokens[tkobj.token]
            _strslice = obj.string[obj.ind:obj.ind+len(curtoken)]
            if obj.comparestr(_strslice,curtoken): #Found the current token
                if tkobj.tkind > 0:
                    _dat = tkobj.get(tkobj.tkind-1)
                    _dat.val = obj.string[_dat.ind:obj.ind]
                tkobj.set(tkobj.tkind,obj.ind,_strslice)
                tkobj.tkind += 2
                tkobj.token += 1
                _reset = False
                if tkobj.token == len(obj.tokens):
                    _l = len(obj.string)
                    obj.string = replfunc(obj.string, tkobj)
                    _l2 = len(obj.string)-_l
                    obj.ind += _l2
                    #Reset state, continue parsing ('replace all')
                    obj.token_data[obj.html_lvl] = tkdata()
                    _reset = True
                    tkobj = obj.token_data[obj.html_lvl]
                obj.ind += len(_strslice)
                if tkobj.tkind > 0:
                    _dat = tkobj.set(tkobj.tkind-1,obj.ind,'')
                if _reset:
                    if obj.nest_lvl > 0:
                        # A nested end
                        obj.nest_lvl -= 1
                        obj.down_level()
                continue #End 'found the current token'
            elif tkobj.token > 0 and obj.comparestr(_strslice,obj.tokens[0]):
                #Found a nested start?
                obj.nest_lvl += 1 #Track number of nests
                obj.html_lvl += 1 #Reuse this
                continue #Don't increment obj.ind
            elif _strslice[0] == '<': #Found an html tag?
                squote = False
                dquote = False
                tstr = ''
                last_c = ''
                q = 0
                while obj.ind + q < len(obj.string):
                    _c = obj.string[obj.ind+q]
                    tstr += _c
                    if not dquote and _c == '\'' and last_c != '\\':
                        squote = not squote
                    elif not squote and _c == '"' and last_c != '\\':
                        dquote = not dquote
                    last_c = _c
                    if not squote and not dquote and _c == '>':
                        break #Close html tag
                    q += 1
                if obj.db:
                    print(tstr)
                if re.match(_pat_htmltag,tstr):
                    if len(tstr) > 1 and tstr[1] == '/':
                        if obj.db:
                            print(1,tstr)
                        obj.down_level()
                        if not obj.htmltags and obj.htmlwarn:
                            parse_warn(f'HTML CLOSE WITHOUT OPEN {tstr}')
                        elif obj.htmltags:
                            ind = tstr.find(' ')
                            xstr = tstr[2:-1] if ind < 0 else tstr[2:ind]
                            xlist = [x[1] for x in obj.htmltags]
                            if xstr not in xlist:
                                #Never was opened
                                if obj.htmlwarn:
                                    parse_warn(f'HTML CLOSE WITHOUT OPEN {tstr}')
                            else:
                                while obj.htmlindx > -1 and xstr != obj.htmltags[obj.htmlindx][1]:
                                    if obj.htmlwarn:
                                        parse_warn(f'UNCLOSED HTML TAG {obj.htmltags[obj.htmlindx][0]}')
                                    obj.htmlindx -= 1
                                    del obj.htmltags[-1]
                                if obj.htmlindx > -1: #always true?
                                    obj.htmlindx -= 1
                                    del obj.htmltags[-1]
                        if obj.html_lvl < 0:
                            obj.html_lvl = 0
                    elif len(tstr) > 2 and tstr[-1] == '/':
                        if obj.db:
                            print(2)
                        #self-closing tag
                        pass
                    else:
                        if obj.db:
                            print(3)
                        obj.html_lvl += 1
                        obj.htmlindx += 1
                        ind = tstr.find(' ')
                        xstr = tstr[1:-1] if ind < 0 else tstr[1:ind]
                        tags = (tstr,xstr)
                        if obj.db:
                            print(f'tags[{obj.htmlindx}] = {tags}')
                        if len(obj.htmltags) > obj.htmlindx:
                            obj.htmltags[obj.htmlindx] = tags
                        else:
                            obj.htmltags.append(tags)
                    obj.ind += len(tstr)
                    continue
            else: #Something else?
                _reset = False
                for tk in range(obj.token_data[obj.html_lvl].token+1,len(obj.tokens)):
                    _strslice2 = obj.string[obj.ind:obj.ind+len(obj.tokens[tk])]
                    if obj.comparestr(_strslice2,obj.tokens[tk]): #Token found out-of-order! Reset!
                        #obj.token_data[obj.html_lvl].token = 0
                        obj.down_level(nowarn=True)
                        if obj.nest_lvl > 0:
                            obj.nest_lvl -= 1
                        else:
                            obj.html_lvl += 1
                        _reset = True
                        break
                if _reset:
                    obj.ind += len(_strslice2)
                    continue
            obj.ind += 1
        tkobj = obj.token_data[obj.html_lvl]
        if obj.htmltags and obj.htmlwarn:
            parse_warn(f'UNCLOSED HTML TAGS {obj.htmltags}')
        if obj.nest_lvl > 0:
            #obj.nest_lvl -= 1
            obj.down_level()
        return obj.string
    def down_level(self,nowarn=False):
        if self.html_lvl < 0:
            return
        if not nowarn:
            tkobj = self.token_data[self.html_lvl]
            if tkobj.token > 0 and not self.nowarn:
                parse_warn(f'UNCLOSED TOKEN {tupstr(tuple(self.tokens[:tkobj.token]))} found, {tupstr(tuple(self.tokens[tkobj.token:]))} missing.')
        self.token_data[self.html_lvl] = tkdata()
        self.html_lvl -= 1
def tupstr(t:tuple)->str:
    if len(t) == 1:
        return f"'{t[0]}'"
    return str(t)
def match_leveled(s:str, tok:list, replfunc, insens:bool,nowarn:bool=False,htmlwarn:bool=False):
    tmp = LeveledObj(s,tok,insens,nowarn=nowarn,htmlwarn=htmlwarn)
    return tmp.replacer(replfunc)
def repl_codeSingle(string,tkobj):
    return tkobj.replace(string,f'<span class = "code1">{tkobj.getval(1)}</span>')
def repl_codeMulti(string,tkobj):
    return tkobj.replace(string,f'<span class = "code2">{tkobj.getval(1)}</span>')
def parseLink(id,display):
    dat = findBodyTab(id)
    if dat[0] > -1 or dat[1] > -1:
        return f"<a class = 'link' data-link = '{id}'>{display}</a>"
    broken_link(id)
    return f"<a class = 'deadlink ttip'>{display}<span class = 'ttt'>Under Construction!</span></a>"
def repl_links_1(string,tkobj):
    return tkobj.replace(string,parseLink(tkobj.getval(1),tkobj.getval(3)))
def repl_links_2(string,tkobj):
    return tkobj.replace(string,parseLink(tkobj.getval(1),tkobj.getval(1)))
def parseTTip(display,ttip):
    if ttip[0] == '$':
        ttip = findNamed('TTIPS', ttip[1:])
    ttip = parseBody(ttip)
    if ttip.find('<a class = "ttip">') > -1:
        parse_fail(f"Tooltip '{display}' contains other tooltips!")
    return f'<a class = "ttip">{display}<span class = "ttt">{ttip}</span></a>'
def repl_ttip_1(string,tkobj):
    return tkobj.replace(string,parseTTip(tkobj.getval(1),tkobj.getval(3)))
def repl_ttip_2(string,tkobj):
    return tkobj.replace(string,parseTTip(tkobj.getval(1),f'${tkobj.getval(1)}'))
def repl_named_1(string,tkobj):
    return tkobj.replace(string,parseBody(findNamed(tkobj.getval(1),tkobj.getval(3))))
def repl_named_2(string,tkobj):
    return tkobj.replace(string,parseBody(findNamed(None,tkobj.getval(1))))
def parseSpoiler(tip,body):
    return f"<span class = 'spoiler' showtext = {enquote(tip)}><span class = 'showbutton' data-active=0></span><span class = 'spoilbody' style = 'display:none' hidden>{parseBody(body)}</span></span>"
def repl_spoil_1(string,tkobj):
    return tkobj.replace(string,parseSpoiler(tkobj.getval(1),tkobj.getval(3)))
def repl_spoil_2(string,tkobj):
    return tkobj.replace(string,parseSpoiler('',tkobj.getval(1)))

def parseBody(s:str) -> str:
    global main_body
    mb = main_body
    main_body = False
    while True:
        s = match_leveled(s,['${','|','}'],repl_links_1,True,htmlwarn=mb)
        s = match_leveled(s,['${','}'],repl_links_2,True,nowarn=True)
        o_str = s
        s = match_leveled(s,['[[','|',']]'],repl_ttip_1,True)
        s = match_leveled(s,['[[$',']]'],repl_ttip_2,True)
        if o_str != s:
            continue
        s = match_leveled(s,['#{','|','}'],repl_spoil_1,True)
        s = match_leveled(s,['#{','}'],repl_spoil_2,True,nowarn=True)
        s = match_leveled(s,['```','```'],repl_codeMulti,True)
        s = match_leveled(s,['`','`'],repl_codeSingle,True)
        o_str = s
        s = match_leveled(s,['$[','|',']'],repl_named_1,True)
        s = match_leveled(s,['$[',']'],repl_named_2,True,nowarn=True)
        if o_str != s:
            continue
        return s
##
def get_tab_global(global_ind):
    global sheets
    for sheetind in range(len(sheets)):
        sheet = sheets[sheetind]
        if global_ind < sheet['tabind'] + len(sheet['tabs']):
            return sheet['tabs'][global_ind-sheet['tabind']]
def get_tab_sheet(global_ind):
    global sheets
    for sheetind in range(len(sheets)):
        sheet = sheets[sheetind]
        if global_ind < sheet['tabind'] + len(sheet['tabs']):
            return sheetind

def get_sheet(sheetname:str) -> int:
    global sheets
    try:
        return int(sheetname)
    except:
        pass
    for s in range(len(sheets)):
        if sheets[s]['name'] == sheetname:
            return s;

class gen_body:
    def __init__(self,sheetid,tabid,lineid,text):
        self.text = text
        self.sheetid = sheetid
        self.tabid = tabid
        self.lineid = lineid
    def __str__(self):
        return self.text
    def location(self):
        global data_obj
        sh = data_obj['sheets'][self.sheetid]
        tb = sh['tabs'][self.tabid]
        li = tb['lines'][self.lineid]
        return f"{sh['name']}->{tb['name']}->{get_line_display(li)}"

cur_body = None
def generate_output(obj) -> str:
    global data_obj, debug_out, sheets, sz_tabs, broken_links, parse_warnings, cur_body, main_body
    parse_warnings = []
    data_obj = obj
    sz_tabs = 0
    sheets = data_obj['sheets']

    for sheet in sheets:
        sheet['tabind'] = sz_tabs;
        sz_tabs += len(sheet['tabs'])
    
    broken_links = []
    generated_bodies = []
    generated_tabs = []


    parse_state('Generating bodies...')
    for curtab in range(sz_tabs):
        tab = get_tab_global(curtab)
        for lind in range(len(tab['lines'])):
            line = tab['lines'][lind]
            line['body'] = -1
            _val = line['val']
            if not _val or _val[0] == '$':
                continue; #no body to generate for this line
            bid = len(generated_bodies)
            line['body'] = bid
            shind = get_tab_sheet(curtab)
            tind = curtab-sheets[shind]['tabind']
            generated_bodies.append(gen_body(shind,tind,lind,text=f'\t\t<div class = "cntnt" data-bid = {bid} data-tid = -1 hidden>{_val}</div>\n'))

    parse_state('Generating tabs...')
    for curtab in range(sz_tabs):
        tab = get_tab_global(curtab)
        cursheetind = get_tab_sheet(curtab)
        hid_content = ' hidden style = "display:none"' if curtab > 0 else ''
        tab_content = f'\t\t<div class = "tab_container" data-bid = -1 data-tid = {curtab}{hid_content}>\n';
        for line in tab['lines']:
            line['linktab'] = curtab
            _name = line['name']
            _val = line['val']
            _body = line['body']
            disp_name = _name.split(";;",1)[0];
            if _val and _val[0] == '$': #Non-data
                jstr = _val[1:].split('$')
                sheet_ind = get_sheet(jstr[0])
                if sheet_ind < 0:
                    sheet_ind = cursheetind;
                tabid = 0
                if len(jstr) > 1:
                    tabid = int(jstr[1])
                tabid += sheets[sheet_ind]['tabind']
                line['linktab'] = tabid
                targbody = -1
                if len(jstr) > 2:
                    targbody = int(jstr[2])
            else: #Data
                targbody = _body
                tabid = -1
            tab_content += f'\t\t\t<p class = "tab" data-jump = "{trimtags(_name).strip()}" data-bid = {targbody} data-tid = {tabid}>{disp_name}</p>\n'
            #end foreach line
        tab_content += "\t\t</div>\n"
        generated_tabs.append(tab_content);
        #end foreach tab

    parse_state('Parsing special body code...')
    for ind in range(len(generated_bodies)):
        cur_body = generated_bodies[ind]
        main_body = True
        generated_bodies[ind].text = parseBody(generated_bodies[ind].text)
    cur_body = None
    parse_state('Generating final output...')

    generated_data = ''
    for tab in generated_tabs:
        generated_data += tab;
    for body in generated_bodies:
        generated_data += body.text;

    generated_data = re.sub('<block>',"<span class='block'>",generated_data)
    generated_data = re.sub('<iblock>',"<span class='iblock'>",generated_data)
    generated_data = re.sub('<todo>',"<span class='todo'><strong>TODO:</strong> ",generated_data)
    generated_data = re.sub('</(i?block|todo)>',"</span>",generated_data)

    css = """	<style>
		:root
		{
			--page-font: sans-serif;
			--spoiler-font-scale: 80%;
			--lineheight: 1.2;
			--ttip-color: #0078A3;
			--ttip-hover-color: #F00;
			--ttip-bg: rgba(134, 243, 168, 1);
			--button-grad: var(--button-grad);
			font-family: var(--page-font);
		}
		a, abbr, acronym, address, applet, b, big, blockquote, body, center, cite, code, dd, del, dfn, div, dl, dt, em, fieldset, font, form, h1, h2, h3, h4, h5, h6, html, i, iframe, img, ins, kbd, label, legend, li, object, ol, p, pre, q, s, samp, small, span, strike, strong, sub, sup, tt, u, ul, var {
			margin: 0;
			padding: 0;
			border: 0;
			outline: 0;
			font-size: 100%;
			-ms-text-size-adjust: 100%;
			-webkit-text-size-adjust: 100%;
			vertical-align: baseline;
			background: transparent;
		}
		.pagecont
		{
			margin:0 8px;
			line-height: var(--lineheight);
			font-family: var(--page-font);
		}
		.pagecont strong
		{
			font-weight:600;
		}
		main.pagecont
		{
			margin: auto 5%;
		}
		.pagecont .link
		{
			color:#00F;
			cursor:pointer;
			font-weight: bold;
			border-bottom: 1px dashed #00F;
		}
		.pagecont .tab
		{
			text-align:center;
			cursor:pointer;
			color:#00F;
			font-weight:600;
			white-space:nowrap;
			padding:8px;
			margin:0;
		}
		header.pagecont
		{
			text-align:center;
			padding: 10px 0;
		}
		.pagecont .cntnt
		{
			text-align:left;
			padding: 1em;
			align-self:normal;
			width: 100%;
			font-size: 13pt;
			white-space: pre-wrap;
			border:1px solid rgba(0,0,0,1);
			background: #F2FFF2;
		}
		.pagecont .tab_container
		{
			border:1px solid rgba(0,0,0,1);
			background: linear-gradient(90deg, #EEE, #EEE, #EEE, #CCC);
			display:flex;
			flex-flow: column nowrap;
			margin:0;
			padding: 0 0 8px 0;
			align-self:start;
		}
		.pagecont .panel {
			background-color: rgba(255, 255, 255, 1);
			display: flex;
			flex-flow: row nowrap;
			padding: 10px 0 30px 0;
			margin: 0;
			align-items: start;
			align-self: start;
			width: 100%;
		}
		.pagecont .ttip
		{
			position: relative;
			display: inline-block;
			border-bottom: 1px dotted var(--ttip-color);
			color: var(--ttip-color);
			font-weight: bold;
			cursor: pointer;
		}
		.pagecont .ttip:hover
		{
			border-bottom: 1px dotted var(--ttip-hover-color);
			color: var(--ttip-hover-color);
		}

		.pagecont .ttip:hover > .ttt
		{
			display:block;
		}

		.pagecont .ttt
		{
			position: fixed;
			white-space: pre-wrap;
			display: none;
			background-color: var(--ttip-bg);
			border: 1px solid black;
			color: #000;
			padding: 5px;
			border-radius: 12px;
			z-index: 1;
			font-size: 13pt;
			font-weight: initial;
			-webkit-user-select: none;
			-khtml-user-select: none;
			-moz-user-select: none;
			-ms-user-select: none;
			-o-user-select: none;
			user-select: none;
			cursor: default;
		}
		.pagecont .deadlink
		{
			color: #F00;
			cursor: default;
			border-bottom: 1px dashed #F00;
		}

		.pagecont sup {
			vertical-align: super;
			font-size: 75%;
			position: static;
		}
		.pagecont sub {
			vertical-align: sub;
			font-size: 75%;
		}

		.pagecont h1 {
			font-size: 2em;
			font-weight: bold;
		}

		.pagecont h2 {
			font-size: 1.5em;
			font-weight: bold;
			margin: 0;
			padding: 0;
			display: inline-block;
		}

		.pagecont h3 {
			font-size: 1.17em;
			font-weight: bold;
			margin: 0;
			padding: 0;
			display: inline-block;
		}

		.pagecont ul, .pagecont ol {
			padding: 0 0 0 20px;
			white-space: normal;
			display: inline-block;
			margin: 0;
			line-height: var(--lineheight);
			list-style-position: outside;
		}
		.pagecont .vers {
			font-weight: normal;
			font-size: 10pt;
			color: #00C000;
			padding: 0;
			display: none;
		}
		.pagecont li::marker {
			font-weight: bold;
		}
		.pagecont li {
			margin: 0 0 0 10px;
			white-space: pre-wrap;
		}
		.pagecont .code2 {
			display: inline-block;
			padding: 4px 16px;
			border: 1px solid black;
		}
		.pagecont .code1, .pagecont .code2 {
			font-family: monospace;
			background-color: #FFF;
			border: 1px solid black;
			padding: 0 5px;
		}
		.pagecont .code2 {
			padding: 10px;
		}

		.pagecont .spoiler::before {
			content: attr(showtext) '  ';
			font-size: var(--spoiler-font-scale);
		}

		.pagecont .spoiler {
			display:block;
			border: 1px solid black;
			padding: 2px 20px 4px;
			background: linear-gradient(90deg, #DDD, #DDD, #DDD, #DDD, #DDD, #DDD, #AAA);
		}

		.pagecont .spoilbody {
			display: block;
			border-top: 1px solid black;
			padding: 5px 0 0 0;
		}

		.pagecont .showbutton {
			margin-bottom: 3px;
			font-size: var(--spoiler-font-scale);
			-webkit-user-select: none;
			-khtml-user-select: none;
			-moz-user-select: none;
			-ms-user-select: none;
			-o-user-select: none;
			user-select: none;
			border: 1px black solid;
			padding: 0 4px;
			cursor: pointer;
			display: inline-block;
			white-space: pre;
			text-align: center;
			background: linear-gradient(170deg, #FFF, #AAA);
		}

		.pagecont .showbutton[data-active='0']::before {
			content: 'Show More';
		}
		.pagecont .showbutton[data-active='1']::before {
			content: 'Show Less';
		}
		.pagecont .block {
			display: block;
			white-space: pre;
		}
		.pagecont .iblock {
			display: inline-block;
			white-space: pre;
		}
		*[data-iblock] {
			display: inline-block;
		}
		*[data-block] {
			display: block;
		}
		.pagecont .todo {
			color: #0C0;
		}
		#searchprev {
			position: absolute;
			display: flex;
			flex-flow: column nowrap;
			border: black 1px solid;
			padding: 1px;
			color: black;
			background-color: white;
			z-index: 1000;
		}
		#searchprev p {
			padding-left: 2px;
		}
		#searchprev .searchsel {
			color: white;
			background-color: #00C;
		}
		#searchprev .insearch {
			color: red;
			font-style: italic;
		}
		.pagecont .ttt .code1, .pagecont .ttt .code2,
		#ttipmodal .code1, #ttipmodal .code2 {
			background-color: rgba(114, 223, 148, 1);
		}
		.pagecont .modal {
			display: none;
			position: fixed;
			z-index: 5;
			left: 0;
			top: 0;
			width: 100%;
			height: 100%;
			overflow: auto;
			background-color: #AAA; /* fallback */
			background-color: rgba(0,0,0,0.4); /* transparent black */
		}
		#ttipmodal .modal_content {
			background-color: var(--ttip-bg);
			margin: 15% auto;
			padding: 20px;
			border: 1px solid #888;
			width: 80%;
		}
		#modalcontent {
			white-space: pre-wrap;
		}
		.pagecont .closemodal {
			color: #aaa;
			float: right;
			font-size: 28px;
			font-weight: bold;
		}
		.pagecont .closemodal:hover, .pagecont .closemodal:focus {
			color: black;
			text-decoration: none;
			cursor: pointer;
		}
	</style>"""
    js = """<script>
	let verboseLog = false;
	let urlParams = new URLSearchParams(window.location.search);

	let searchbar = document.getElementById('docsearch');
	let searchbutton = document.getElementById('submitsearch');
	let searchform = document.getElementById('searchform');
	let searchpreview = document.getElementById('searchprev');
	let searchElementList = {length:0};
	let validJumps = [];

	let doFocusSearch = false;
	function update_selected_search_event(e)
	{
		let sels = searchpreview.querySelectorAll('p');
		for(let q = 0; q < sels.length; ++q)
		{
			if(e.target === sels[q])
			{
				searchElementList.selected = q;
				update_selected_search();
				return;
			}
		}
	}
	function update_selected_search()
	{
		let sels = searchpreview.querySelectorAll('p');
		for(let q = 0; q < sels.length; ++q)
		{
			sels[q].classList = null;
			if(q===searchElementList.selected)
			{
				sels[q].classList.add('searchsel');
			}
		}
	}
	function update_search_prev(list)
	{
		let sel = searchElementList.selected;
		if(list.length < 1)
		{
			hide(searchpreview,false);
		}
		else
		{
			hide(searchpreview,true);
			searchpreview.innerHTML = '';
			for(let it_1 = 0; it_1 < list.length; ++it_1)
			{
				searchpreview.innerHTML += '<p'
					+ (sel === it_1 ? ' class=\\'searchsel\\'' : '')
					+ '>'+list[it_1]+'</p>';
			}
			let ch = searchpreview.querySelectorAll('p');
			for(let it_1 = 0; it_1 < ch.length; ++it_1)
			{
				ch[it_1].addEventListener('mouseover', update_selected_search_event);
				ch[it_1].addEventListener('mousedown', function(e){
					searchstr(e.target.innerText);
				});
			}
		}
	}

	function doSearchHL(str,reg)
	{
		let m = str.match(reg);
		let ret = '';
		while(m !== null)
		{
			ret += str.slice(0,m.index) +'<span class=\\'insearch\\'>'+m[1]+'</span>';
			str = str.slice(m.index + m[1].length);
			m = str.match(reg);
		}
		return ret+str;
	}
	function escstr(str)
	{
		let ret = '';
		for(let q = 0; q < str.length; ++q)
		{
			let c = str.charAt(q);
			switch(c)
			{
				case '\\\\': case '[': case ']': case '(': case ')': case '-':
					ret += '\\\\' + c;
					break;
				default:
					try {
						if(new RegExp(c).test('')) //filter out special characters
							ret += '\\\\' + c;
						else ret += c;
					}
					catch
					{
						ret += '\\\\' + c;
					}
			}
		}
		return ret;
	}
	function update_search_list(str)
	{
		str = str.trim();
		searchElementList = {selected:0,length:0};
		if(str === '') return;
		str = escstr(str);
		let reg = new RegExp('^('+str+')','i');
		const max_search_list = 20;
		let anyreg = new RegExp('('+str+')','i');

		let wasDone = [];
		for(let q = 0; q < validJumps.length; ++q)
		{
			if(/::/g.test(validJumps[q])) continue; //delay '::' names
			if(reg.test(validJumps[q]))
			{
				wasDone[q] = true;
				let elmnt = doSearchHL(validJumps[q],anyreg);
				searchElementList[searchElementList.length++] = elmnt;
				if(searchElementList.length >= max_search_list)
					return;
			}
		}
		reg = anyreg;
		for(let q = 0; q < validJumps.length; ++q)
		{
			if(wasDone[q]) continue;
			if(/::/g.test(validJumps[q])) continue; //delay '::' names
			if(reg.test(validJumps[q]))
			{
				wasDone[q] = true;
				let elmnt = doSearchHL(validJumps[q],anyreg);
				searchElementList[searchElementList.length++] = elmnt;
				if(searchElementList.length >= max_search_list)
					return;
			}
		}
		reg = new RegExp('^('+str+')','i');
		for(let q = 0; q < validJumps.length; ++q)
		{
			if(wasDone[q]) continue;
			if(!/::/g.test(validJumps[q])) continue; //only '::' names
			if(reg.test(validJumps[q]))
			{
				wasDone[q] = true;
				let elmnt = doSearchHL(validJumps[q],anyreg);
				searchElementList[searchElementList.length++] = elmnt;
				if(searchElementList.length >= max_search_list)
					return;
				continue;
			}
			let spl = validJumps[q].split('::');
			for(let q2 = 0; q2 < spl.length; ++q2)
			{
				if (reg.test(spl[q2]))
				{
					wasDone[q] = true;
					let elmnt = doSearchHL(validJumps[q], anyreg);
					searchElementList[searchElementList.length++] = elmnt;
					if (searchElementList.length >= max_search_list)
						return;
					break;
				}
			}
		}
		reg = anyreg;
		for(let q = 0; q < validJumps.length; ++q)
		{
			if(wasDone[q]) continue;
			if(!/::/g.test(validJumps[q])) continue; //only '::' names
			if(reg.test(validJumps[q]))
			{
				wasDone[q] = true;
				let elmnt = doSearchHL(validJumps[q],anyreg);
				searchElementList[searchElementList.length] = elmnt;
				if(searchElementList.length >= max_search_list)
					return;
			}
		}
	}
	function searchstr(str)
	{
		if(str==='') return;
		for(let q = 0; q < validJumps.length; ++q)
		{
			if(str.toLowerCase() === validJumps[q].toLowerCase())
			{
				if(searchbar === document.activeElement)
					update_search_prev(searchElementList);
				else update_search_prev([]);
				jumpstate(str);
			}
		}
	}
	function search()
	{
		let j = (searchElementList.length < 1 ? searchbar.value
			: trimtags(searchElementList[searchElementList.selected]));
		if(j === '') return;
		for(let q = 0; q < validJumps.length; ++q)
		{
			if(j.toLowerCase() === validJumps[q].toLowerCase())
			{
				if(searchbar === document.activeElement)
					update_search_prev(searchElementList);
				else update_search_prev([]);
				jumpstate(j);
				return;
			}
		}
	}
	function setup_searchbar()
	{
		update_search_prev([]);
		searchpreview.style.minWidth = searchbar.clientWidth+'px';
		searchbar.addEventListener('input', function(e){
			update_search_list(searchbar.value);
			update_search_prev(searchElementList);
		});
		searchbar.addEventListener('keydown', function(e){
			switch(e.key)
			{
				case 'ArrowDown':
				{
					if(searchElementList.selected+1 < searchElementList.length)
						++searchElementList.selected;
					break;
				}
				case 'ArrowUp':
				{
					if(searchElementList.selected > 0)
						--searchElementList.selected;
					break;
				}
				case 'Enter':
				{
					search();
					return;
				}
				default: return;
			}
			update_selected_search();
			e.preventDefault();
		});
		searchbar.addEventListener('blur', function(e){
			if(doFocusSearch)
			{
				searchbar.focus();
				update_search_prev(searchElementList);
				doFocusSearch = false;
			}
			else update_search_prev([]);
		});
		searchbar.addEventListener('focusin', function(e){
			update_search_prev(searchElementList);
		});
		searchform.addEventListener('submit',function(e){
			search();
			e.preventDefault();
		});
	}

	function seltab(tab, sel)
	{
		if(sel)
		{
			tab.style.cursor = 'default';
			tab.style.color = 'black';
			tab.dataset.sel = 'a';
		}
		else
		{
			tab.style.cursor = null;
			tab.style.color = null;
			tab.removeAttribute('data-sel');
		}
	}

	function hide(ob,vis)
	{
		ob.hidden = !vis;
		ob.style.display = vis ? null : 'none';
	}
	let curbody = -1;
	function setBody(ind)
	{
		let bodies = document.getElementsByClassName('cntnt');
		for(let it_1 = 0; it_1 < bodies.length; ++it_1)
		{
			hide(bodies[it_1],it_1==ind);
		}
		curbody = ind;
		let tabs = document.getElementsByClassName('tab_container')[curtab].getElementsByClassName('tab');
		for(let it_1 = 0; it_1 < tabs.length; ++it_1)
		{
			let tab = tabs[it_1];
			seltab(tab,(curbody===tab.dataset.bid || (tab.dataset.bid<0 && tab.dataset.tid < 0)));
		}
	}
	let curtab = 0;
	let cachetab = {};
	function setTab_int(ind)
	{
		let tabconts = document.getElementsByClassName('tab_container');
		let found = false;
		for(let it_1 = 0; it_1 < tabconts.length; ++it_1)
		{
			if(it_1==ind) found = true;
			hide(tabconts[it_1],it_1==ind);
		}
		if(!found)
		{
			hide(tabconts[0], true);
			curtab = 0;
		}
		else
		{
			curtab = ind;
		}
		setBody(cachetab[curtab]);
	}
	function setTab(ind)
	{
		if(cachetab[curtab] > -1)
			cachetab[curtab] = curbody;
		setTab_int(ind);
	}
	function showTooltip(e) {
		let elem = e.target;
		while(elem != document.documentElement)
		{
			if(elem.classList.contains('ttip'))
				break;
			elem = elem.parentNode;
		}
		let elems = elem.getElementsByClassName('ttt')
		if(elems.length < 1 || elems[0] === null)
			return;
		let tooltip = elems[0];
		let vw = document.documentElement.clientWidth;
		let vh = document.documentElement.clientHeight;
		let mx = e.clientX;
		let my = e.clientY;
		tooltip.style.left = 0;
		tooltip.style.top = 0;
		tooltip.style.left =
				(mx + tooltip.clientWidth + 10 < vw)
						? (mx + 10 + 'px')
						: (vw - 5 - tooltip.clientWidth + 'px');
		tooltip.style.top =
				(my + tooltip.clientHeight + 10 < vh)
						? (my + 10 + 'px')
						: (vh - 5 - tooltip.clientHeight + 'px');
	}
	function popupTooltip(ttip) {
		let innerhtml = ttip.innerHTML;
		const tttreg = /(.+)\\<span[ \	]+class[ \	]*=[ \	]*\"ttt\"[^\\>]+\\>(([^<]+|<)+)\\<\\/span\\>/
		let contentmatch = innerhtml.match(tttreg);
		if(contentmatch.length < 3) return;
		
		let content = contentmatch[2];
		let title = contentmatch[1];
		
		if(ttip.closest('.deadlink') !== null)
			return;
		
		document.getElementById('modaltitle').innerHTML = title;
		document.getElementById('modalcontent').innerHTML = content;
		document.getElementById('ttipmodal').style.display = 'block';
	}
	function replEntities(str)
	{
		str = str.replaceAll('&nbsp;',' ').replaceAll('&lt;','<')
			.replaceAll('&gt;','>').replaceAll('&amp;','&')
			.replaceAll('&quot;','\"').replaceAll('&apos;','\\'')
			.replaceAll('&cent;','¢').replaceAll('&pound;','£')
			.replaceAll('&yen;','¥').replaceAll('&euro;','€')
			.replaceAll('&copy;','©').replaceAll('&reg;','®');
		let reg = /&#([0-9]+);/i;
		let match = str.match(reg);
		while (match != null)
		{
			str = str.replace(reg,String.fromCharCode(parseInt(match[1])));
			match = str.match(reg);
		}
		return str;
	}
	function handle_codeblocks()
	{
		let code2 = document.querySelectorAll('.code2');
		let reg = /<span class='code1'>([^<]+)<\\/span>/i
		for(let q = 0; q < code2.length; ++q)
		{
			let str = code2[q].innerHTML;
			let match = str.match(reg);
			while (match != null)
			{
				str = str.replace(reg,'`'+match[1]+'`');
				match = str.match(reg);
			}
			code2[q].innerText = replEntities(str);
		}
		//let code1 = document.querySelectorAll('.code1');
	}
	function handle_spoilers()
	{
		let sp = document.querySelectorAll('.spoiler');
		for(let q = 0; q < sp.length; ++q)
		{
			let button = sp[q].getElementsByClassName('showbutton')[0];
			let block = sp[q].getElementsByClassName('spoilbody')[0];
			button.addEventListener('click', function(){
				if(button.dataset.active == '0')
				{
					button.dataset.active = '1';
					hide(block,true);
				}
				else
				{
					button.dataset.active = '0';
					hide(block,false);
				}
			});
		}
	}
	function trimtags(str)
	{
		return str.replace( /(<([^>]+)>)/ig, '').trim();
	}
	function trimsym(str)
	{
		return str.replace(/&[gl]t;/ig,'').replace(/[<>\\-]/ig,'').trim();
	}
	function dojump(obj)
	{
		if(obj.t > -1)
			setTab(obj.t);
		if(obj.b > -1)
			setBody(obj.b);
		return obj.t > -1 || obj.b > -1;
	}
	function goodSplit(str,sep,lim)
	{
		let tsep = sep.toString();
		let regopt = ''
		if(sep.toString().charAt(0) === '/')
		{
			regopt = tsep.slice(1+tsep.lastIndexOf('/'));
			tsep = tsep.slice(1, tsep.lastIndexOf('/'));
		}
		else tsep = escstr(tsep);
		let spl = str.split(sep);
		if(spl.length <= lim) return spl;
		let tspl = str.split(new RegExp('('+tsep+')', regopt));
		let tstr = str;
		for(let q = 0; q < lim*2-1; ++q)
		{
			tstr = tstr.slice(tspl[q].length);
		}
		spl = str.split(sep, lim);
		spl[spl.length-1] += tstr;
		return spl;
	}
	function loadLinkTargets()
	{
		validJumps = [];
		let tabconts = document.getElementsByClassName('tab_container');
		for(let it_1 = 0; it_1 < tabconts.length; ++it_1)
		{
			let tabcont = tabconts[it_1];
			let tabs = tabcont.getElementsByClassName('tab');
			for(let it_2 = 0; it_2 < tabs.length; ++it_2)
			{
				let tab = tabs[it_2];
				//Does it even go anywhere?
				let jb = tab.dataset.bid;
				let jt = tab.dataset.tid;
				if(jt < 0 && jb < 0)
					continue; //Goes nowhere, skip
				//Calculate the valid ways to jump here
				let tj = tab.dataset.jump;
				if(tj === null || tj === undefined)
					tj = tab.innerHTML;
				tj = trimsym(trimtags(tj));
				let jump_spl = tj.split(/\\/|;;/gm);
				if(/;;/g.test(tj))
				{
					jump_spl = goodSplit(tj, ';;', 2)[1].split(/\\/|;;/gm);
				}
				for(let it_3 = 0; it_3 < jump_spl.length; ++it_3)
				{
					let jstr = jump_spl[it_3].trim();
					if(jstr==='') continue;
					let dojstr = true;
					for(let it_4 = 0; it_4 < validJumps.length; ++it_4)
					{
						if(validJumps[it_4].toLowerCase() === jstr.toLowerCase())
						{
							dojstr = false;
							break;
						}
					}
					if(dojstr)
					{
						validJumps[validJumps.length] = jstr;
					}
				}
			}
		}
		validJumps.sort();
	}
	function checkjump(str)
	{
		let ret = {t:-1,b:-1,valid:false};
		let jumpname = str.toLowerCase().trim();
		if(jumpname.charAt(0) === '$')
		{
			let spl = jumpname.slice(1).split('$');
			let jt = parseInt(spl[0]);
			if(!isNaN(jt) && jt > -1)
				ret.t = jt;
			if(spl.length > 1)
			{
				let jb = parseInt(spl[1]);
				if(!isNaN(jb) && jb > -1)
					ret.b = jb;
			}
			return ret;
		}
		else if(jumpname === 'root')
		{
			ret.t = 0;
			return ret;
		}
		let tabconts = document.getElementsByClassName('tab_container');
		let jt = -1;
		let jb = -1;
		for(let it_1 = 0; it_1 < tabconts.length; ++it_1)
		{
			let tabcont = tabconts[it_1];
			let tabs = tabcont.getElementsByClassName('tab');
			for(let it_2 = 0; it_2 < tabs.length; ++it_2)
			{
				let tab = tabs[it_2];
				let tj = tab.dataset.jump;
				if(tj === null || tj === undefined)
					tj = tab.innerHTML;
				tj = trimsym(trimtags(tj)).toLowerCase();
				let jump_spl = tj.split(/\\/|;;/gm);
				if(/;;/g.test(tj))
				{
					jump_spl = goodSplit(tj, ';;', 2)[1].split(/\\/|;;/gm);
				}
				for(let it_3 = 0; it_3 < jump_spl.length; ++it_3)
				{
					if(jump_spl[it_3].trim() === jumpname)
					{
						jb = tab.dataset.bid;
						jt = tab.dataset.tid;
						if(jt < 0)
							jt = tab.parentNode.dataset.tid;
						if(jt > -1)
							ret.t = jt;
						if(jb > -1)
							ret.b = jb;
						if(jt > -1 || jb > -1)
							ret.valid = true;
						return ret;
					}
				}
			}
		}
		return ret;
	}
	function jump(str)
	{
		return dojump(checkjump(str));
	}
	function jumpstate(str)
	{
		let t = curtab, b = curbody;
		jump(str);
		if(t !== curtab || b !== curbody)
		{
			pushState(str);
		}
	}
	function pushState(str)
	{
		urlParams.set('jump',str.toLowerCase());
		window.history.pushState({t:curtab,b:curbody},'','?'+urlParams.toString());
	}
	function setupTabs()
	{
		let tabconts = document.getElementsByClassName('tab_container');
		for(let it_1 = 0; it_1 < tabconts.length; ++it_1)
		{
			cachetab[it_1] = -1;
			let tabcont = tabconts[it_1];
			let tabs = tabcont.getElementsByClassName('tab');
			for(let it_2 = 0; it_2 < tabs.length; ++it_2)
			{
				let tab = tabs[it_2];
				let id = tab.id.toString();
				if(tab.dataset.bid > -1 && cachetab[it_1] < 0)
					cachetab[it_1] = tab.dataset.bid;
				if(tab.innerHTML === '--')
				{
					hide(tab, false);
					continue;
				}
				let tabjumpstr = trimsym(trimtags(tab.dataset.jump)).toLowerCase();
				if(tabjumpstr.includes(';;'))
					tabjumpstr = tabjumpstr.split(';;')[1];
				let spl = tabjumpstr.toLowerCase().split(/\\/|;;/gm);
				let mainspl = tabjumpstr.split(/\\/|;;/gm);
				tab.dataset.link = '';
				let tid = tab.dataset.tid;
				if(tid < 0) tid = tab.parentNode.dataset.tid;
				let errstr = 'Error for: \\'' + tab.innerHTML + '\\'\\n';
				let ignore = false;
				if(tid == 0)
				{
					tab.dataset.link = 'root';
				}
				else if(tab.dataset.tid < 0 && tab.dataset.bid < 0)
					ignore = true;
				else
				{
					let pos = {i:0};
					for (let it_3 = 0; it_3 < spl.length; ++it_3)
					{
						let obj = checkjump(spl[it_3]);
						if (obj.t == tid && obj.b == tab.dataset.bid)
						{
							pos[pos.i++] = spl[it_3].trim();
						}
						else
						{
							errstr += '\	Substring \\'' + mainspl[it_3].trim() + '\\':\\n'
									+ '\	\	Gave \\'' + obj.t + '', '' + obj.b + '\\'\\n'
									+ '\	\	Want \\'' + tid + '', '' + tab.dataset.bid + '\\'\\n';
						}
					}
					if(pos.i > 0)
					{
						let d = pos[0].toString();
						for (let it_3 = 1; it_3 < pos.i; ++it_3)
						{
							if(d.includes(' ') && !pos[it_3].includes(' '))
							{
								d = pos[it_3];
							}
							else if(d.length > pos[it_3].length)
							{
								d = pos[it_3];
							}
						}
						tab.dataset.link = d;
					}
				}
				if(!ignore && tab.dataset.link === '')
				{
					if(verboseLog) console.warn(errstr);
					tab.dataset.link = '$' + tid + (tab.dataset.bid>-1 ? ('$' + tab.dataset.bid) : '');
				}
				tab.addEventListener('click', function() {
					if(tab.dataset.sel==='a')
						return;
					let bid = tab.dataset.bid;
					let tid = tab.dataset.tid;
					if(tid > -1)
						setTab(tid);
					if(bid > -1)
						setBody(bid);
					pushState(tab.dataset.link);
				});
			}
		}
	}
	window.onload = function() {
		if(urlParams.has('v'))
			verboseLog = true;
		let dbtn = document.getElementById('debug');
		if(dbtn !== undefined)
			dbtn.addEventListener('click',function(){
				verboseLog = true;
				if(!urlParams.has('v'))
					urlParams = new URLSearchParams('?v&'+urlParams.toString());
				window.history.pushState({t:curtab,b:curbody},'','?'+urlParams.toString());
			});
		window.addEventListener('mousemove', showTooltip);
		let modal = document.getElementById('ttipmodal');
		window.addEventListener('click', function(e){
			let tt = e.target.closest('.ttip')
			if(e.target === modal)
				modal.style.display = 'none';
			else if(tt !== null)
				popupTooltip(tt);
			else if(e.target.classList.contains('closemodal'))
				modal.style.display = 'none';
			else if(e.target.classList.contains('link'))
			{
				if(e.target.dataset.sel!=='a')
					jumpstate(e.target.dataset.link);
			}
		});
		handle_codeblocks();
		handle_spoilers();
		let bodies = document.getElementsByClassName('cntnt');
		setupTabs();
		setBody(cachetab[0]);

		loadLinkTargets();
		setup_searchbar();

		if(urlParams.has('jump'))
			jump(urlParams.get('jump'));
		else if(jump('_preview_jump'))
			pushState('_preview_jump');
	}

	window.onpopstate = function(e)
	{
		if(e.state)
		{
			setTab(e.state.t);
			setBody(e.state.b);
			urlParams = new URLSearchParams(window.location.search);
		}
	}
</script>"""
    if debug_out:
        if len(broken_links) < 1:
            print('[REPORT] No Broken Links!')
        else:
            longest = 0
            for s in broken_links:
                if len(s) > longest:
                    longest = len(s)
            longest += 4
            num = math.floor(150 / longest)
            print('[REPORT] Broken Links:')
            for ind in range(len(broken_links)):
                if ind % num != num-1:
                    print(broken_links[ind],end=' '*(longest-len(broken_links[ind])))
                else:
                    print(broken_links[ind])
            print()
    parse_state('Cleaning up...')
    for sheet in sheets:
        _dict_del(sheet,'tabind')
        for tab in sheet['tabs']:
            for line in tab['lines']:
                _dict_del(line,'linktab')
                _dict_del(line,'body')
    
    ret = f"""<!DOCTYPE html>
<html lang="en" dir="ltr">
<head>
	<meta charset="utf-8">
	<title>ZSDocs</title>
{css}
</head>
<body class = "pagecont">
<header class = "pagecont">
	<h1>ZScript Documentation (WIP)</h1>
	<p>Last Updated: {update_date()} (<a href="https://www.zeldaclassic.com/zscript-docs/" target="_blank" rel="noopener noreferrer">Latest</a>)</p>
</header>
<main class = "pagecont">
	<div class='modal' id = 'ttipmodal'>
		<div class='modal_content'>
			<span class='closemodal'>&times;</span>
			<h3 id='modaltitle'></h3>
			<hr/>
			<div id='modalcontent'></div>
		</div>
	</div>
	<a class = "link" data-link = "root" data-bid = 0 data-tid = 0>Return to Top</a>
	<form id='searchform' style='display:inline;float:right'>
		<div style='display:inline'>
			<input type='text' id='docsearch' autocomplete='off' placeholder='Search...' />
			<div id='searchprev' style='display:none' hidden></div>
		</div>
		<input type='submit' id='submitsearch' value='Jump' />
	</form>
	<div class = "panel">
{generated_data}
	</div>
</main>
{js}
</body>
</html>
"""
    return ret

debug_out = False
data_obj = None
sheets = []
sz_tabs = 0
broken_links = []
