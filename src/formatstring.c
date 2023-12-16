/* formatting a pango string inserting wrapping a key word in between start and end formats
 * example:
	char *startFormat = "<span size='13pt' weight='bold' bgcolor='#A4EA00' foreground='red'>";
	char *endFormat = "</span>";
	char *key = "dir";
	const char *str = "/home/diogenes/mydirectory";
	char *result = format_string(str, key, startFormat, endFormat);
	printf("%s\n", result);
	output:
	/home/diogenes/my<span size='13pt' weight='bold' bgcolor='#A4EA00' foreground='red'>dir</span>ectory
	free(result);
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *format_string(const char *str, const char *key, const char *startFormat, const char *endFormat) {
	const char *keyPos = strstr(str, key);

	if (keyPos == NULL) {
		// Key not found in the string
		return strdup(str);
    }

	size_t prefixLength = keyPos - str;
	size_t suffixLength = strlen(str) - prefixLength - strlen(key);

	// Allocate memory for the new string
	char *result = (char *)malloc(prefixLength + strlen(startFormat) + strlen(key) + strlen(endFormat) + suffixLength + 1);

	if (result == NULL) {
		perror("Memory allocation failed");
		exit(EXIT_FAILURE);
	}

	// Copy the prefix
	strncpy(result, str, prefixLength);
	result[prefixLength] = '\0';

	// Concatenate the startFormat and key
	strcat(result, startFormat);
	strcat(result, key);

	// Concatenate the endFormat
	strcat(result, endFormat);

	// Concatenate the suffix
	strcat(result, keyPos + strlen(key));

	return result;
}
