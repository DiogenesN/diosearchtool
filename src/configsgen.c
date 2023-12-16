/* creates initial config directory and file */

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

void create_configs() {
	const char *HOME = getenv("HOME");

	if (HOME == NULL) {
		fprintf(stderr, "Unable to determine the user's home directory.\n");
		return;
	}

	char dirConfigBuff[777];
	char fileConfigBuff[777];
	const char *dirConfig	= "/.config/diosearchtool";
	const char *fileConfig	= "/diosearchtool.conf";

	snprintf(dirConfigBuff, sizeof(dirConfigBuff), "%s%s", HOME, dirConfig);
	snprintf(fileConfigBuff, sizeof(fileConfigBuff), "%s%s%s", HOME, dirConfig, fileConfig);

	DIR *confDir = opendir(dirConfigBuff);

	// cheks if the file already exists
	if (confDir) {
		// directory exists do nothing
		closedir(confDir);
		return;
	}
	else {
		// directory not found
		printf("created initial configs\n");
		mkdir(dirConfigBuff, 0755);
		closedir(confDir);
	}

	FILE *config = fopen(fileConfigBuff, "w+");

	fprintf(config, "%s\n", "open=");
	fprintf(config, "%s\n", "filemanager=");

	fclose(config);
}
