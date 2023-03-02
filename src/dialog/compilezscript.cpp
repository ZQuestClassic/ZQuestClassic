#include "compilezscript.h"
#include <gui/builder.h>
#include "zquest.h"
#include "alert.h"
#include "alertfunc.h"
#include "headerdlg.h"
#include "ffscript.h"
#include "qst.h"
#include "ffasmexport.h"
#include "zscrdata.h"
#include "info.h"
using std::string;

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include "base/emscripten_utils.h"
#endif

void doEditZScript(int32_t bg,int32_t fg);
void clear_map_states();
void do_script_disassembly(map<string, disassembled_script_data>& scripts, bool fromCompile);
bool do_slots(map<string, disassembled_script_data> &scripts);
int32_t onZScriptCompilerSettings();

extern string zScript;
extern FFScript FFCore;
extern std::vector<string> asffcscripts, asglobalscripts, asitemscripts,
	asnpcscripts, aseweaponscripts, aslweaponscripts,
	asplayerscripts, asdmapscripts, asscreenscripts,
	asitemspritescripts, ascomboscripts, asgenericscripts;
extern std::map<int32_t, script_slot_data > globalmap;

int32_t onCompileScript()
{
	CompileZScriptDialog().show();
	return D_O_K;
}

bool doCompileOpenHeaderDlg()
{
    call_header_dlg();
    return false;
}

byte compile_success_sample = 0;
byte compile_error_sample = 0;
byte compile_finish_sample = 0;
byte compile_audio_volume = 0;
void compile_sfx(bool success)
{
	if ( success )
	{
		compile_error_sample = 0;
		compile_success_sample = vbound(zc_get_config("Compiler","compile_success_sample",20),0,255);
		if ( compile_success_sample > 0 )
		{
			compile_audio_volume = vbound(zc_get_config("Compiler","compile_audio_volume",100),0,255);
			if(sfxdat)
				sfx_voice[compile_success_sample]=allocate_voice((SAMPLE*)sfxdata[compile_success_sample].dat);
			else sfx_voice[compile_success_sample]=allocate_voice(&customsfxdata[compile_success_sample]);
			voice_set_volume(sfx_voice[compile_success_sample], compile_audio_volume);
			voice_start(sfx_voice[compile_success_sample]);
		}
	}
	else
	{
		compile_success_sample = 0;
		compile_error_sample = vbound(zc_get_config("Compiler","compile_error_sample",28),0,255);
		if ( compile_error_sample > 0 )
		{
			compile_audio_volume = vbound(zc_get_config("Compiler","compile_audio_volume",100),0,255);
			if(sfxdat)
				sfx_voice[compile_error_sample]=allocate_voice((SAMPLE*)sfxdata[compile_error_sample].dat);
			else sfx_voice[compile_error_sample]=allocate_voice(&customsfxdata[compile_error_sample]);
			voice_set_volume(sfx_voice[compile_error_sample], compile_audio_volume);
			voice_start(sfx_voice[compile_error_sample]);
		}
	}
}

CompileZScriptDialog::CompileZScriptDialog()
{}

static int32_t Type_Checked = 0, lowcombo = 0, highcombo = 65279, damage = 4;

void CompileZScriptDialog::updateLabels()
{
	char buf[64];
	sprintf(buf, "Last Compiled Using ZScript: v.%d",(FFCore.quest_format[vLastCompile]));
	labels[0]->setText(buf);
	sprintf(buf, "%d Bytes in Buffer", (int32_t)(zScript.size()));
	labels[1]->setText(buf);
}

#define BTN(txt, msg, w) \
Button( \
	fitParent = true, \
	padding = 0_px, \
	text = (txt), \
	width = (w), \
	height = 3_em, \
	onClick = message::msg \
)
#define BTNF(txt, msg, w) \
Button( \
	focused = true, \
	fitParent = true, \
	padding = 0_px, \
	text = (txt), \
	width = (w), \
	height = 3_em, \
	onClick = message::msg \
)

