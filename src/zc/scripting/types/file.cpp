#include "zc/scripting/types/file.h"

#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/scripting/common.h"
#include "zc/scripting/script_object.h"

#ifdef _WIN32
#define SCRIPT_FILE_MODE	(_S_IREAD | _S_IWRITE)
#else
#define SCRIPT_FILE_MODE	(S_ISVTX | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#endif

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

namespace {

UserDataContainer<user_file, MAX_USER_FILES> user_files = {script_object_type::file, "file"};

void check_file_error(int32_t ref)
{
	if(user_file* f = checkFile(ref, true, true))
	{
		int32_t err = ferror(f->file);
		if(err != 0)
		{
			Z_scripterrlog("File with UID '%d' encountered an error.\n", ref);
			Z_scripterrlog("File error: %s\n", strerror(err));
		}
	}
}

void do_fopen(const bool v, const char* f_mode)
{
	int32_t arrayptr = SH::get_arg(sarg1, v);
	string user_path;
	ArrayH::getString(arrayptr, user_path, 512);

	SET_D(rEXP1, 0); //Presume failure; update to 10000L on success
	SET_D(rEXP2, 0);

	std::string resolved_path;
	if (auto r = parse_user_path(user_path, true); !r)
	{
		scripting_log_error_with_context("Error: {}", r.error());
		return;
	} else resolved_path = r.value();

	user_file* f = checkFile(GET_REF(fileref), false, true);
	if(!f) //auto-allocate
	{
		ri->fileref = user_files.get_free();
		f = checkFile(GET_REF(fileref), false, true);
	}
	SET_D(rEXP2, ri->fileref); //Returns to the variable!
	if(f)
	{
		f->close(); //Close the old FILE* before overwriting it!
		bool create = false;
		for(int32_t q = 0; f_mode[q]; ++q)
		{
			if(f_mode[q] == 'w' || f_mode[q] == 'a')
			{
				create = true;
				break;
			}
		}
		if(!create || make_dirs_for_file(resolved_path))
		{
			f->file = fopen(resolved_path.c_str(), f_mode);
			fflush(f->file);
			zc_chmod(resolved_path.c_str(), SCRIPT_FILE_MODE);
			f->setPath(resolved_path.c_str());
			//r+; read-write, will not create if does not exist, will not delete content if does exist.
			//w+; read-write, will create if does not exist, will delete all content if does exist.
			if(f->file)
			{
				SET_D(rEXP1, 10000L); //Success
				return;
			}
		}
		else
		{
			Z_scripterrlog("Script failed to create directories for file path '%s'.\n", resolved_path.c_str());
			SET_D(rEXP2, 0);
			return;
		}
	}
}

void do_fremove()
{
	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		zprint2("Removing file %d\n", GET_REF(fileref));
		SET_D(rEXP1, f->do_remove() ? 0L : 10000L);
	}
	else SET_D(rEXP1, 0L);
}

void do_fclose()
{
	if(user_file* f = checkFile(GET_REF(fileref), false, true))
	{
		f->close();
	}
	//No else. If invalid, no error is thrown.
}

void do_allocate_file()
{
	//Get a file and return it
	ri->fileref = user_files.get_free();
	SET_D(rEXP2, ri->fileref); //Return to ptr
	SET_D(rEXP1, (ri->d[rEXP2] == 0 ? 0L : 10000L));
}

void do_deallocate_file()
{
	user_file* f = checkFile(GET_REF(fileref), false, true);
	if(f) free_script_object(f->id);
}

void do_file_isallocated() //Returns true if file is allocated
{
	user_file* f = checkFile(GET_REF(fileref), false, true);
	SET_D(rEXP1, (f) ? 10000L : 0L);
}

void do_file_isvalid() //Returns true if file is allocated and has an open FILE*
{
	user_file* f = checkFile(GET_REF(fileref), true, true);
	SET_D(rEXP1, (f) ? 10000L : 0L);
}

void do_fflush()
{
	SET_D(rEXP1, 0L);

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		if(!fflush(f->file))
			SET_D(rEXP1, 10000L);
		check_file_error(GET_REF(fileref));
	}
}

