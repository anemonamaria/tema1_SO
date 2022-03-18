#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MY_DEF 4
#define BUFSIZE 256
#define TABLESIZE 256

typedef struct hashmap {
	char symbol[BUFSIZE];
	char mapping[BUFSIZE];
	struct hashmap *next;
} hashmap;

// function to generate the hash of the table
int gen_hash(char symbol[]) {
	int hashed = 0;

	for(int i = 0; i < strlen(symbol); i++) {
		hashed += symbol[i];
	}

	return hashed;
}

// function to insert a value into the hash table
void insert(hashmap *table[], char symbol[], char mapping[]) {
	int hash_key = gen_hash(symbol) % TABLESIZE;
	hashmap *map = table[hash_key];
	hashmap *aux = map;
	hashmap *to_add = (hashmap *) malloc(sizeof(hashmap));

	while(aux) {
		if(strcmp(aux->symbol, symbol) == 0) {
			strcpy(aux->mapping, mapping);
			return;
		}
		aux = aux->next;
	}

	strcpy(to_add->symbol, symbol);
	strcpy(to_add->mapping, mapping);
	to_add->next = map;
	table[hash_key] = to_add;
}

// function to seargh for a mapping in our hashmap
char *getMapping(hashmap *table[], char mapping[]) {
	int hash_key = gen_hash(mapping) % TABLESIZE;
	hashmap *map = table[hash_key];
	hashmap *aux = map;

	while(aux) {
		if (strcmp(aux->mapping, mapping) == 0)
			return aux->mapping;
		aux = aux->next;
	}

	return "";
}

// removes from the hashmap the symbol
void removeFromHash(hashmap *table[], char symbol[]) {
	int hash_key = gen_hash(symbol) % TABLESIZE;
	hashmap *map = table[hash_key];
	hashmap *aux = map;
	hashmap *prev;

	if (aux != NULL && strcmp(aux->symbol, symbol) == 0) {
		map = aux->next;
		table[hash_key] = map;
		free(aux);
		return;
	}

	while(aux != NULL && strcmp(aux->symbol, symbol)) {
		prev = aux;
		aux = aux->next;
	}

	if(aux == NULL)
		return;

	prev->next = aux->next;
	table[hash_key] = map;
	free(aux);
}
////////////////////////////TODO MODIFY
char *getDir(char *fileName)
{
	char direct[BUFSIZE] = {0};
	char *token;
	char aux[BUFSIZE] = {0};

	token = strtok(fileName, "/");
	if (!token)
		return "./";
	while (token) {
		strcpy(aux, token);
		strcat(aux, "/");
		token = strtok(NULL, "/");
		if (token == NULL)
			break;
		strcat(direct, aux);
	}
	strcpy(fileName, direct);
	return fileName;
}


void freeMem(hashmap *table[], int size, char *directory[])
{
	int i;
	hashmap *aux;
	hashmap *aux2;

	for (i = 0; i < size; i++)
		free(directory[i]);
	for (i = 0; i < TABLESIZE; i++) {
		aux = table[i];
		while (aux) {
			aux2 = aux;
			aux = aux->next;
			free(aux2);
		}
	}
}

