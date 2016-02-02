
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#include <windows.h>
#include <direct.h>
#include <string>

/// Currently WIN32 only
/// Outputs a list of all files of type in a directory.
static void GenerateUnityFileList(const char* directory, const char* outputFilename, const char* filter = "*.*")
{
	std::string str = directory;
	str += filter;

	WIN32_FIND_DATAA findData = {};
	HANDLE handle = FindFirstFileA(str.c_str(), &findData);

	FILE* f = fopen(outputFilename, "w");

	if(handle != INVALID_HANDLE_VALUE)
	{
		std::string fileName;
		do
		{
			fileName = findData.cFileName;

			if(!fileName.empty())
			{
				if( (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
					(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
					continue;

				if(fileName.size() >= 4 &&
					(fileName.substr(fileName.size()-4, 4) == ".cpp")
				{
					fileName.insert(0, "#include \"");
					fileName += "\"\n";

					fwrite(fileName.c_str(), 1, fileName.size(), f);
				}
			}
		}
		while(FindNextFileA(handle, &findData));

		FindClose(handle);
	}

	fclose(f);
}

