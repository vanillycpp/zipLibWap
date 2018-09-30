
#include <stdio.h>
#include "../../ZipLibWrap.h"

int main()
{
	std::string fname = "../vanilly.zip";
	std::string repl_data = "123askdhalsdnlasdssssssssssssssssssss";
	std::vector<std::string> files;	

	ZipLibWrap zip(fname);
	
	if(zip.replaceFile("1.json", (unsigned char*)repl_data.c_str(), repl_data.size()))
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