std::shared_ptr<GUI::Widget> CompileZScriptDialog::view()
{
	using namespace GUI::Builder;
	using namespace GUI::Props;
	const GUI::Size panel_width = 150_px;
	const GUI::Size col_spacing = 4_px;
	const GUI::Size row_spacing = 4_px;
	
	window = Window(
		title = "Compile ZScript",
		onClose = message::CANCEL,
		Column(
			labels[0] = Label(hAlign = 0.0),
			labels[1] = Label(hAlign = 0.0),
			Column(hPadding = 0_px, vPadding = DEFAULT_PADDING*2,
				rowSpacing = row_spacing,
				//
				Row(padding = 0_px,
					columnSpacing = col_spacing,
					BTN("&Load",LOAD,panel_width/3),
					BTN("E&xport",EXPORT,panel_width/3),
					BTN("&Edit",EDIT,panel_width/3)
				),
				//
				Row(padding = 0_px,
					columnSpacing = col_spacing,
					BTN("&Docs",DOCS,panel_width/3),
					BTN("&std_zh",STD_ZH,panel_width/3),
					BTN("Settings",SETTINGS,panel_width/3)
				),
				//
				Row(padding = 0_px,
					columnSpacing = col_spacing,
					BTNF("&Compile",COMPILE,panel_width/2+col_spacing/2),
					BTN("Cancel",CANCEL,panel_width/2+col_spacing/2)
				)
			)
		)
	);
	updateLabels();
	return window;
}