char *changeLine(char *line, hashmap *table[])
{
	char aux2[BUFSIZE], buffer[BUFSIZE], value[BUFSIZE];
	char *aux, *p, *q;
	int len = 0;

	strcpy(aux2, line);
	strcpy(buffer, line);
	// printf("am linia %s asta\n", line);
	aux = strtok(line, "\t[]{}<>=+-*/%!&|^.,:;()\\\n ");
	// printf("%s line\n", line);
	while (aux) {
		if (strcmp(getMapping(table, aux), "") != 0) {  // TODO E STRICAT ASTA, TB CU CONTAINS KEY
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
			printf("%s am aux\n", aux);
			strcpy(value, getMapping(table, aux));
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
void parseFile(hashmap *table[], FILE *input_file, FILE *output_file, int dir_size,
	char *directory[]);
//  function to verify if the line read from the file is of type #include
int include_line(hashmap *table[], char line[], int dir_size, FILE *output_file,
		char *directory[]) {
	char *incl;
	char *fileName;
	char buffer[BUFSIZE] = {0};
	int i, ret = 0, found = 0;

	incl = strstr(line, "#include");
	if (incl) {
		ret = 1;
		strtok(line, "\"");
		fileName = strtok(NULL, "\"");
		for (i = 0; i < dir_size; i++) {
			strcpy(buffer, directory[i]);
			strcat(buffer, "/");
			strcat(buffer, fileName);
			FILE *hf = fopen(buffer, "r");
			if (hf) {
				parseFile(table, hf, output_file, dir_size, directory);
				found = 1;
				break;
			}
		}
		if (!found) {
			freeMem(table, dir_size, directory);
			exit(1);
		}
	}
	return ret;
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

int ifnd_line(hashmap *table[], FILE *input_file, char line[]) {
	int ret = 0;


	return ret;
}
//////////////////////////////////

// function to parse the file
void parseFile(hashmap *table[], FILE *input_file, FILE *output_file, int dir_size,
	char *directory[]) {
	char line[BUFSIZE];

	while(fgets(line, BUFSIZE, input_file)) {
		// printf("axix   %sjjj\n", line);

		// if(include_line(table, line, dir_size, output_file, directory))
		// 	continue;
		if(define_line(table, input_file, line))
			continue;
		// if(ifndf_line(table, input_file, line))
		// 	continue;
		if(strcmp(line, "\n") != 0) {
			// printf("din hashtable am %s ", getMapping(table, "")); // TODO REPARA HASHTABLE
			// printf("intri?");
			fputs(changeLine(line, table), output_file);

		}
// TODO HERE !! UNFINISHED PROCESSED FILE
	}
}


int main(int argc, char *argv[]) {
	// printf("Hello world %d.\n", MY_DEF);
	FILE *input_file, *output_file;
	input_file = stdin;
	output_file = stdout;

	int dir_size = 1;
	int check_out_file = 0, check_in_file = 0;
	char *directory[TABLESIZE];
	char infile[BUFSIZE] = {0};
	hashmap *table[TABLESIZE];

	for(int i = 0; i < TABLESIZE; i++) {
		table[i]= NULL;
	}

	char *symbol, *mapping;

	// Start checking arguments
	for (int i = 1; i < argc; i++) {
		if(strncmp(argv[i], "-D", 2) == 0) {
			if(strlen(argv[i]) == 2) {
				// printf("am argumentul %s \n", argv[i+1]);
				symbol = strtok(argv[++i],"=");
				mapping = strtok(NULL, " ");
				// i++;
				// printf("AM MAPPING %s\n\n", mapping);
			} else {
				// char *aux = strtok(argv[i], "-D");
				symbol =  strtok(argv[i] + 2, "=");  //strtok(NULL, "=");
				mapping = strtok(NULL, " ");
				// printf("AM MAPPING %s\n\n", mapping);

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
			if (input_file == NULL)
				exit(-1);
			directory[0] = (char *) calloc(strlen(argv[i]) + 1, sizeof(char));
			if (!directory[0])
				exit(12);
			strcpy(directory[0], getDir(argv[i]));  // TODO getDir
			check_in_file = 1;
			continue;
		}

		// if (!check_out_file && argv[i]) {
		// 	if (strcmp(argv[i], infile) == 0)
		// 		exit(-1);
		// 	output_file = fopen(argv[i], "w");
		// 	if (output_file == NULL)
		// 		exit(-1);
		// 	check_out_file = 1;
		// 	continue;
		// }

		// if (argv[i])
		// 	exit(-1);
	}


	parseFile(table, input_file, output_file, dir_size, directory);
	// freeMem(table);


	if(input_file)
		fclose(input_file);
	if(output_file)
		fclose(output_file);

	return 0;
}