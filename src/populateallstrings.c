/* this function takes a path to a text file containing data and a char array and populates
 * the chararray wtth the content of the text file
 * example:
	const char *filePath = "/tmp/.diosearchtool";
	char **strArr;
	populate_allstrings(filePath, &strArr);
	// Don't forget to free the allocated memory when done
	for (int i = 0; strArr[i] != NULL; i++) {
		printf("str[%d]=%s\n", i, strArr[i]);
		free(strArr[i]);
	}
	free(strArr);
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void populate_allstrings(const char *pathToFile, char ***allStrings) {
	int lineCount = 0;
	char stringsBuff[2048];
	*allStrings = (char **)malloc(sizeof(char *) * 3);

	FILE *stringsFile = fopen(pathToFile, "r");

	while (fgets(stringsBuff, sizeof(stringsBuff), stringsFile) != NULL) {
		// Remove the newline character if present
		size_t length = strlen(stringsBuff);
		if (length > 0 && stringsBuff[length - 1] == '\n') {
			stringsBuff[length - 1] = '\0';
		}

		// Check if the array needs to be resized
		if (lineCount > 0) {
			char **new_allStrings = (char **)realloc(*allStrings, sizeof(char *) * (lineCount + 3));
			if (new_allStrings == NULL) {
				// Handle realloc failure
				fprintf(stderr, "Memory reallocation failed\n");
				// Cleanup and exit
				fclose(stringsFile);
				for (int i = 0; i < lineCount; i++) {
					free((*allStrings)[i]);
				}
				free(*allStrings);
				return;
			}
			*allStrings = new_allStrings;
		}

		// Allocate memory for the current line
		(*allStrings)[lineCount] = strdup(stringsBuff);

		if ((*allStrings)[lineCount] == NULL) {
			// Handle strdup failure
			fprintf(stderr, "Memory allocation failed\n");
			// Cleanup and exit
			fclose(stringsFile);
			for (int i = 0; i < lineCount; i++) {
				free((*allStrings)[i]);
			}
			free(*allStrings);
			return;
		}
		// Increment line counter
		lineCount++;
	}
	(*allStrings)[lineCount] = NULL;

	fclose(stringsFile);
}
