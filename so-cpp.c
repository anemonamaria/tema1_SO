#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// #define MY_DEF 4
#define BUFSIZE 256
#define HASHSIZE 256

typedef struct hashmap {
	char symbol[BUFSIZE];
	char mapping[BUFSIZE];
	struct hashmap *next;
} hashmap;

// function to generate the hash of the table
int hashCode(char symbol[]) {
	int hashed = 0;
	int i;

	for(i = 0; i < strlen(symbol); i++)
		hashed += symbol[i] % HASHSIZE;

	return hashed % HASHSIZE;
}

// function to insert a value into the hash table
void insert(hashmap *hmap[], char symbol[], char mapping[]) {
	hashmap *map = hmap[hashCode(symbol)];
	hashmap *to_add = (hashmap *) malloc(sizeof(hashmap));

	if(to_add == NULL)
		exit(12);
	else {
		strcpy(to_add->symbol, symbol);
		strcpy(to_add->mapping, mapping);
		to_add->next = map;
		hmap[hashCode(symbol)] = to_add;
	}
}

// function to search for a mapping in our hashmap
char *getMapping(hashmap *hmap[], char symbol[]) {
	hashmap *map = hmap[hashCode(symbol)];

	while(map) {
		if (strcmp(map->symbol, symbol) == 0) {
			return map->mapping;
			break;
		}
		map = map->next;
	}

	return "";
}

// removes from the hashmap the symbol
void removeFromHash(hashmap *hmap[], char symbol[]) {
	hashmap **map = &hmap[hashCode(symbol)];

	while(*map) {
		hashmap *tmp = *map;
		if(strcmp(tmp->symbol, symbol) == 0) {
			*map = tmp->next;
			free(tmp);
			break;
		} else {
			map = &(*map)->next;
		}
	}
}

// function to free the hashmap
void free_hashmap(hashmap *hmap[])
{
	int i;
	hashmap *aux;
	hashmap *current;

	for (i = 0; i < HASHSIZE; i++) {
		aux = hmap[i];
		while (aux) {
			current = aux;
			aux = aux->next;
			free(current);
		}
	}
}

// TODO UNDMODIFIED
// function to return a new line for the processed file
char *changeLine(char *line, hashmap *hmap[])
{
	char aux2[BUFSIZE], buffer[BUFSIZE], value[BUFSIZE];
	char *aux, *p, *q;
	int len = 0;
	strcpy(aux2, line);
	strcpy(buffer, line);
	aux = strtok(line, "\t[]{}<>=+-*/%!&|^.,:;()\\\n ");
	while (aux) {
		char *mapping = getMapping(hmap, aux);
		if (strcmp(mapping, "") != 0) {

			q = strstr(aux2, "\"");
			if (q) {
				q += 1;
				p = strstr(aux2, aux);
				if (q <= p) {
					q = strstr(q, "\"");
					p += strlen(aux);
					if (p - aux2 < q - aux2)
						p = strstr(q, aux);
					else
						p = strstr(aux2, aux);
				}
			} else
				p = strstr(aux2, aux);

			strncpy(buffer, aux2, p-aux2);
			strcpy(value, getMapping(hmap, aux));
			len = strlen(aux);
			sprintf(buffer+(p-aux2), "%s%s", value, p + len);
			strcpy(aux2, buffer);

		}
		aux = strtok(NULL, "\t[]{}<>=+-*/%!&|^.,:;()\\\n ");
	}
	strcpy(line, buffer);

	return line;
}
////////////////////////////////////////////


void parseFile(hashmap *hmap[], FILE *input_file, FILE *output_file, int dir_size,
	char *directory[]);

//  function to verify if the line read from the file is of type #include
int include_line(hashmap *hmap[], char line[], int dir_size, FILE *output_file,
		char *directory[]) {

	if (strstr(line, "#include")) {
		char *path_to_file = malloc(100 * sizeof(char));
		if(!path_to_file)
			exit(12);
		strcpy(path_to_file, "");
		char *input_file = strtok(line, "\"");
		input_file = strtok(NULL, "\"");
		int i;
		FILE *open_input_file;

		for (i = 0; i < dir_size; i++) {
			strncpy(path_to_file, directory[i], strlen(directory[i]) + 1);
			strncat(path_to_file, "/", 1);
			strncat(path_to_file, input_file, strlen(input_file) + 1);
			open_input_file = fopen(path_to_file, "r");

			if (open_input_file) {
				parseFile(hmap, open_input_file, output_file, dir_size, directory);
				break;
			}
		}
		free(path_to_file);
		if (!open_input_file) {
			for (i = 0; i < dir_size; i++)
				free(directory[i]);
			free_hashmap(hmap);
			exit(12);
		}
		return 1;
	}
	return 0;
}

