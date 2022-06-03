#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "card.h"

/*
 * I've left these definitions in from the
 * solution program. You don't have to
 * use them, but the `dupe_check()` function
 * unit test expects certain values to be
 * returned for certain situations!
 */
#define DUPE -1
#define NO_DUPE -2

/*
 * These are the special strings you need to
 * print in order for the text in the terminal
 * to be bold, italic or normal (end)
 */
#define BOLD "\e[1m"
#define ITALIC "\e[3m"
#define END "\e[0m"

/*
 * You will have to implement all of these functions
 * as they are specifically unit tested by Mimir
 */
int dupe_check(unsigned, char*);
char *fix_text(char*);
void free_card(CARD_T*);
CARD_T *parse_card(char*);
void print_card(CARD_T*);

/*
 * We'll make these global again, to make
 * things a bit easier
 */
CARD_T **cards = NULL;
size_t total_cards = 0;

int main(int argc, char **argv) {
	// TODO: 1. Open the file
	FILE *fp;
	
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		return EXIT_FAILURE;
	}
	fp = fopen(argv[1], "r");
	if (fp == NULL)
	{
		printf("Unable to open file\n");
		return 1;
	}

	char *buffer = NULL;
	size_t n = 0;
	
	// get rid of the first line
	getline(&buffer, &n, fp);

	// 2. Read lines from the file...
	
	int read = getline(&buffer, &n, fp);
	
	while(read != EOF){

		// a. for each line, `parse_card()`
		CARD_T *card = parse_card(buffer);
		if(card != NULL){

			// b. add the card to the array
			cards = (CARD_T **) realloc(cards, sizeof(CARD_T *)*(total_cards+1));
			cards[total_cards] = card;	
			total_cards++;
		}
		
		read =getline(&buffer, &n, fp);
	}

	// 3. Sort the array
	CARD_T *temp;
	for(int h = 0; h < total_cards; h++){
		for (int g = h + 1; g < total_cards; g++){
			int comp = strcmp(cards[h]->name, cards[g]->name);
			if (comp > 0){
				temp = cards[h];
				cards[h] = cards[g];
				cards[g] = temp;
			}
		}
	}
	
	// 4. Print and free the cards
	for(int i = 0; i<total_cards; i++){
		print_card(cards[i]);
		free_card(cards[i]);
	}

	// 5. Clean up!
	free(buffer);
	free(cards);
	fclose(fp);

	return 0;
}

/*
 * This function has to return 1 of 3 possible values:
 *     1. NO_DUPE (-2) if the `name` is not found
 *        in the `cards` array
 *     2. DUPE (-1) if the `name` _is_ found in
 *        the `cards` array and the `id` is greater
 *        than the found card's id (keep the lowest one)
 *     3. The last case is when the incoming card has
 *        a lower id than the one that's already in
 *        the array. When that happens, return the
 *        index of the card so it may be removed...
 */
int dupe_check(unsigned id, char *name) {
	for(int i = 0; i < total_cards; i++){
		if (strcmp(name, cards[i]->name) == 0)
		{
			if (id > cards[i]->id)
			{
				return DUPE;
			}
			else
			{
				return i;
			}
		}
	}
	return NO_DUPE;
}

/*
 * This function has to do _five_ things:
 *     1. replace every "" with "
 *     2. replace every \n with `\n`
 *     3. replace every </b> and </i> with END
 *     4. replace every <b> with BOLD
 *     5. replace every <i> with ITALIC
 *
 * The first three are not too bad, but 4 and 5
 * are difficult because you are replacing 3 chars
 * with 4! You _must_ `realloc()` the field to
 * be able to insert an additional character else
 * there is the potential for a memory error!
 */
char *fix_text(char *haystack) {
	int index;
	char *needle;

	// replace every "" with "
	needle = strstr(haystack, "\"\"");
	while(needle != NULL){
		index = needle - haystack;

		// move from the second double quote to the left by 1
		memmove(needle, needle + 1, strlen(needle));
		needle = strstr(haystack, "\"\"");
	}

	// replace every \n with `\n`
	needle = strstr(haystack, "\\n");
	while(needle != NULL){
		index = needle - haystack;
		char x = '\n';

		// move the bytes to make space for the 2 additional bytes
		memmove(needle + 1, needle + 2, strlen(needle) - 1);

		// copy the newline character
		memcpy(haystack + index, &x, 1);
		needle = strstr(haystack, "\\n");
	}

	// replace every </b> and </i> with END
    needle = strstr(haystack, "</");
    while(needle != NULL){
        index = needle - haystack;
        memcpy(haystack + index, END, 4);
        needle = strstr(haystack, "</");
    }

	// replace every <b> with BOLD
	needle = strstr(haystack, "<b>");
	while (needle != NULL){
		index = needle - haystack;

		// `realloc()` the field to insert an additional character and a null terminator
		haystack = (char *) realloc(haystack, sizeof(char)*((strlen(haystack) + 1) + 1));
		needle = haystack + index;
		
		// move to the right by 1
		memmove(needle + 1, needle, strlen(needle) + 1);
		memcpy(needle, BOLD, 4);
		needle = strstr(haystack, "<b>");	
	}

	// replace every <i> with ITALIC
	needle = strstr(haystack, "<i>");
	while(needle != NULL){
		index = needle - haystack;
		haystack = (char *) realloc(haystack, sizeof(char)*((strlen(haystack) + 1) + 1));	
		needle = haystack + index;

		memmove(needle + 1, needle, strlen(needle)+1);
		memcpy(needle, ITALIC, 4);

		needle = strstr(haystack, "<i>");
	}

    return haystack;
}