void do_file_readchars()
{
	int32_t arrayptr = get_register(sarg1);
	int32_t count = get_register(sarg2) / 10000;
	uint32_t pos = zc_max(GET_D(rINDEX) / 10000,0);
	SET_D(rEXP1, 0L);

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		if(count == 0) return;

		ArrayManager am(arrayptr);
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		if(pos >= sz)
		{
			Z_scripterrlog("Pos (%d) passed to %s is outside the bounds of array %d. Aborting.\n", pos, "ReadChars()", arrayptr);
			return;
		}
		if(count < 0 || unsigned(count) > sz-pos) count = sz-pos;
		int32_t limit = pos+count;
		char c;
		word q;
		for(q = pos; q < limit; ++q)
		{
			c = fgetc(f->file);
			if(feof(f->file) || ferror(f->file))
				break;
			if(c <= 0)
				break;
			am.set(q,c * 10000L);
			++ri->d[rEXP1]; //Don't count nullchar towards length
		}
		if(q >= limit)
		{
			--q;
			--ri->d[rEXP1];
			ungetc(am.get(q), f->file); //Put the character back before overwriting it
		}
		am.set(q,0); //Force null-termination
		ri->d[rEXP1] *= 10000L;
		check_file_error(GET_REF(fileref));
	}
}

void do_file_readbytes()
{
	int32_t arrayptr = get_register(sarg1);
	int32_t count = get_register(sarg2) / 10000;
	uint32_t pos = zc_max(GET_D(rINDEX) / 10000,0);
	SET_D(rEXP1, 0L);

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		if(count == 0) return;

		ArrayManager am(arrayptr);
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		if(pos >= sz)
		{
			Z_scripterrlog("Pos (%d) passed to %s is outside the bounds of array %d. Aborting.\n", pos, "ReadBytes()", arrayptr);
			return;
		}
		if(count < 0 || unsigned(count) > sz-pos) count = sz-pos;
		std::vector<uint8_t> data(count);
		SET_D(rEXP1, 10000L * fread((void*)&(data[0]), 1, count, f->file));
		for(int32_t q = 0; q < count; ++q)
		{
			am.set(q+pos, 10000L * data[q]);
		}
		check_file_error(GET_REF(fileref));
	}
}

void do_file_readstring()
{
	int32_t arrayptr = get_register(sarg1);
	SET_D(rEXP1, 0L);

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		ArrayManager am(arrayptr);
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		int32_t limit = sz;
		int32_t c;
		word q;
		for(q = 0; q < limit; ++q)
		{
			c = fgetc(f->file);
			if(feof(f->file) || ferror(f->file))
				break;
			if(c <= 0)
				break;
			am.set(q,c * 10000L);
			++ri->d[rEXP1]; //Don't count nullchar towards length
			if(c == '\n')
			{
				++q;
				break;
			}
		}
		if(q >= limit)
		{
			--q;
			--ri->d[rEXP1];
			ungetc(am.get(q), f->file); //Put the character back before overwriting it
		}
		am.set(q,0); //Force null-termination
		ri->d[rEXP1] *= 10000L;
		check_file_error(GET_REF(fileref));
	}
}

void do_file_readints()
{
	int32_t arrayptr = get_register(sarg1);
	int32_t count = get_register(sarg2) / 10000;
	uint32_t pos = zc_max(GET_D(rINDEX) / 10000,0);
	SET_D(rEXP1, 0L);

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		if(count == 0) return;

		ArrayManager am(arrayptr);
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		if(pos >= sz) 
		{
			Z_scripterrlog("Pos (%d) passed to %s is outside the bounds of array %d. Aborting.\n", pos, "ReadInts()", arrayptr);
			return;
		}
		if(count < 0 || unsigned(count) > sz-pos) count = sz-pos;
		
		std::vector<int32_t> data(count);
		SET_D(rEXP1, 10000L * fread((void*)&(data[0]), 4, count, f->file));
		for(int32_t q = 0; q < count; ++q)
		{
			am.set(q+pos,data[q]);
		}
		check_file_error(GET_REF(fileref));
	}
}

void do_file_writechars()
{
	int32_t arrayptr = get_register(sarg1);
	int32_t count = get_register(sarg2) / 10000;
	int32_t pos = zc_max(GET_D(rINDEX) / 10000,0);
	SET_D(rEXP1, 0L);

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		if(count == 0) return;
		if(count == -1 || count > (MAX_ZC_ARRAY_SIZE-pos)) count = MAX_ZC_ARRAY_SIZE-pos;
		string output;
		ArrayH::getString(arrayptr, output, count, pos);
		uint32_t q = 0;
		for(; q < output.length(); ++q)
		{
			if(fputc(output[q], f->file)<0)
				break;
		}
		SET_D(rEXP1, q * 10000L);
		check_file_error(GET_REF(fileref));
	}
}