///////////////////////////////TODO MODIFY
int define_line(hashmap *table[], FILE *input_file, char line[]) {
	char *df, *mdf, *key, *value, *nmdf, *ndf, *aux;
	int ret = 0;
	char auxval[BUFSIZE] = {0};
	char auxkey[BUFSIZE], auxline[BUFSIZE];
	int p = 0, nrSpace = 0, space = 0, lastLine = 0, len = 0;
	char c;

	strcpy(auxline, line);
	// printf("auxline %s\n", auxline);
	ndf = strstr(auxline, "\"#define");
	df = strstr(line, "#define");
	// printf("ndf %s and df %s\n", ndf, df);
	if (!ndf && df) {
		ret = 1;
		strcpy(auxline, line);
		nmdf = strstr(auxline, "\\n");
		mdf = strstr(line, "\\");
		if (!nmdf && mdf) {
			strtok(line, " ");
			key = strtok(NULL, " ");
			strcpy(auxkey, key);
			value = strtok(NULL, "\\");
			strcpy(auxval, value);
			p = strlen(value);
			while (mdf && fgets(line, BUFSIZE, input_file)) {
				mdf = strstr(line, "\\");
				if (!mdf)
					lastLine = 1;
				if (mdf || lastLine) {
					c = line[0];
					nrSpace = 0;
					while (c == ' ') {
						nrSpace++;
						c = line[nrSpace];
					}
					aux = line + nrSpace;
					if (lastLine)
						value = strtok(aux, "\n");
					else
						value = strtok(aux, "\\");
					if (space) {
						strncpy(auxval + p, " ", 1);
						p += 1;
					}
					len = strlen(value);
					strncpy(auxval + p, value, len);
					space = 1;
					p += strlen(value);
				}
			}
			insert(table, auxkey, auxval);
		} else {
			strcpy(auxkey, line);
			strtok(line, " ");
			key = strtok(NULL, " ");
			value = strtok(NULL, "'\n'");
			if (value)
				insert(table, key, changeLine(value, table));
		}
	}
	return ret;
}

int ifdfLine(hashmap *table[], FILE *input_file, FILE *output_file, char line[]) {
	char *iffdef, *key;
	int ret = 0, noPrint = 0, branch;

	iffdef = strstr(line, "#ifdef");
	if (iffdef) {
		ret = 1;
		strtok(line, " ");
		key = strtok(NULL, "\n");
		if (strcmp(getMapping(table, key), "") == 0)
			noPrint = 1;
		while (fgets(line, BUFSIZE, input_file)) {
			if (strstr(line, "else")) {
				if (strcmp(getMapping(table, key), "") == 0)
					noPrint = 0;
				else
					noPrint = 1;
				continue;
			}
			if (!noPrint && define_line(table, input_file, line))
				continue;
			if (strstr(line, "#undef")) {
				strtok(line, " ");
				key = strtok(NULL, "\n");
				removeFromHash(table, key);
				continue;
			}
			if (!noPrint && strstr(line, "#endif") == NULL)
				fputs(changeLine(line, table), output_file);
			else if (strstr(line, "endif"))
				break;
		}
	}
	return ret;
}

int if_line(hashmap *table[], FILE *input_file, FILE *output_file, char line[]) {
	char *iff, *value, *chg, *eif;
	int noPrint, choose = 0, ret = 0;

	iff = strstr(line, "#if ");
	if (iff) {
		ret = 1;
		noPrint = 0;
		strtok(line, " ");
		value = strtok(NULL, "\n");
		if (strcmp(changeLine(value, table), "0") != 0) {
			while (fgets(line, BUFSIZE, input_file)) {
				if (strstr(line, "#else"))
					noPrint = 1;
				if (!noPrint && define_line(table, input_file, line))
					continue;
				if (!noPrint && strstr(line, "#endif") == NULL)
					fputs(changeLine(line, table), output_file);
				if (strstr(line, "#endif") != NULL)
					break;
			}
		} else {
			noPrint = 1;
			while (fgets(line, BUFSIZE, input_file)) {
				if (strstr(line, "#elif")) {
					strtok(line, " ");
					value = strtok(NULL, "\n");
					chg = changeLine(value, table);
					if (strcmp(chg, "0") != 0) {
						noPrint = 0;
						choose = 1;
						fgets(line, BUFSIZE, input_file);
					}
				}
				if (strstr(line, "#else")) {
					if (choose)
						noPrint = 1;
					else
						noPrint = 0;
					fgets(line, BUFSIZE, input_file);
				}
				if (strstr(line, "#endif"))
					break;
				eif = strstr(line, "#endif");
				if (!noPrint && eif == NULL) {
					fputs(changeLine(line, table), output_file);
					continue;
				} else
					continue;
			}
		}
	}
	return ret;
}

int ifndf_line(hashmap *table[], FILE *input_file, FILE *output_file, char line[], int dir_size,
		char **directory) {
	char *iffndef, *key;
	int ret = 0;

	iffndef = strstr(line, "#ifndef");
	if (iffndef) {
		ret = 1;
		strtok(line, " ");
		key = strtok(NULL, "\n");
		if (strcmp(getMapping(table, key), "") == 0) {
			while (fgets(line, BUFSIZE, input_file)) {
				if (include_line(table, line, dir_size, output_file, directory))
					continue;
				if (ifndf_line(table, input_file, output_file, line, dir_size, directory))
					continue;
				if (define_line(table, input_file, line))
					continue;
				if (ifdfLine(table, input_file, output_file, line))
					continue;
				if (if_line(table, input_file, output_file, line))
					continue;
				else if (strstr(line, "#endif")) {
					fgets(line, BUFSIZE, input_file);
					break;
				} else if (strcmp(line, "\n") != 0) {
					fputs(changeLine(line, table), output_file);
				}
			}
		} else {
			while (fgets(line, BUFSIZE, input_file))
				if (strstr(line, "#endif"))
					break;
		}
	}
	return ret;
}
//////////////////////////////////