/*
 * This short function simply frees both fields
 * and then the card itself
 */
void free_card(CARD_T *card) {
	free(card->name);
	free(card->text);
	free(card);
}

/*
 * This is the tough one. There will be a lot of
 * logic in this function. Once you have the incoming
 * card's id and name, you should call `dupe_check()`
 * because if the card is a duplicate you have to
 * either abort parsing this one or remove the one
 * from the array so that this one can go at the end.
 *
 * To successfully parse the card text field, you
 * can either go through it (and remember not to stop
 * when you see two double-quotes "") or you can
 * parse backwards from the end of the line to locate
 * the _fifth_ comma.
 *
 * For the fields that are enum values, you have to
 * parse the field and then figure out which one of the
 * values it needs to be. Enums are just numbers!
 */
CARD_T *parse_card(char *line) {
	CARD_T * card =(CARD_T *) malloc(sizeof(CARD_T));

	char *stringp = line, *token;
	int temp;  

	// get ID 
	token = strsep(&stringp, ",");
	temp  = atoi(token);
	card->id = temp;

	// get name
	stringp++;
	token = strsep(&stringp, "\"");
	card->name = strdup(token);

	// call dupe_check with incoming card's id and name to check for duplicates
	int card_index = dupe_check(card->id, card->name);

	// if the incoming card is a duplicate and has greater id, abort parsing
	if(card_index == DUPE){
		free(card->name);
		free(card);
		return NULL;
	} 

	// else if the incoming card is a duplicate and has smaller id
	else if (card_index != NO_DUPE){
		free(cards[card_index]->name);
		free(cards[card_index]->text);
		free(cards[card_index]);

		// remove the card from the array by moving all the cards to the left by 1
		memmove(cards + card_index, cards + card_index + 1, sizeof(CARD_T *)*(total_cards - card_index - 1));
		total_cards--;	
	}

	// get cost;
	stringp++;
	token = strsep(&stringp, ",");
	
	// if the cost field is empty, fill it with 0
	if (strlen(token) == 0) {
		card->cost = 0;
	}
	else{
		temp = atoi(token);
		card->cost = temp;
	}

	// get text
	int count = 0, i = strlen(stringp) - 1;

	// parse backwards from the end of the line to locate the fifth comma
	while(count < 5){
		if(stringp[i] == ','){
			count++;
		}
		i--;
	}

	// if the text field is empty,
	if (i < 0){
		card->text = strdup("");
		stringp++;	
	}
	
	else {
		char *text = strndup(stringp + 1, i);

		// null terminating the text field
		text[i - 1] = 0;
	
		// fix the text field 
		text = fix_text(text);
		
		card->text = strdup(text);
		free(text);
		stringp += (i + 2);
	}

	// get attack
	token = strsep(&stringp, ",");

	// if the attack field is empty, fill it with 0
	if (strlen(token) == 0) {
		card->attack = 0;
	}
	else {
		temp = atoi(token);
		card->attack = temp;
	}

	// get health
	token = strsep(&stringp, ",");

	// if the health field is empty, fill it with 0
	if (strlen(token) == 0) {
		card->health = 0;
	}
	else {
		temp = atoi(token);
		card->health = temp;
	}

	// get type
	// to skip the first double quote, increment stringp by 1
	stringp++;
	token = strsep(&stringp, "\"");
	int size1 = sizeof(type_str)/sizeof(char *);

	for(int i = 0; i < size1; i++){
		if(strcasecmp(token, type_str[i]) == 0){
			Type type = i;
			card->type = type;
		}
	}	
	
	// get class
	stringp += 2;
	token = strsep(&stringp, "\"");
	int size2 = sizeof(class_str)/sizeof(char *);

	for(int i = 0; i < size2; i++){
		if(strcasecmp(token, class_str[i]) == 0){
			Class class = i;
			card->class = class;
		}
	}	

	// get rarity
	stringp += 2;
	token = strsep(&stringp, "\"");
	int size3 = sizeof(rarity_str)/sizeof(char *);

	for(int i = 0; i < size3; i++){
		if(strcasecmp(token, rarity_str[i]) == 0){
			Rarity rarity = i;
			card->rarity = rarity;	
		}
	}

   	return card;
}

/*
 * Because getting the card class centered is such
 * a chore, you can have this function for free :)
 */
void print_card(CARD_T *card) {
	printf("%-29s %2d\n", card->name, card->cost);
	unsigned length = 15 - strlen(class_str[card->class]);
	unsigned remainder = length % 2;
	unsigned margins = length / 2;
	unsigned left = 0;
	unsigned right = 0;
	if (remainder) {
		left = margins + 2;
		right = margins - 1;
	} else {
		left = margins + 1;
		right = margins - 1;
	}
	printf("%-6s %*s%s%*s %9s\n", type_str[card->type], left, "", class_str[card->class], right, "", rarity_str[card->rarity]);
	printf("--------------------------------\n");
	printf("%s\n", card->text);
	printf("--------------------------------\n");
	printf("%-16d%16d\n\n", card->attack, card->health);
}

