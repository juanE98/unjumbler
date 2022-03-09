#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdbool.h> 
#include <ctype.h> 

/* exit status */ 
#define INVALID_COMMAND_LINE 1 
#define DICT_DOES_NOT_EXIST 2 
#define LETTERS_LESS_THAN_THREE 3 
#define LETTERS_NOT_ALPHABETIC 4
#define NO_MATCHES_FOUND 10

/* Buffer size for char */ 
#define DICT_WORD_BUFFER_SIZE 51 

/* Contains information for unjumbling the letters given
 * 
 * uniqueLetters: array of unique letters in a given word  
 * dictionaryResults: array of results from dictionary file 
 * sortedResults: array of sorted results
 * alphaResults: array of results in alphabeitcal (lexiographic) order
 * resultsSize: size of dictionaryResults
 * sortedResultsSize: size of sortedResults 
 */ 
typedef struct {
    char* uniqueLetters; 
    char* dictionaryResults;
    char* sortedResults; 
    char** alphaResults; 
    int resultsSize; 
    int sortedResultsSize; 
} Word; 

/**
 * Stores information of program input  
 *
 * setting1: -alpha OR -len OR -longest OR NULL 
 * setting2: -include OR NULL 
 * includeLetter: is not NULL iff setting2 != NULL 
 * letters: set of letters to be unjumbled 
 * dict: path of dictionary file, otherwise use default dictionary path 
 */
typedef struct {
    char* setting1; 
    char* setting2; 
    char* includeLetter; 
    char* letters; 
    char* dict; 
} Data; 

/* Exits program */ 
void exit_program(int exitStatus); 

/*Processes command line arguments given*/ 
void parse_cmd_args(int argc, char** argv, Data* data);

/*Validates command line arguments */ 
void validate_args(int argc, Data* data); 

/*Initialises data struct*/ 
void initialise_data(Data* data);

/* Checks if string starts with specific prefix */ 
bool starts_with(const char* prefix, char* word);

/* Check if word is only alphabet*/ 
bool word_is_alphabet(char* word); 

/* Counts the number of times a letter occurs in a word */ 
int letter_frequency(char* word, char letter);

/* Main method to unjumble the word and match to words in dictionary */ 
void unjumble(Data* data, Word* word);

/*Reads the dictionary file and proccesses words that can be unjumbled with
 * letters given.
 */ 
void read_dictionary(char* filename, Word* word, Data* data);

/* Returns unique letters of string */ 
char* find_unique_letters(char* word); 

/* Displays result output to stdout */ 
void display_result(char* results); 

/* Main sort function */ 
void sort(Data* data, Word* word); 

/* Removes word in results that does not contain specific letter */ 
void include_sort(Word* word, char letter);

/* Sorts results in alphabetical order */ 
void alpha_sort(Word* word);

/* Sorts results in descending order of length 
 *  (i.e. longest length word first) then in alphabeitcal (lexiographic) 
 *  if length is equal */ 
void len_sort(Word* word); 

/* Sorts results based on longest matching word(s) only, then sort as in 
 * alphabetical (lexiographic) if length is equal */ 
void longest_sort(Word* word);

/* Comparator function for alpha sort */ 
static int cmp_alpha(const void* lhs, const void* rhs);

/* Comparator function for len sort */ 
static int cmp_desc(const void* lhs, const void* rhs);

/* Appends letter to string */ 
void append(char* string, char letter); 

/** Main function */ 
int main(int argc, char** argv) {
    if (argc < 2 || argc > 6) {
        exit_program(INVALID_COMMAND_LINE);
    }
    //Instantiate data and word structs 
    Data data;
    Word letters; 
    //check cmd args 
    initialise_data(&data); 
    parse_cmd_args(argc, argv, &data);
    validate_args(argc, &data);

    unjumble(&data, &letters); 

    return 0; 
}

/**
 * Main method to unjumble the letters given and match to words in the 
 * dictionary. 
 *
 * @param data Data struct 
 * @param word Word struct 
 */ 