void do_file_writebytes()
{
	int32_t arrayptr = get_register(sarg1);
	int32_t arg = get_register(sarg2) / 10000;
	uint32_t pos = zc_max(GET_D(rINDEX) / 10000,0);
	SET_D(rEXP1, 0L);

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		if(arg == 0) return;

		uint32_t count = ((arg<0 || unsigned(arg) >(MAX_ZC_ARRAY_SIZE - pos)) ? MAX_ZC_ARRAY_SIZE - pos : unsigned(arg));
		string output;
		ArrayManager am(arrayptr);
		if(am.invalid()) return;
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		if(pos >= sz)
		{
			Z_scripterrlog("Pos (%d) passed to %s is outside the bounds of array %d. Aborting.\n", pos, "WriteBytes()", arrayptr);
			return;
		}
		if (count > sz-pos) count = sz-pos;
		std::vector<uint8_t> data(count);
		for(uint32_t q = 0; q < count; ++q)
		{
			data[q] = am.get(q+pos) / 10000;
		}
		SET_D(rEXP1, 10000L * fwrite((const void*)&(data[0]), 1, count, f->file));
		check_file_error(GET_REF(fileref));
	}
}

void do_file_writestring()
{
	int32_t arrayptr = get_register(sarg1);
	SET_D(rEXP1, 0L);

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		string output;
		ArrayH::getString(arrayptr, output, ZSCRIPT_MAX_STRING_CHARS);
		uint32_t q = 0;
		for(; q < output.length(); ++q)
		{
			if(fputc(output[q], f->file)<0)
				break;
		}
		SET_D(rEXP1, q * 10000L);
		check_file_error(GET_REF(fileref));
	}
}

void do_file_writeints()
{
	int32_t arrayptr = get_register(sarg1);
	int32_t count = get_register(sarg2) / 10000;
	uint32_t pos = zc_max(GET_D(rINDEX) / 10000,0);
	SET_D(rEXP1, 0L);

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		if(count == 0) return;
		ArrayManager am(arrayptr);
		if(am.invalid()) return;
		int32_t sz = am.size();
		if(sz <= 0)
			return;
		if(pos >= sz) 
		{
			Z_scripterrlog("Pos (%d) passed to %s is outside the bounds of array %d. Aborting.\n", pos, "WriteInts()", arrayptr);
			return;
		}
		
		if(count < 0 || unsigned(count) > sz-pos) count = sz-pos;
		std::vector<int32_t> data(count);
		for(int32_t q = 0; q < count; ++q)
		{
			data[q] = am.get(q+pos);
		}
		SET_D(rEXP1, 10000L * fwrite((const void*)&(data[0]), 4, count, f->file));
		check_file_error(GET_REF(fileref));
	}
}

void do_file_getchar()
{
	SET_D(rEXP1, -10000L); //-1 == EOF; error value

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		SET_D(rEXP1, fgetc(f->file) * 10000L);
		check_file_error(GET_REF(fileref)); // TODO: should be checking file error before setting rEXP1.
	}
}

void do_file_putchar()
{
	int32_t c = get_register(sarg1) / 10000;
	SET_D(rEXP1, -10000L); //-1 == EOF; error value

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		if(char(c) != c)
		{
			Z_scripterrlog("Invalid character val %d passed to PutChar(); value will overflow.", c);
			c = char(c);
		}
		SET_D(rEXP1, fputc(c, f->file) * 10000L);
		check_file_error(GET_REF(fileref)); // TODO: should be checking file error before setting rEXP1.
	}
}

void do_file_ungetchar()
{
	int32_t c = get_register(sarg1) / 10000;

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		if(char(c) != c)
		{
			Z_scripterrlog("Invalid character val %d passed to UngetChar(); value will overflow.", c);
			c = char(c);
		}
		SET_D(rEXP1, ungetc(c,f->file) * 10000L);
		check_file_error(GET_REF(fileref)); // TODO: should be checking file error before setting rEXP1.
		return;
	}
	SET_D(rEXP1, -10000L); //-1 == EOF; error value
}

void do_file_seek()
{
	int32_t pos = get_register(sarg1); //NOT /10000 -V
	int32_t origin = get_register(sarg2) ? SEEK_CUR : SEEK_SET;
	SET_D(rEXP1, 0);

	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		SET_D(rEXP1, fseek(f->file, pos, origin) ? 0L : 10000L);
		check_file_error(GET_REF(fileref)); // TODO: should be checking file error before setting rEXP1.
	}
}

void do_file_rewind()
{
	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		//fseek(f->file, 0L, SEEK_END);
		rewind(f->file);
		check_file_error(GET_REF(fileref));
	}
}

void do_file_clearerr()
{
	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		clearerr(f->file);
	}
}

void do_file_geterr()
{
	if(user_file* f = checkFile(GET_REF(fileref), true))
	{
		int32_t err = ferror(f->file);
		int32_t arrayptr = get_register(sarg1);
		if(err)
		{
			string error = strerror(err);
			ArrayH::setArray(arrayptr, error);
		}
		else
		{
			ArrayH::setArray(arrayptr, "\0");
		}
	}
}

}

