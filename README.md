# Project Specification
This is a C program that parses a `.csv` file full of Hearthstone data. It will also de-duplicate, sort, and then print out the cards.

## Array of Pointers to Structs
The data structure used in this program is a **dynamic array of `CARD_T`** or pointers to `CARD_T` structs. This means the data type of `cards` needs to be `CARD_T`.

## Functions
There are a few functions that shapes the main body of the program, which are:
1. `dupe_check()`, which returns 1 of 3 possible values:
   1. **NO_DUPE** (-2) if the `name` is not found in the `cards` array
   2. **DUPE** (-1) if the `name` is found in the `cards` array and the `id` is greater than the found card's id (keep the lowest one)
   3. The last case is when the incoming card has a lower id than the one that's already in the array. When that happens, return the index of the card so it may be removed
2. `fix_text()`, which has to do the following five things:
   1. replace every "" with "
   2. replace every '\\\n' with '\n'
   3. replace every ![Screen Shot 2022-06-07 at 4 34 40 PM](https://user-images.githubusercontent.com/105037989/172477329-f1cbb34c-b7b4-4e0c-a248-3ddd5e80fc9c.png) and ![Screen Shot 2022-06-07 at 4 35 14 PM](https://user-images.githubusercontent.com/105037989/172477423-22b016bc-c139-448a-b010-99b6bed2af03.png) with '\e[0m'
   4. replace every ![Screen Shot 2022-06-07 at 4 32 59 PM](https://user-images.githubusercontent.com/105037989/172477044-aa70264e-66a3-412c-8acc-7fc90d15641b.png) with '\e[1m'
   5. replace every ![Screen Shot 2022-06-07 at 4 33 48 PM](https://user-images.githubusercontent.com/105037989/172477190-dbc22635-a509-42bb-8368-4f88173e2853.png) with '\e[3m'

   These text replacement are done as the **card text** field contains a number of issues that makes it gross to read in terminal output. For one and two, two characters are replaced by one, which means every character in front of the change to the end of the text field need to be moved back by one element, therefore `memmove()` should be used. For four and five, three characters are replaced with four, so a call to `realloc()` is required to move all the characters in front of this sequence forward by one element and guarantee there is enough space in memory for this addition. 

3. `free_card()`, which simply frees both `card->name` and `card->text` fields and then the card itself.
4. `parse_card()`, which calls the `dupe_check()` function after obtaining the incoming card's id and name because if the card is a duplicate and has greater id, parsing is aborted or the card is removed from the array. The hardest field to parse is the **card text** field because it could contain a comma or *escaped double quotes*. To successfully parse through this field, the fifth comma is located by parsing backwards from the end of the line. The `fix_text()` function is used here to perform text replacement.
5. `print_card()`, which prints the cards out nicely.

## Example Output
<img src= "https://user-images.githubusercontent.com/105037989/172483394-f8c0c82e-50b1-4ef0-9f38-5cb5711f1d8c.png" width="700" height="1300">

>The $ character represents the terminal prompt.