void unjumble(Data* data, Word* word) {

    //initialise word struct 
    word->uniqueLetters = find_unique_letters(data->letters);
    word->dictionaryResults = malloc(DICT_WORD_BUFFER_SIZE * sizeof(char));
    word->dictionaryResults = NULL;
    word->sortedResults = NULL; 

    //match for each word in dictionary file
    read_dictionary(data->dict, word, data);

    //sort results if required 
    if (data->setting1 != NULL || data->setting2 != NULL) {
        word->sortedResults = malloc(DICT_WORD_BUFFER_SIZE * sizeof(char));
        word->sortedResultsSize = 0;
        sort(data, word);
        //display_result(word->sortedResults);
    } else {
        //print unsorted results to stdout 
        display_result(word->dictionaryResults);

    }
    free(word->dictionaryResults); 
    free(word->sortedResults); 
}

/**
 * Function to sort initial results unjumbled  
 *
 * @param word Word struct
 * @param data Data struct 
 */
void sort(Data* data, Word* word) {
    
    if (data->setting2 != NULL) {
        include_sort(word, data->includeLetter[0]);
    } else {
        word->sortedResults = strdup(word->dictionaryResults);
        word->sortedResultsSize = word->resultsSize;
    }

    if (data->setting1 != NULL) {
        alpha_sort(word);
        if ((strcmp(data->setting1, "-len") == 0)) {
            len_sort(word); 
        }
        if ((strcmp(data->setting1, "-longest") == 0)) {
            longest_sort(word); 
        }
        //display sorted results with setting1
        for (int i = 0; i < word->sortedResultsSize; i++) {
            printf("%s\n", word->alphaResults[i]); 
        }
    } else {
        display_result(word->sortedResults);
    }
}

/**
 * Removes results that do not include a specfic letter
 *
 * @param resutls Initial resutls from dictionary file
 * @param letter Letter required for each word in the results array
 */ 
void include_sort(Word* word, char letter) {
    char* copy = strdup(word->dictionaryResults); 
    char* dictWord = strtok(copy, "\n");
    word->sortedResults[0] = '\0';
    //buffer size for sortedResults array 
    int sortBufferSize = DICT_WORD_BUFFER_SIZE;
    
    char lowerLetter = tolower(letter);  
    char upperLetter = toupper(letter); 

    //iterate through each word  
    while (dictWord != NULL) {
        int wordLen = strlen(dictWord); 
        //find word containing letter, upper or lower case 
        if (strchr(dictWord, lowerLetter) != NULL || 
                strchr(dictWord, upperLetter) != NULL) {

            sortBufferSize += (wordLen + 1); 
            word->sortedResultsSize++; 
            word->sortedResults = realloc(word->sortedResults, 
                    ((sortBufferSize) * sizeof(char)));

            //append to sorted results array 
            strcat(word->sortedResults, dictWord);
            strcat(word->sortedResults, "\n"); 
        }
        dictWord = strtok(NULL, "\n"); 
    }
}

/**
 * Sorts results in alphabetical order (lexiographic) first then in ASCII 
 * order. 
 *
 * @param word Word struct 
 */ 
void alpha_sort(Word* word) {
    //work with word->sortedResults 
    char* copy = strdup(word->sortedResults);
    char* dictWord = strtok(copy, "\n"); 
    word->alphaResults = malloc(sizeof(char*) * word->sortedResultsSize);
    //Allocate memory for results 2D array
    for (int i = 0; i < word->sortedResultsSize; i++) {
        word->alphaResults[i] = malloc(sizeof(char*) * DICT_WORD_BUFFER_SIZE);
    }
    int i = 0;
    int bufferSize = DICT_WORD_BUFFER_SIZE;
    //Transfer words from word->sortedResults to word->alphaResults 
    while (dictWord != NULL) {
        int wordLen = strlen(dictWord);
        bufferSize += (wordLen + 1); 
        word->alphaResults[i] = realloc(word->alphaResults[i], (bufferSize * 
                sizeof(char))); 
        word->alphaResults[i] = dictWord; 
        i++; 
        dictWord = strtok(NULL, "\n"); 
    }
    word->sortedResultsSize = i; 
    qsort(word->alphaResults, i, sizeof(word->alphaResults[0]), cmp_alpha);
}