user_file::~user_file()
{
	if (file)
		fclose(file);
}

void user_file::close()
{
	if(file) fclose(file);
	file = NULL;
	filepath = "";
}

int32_t user_file::do_remove()
{
	if(file) fclose(file);
	file = NULL;
	int32_t r = remove(filepath.c_str());
	filepath = "";
	return r;
}

void user_file::setPath(const char* buf)
{
	if(buf)
		filepath = buf;
	else filepath = "";
}

void files_init()
{
	user_files.clear();
}

user_file* checkFile(int32_t ref, bool req_file, bool skipError)
{
	user_file* file = user_files.check(ref, skipError);
	if (file && req_file && !file->file)
	{
		if (skipError) return NULL;

		scripting_log_error_with_context("Script attempted to reference an invalid file!");
		Z_scripterrlog("File with UID = %d does not have an open file connection!\n", ref);
		Z_scripterrlog("Use '->Open()' or '->Create()' to hook to a system file.\n");
		return NULL;
	}
	return file;
}

std::optional<int32_t> file_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case FILEPOS:
		{
			if (auto f = checkFile(GET_REF(fileref), true))
				ret = ftell(f->file); //NOT *10000 -V
			else ret = -10000L;
			break;
		}
		case FILEEOF:
		{
			if (auto f = checkFile(GET_REF(fileref), true))
				ret = feof(f->file) ? 10000L : 0L; //Boolean
			else ret = -10000L;
			break;
		}
		case FILEERR:
		{
			if (auto f = checkFile(GET_REF(fileref), true))
				ret = ferror(f->file) * 10000L;
			else ret = -10000L;
			break;
		}

		default: return std::nullopt;
	}

	return ret;
}

bool file_set_register(int32_t reg, int32_t value)
{
	return false;
}

std::optional<int32_t> file_run_command(word command)
{
	extern ScriptType curScriptType;
	extern word curScriptNum;
	extern int32_t curScriptIndex;

	ScriptType type = curScriptType;
	int32_t i = curScriptIndex;

	switch (command)
	{
		case FILECLOSE:
		{
			do_fclose();
			break;
		}
		case FILEFREE:
		{
			do_deallocate_file();
			break;
		}
		case FILEOWN:
		{
			if (auto f = checkFile(GET_REF(fileref), false))
				own_script_object(f, type, i);
			break;
		}
		case FILEISALLOCATED:
		{
			do_file_isallocated();
			break;
		}
		case FILEISVALID:
		{
			do_file_isvalid();
			break;
		}
		case FILEALLOCATE:
		{
			do_allocate_file();
			break;
		}
		case FILEFLUSH:
		{
			do_fflush();
			break;
		}
		case FILEREMOVE:
		{
			do_fremove();
			break;
		}
		case FILEGETCHAR:
		{
			do_file_getchar();
			break;
		}
		case FILEREWIND:
		{
			do_file_rewind();
			break;
		}
		case FILECLEARERR:
		{
			do_file_clearerr();
			break;
		}
		case FILEOPEN:
		{
			do_fopen(false, "rb+");
			break;
		}
		case FILECREATE:
		{
			do_fopen(false, "wb+");
			break;
		}
		case FILEOPENMODE:
		{
			int32_t arrayptr = get_register(sarg2);
			string mode;
			ArrayH::getString(arrayptr, mode, 16);
			do_fopen(false, mode.c_str());
			break;
		}
		case FILEREADSTR:
		{
			do_file_readstring();
			break;
		}
		case FILEWRITESTR:
		{
			do_file_writestring();
			break;
		}
		case FILEPUTCHAR:
		{
			do_file_putchar();
			break;
		}
		case FILEUNGETCHAR:
		{
			do_file_ungetchar();
			break;
		}
		case FILEREADCHARS:
		{
			do_file_readchars();
			break;
		}
		case FILEREADBYTES:
		{
			do_file_readbytes();
			break;
		}
		case FILEREADINTS:
		{
			do_file_readints();
			break;
		}
		case FILEWRITECHARS:
		{
			do_file_writechars();
			break;
		}
		case FILEWRITEBYTES:
		{
			do_file_writebytes();
			break;
		}
		case FILEWRITEINTS:
		{
			do_file_writeints();
			break;
		}
		case FILESEEK:
		{
			do_file_seek();
			break;
		}
		case FILEGETERROR:
		{
			do_file_geterr();
			break;
		}

		default: return std::nullopt;
	}

	return RUNSCRIPT_OK;
}