bool CompileZScriptDialog::handleMessage(const GUI::DialogMessage<message>& msg)
{
	bool ctrl = key_shifts & KB_CTRL_FLAG;
	switch(msg.message)
	{
		case message::LOAD:
		{
			//Load from File
			if(zScript.size() > 0)
			{
				if(jwin_alert("Confirm Overwrite","Loading will erase the current buffer.","Proceed anyway?",NULL,"Yes","No",'y','n',lfont)==2)
					return false;
					
				zScript.clear();
				updateLabels();
				saved = false;
			}
			
			if(!getname("Load ZScript (.z, .zh, .zs, .zlib, etc.)", (char *)"z,zh,zs,zlib,zasm,zscript,squid" ,NULL,datapath,false))
				return false;
				
			FILE *zscript = fopen(temppath,"r");
			
			if(zscript == NULL)
			{
				jwin_alert("Error","Cannot open specified file!",NULL,NULL,"O&K",NULL,'k',0,lfont);
				return false;
			}
			
			char c = fgetc(zscript);
			
			while(!feof(zscript))
			{
				zScript += c;
				c = fgetc(zscript);
			}
			
			fclose(zscript);
			saved = false;
			updateLabels();
			return false;
		}
		
		case message::EXPORT:
		{
			if(!getname("Save ZScript (.zs)", "zs", NULL,datapath,false))
				break;
				
			if(exists(temppath))
			{
				if(jwin_alert("Confirm Overwrite","File already exists.","Overwrite?",NULL,"Yes","No",'y','n',lfont)==2)
					break;
			}
			
			FILE *zscript = fopen(temppath,"w");
			
			if(!zscript)
			{
				jwin_alert("Error","Unable to open file for writing!",NULL,NULL,"O&K",NULL,'k',0,lfont);
				break;
			}
			
			int32_t written = (int32_t)fwrite(zScript.c_str(), sizeof(char), zScript.size(), zscript);
			
			if(written != (int32_t)zScript.size())
				jwin_alert("Error","IO error while writing script to file!",NULL,NULL,"O&K",NULL,'k',0,lfont);
				
			fclose(zscript);
			return false;
		}
		
		case message::EDIT:
			doEditZScript(vc(15),vc(0));
			updateLabels();
			return false;
		
		case message::DOCS:
			if(fileexists("docs/ZScript_Docs.html"))
				launch_file("docs/ZScript_Docs.html");
			return false;
		
		case message::STD_ZH:
			if(util::checkPath("include/std_zh",true))
				launch_file("include/std_zh");
			return false;
		
		case message::SETTINGS:
			onZScriptCompilerSettings();
			return false;
		
		case message::COMPILE:
		{
			char tmpfilename[L_tmpnam];
			std::tmpnam(tmpfilename);
			FILE *tempfile = fopen(tmpfilename,"w");

			char consolefilename[L_tmpnam];
			std::tmpnam(consolefilename);

			if(!tempfile)
			{
				jwin_alert("Error","Unable to create a temporary file in current directory!",NULL,NULL,"O&K",NULL,'k',0,lfont);
				return true;
			}
			
			fwrite(zScript.c_str(), sizeof(char), zScript.size(), tempfile);
			fclose(tempfile);
			
			script_data old_init_script(*globalscripts[0]);
			uint32_t lastInitSize = old_init_script.size();
			map<string, ZScript::ScriptTypeID> stypes;
			map<string, disassembled_script_data> scripts;
			
			std::string quest_rules_hex = get_qr_hexstr();
			clock_t start_compile_time = clock();
			bool hasWarnErr = false;
#ifdef __EMSCRIPTEN__
			int32_t code = em_compile_zscript(tmpfilename, consolefilename, quest_rules_hex.c_str());
#else
			int32_t code = -9999;
			if(!fileexists(ZSCRIPT_FILE))
			{
				InfoDialog("Parser", ZSCRIPT_FILE " was not found!").show();
				return false;
			}
			parser_console.kill();
			if (!DisableCompileConsole) 
			{
				parser_console.Create("ZScript Parser Output", 600, 200, NULL, "zconsole.exe");
				parser_console.cls(CConsoleLoggerEx::COLOR_BACKGROUND_BLACK);
				parser_console.gotoxy(0,0);
				_print_zconsole("External ZScript Parser\n","[INFO] ",INFO_COLOR);
			}
			else
			{
				box_start(1, "Compile Progress", get_custom_font_a5(CFONT_TITLE), get_custom_font_a5(CFONT_DLG),true, 512, 280);
			}

			std::vector<std::string> args = {
				"-input", tmpfilename,
				"-console", consolefilename,
				"-qr", quest_rules_hex.c_str(),
				"-linked",
			};
			if(zc_get_config("Compiler","noclose_compile_console",0))
				args.push_back("-noclose");
			#ifdef _DEBUG
			if(ctrl)
				args.push_back("-delay");
			#endif
			process_manager* pm = launch_piped_process(ZSCRIPT_FILE, args);
			if(!pm)
			{
				InfoDialog("Parser","Failed to launch " ZSCRIPT_FILE).show();
				return false;
			}
			
			int current = 0, last = 0;
			int syncthing = 0;
			pm->read(&syncthing, sizeof(int32_t));

			FILE *console = fopen(consolefilename, "r");
			bool running = true;
			if (console) 
			{
				char buf4[4096];
				while(running)
				{
					pm->read(&code, sizeof(int32_t));
					switch(code)
					{
						case ZC_CONSOLE_DB_CODE:
						case ZC_CONSOLE_ERROR_CODE:
						case ZC_CONSOLE_WARN_CODE:
							hasWarnErr = true;
							[[fallthrough]];
						case ZC_CONSOLE_INFO_CODE:
							fseek(console, 0, SEEK_END);
							current = ftell(console);
							if (current != last)
							{
								int amount = (current-last);
								fseek(console, last, SEEK_SET);
								last = current;
								int end = fread(&buf4, sizeof(char), amount, console);
								buf4[end] = 0;
								ReadConsole(buf4, code);
							}
							pm->write(&code, sizeof(int32_t));
							break;
						default:
							running = false;
							break;
					}
				}
			}
			pm->read(&code, sizeof(int32_t));
			delete pm;
#endif
			clock_t end_compile_time = clock();
			
			char tmp[128] = {0};
			sprintf(tmp,"%lf",(end_compile_time - start_compile_time)/((double)CLOCKS_PER_SEC));
			for(size_t ind = strlen(tmp)-1; ind > 0; --ind)
			{
				if (tmp[ind] == '0' && tmp[ind - 1] != '.') tmp[ind] = 0;
				else break;
			}
			char buf[1024] = {0};
			sprintf(buf, "ZScript compilation: Returned code '%d' (%s)\n"
				"Compile took %s seconds (%ld cycles)%s",
				code, code ? "failure" : "success",
				tmp, (long)end_compile_time - start_compile_time,
				code ? (!DisableCompileConsole?"\nCompilation failed. See console for details.":"\nCompilation failed.") : "");
			
			if(!code)
			{
				read_compile_data(stypes, scripts);
				if (!(DisableCompileConsole || hasWarnErr)) 
				{
					parser_console.kill();
				}
			}
			else if (DisableCompileConsole)
			{
				char buf3[256] = {0};
				sprintf(buf3, "Compile took %lf seconds (%ld cycles)", (end_compile_time - start_compile_time)/((double)CLOCKS_PER_SEC),(long)end_compile_time - start_compile_time);
				box_out(buf3);
				box_eol();
				box_out("Compilation failed.");
				box_eol();
			}
			compile_sfx(!code);
			if (DisableCompileConsole) box_end(true);
			
			bool cancel = code;
			if (!DisableCompileConsole) 
			{
				if(code)
					InfoDialog("ZScript Parser", buf).show();
				else AlertDialog("ZScript Parser", buf,
					[&](bool ret,bool)
					{
						cancel = !ret;
					}, "Continue", "Cancel").show();
			}
			if ( compile_success_sample > 0 )
			{
				if(sfx_voice[compile_success_sample]!=-1)
				{
					deallocate_voice(sfx_voice[compile_success_sample]);
					sfx_voice[compile_success_sample]=-1;
				}
			}
			if ( compile_error_sample > 0 )
			{
				if(sfx_voice[compile_error_sample]!=-1)
				{
					deallocate_voice(sfx_voice[compile_error_sample]);
					sfx_voice[compile_error_sample]=-1;
				}
			}
			//refresh(rALL);
			
			if(cancel)
				return false;
			
			asffcscripts.clear();
			asffcscripts.push_back("<none>");
			asglobalscripts.clear();
			asglobalscripts.push_back("<none>");
			asitemscripts.clear();
			asitemscripts.push_back("<none>");
			asnpcscripts.clear();
			asnpcscripts.push_back("<none>");
			aseweaponscripts.clear();
			aseweaponscripts.push_back("<none>");
			aslweaponscripts.clear();
			aslweaponscripts.push_back("<none>");
			asplayerscripts.clear();
			asplayerscripts.push_back("<none>");
			asdmapscripts.clear();
			asdmapscripts.push_back("<none>");
			asscreenscripts.clear();
			asscreenscripts.push_back("<none>");
			asitemspritescripts.clear();
			asitemspritescripts.push_back("<none>");
			ascomboscripts.clear();
			ascomboscripts.push_back("<none>");
			asgenericscripts.clear();
			asgenericscripts.push_back("<none>");
			clear_map_states();
			globalmap[0].updateName("~Init"); //force name to ~Init
			
			using namespace ZScript;
			for (auto it = stypes.begin(); it != stypes.end(); ++it)
			{
				string const& name = it->first;
				switch(it->second)
				{
					case scrTypeIdFfc:
						asffcscripts.push_back(name);
						break;
					case scrTypeIdItem:
						asitemscripts.push_back(name);
						break;
					case scrTypeIdNPC:
						asnpcscripts.push_back(name);
						break;
					case scrTypeIdEWeapon:
						aseweaponscripts.push_back(name);
						break;
					case scrTypeIdLWeapon:
						aslweaponscripts.push_back(name);
						break;
					case scrTypeIdPlayer:
						asplayerscripts.push_back(name);
						break;
					case scrTypeIdDMap:
						asdmapscripts.push_back(name);
						break;
					case scrTypeIdScreen:
						asscreenscripts.push_back(name);
						break;
					case scrTypeIdItemSprite:
						asitemspritescripts.push_back(name);
						break;
					case scrTypeIdComboData:
						ascomboscripts.push_back(name);
						break;
					case scrTypeIdGeneric:
						asgenericscripts.push_back(name);
						break;
					case scrTypeIdGlobal:
						if (name != "~Init")
						{
							asglobalscripts.push_back(name);
						}
						break;
				}
			}
		
			//scripts are compiled without error, so store the zscript version here: -Z, 25th July 2019, A29
			misc.zscript_last_compiled_version = V_FFSCRIPT;
			FFCore.quest_format[vLastCompile] = V_FFSCRIPT;
			zprint2("Compiled scripts in version: %d\n", misc.zscript_last_compiled_version);
						
			do_script_disassembly(scripts, true);
			
			//assign scripts to slots
			do_slots(scripts);
			
			if(WarnOnInitChanged)
			{
				script_data const& new_init_script = *globalscripts[0];
				if(new_init_script != old_init_script) //Global init changed
				{
					auto newInitSize = new_init_script.size();
					AlertFuncDialog("Init Script Changed",
						"Either global variables, or your global script Init, have changed. ("+to_string(lastInitSize)+"->"+to_string(newInitSize)+")\n\n"
						"This can break existing save files of your quest. To prevent users "
						"from loading save files that would break, you can raise the \"Quest "
						"Ver\" and \"Min. Ver\" in the Header menu (Quest>>Options>>Header)\n\n"
						"Ensure that both versions are higher than \"Quest Ver\" was previously, "
						"and that \"Quest Ver\" is the same or higher than \"Min. Ver\"",
						2, 1, //2 buttons, where buttons[1] is focused
						"Header", doCompileOpenHeaderDlg,
						"OK", NULL
					).show();
				}
			}
			return true;
		}
		
		case message::CANCEL:
		default:
			return true;
	}
	return false;
}
