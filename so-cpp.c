#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 256
#define HASHSIZE 256

typedef struct hashmap {
	char symbol[SIZE];
	char mapping[SIZE];
	struct hashmap *next;
} hashmap;

// TODO scrie in readme despre codingstyle
// TODO revizuieste tema

void read_file(hashmap *hmap[], FILE *input_file, FILE
	*output_file, int directories_number, char
	*dir_path[]);

// function to generate the hash of the table
int hashCode(char *symbol)
{
	int hashed = 0;
	int i;

	for (i = 0; i < strlen(symbol); i++)
		hashed += symbol[i] % HASHSIZE;

	return hashed % HASHSIZE;
}

// function to insert a value into the hash table
void insert(hashmap *hmap[], char *symbol, char *mapping)
{
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
char *getMapping(hashmap *hmap[], char *symbol)
{
	hashmap *map = hmap[hashCode(symbol)];

	while(map) {
		if (strcmp(map->symbol, symbol) == 0)
			return map->mapping;
		map = map->next;
	}

	return "";
}

// removes from the hashmap the symbol
void removeFromHash(hashmap *hmap[], char *symbol)
{
	hashmap *tmp, **map = &hmap[hashCode(symbol)];

	while(*map) {
		tmp = *map;
		if(strcmp(tmp->symbol, symbol) != 0) {
			map = &(*map)->next;
		} else {
			*map = tmp->next;
			free(tmp);
			break;
		}
	}
}

// function to free the hashmap
void free_hashmap(hashmap *hmap[])
{
	hashmap *tmp, *current;
	int i;

	for (i = 0; i < HASHSIZE; i++) {
		tmp = hmap[i];
		while (tmp) {
			current = tmp;
			tmp = tmp->next;
			free(current);
		}
	}
}

// function to return a new line for the processed file
char *overwrite(char *line, hashmap *hmap[], FILE
	*output_file)
{
	char  *symbol, buffer[SIZE], tmp[SIZE], *counter,
		*needle, *tok;

	strcpy(tmp, line);
	strcpy(buffer, line);
	tok = strtok(line, "\t []{}<>=+-*/%!&|^.,:;()\\");

	while (tok) {
		if (strcmp(getMapping(hmap, tok), "") != 0) {
			needle = strstr(tmp, "\"");
			if (needle) {
				needle += 1;
				counter = strstr(tmp, tok);
				if (needle <= counter) {
					needle = strstr(needle, "\"");
					counter += strlen(tok);
					if (counter - tmp < needle - tmp)
						counter = strstr(needle, tok);
					else
						counter = strstr(tmp, tok);
				}
			} else
				counter = strstr(tmp, tok);

			strncpy(buffer, tmp, counter-tmp);
			symbol = malloc(sizeof(char) * strlen(
				getMapping(hmap, tok)) + 1);
			if(!symbol)
				exit(12);

			memcpy(symbol, getMapping(hmap, tok),
				strlen(getMapping(hmap, tok)) + 1);
			sprintf(buffer+(counter-tmp), "%s%s", symbol,
				counter + strlen(tok));
			strcpy(tmp, buffer);
			free(symbol);

		}
		tok = strtok(NULL, "\t []{}<>=+-*/%!&|^.,:;()\\");
	}
	strcpy(line, buffer);

	if(output_file != NULL)
		fputs(line, output_file);

	return line;
}

//  function to verify if the line read from the file is
// of type #include
void include_line(hashmap *hmap[], char *line, int
	directories_number, FILE *output_file, char
	*dir_path[])
{
	char *path_to_file;
	int i;
	FILE *open_input_file;
	char *input_file;

	input_file = strtok(line, "\"");
	path_to_file = malloc(SIZE * sizeof(char));
	if(!path_to_file)
		exit(12);

	strcpy(path_to_file, "");
	input_file = strtok(NULL, "\"");


	for (i = 0; i < directories_number; i++) {
		memcpy(path_to_file, dir_path[i], strlen(
			dir_path[i]) + 1);
		strncat(path_to_file, "/", 1);
		strncat(path_to_file, input_file, strlen(
			input_file) + 1);
		open_input_file = fopen(path_to_file, "r");

		if (open_input_file) {
			read_file(hmap, open_input_file, output_file,
			directories_number,	dir_path);
			break;
		}
	}

	free(path_to_file);
	if (!open_input_file) {
		for (i = 0; i < directories_number; i++)
			free(dir_path[i]);
		free_hashmap(hmap);
		exit(12);
	}
}

// function to parse a define_line
void define_line(hashmap *hmap[], FILE *input_file, char
	*line)
{
	char  *has_more_arguments, *symbol, *mapping;
	char *auxmapping, *auxsymbol;
	int length_mapping = 0, number_of_spaces = 0;


	has_more_arguments = strstr(line, "\\");
	symbol = strtok(line, " ");
	symbol = strtok(NULL, " ");
	if (has_more_arguments) {
		auxsymbol = malloc(sizeof(char *) * (strlen(symbol)
		+ 1));
		if(!auxsymbol)
			exit(12);

		memcpy(auxsymbol, symbol, (strlen(symbol) +1));
		mapping = strtok(NULL, "\\");

		auxmapping = calloc(strlen(mapping) + 1, sizeof(
			char *));
		if(!auxmapping)
			exit(12);

		memcpy(auxmapping, mapping, strlen(mapping) + 1);
		length_mapping = strlen(mapping);

		while (has_more_arguments && fgets(line, SIZE,
			input_file)) {
			has_more_arguments = strstr(line, "\\");
			number_of_spaces = 0;
			while (line[number_of_spaces] == ' ') {
				number_of_spaces++;
				line++;
			}
			if (!has_more_arguments)
				mapping = strtok(line, "\n");
			else
				mapping = strtok(line, "\\");

			strncpy(auxmapping + length_mapping, mapping,
				strlen(mapping));
			length_mapping += strlen(mapping);
		}
		if(strcmp(getMapping(hmap, auxsymbol), "") == 0)
			insert(hmap, auxsymbol, auxmapping);
		free(auxsymbol);
		free(auxmapping);
	} else {
		auxsymbol = malloc(sizeof(char *) * (strlen(line)
			+ 1));
		if(!auxsymbol)
			exit(12);

		// memcpy(auxsymbol, line, strlen(line) + 1);

		mapping = strtok(NULL, "'\n'");
		if (mapping && strcmp(getMapping(hmap, symbol), "")
			== 0)
			insert(hmap, symbol, overwrite(mapping, hmap,
				NULL));
		free(auxsymbol);
	}
}

// function to parse a ifdef line
void ifdef_line(hashmap *hmap[], FILE *input_file, FILE
	*output_file, char *line)
{
	char *symbol = strtok(line, " ");

	symbol = strtok(NULL, "\n");

	while (fgets(line, SIZE, input_file)) {
		if (!strcmp(getMapping(hmap, symbol), "") == 0 &&
			strncmp(line,
			"#define", 7) == 0) {
			define_line(hmap, input_file, line);
			continue;
		}
		if (strstr(line, "#undef")) {
			symbol = strtok(line, " ");
			removeFromHash(hmap, strtok(NULL, "\n"));
			continue;
		}
		if (!strcmp(getMapping(hmap, symbol), "") == 0 &&
			strstr(line, "#endif") == NULL)
			overwrite(line, hmap, output_file);
		else if (strstr(line, "endif"))
			break;
	}
}

// function tp parse a if line
void if_line(hashmap *hmap[], FILE *input_file, FILE
	*output_file, char *line)
{
	char *tmp = strtok(line, " ");
	int can_write = 0, aux = 0;

	if (strncmp(overwrite(strtok(NULL, "\n"), hmap, NULL),
		"0", 1) != 0) {
		while (fgets(line, SIZE, input_file)) {
			if (strncmp(line, "#endif", 6) == 0)
				break;
			else if (strncmp(line, "#else", 5) == 0)
				can_write = 1;
			else if (!can_write && strncmp(line, "#define",
				7) == 0) {
				define_line(hmap, input_file, line);
				continue;
			} else if (!can_write && strncmp(line, "#endif",
				7) != 0)
				overwrite(line, hmap, output_file);

		}
	} else {
		can_write = 1;
		while (fgets(line, SIZE, input_file)) {
			if (strncmp(line, "#elif", 5) == 0) {
				tmp = strtok(line, " ");
				if (strncmp(overwrite(strtok(NULL, "\n"),
					hmap, NULL), "0", 1) != 0) {
					can_write = 0;
					aux = 1;
					fgets(line, SIZE, input_file);
				}
			}
			if (strncmp(line, "#else", 5) == 0) {
				if (aux)
					can_write = 1;
				else
					can_write = 0;
				fgets(line, SIZE, input_file);
			}
			if (strncmp(line, "#endif", 6) == 0)
				break;
			else if (!can_write) {
				overwrite(line, hmap, output_file);
				continue;
			}
		}
	}
}

// function to parse an ifndef line
void ifndef_line(hashmap *hmap[], FILE *input_file, FILE
	*output_file, char *line, int directories_number,
	char **dir_path)
{
	char *symbol = strtok(line, " ");

	symbol = strtok(NULL, "\n");

	while (fgets(line, SIZE, input_file)) {
		if (strcmp(getMapping(hmap, symbol), "") == 0) {
			if (strncmp(line, "#define", 7) == 0) {
				define_line(hmap, input_file, line);
				continue;
			}
			if (strncmp(line, "#if ", 4) == 0) {
				if_line(hmap, input_file, output_file,
					line);
				continue;
			}
			if (strncmp(line, "#ifdef", 6) == 0) {
				ifdef_line(hmap, input_file, output_file,
					line);
				continue;
			}
			if (strncmp(line, "#ifndef", 7) == 0) {
				ifndef_line(hmap, input_file, output_file,
					line,
					directories_number, dir_path);
				continue;
			}
			if (strncmp(line, "#endif", 6) == 0) {
				fgets(line, SIZE, input_file);
				break;
			}
			if (strncmp(line, "#include", 8) == 0) {
				include_line(hmap, line, directories_number,
					output_file,
					dir_path);
				continue;
			}
			if (strcmp(line, "\n") != 0) {
				overwrite(line, hmap, output_file);
				continue;
			}
		} else if (strncmp(line, "#endif", 6) == 0)
			break;
	}
}

// function to parse an #undef line
int undef_line(hashmap *hmap[], FILE *input_file, char
	*line)
{
	int result = 0;

	if(strstr(line, "#undef")) {
		strtok(line, " ");
		removeFromHash(hmap, strtok(NULL, "\n"));
		result = 1;
	}
	return result;
}

// function to parse the file
void read_file(hashmap *hmap[], FILE *input_file, FILE
	*output_file, int directories_number, char *dir_path[])
{
	char line[SIZE];

	while(fgets(line, SIZE, input_file)) {

		if(strstr(line, "#include")) {
			include_line(hmap, line, directories_number,
				output_file, dir_path);
			continue;
		}
		if(strncmp(line, "#define", 7) == 0) {
			define_line(hmap, input_file, line);
			continue;
		}
		if(strstr(line, "#ifndef")) {
			ifndef_line(hmap, input_file, output_file,
				line, directories_number, dir_path);
			continue;
		}
		if(strstr(line, "#ifdef")) {
			ifdef_line(hmap, input_file, output_file,
				line);
			continue;
		}
		if(strstr(line, "#if ")) {
			if_line(hmap, input_file, output_file, line);
			continue;
		}
		if(undef_line(hmap, input_file, line))
			continue;
		else if(strcmp(line, "\n") != 0)
			overwrite(line, hmap, output_file);
		else
			fputs("\n", output_file);
	}
}


int main(int argc, char *argv[])
{
	FILE *input_file, *output_file;
	int i;
	char *symbol, *mapping;
	char *aux, *token, *argument;
	hashmap **hmap;
	int directories_number = 1;
	int check_in_file = 0;
	char *dir_path[HASHSIZE];
	char *infile = calloc(SIZE, sizeof(char));

	if(!infile)
		exit(12);

	input_file = stdin;
	output_file = stdout;

	hmap = calloc(HASHSIZE, sizeof(hashmap *));

	// Start checking arguments
	for (i = 1; i < argc; i++) {
		if(strncmp(argv[i], "-D", 2) == 0) {
			if(strlen(argv[i]) == 2) {
				symbol = strtok(argv[++i],"=");
				mapping = strtok(NULL, " ");
			} else {
				symbol =  strtok(argv[i] + 2, "=");
				mapping = strtok(NULL, " ");
			}

			insert(hmap, symbol, (mapping != NULL) ?
				mapping : "");
			continue;
		} else if(strncmp(argv[i], "-I", 2) == 0) {
			if(strlen(argv[i]) == 2)
				argument = argv[i++];
			else
				argument = argv[i] + 2;

			dir_path[directories_number] = calloc(strlen(
					argument) + 1, sizeof(char));
			if (!dir_path[directories_number])
				exit(12);

			strcpy(dir_path[directories_number],
				argument);
			directories_number++;
			continue;
		}else if(strncmp(argv[i], "-o", 2) == 0 &&
			output_file == NULL) {
			if(strlen(argv[i]) == 2)
				argument = argv[i++];
			else
				argument = strtok(argv[i] + 2, " ");
			output_file = fopen(argument, "w");
			continue;
		} else if (check_in_file == 0 && argv[i]) {
			input_file = fopen(argv[i], "r");
			strcpy(infile, argv[i]);
			if (input_file == NULL)
				exit(12);
			dir_path[0] = calloc(strlen(argv[i]) + 1,
				sizeof(char));
			if (!dir_path[0])
				exit(12);

			token = strtok(argv[i], "/");
			if(!token)
				strcpy(dir_path[0], "./");
			while(token) {
				aux = calloc(strlen(token) + 2,
					sizeof(char));
				if(!aux)
					exit(12);
				memcpy(aux, token, strlen(token) + 1);
				strcat(aux, "/");
				token = strtok(NULL, "/");
				if(token == NULL)
					break;
				strcat(dir_path[0], aux);
				free(aux);
			}
			free(aux);

			check_in_file = 1;
			continue;
		} else if (output_file == stdout && argv[i]) {
			if (strcmp(argv[i], infile) == 0)
				exit(12);
			output_file = fopen(argv[i], "w");
			if (output_file == NULL)
				exit(12);
			continue;
		} else if (argv[i])
			exit(12);
	}

	read_file(hmap, input_file, output_file,
		directories_number, dir_path);


	for (i = 0; i < directories_number; i++)
		free(dir_path[i]);
	free_hashmap(hmap);
	free(hmap);
	free(infile);

	if(input_file)
		fclose(input_file);
	if(output_file)
		fclose(output_file);

	return 0;
}
