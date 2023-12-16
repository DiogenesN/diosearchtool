/* this function searches case insensitive for files and directories and has a flag for recursive
 * it writes the results to the given file
 * example:
	const char *substring = "file";
	const char *directoryPath = "/home/username/testdir";
	const char *outputFilePath = "/home/username/output";
	int casesensitive = 0; // 1 for case sensitive
	int ifregular = 0; // 1 search for regular files only and skip folder names
	int recursive = 1; // 1 for recursive search and 0 for current dir search
	FILE *output = fopen("/tmp/.diosearchtool", "w+");
	gint excludeHome = 0 // 1 for excluding HOME from search
	search_all_insensitive(substring, fullpath, outputFilePath, casesensitive, ifregular,
																	recursive, excludeHome, output);
	fclose(output);
 */

#include <glib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

int strcasecmp(const char *s1, const char *s2) {
	while (*s1 && *s2 && tolower(*s1) == tolower(*s2)) {
		s1++;
		s2++;
	}
	return tolower(*s1) - tolower(*s2);
}

char *strcasestr(const char *haystack, const char *needle) {
	size_t len = strlen(needle);
	while (*haystack) {
		if (strncasecmp(haystack, needle, len) == 0) {
			return (char *)haystack;
		}
		haystack++;
	}
	return NULL;
}

void search_all_insensitive(const char *substring, const char *path, const char *outputFilePath,
								int casesensitive, int ifregular, int recursive, int excludeHome,
																					FILE *output) {

	DIR *dir;
	dir = opendir(path);

	if (!dir) {
		perror("Error opening directory");
		return;
	}

	struct dirent *entry;
	struct stat statbuf;

	while ((entry = readdir(dir)) != NULL) {
		// Ignore current and parent directories
		if (strcasecmp(entry->d_name, ".") == 0 || strcasecmp(entry->d_name, "..") == 0) {
			continue;
		}

		// fix double slash if root / is the current dir
		char fullpath[PATH_MAX];
		if (strcmp(path, "/") == 0) {
			snprintf(fullpath, sizeof(fullpath), "/%s", entry->d_name);
		}
		else {
			snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
		}

		if (lstat(fullpath, &statbuf) == -1) {
			if (errno == ENOENT) {
				continue;
			}
			perror("Error getting file status");
			continue;
		}

		// Check if the entry is a symlink and skip symlinks
		if (S_ISLNK(statbuf.st_mode)) {
			// Skip symlinks
			continue;
		}

		// skipping home on switch
		if (excludeHome == 1) {
			gchar *omit = g_strdup_printf("%s/%s", path, entry->d_name);
			gchar *homeDir = g_strdup_printf("%s", getenv("HOME"));
			if (strcmp(omit, homeDir) == 0) {
				printf("\nomitting HOME\n\n");
				continue;
			}
			g_free(omit);
			g_free(homeDir);
		}

		////////////////////////////// check if case sensitive is on /////////////////////////
		if (casesensitive == 0 && ifregular == 0) {
		// search for all files and directories case insensitive
			if (strcasestr(entry->d_name, substring) != NULL) {
				if (S_ISDIR(statbuf.st_mode)) {
					//printf("Found directory: %s/%s\n", path, entry->d_name);
					fprintf(output, "%s\n", "folder");
					fprintf(output, "%s/%s\n", path, entry->d_name);
				}
				else if (S_ISREG(statbuf.st_mode)) {
					//printf("Found file: %s/%s\n", path, entry->d_name);
					fprintf(output, "%s\n", "emblem-documents-symbolic");
					fprintf(output, "%s/%s\n", path, entry->d_name);
				}
			}
		}
		// search for all files and directories case sensitive
		else if (casesensitive == 1 && ifregular == 0) {
			if (strstr(entry->d_name, substring) != NULL) {
				if (S_ISDIR(statbuf.st_mode)) {
					//printf("Found directory: %s/%s\n", path, entry->d_name);
					fprintf(output, "%s\n", "folder");
					fprintf(output, "%s/%s\n", path, entry->d_name);
				}
				else if (S_ISREG(statbuf.st_mode)) {
					//printf("Found file: %s/%s\n", path, entry->d_name);
					fprintf(output, "%s\n", "emblem-documents-symbolic");
					fprintf(output, "%s/%s\n", path, entry->d_name);
				}
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////
		////////////////////////// check if regular files only is on //////////////////////////
		// search for regular files only (skip directories) case insensitive
		else if (casesensitive == 0 && ifregular == 1) {
			if (strcasestr(entry->d_name, substring) != NULL) {
				if (S_ISREG(statbuf.st_mode)) {
					//printf("Found file: %s/%s\n", path, entry->d_name);
					fprintf(output, "%s\n", "emblem-documents-symbolic");
					fprintf(output, "%s/%s\n", path, entry->d_name);
				}
			}
		}
		// search for regular files only (skip directories) case sensitive
		else if (casesensitive == 1 && ifregular == 1) {
			if (strstr(entry->d_name, substring) != NULL) {
				if (S_ISREG(statbuf.st_mode)) {
					//printf("Found file: %s/%s\n", path, entry->d_name);
					fprintf(output, "%s\n", "emblem-documents-symbolic");
					fprintf(output, "%s/%s\n", path, entry->d_name);
				}
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////// Recursive call for directories /////////////////////////
		if (recursive == 1 && S_ISDIR(statbuf.st_mode)) {
			search_all_insensitive(substring, fullpath, outputFilePath, casesensitive, ifregular,
																	recursive, excludeHome, output);
		}
	}
	free(entry);
	closedir(dir);
}