/* Comparator function to compare strings in lexiographic order, if strings 
 * still match, then compare with ASCII values 
 *
 * @param lhs string 1 to be compared with 
 * @param rhs string 2 to be compared with 
 *
 * @return 0 if equal, >1 if (lhs > rhs) and <1 if (lhs < rhs) 
 **/ 
static int cmp_alpha(const void* lhs, const void* rhs) {
    char* stringLhs = *(char**) lhs; 
    char* stringRhs = *(char**) rhs;
    char lowerLhs[strlen(stringLhs)], lowerRhs[strlen(stringRhs)];
    lowerLhs[0] = '\0'; 
    lowerRhs[0] = '\0'; 
    int lenLhs = strlen(stringLhs); 
    int lenRhs = strlen(stringRhs);
    int compareLen = 0;
    
    if (lenLhs < lenRhs) {
        compareLen = lenLhs; 
    } else {
        compareLen = lenRhs; 
    }
    //Create lowercase version of strings to be compared lexiographically 
    for (int i = 0; i < lenLhs; i++) {
        char lowerLetterL = (char)tolower((int)stringLhs[i]); 
        append(lowerLhs, lowerLetterL); 
    }
    for (int j = 0; j < lenRhs; j++) {
        char lowerLetterR = (char)tolower((int)stringRhs[j]); 
        append(lowerRhs, lowerLetterR);
    }
    for (int k = 0; k < compareLen; k++) {
        if (strcmp(lowerLhs, lowerRhs) != 0) {
            return strcmp(lowerLhs, lowerRhs); 
        }  
    }
    //Compare by ASCII values 
    if (lenLhs < lenRhs) {
        return -1;
    } else if (lenLhs > lenRhs) {
        return 1; 
    } else {
        return strcmp(*(const char**)lhs, *(const char**)rhs); 
    }
}

/* Comparator function to sort strings in decresasing length
 *
 * @param lhs string 1 to be compared with 
 * @param rhs string 2 to be compared with 
 *
 * @return 1 if lhs is shorter, -1 if lhs is longer, otherwise compare as per
 * -alpha sorting 
 **/
static int cmp_desc(const void* lhs, const void* rhs) {
    const char* plhs = *(char* const*) lhs; 
    const char* prhs = *(char* const*) rhs; 
    int lenLhs = strlen(plhs); 
    int lenRhs = strlen(prhs); 

    if (lenLhs < lenRhs) {
        return 1; 
    } 
    if (lenLhs > lenRhs) {
        return -1; 
    }
    return cmp_alpha(lhs, rhs); 
}

/* 
 * Sorts results in descending order of length with words of same length 
 * sorted as per -alpha sorting
 *
 * @param word Word struct 
 **/ 
void len_sort(Word* word) {
    qsort(word->alphaResults, word->sortedResultsSize, 
            sizeof(word->alphaResults[0]), cmp_desc); 
}

/*
 * Sorts results in based on only the longest matching word(s). If there are
 * multiple words, they are sorted as per -alpha sorting 
 *
 * @param word Word struct 
 **/ 
void longest_sort(Word* word) {
    len_sort(word); 
    int maxLength = strlen(word->alphaResults[0]); 
    int bufferSize = DICT_WORD_BUFFER_SIZE; 
    int nWords = 0; 
    for (int i = 0; i < word->sortedResultsSize; i++) {
        int currLength = strlen(word->alphaResults[i]);
        bufferSize += currLength; 
        nWords++; 
        if (currLength < maxLength) {
            word->alphaResults = realloc(word->alphaResults, bufferSize * 
                    sizeof(char*));

            word->alphaResults[i] = '\0';
            word->sortedResultsSize = nWords - 1; 
            break; 
        }
    }
}

/**
 * Reads dictionary file line by line, each word is separated by '\n'
 *
 * Computes whether or not the word can be built from given letters
 *
 * @param filename filename of the dictionary 
 * @param word Word struct containing letters information 
 * @param data Data struct containing param args 
 */ 
