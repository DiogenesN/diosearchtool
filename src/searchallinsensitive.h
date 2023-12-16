#ifndef SEARCHALLINSENSITIVE_H_
#define SEARCHALLINSENSITIVE_H_

	void search_all_insensitive(const char *substring, const char *path, const char *outputFilePath,
									int casesensitive, int ifregular, int recursive, int excludeHome,
																						FILE *output);

#endif