int undefLine(hashmap *table[], FILE* input_file, char line[]) {
	int ret = 0;

	char *isUndefine = strstr(line, "#undef");
	if(isUndefine) {
		strtok(line, " ");
		removeFromHash(table, strtok(NULL, "\n"));
		ret = 1;
	}
	return ret;
}

// function to parse the file
void parseFile(hashmap *table[], FILE *input_file, FILE *output_file, int dir_size,
	char *directory[]) {
	char line[BUFSIZE];

	while(fgets(line, BUFSIZE, input_file)) {

		if(include_line(table, line, dir_size, output_file, directory))
			continue;
		if(define_line(table, input_file, line))
			continue;
		if(ifndf_line(table, input_file, output_file, line, dir_size, directory))
			continue;
		if(ifdfLine(table, input_file, output_file, line))
			continue;
		if(if_line(table, input_file, output_file, line))
			continue;
		if(undefLine(table, input_file, line))
			continue;
		else if(strcmp(line, "\n") != 0) {
			fputs(changeLine(line, table), output_file);

		} else
			fputs("\n", output_file);
	}
}


int main(int argc, char *argv[]) {
	// printf("Hello world %d.\n", MY_DEF);
	FILE *input_file, *output_file;
	input_file = stdin;
	output_file = stdout;

	int dir_size = 1;
	int check_out_file = 0, check_in_file = 0;
	char *directory[HASHSIZE];
	char infile[BUFSIZE] = {0};
	hashmap *table[HASHSIZE];

	for(int i = 0; i < HASHSIZE; i++) {
		table[i]= NULL;
	}

	char *symbol, *mapping;

	// Start checking arguments
	for (int i = 1; i < argc; i++) {
		if(strncmp(argv[i], "-D", 2) == 0) {
			if(strlen(argv[i]) == 2) {
				symbol = strtok(argv[++i],"=");
				mapping = strtok(NULL, " ");
			} else {
				symbol =  strtok(argv[i] + 2, "=");
				mapping = strtok(NULL, " ");

			}

			insert(table, symbol, (mapping != NULL) ? mapping : "");
			continue;
		}

		if(strncmp(argv[i], "-I", 2) == 0) {
			if(strlen(argv[i]) == 2) {
				directory[dir_size] = (char *) calloc(strlen(argv[i + 1]) + 1, sizeof(char));
				if (!directory[dir_size])
					exit(12);
				strcpy(directory[dir_size], argv[i++]);
				dir_size++;
				continue;
			} else {
				directory[dir_size] = (char *) calloc(strlen(argv[i]) - 1, sizeof(char));
				if (!directory[dir_size])
					exit(12);
				strcpy(directory[dir_size], argv[i] + 2);
				dir_size++;
				continue;
			}
		}

		if(strncmp(argv[i], "-o", 2) == 0 && check_out_file == 0) {
			if(strlen(argv[i]) == 2) {
				output_file = fopen(argv[i++], "w");
			} else {
				output_file = fopen(strtok(argv[i] + 2, " "), "w");
			}

			check_out_file = 1;
			continue;
		}

		if (!check_in_file && argv[i]) {
			input_file = fopen(argv[i], "r");
			strcpy(infile, argv[i]);
			// printf("din primul %s in file si %s argv[i]\n", infile, argv[i]);
			if (input_file == NULL)
				exit(12);
			directory[0] = (char *) calloc(strlen(argv[i]) + 1, sizeof(char));
			if (!directory[0])
				exit(12);
			// strcpy(directory[0], getDir(argv[i]));

			// generate the directory name  --->MOD HERE MAYBE?
			char result[BUFSIZE] = {0};
			char aux[BUFSIZE] = {0};
			char *token = strtok(argv[i], "/");
			if(!token)
				strcpy(directory[0], "./");
			while(token) {
				strcpy(aux, token);
				strcat(aux, "/");
				token = strtok(NULL, "/");
				if(token == NULL)
					break;
				strcat(result, aux);
			}
			strcpy(directory[0], result);

			check_in_file = 1;
			continue;
		}

		if (!check_out_file && argv[i]) {
			if (strcmp(argv[i], infile) == 0)
				exit(12);
			output_file = fopen(argv[i], "w");
			if (output_file == NULL)
				exit(12);
			check_out_file = 1;
			continue;
		}

		if (argv[i])
			exit(12);
	}


	parseFile(table, input_file, output_file, dir_size, directory);
	int i;
	for (i = 0; i < dir_size; i++)
		free(directory[i]);
	free_hashmap(table);


	if(input_file)
		fclose(input_file);
	if(output_file)
		fclose(output_file);

	return 0;
}