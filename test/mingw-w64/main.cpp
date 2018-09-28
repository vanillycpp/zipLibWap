
#include <stdio.h>
#include "../../ZipLibWrap.h"

int main()
{
	std::string fname = "../vanilly.zip";
	std::string repl_data = "dddddddddddd";

	ZipLibWrap zip(fname);

	std::vector<std::string> files;
	zip.replaceFile("1.json", (void*)repl_data.c_str(), repl_data.size());
	
	zip.listFiles(files);
	for(std::string file : files)
	{
		printf("%s\n", file.c_str());
	}

	zip.saveToFile("../vanilly.zip");

	if(zip.getLastError() != "")
	{
		printf("have zip error - %s\n", zip.getLastError().c_str());
	}

	return 0;
}