void read_dictionary(char* filename, Word* word, Data* data) {
    FILE* dict = fopen(filename, "r");
    if (dict == NULL) {
        fprintf(stderr, "unjumble: file \"%s\" can not be "
                "opened\n", filename); 
        exit(DICT_DOES_NOT_EXIST); 
    }
    //buffer size for the dictionaryResults array 
    int bufferSize = DICT_WORD_BUFFER_SIZE;
    char line[DICT_WORD_BUFFER_SIZE];
    word->resultsSize = 0; 
    while (fgets(line, sizeof(line), dict)) {
        //counter for letters that match between word and dictionary word 
        int matchLetter = 0;
        //length of line, not including '\n' 
        int dictWordLen = strlen(line) - 1;

        //match dictionary word's letter with letter given
        for (int i = 0; i < dictWordLen; i++) {
            for (int j = 0; word->uniqueLetters[j] != '\0'; j++) {
                //convert both letters to lowercase for comparison
                char lowerDictLetter = tolower(line[i]); 
                char lowerUniqueLetter = tolower(word->uniqueLetters[j]); 
                if (lowerDictLetter == lowerUniqueLetter) {
                    //count letter frequency in word and dictionary word
                    int dictCount = letter_frequency(line, line[i]); 
                    int letterCount = letter_frequency(data->letters, 
                            word->uniqueLetters[j]); 
                    if (dictCount <= letterCount) {
                        matchLetter++;
                    }
                }
            }
        }
        //successful match, then add dictionary word to results array  
        if (dictWordLen <= matchLetter && dictWordLen > 2) {
            word->resultsSize++; 
            bufferSize += (dictWordLen + 1); 
            word->dictionaryResults = realloc(word->dictionaryResults, 
                    (bufferSize * sizeof(char)));
            strcat(word->dictionaryResults, line);
        }
    }
    fclose(dict); 
}

/**
 * Displays the final result to stdout
 *
 * @param word valid result from the dictionary 
 */
void display_result(char* results) {
    if (results == NULL) {
        exit(NO_MATCHES_FOUND); 
    } else {
        printf("%s", results);  
    }
}

/**
 * Finds the unique letters of a given word, not case sensitive
 * Finds the unique letters of a given word, not case sensitive
 *
 * @param word the given word 
 *
 * @return char* of unique letters of word 
 */
char* find_unique_letters(char* word) {
    char* uniqueLetters = malloc(strlen(word) * sizeof(char)); 
    int uniqueLetterCounter = 1; 

    for (int i = 0; word[i] != '\0'; i++) {
        if (strchr(uniqueLetters, tolower(word[i])) == NULL) {
            uniqueLetters = realloc(uniqueLetters, (sizeof(char) * 
                    uniqueLetterCounter));
            //frequency of each letter in the word 
            int freq = letter_frequency(uniqueLetters, word[i]); 
            if (freq < 2) {
                char c = tolower(word[i]); 
                strncat(uniqueLetters, &c, 1); 
                uniqueLetterCounter++; 
            }
        }
    }
    return uniqueLetters; 
}

/** Returns the count of a letter in a word, non case sensitive
 * 
 * @param word word to be searched 
 * @param letter letter to be counted 
 *
 * @return frequency of letter in word 
 *
 **/ 
int letter_frequency(char* word, char letter) {
    int count = 0; 
    for (int i = 0; word[i] != '\0'; i++) {
        char wordLetterLower = tolower(word[i]); 
        char letterLower = tolower(letter); 
        if (wordLetterLower == letterLower) {
            count++; 
        }
    }
    return count; 
}

/* Initialises data struct to NULL
 *
 * @param data Data struct 
 */ 
void initialise_data(Data* data) {
    data->setting1 = NULL; 
    data->setting2 = NULL;
    data->includeLetter = NULL; 
    data->letters = NULL; 
   
    //initialise to deafault dictionary file path 
    data->dict = "/usr/share/dict/words"; 
}

/*
 * Processes command line arguments 
 *
 * @param argc Number of arguments given 
 * @param argv Arguments passed
 * @param data Data struct 
 *
 */ 
