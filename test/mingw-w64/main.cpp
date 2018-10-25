
#include <stdio.h>
#include "../../ZipLibWrap.h"

int main()
{
	std::string fname = "../PokerMaster_2.6.5_official.apk";
	std::string repl_data = "Çהוסü בûכ main.luac";
  std::string version_data = "Version=0.0.1";
	std::vector<std::string> files;	

	ZipLibWrap zip(fname);
	
	if(zip.replaceFile("assets/src/main.luac", (unsigned char*)repl_data.c_str(), repl_data.size())&&
    zip.addFile("assets/src/versionInfo/loggerVersion.lua",(unsigned char*)version_data.c_str(),version_data.size()))
	{	
		// zip.listFiles(files);
		// for(std::string file : files)
		// {
		// 	printf("%s\n", file.c_str());
		// }

		zip.saveToFile(fname);
	}

	if(zip.getLastError() != "")
	{
		printf("have zip error - %s\n", zip.getLastError().c_str());
	}

	printf("Done\n");

	return 0;
}
