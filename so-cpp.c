#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MY_DEF 4
#define BUFSIZE 256
#define TABLESIZE 100

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

// TODO add future functions for the hashmap

int main(int argc, char *argv[]) {
	printf("Hello world %d.\n", MY_DEF);
}