void parse_cmd_args(int argc, char** argv, Data* data) {
    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-include") == 0) && data->setting2 == NULL) {
            data->setting2 = argv[i];
            data->includeLetter = argv[++i]; 

            //-include already parsed 
        } else if ((strcmp(argv[i], "-include") == 0)
                && data->setting2 != NULL) {
            exit_program(INVALID_COMMAND_LINE); 
        } else if ((starts_with("-", argv[i]) && data->setting1 == NULL)) {
            data->setting1 = argv[i];
        } else if (starts_with("-", argv[i]) && data->setting1 != NULL) {
            //one of valid option args appears more than once 
            exit_program(INVALID_COMMAND_LINE); 
        } else {
            data->letters = argv[i];
            //dictionary file is after letters argument if included 
            if (i + 1 <= argc - 1) {
                data->dict = argv[++i];
            }
            //check if there are more args after dictionary file name
            if (i < argc - 1) {
                exit_program(INVALID_COMMAND_LINE); 
            } else {
                break; 
            }
        }
    }
}

/**
 * Validates whether or not command line arguments can run program. 
 *
 * @param data Data struct 
 * @param argc number of arguments  
 */ 
void validate_args(int argc, Data* data) {
    if (data->setting1 != NULL) {
        if (strcmp(data->setting1, "-alpha") != 0 &&
                strcmp(data->setting1, "-len") != 0 &&
                strcmp(data->setting1, "-longest") != 0) {
            exit_program(INVALID_COMMAND_LINE); 
        }
    }
    if (data->setting2 != NULL) {
        if (strcmp(data->setting2, "-include") != 0) {
            exit_program(INVALID_COMMAND_LINE);
        }
        char letter = data->includeLetter[0];    
        if (strlen(data->includeLetter) > 1 || isalpha(letter) == 0) {
            exit_program(INVALID_COMMAND_LINE); 
        }
    }
    //check if letters argument is present 
    if (data->letters == NULL) {
        exit_program(INVALID_COMMAND_LINE); 
    }
    //check if letters argument contains at least 3 char 
    if (strlen(data->letters) < 3) {
        exit_program(LETTERS_LESS_THAN_THREE); 
    }
    //check if letters argument only contains upper/lower case letters
    if (!word_is_alphabet(data->letters)) {
        exit_program(LETTERS_NOT_ALPHABETIC); 
    }
}

/**
 * Exits the program following invalid command line input. 
 * Prints message to stderr 
 *
 * @param exitStatus cause for the program to exit. 
 * @exits with exitStatus
 */
void exit_program(int exitStatus){
    switch (exitStatus) {
        case INVALID_COMMAND_LINE: 
            fprintf(stderr, "Usage: unjumble [-alpha|-len|-longest] " 
                    "[-include letter] letters [dictionary]\n"); 
            break;        
        case LETTERS_LESS_THAN_THREE: 
            fprintf(stderr, "unjumble: must supply at least three " 
                    "letters\n"); 
            break; 
        case LETTERS_NOT_ALPHABETIC: 
            fprintf(stderr, "unjumble: can only unjumble alphabetic "
                    "characters\n"); 
            break; 
        default: 
            break; 
    }
    exit(exitStatus);
}

/**
 * Checks if string starts with specific character 
 *
 * @param prefix starting prefix of word to verify 
 * @param word the word to be checked 
 *
 * @return true if word starts with prefix, false otherwise 
 */
bool starts_with(const char* prefix, char* word) {
    return strncmp(prefix, word, 1) == 0; 
}

/**
 * Checks if string only contains uppercase and lowercase letters
 *
 * @param word string to be checked 
 *
 * @return true if word only contains letters, false otherwise 
 */
bool word_is_alphabet(char* word) {
    for (int i = 0; word[i] != '\0'; i++) {
        if (isalpha(word[i]) == 0) {
            return false; 
        }
    }
    return true; 
}

/**
 * Appends a ltter to a string 
 *
 * @param string the string to be modified 
 * @param letter the letter to be appended 
 */  
void append(char* string, char letter) {
    int len = strlen(string); 
    string[len] = letter; 
    string[len + 1] = '\0'; 
}
