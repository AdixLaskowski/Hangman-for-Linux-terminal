#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sqlite3.h>
#include <string.h>
#include <ctype.h>

char DrawMenu(void);
void Play(void);
void GetRandomWord(void);
void ToUpperCase(char *str);
void SeeWordsList(void);
void AddNewWord(void);
bool CheckIfWordExists(char *word);
void RemoveWord(void);
void RemoveWordFromDb(const char *word);
void WaitForEnter(void);
void Exit(void);
char *DisplayWordToGuess(const unsigned char *word);
void RevealLetterInWord(const char *originalWord, char *newWord, char guessedLetter);
void GuessLetter(void);
bool WasLetterUsed(const char *usedLetters, int numberOfUsedLetters, char letter);
void AddUsedLetter(char **usedLetters, int *numberOfUsedLetters, char letter);
void GameOver(void);
void ClearMemory(void);
bool isAlphabetic(char *str);

const unsigned char *randomWord;
const unsigned char *randomCategory;
char *hiddenWord;
char *usedLetters = NULL;
int usedCount = 0;
int chances = 0;

char DrawMenu(void)
{
    system("clear");
    char option[4];

    printf("1. Play\n");
    printf("2. See words list\n");
    printf("3. Add a new word\n");
    printf("4. Remove a word\n");
    printf("5. Exit\n");

    fgets(option, sizeof(option), stdin);

    if (strchr(option, '\n') == NULL) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }

    return option[0];
}

void GetRandomWord(void)
{
    sqlite3 *myDatabase;
    sqlite3_stmt *stmt;

    int status = sqlite3_open("hangman.db", &myDatabase);

    if(status != SQLITE_OK)
    {
        fprintf(stderr, "Error! Can't open database! %s\n", sqlite3_errmsg(myDatabase));
        WaitForEnter();
        return;
    }

    const char *sql = "SELECT category, word FROM words ORDER BY RANDOM() LIMIT 1";

    status = sqlite3_prepare_v2(myDatabase, sql, -1, &stmt, NULL);

    if(status != SQLITE_OK)
    {
        fprintf(stderr, "Error! Problem with preparing statement! %s\n", sqlite3_errmsg(myDatabase));
        sqlite3_close(myDatabase);
        WaitForEnter();
        return;
    }

    if((status = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const unsigned char *cat = sqlite3_column_text(stmt, 0);
        const unsigned char *word = sqlite3_column_text(stmt, 1);

        randomCategory = strdup((const char *)cat);
        randomWord = strdup((const char *)word);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(myDatabase);
}

void SeeWordsList(void)
{
    system("clear");

    sqlite3 *myDatabase;
    sqlite3_stmt *stmt;

    int status = sqlite3_open("hangman.db", &myDatabase);

    if(status != SQLITE_OK)
    {
        fprintf(stderr, "ERROR!1 %s\n", sqlite3_errmsg(myDatabase));
        WaitForEnter();
        return;
    }

    const char *sql = "SELECT * FROM words";

    status = sqlite3_prepare_v2(myDatabase, sql, -1, &stmt, NULL);

    if(status != SQLITE_OK)
    {
        fprintf(stderr, "ERROR!2 %s\n", sqlite3_errmsg(myDatabase));
        WaitForEnter();
        sqlite3_close(myDatabase);
        return;
    }


    while((status = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *category = sqlite3_column_text(stmt, 1);
        const unsigned char *word = sqlite3_column_text(stmt, 2);

        printf("%i\n", id);
        printf("%s\n", category);
        printf("%s\n", word);
        printf("\n");
    }

    if(status != SQLITE_DONE)
    {
        fprintf(stderr, "Error!3 %s\n", sqlite3_errmsg(myDatabase));
        sqlite3_finalize(stmt);
        sqlite3_close(myDatabase);
        WaitForEnter();
        return;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(myDatabase);

    WaitForEnter();

}

void WaitForEnter(void)
{
    printf("Press enter to continue: \n");
    while(getchar()!='\n');
}

void ToUpperCase(char *str)
{
    while (*str)
    {
        *str = toupper((unsigned char)*str);
        str++;
    }
}

bool isAlphabetic(char *str)
{
    for(int i=0; str[i] != '\0'; i++)
    {
        if(str[i] == '\n') continue;

        if(!isalpha((unsigned char)str[i]))
        {
            return false;
        }
    }
    return true;
}

void AddNewWord(void)
{

    char category[26];
    char word[26];

    do{
        system("clear");
        printf("Enter new word's category (Max 24 characters): \n");
        fgets(category, sizeof(category), stdin);

        if(strlen(category) > 24)
        {
            fprintf(stderr, "Error! Category must be no longer than 24 characters.");
            WaitForEnter();
            return;
        }

        if(!isAlphabetic(category))
        {
            fprintf(stderr, "Error! Category must containt only letters.\n");
            WaitForEnter();
            return;
        }

        printf("Enter new word: \n");
        fgets(word, sizeof(word), stdin);

        if(strlen(word) > 24)
        {
            fprintf(stderr, "Error! Word must be no longer than 24 characters.");
            WaitForEnter();
            return;
        }

        if(!isAlphabetic(word))
        {
            fprintf(stderr, "Error! Word must containt only letters.\n");
            WaitForEnter();
            return;
        }

    }while(CheckIfWordExists(word));

    sqlite3 *myDatabase;
    sqlite3_stmt *stmt;

    int status = sqlite3_open("hangman.db", &myDatabase);

    if(status != SQLITE_OK)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(myDatabase));
        WaitForEnter();
        return;
    }

    const char *sql = "INSERT INTO words (category, word) VALUES (?, ?)";

    status = sqlite3_prepare_v2(myDatabase, sql, -1, &stmt, NULL);

    if(status != SQLITE_OK)
    {
        fprintf(stderr, "Error with preparing the statement: %s\n", sqlite3_errmsg(myDatabase));
        sqlite3_close(myDatabase);
        WaitForEnter();
        return;
    }

    category[strcspn(category, "\n")] = 0;
    word[strcspn(word, "\n")] = 0;

    ToUpperCase(category);
    ToUpperCase(word);

    sqlite3_bind_text(stmt, 1, category, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, word, -1, SQLITE_TRANSIENT);

    status = sqlite3_step(stmt);

    if(status != SQLITE_DONE)
    {
        fprintf(stderr, "Error! Problem with statement's execution: %s\n", sqlite3_errmsg(myDatabase));
        sqlite3_finalize(stmt);
        sqlite3_close(myDatabase);
        WaitForEnter();
        return;

    }else {

        printf("A new word has been added successfully!\n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(myDatabase);

    WaitForEnter();

}

bool CheckIfWordExists(char *word)
{
    sqlite3 *myDatabase;
    sqlite3_stmt *stmt;
    bool exists = true;

    int status = sqlite3_open("hangman.db", &myDatabase);

    if(status != SQLITE_OK)
    {
        fprintf(stderr, "Error! Can't open the database: %s\n", sqlite3_errmsg(myDatabase));
        return true;
    }

    word[strcspn(word, "\n")] = 0;

    ToUpperCase(word);

    const char *sql = "SELECT COUNT(*) FROM words WHERE word = ?";

    status = sqlite3_prepare_v2(myDatabase, sql, -1, &stmt, NULL);

    if(status != SQLITE_OK)
    {
        fprintf(stderr, "Error! Problem with preparing statement: %s\n", sqlite3_errmsg(myDatabase));
        sqlite3_close(myDatabase);
        return true;
    }
    
    sqlite3_bind_text(stmt, 1, word, -1, SQLITE_TRANSIENT);

    if(sqlite3_step(stmt) == SQLITE_ROW)
    {
        int count = sqlite3_column_int(stmt, 0);
        exists = (count > 0);
        if(exists){printf("Given word already exists in database!\n");}
        
    }

    sqlite3_finalize(stmt);
    sqlite3_close(myDatabase);

    WaitForEnter();
    return exists;

}

void RemoveWord(void)
{
    system("clear");
    printf("What word you want to remove?\n");

    char buffer[128];
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    ToUpperCase(buffer);

    RemoveWordFromDb(buffer);

    WaitForEnter();
}

void RemoveWordFromDb(const char *word)
{
    sqlite3 *myDatabase;
    sqlite3_stmt *stmt;

    int status = sqlite3_open("hangman.db", &myDatabase);

    if(status != SQLITE_OK){

        fprintf(stderr, "Error! Can't open database! %s\n", sqlite3_errmsg(myDatabase));
        WaitForEnter();
        return;
    }

    const char *sql = "DELETE FROM words WHERE word = ?";

    status = sqlite3_prepare_v2(myDatabase, sql, -1, &stmt, NULL);

    if(status != SQLITE_OK)
    {
        fprintf(stderr, "Error! Problem with preparing statemant! %s\n", sqlite3_errmsg(myDatabase));
        sqlite3_close(myDatabase);
        WaitForEnter();
        return;
    }

    status = sqlite3_bind_text(stmt, 1, word, -1, SQLITE_TRANSIENT);

    if(status != SQLITE_OK)
    {
        fprintf(stderr, "Error! Problem with binding! %s\n", sqlite3_errmsg(myDatabase));
        sqlite3_finalize(stmt);
        sqlite3_close(myDatabase);
        return;
    }

    status = sqlite3_step(stmt);

    if(status != SQLITE_DONE)
    {
        fprintf(stderr, "Error! Problem with executing SQL. %s\n", sqlite3_errmsg(myDatabase));
        sqlite3_finalize(stmt);
        sqlite3_close(myDatabase);
        return;

    } else{

        printf("The word %s has been successfully removed from the database.\n", word);
        sqlite3_finalize(stmt);
        sqlite3_close(myDatabase);
        return;
    }
}

void Exit(void)
{
    printf("Bye, bye!\n");
    exit(0);
}

bool IsWordGuessed(const char *hiddenWord)
{
    for(int i = 0; hiddenWord[i] != '\0'; i++)
    {
        if(hiddenWord[i] == '_')
            return false;
    }
    return true;
}

void Play(void)
{
    system("clear");
    GetRandomWord();
    hiddenWord = DisplayWordToGuess(randomWord);
    printf("USED LETTERS: \n");
    printf("CATEGORY: %s\n", randomCategory);
    printf("WORD: %s\n", hiddenWord);
    printf("\n");

    bool isWordGuessed = false;
    bool isWordUnknown = false;
     
    int numb = 0;
    usedLetters = NULL;
    usedCount = 0;
    chances = 10;

    while(true)
    {
        isWordGuessed = IsWordGuessed(hiddenWord);

        if(!isWordGuessed && !isWordUnknown){
            GuessLetter();
            system("clear");
            printf("USED LETTERS: %s\n", usedLetters);
            printf("CHANCES: %i\n", chances);
            printf("CATEGORY: %s\n", randomCategory);
            printf("WORD: %s\n", hiddenWord);
            printf("\n");

        }else{
            system("clear");
            printf("CONGRATULATIONS!\n");
            printf("THE ANSWER WAS: %s\n", randomWord);
            printf("\n");
            
            ClearMemory();
            WaitForEnter();
            return;
        }

        if(chances < 1)
        {
            isWordUnknown = true;
            ClearMemory();
            system("clear");
            GameOver();
            WaitForEnter();
            return;
        }
    }

    }

void ClearMemory(void)
{
    free(usedLetters);
    free((void *)hiddenWord);
    free((void *)randomWord);
    free((void *)randomCategory);

    randomWord = NULL;
    randomCategory = NULL;
    hiddenWord = NULL;
    usedLetters = NULL;
    
}

char *DisplayWordToGuess(const unsigned char *word)
{ 
    int len = strlen(word);
    char *hiddenWord = malloc(3 * len + 1);

    if(hiddenWord == NULL)
    {
        fprintf(stderr, "No word to hide.\n");
        return NULL;
    }

    int j=0;
    for(int i=0; i<len; i++)
    {
        if(word[i] == ' ')
        {
            hiddenWord[j++] = ' ';
            hiddenWord[j++] = ' ';
            hiddenWord[j++] = ' ';
        }else{

            hiddenWord[j++] = '_';
            hiddenWord[j++] = ' ';
        }

    }
    hiddenWord[j] = '\0';

    return hiddenWord;
}

void RevealLetterInWord(const char *originalWord, char *newWord, char guessedLetter)
{
    int hiddenIndex = 0;
    bool wasLetterFound = false;

    for(int i=0; originalWord[i] != '\0'; i++)
    {
        if(originalWord[i] == ' ')
        {
            hiddenIndex += 3;
        }else{

            if(originalWord[i] == guessedLetter)
            {
                newWord[hiddenIndex] = guessedLetter;
                wasLetterFound = true;
            }

            hiddenIndex +=2;
        }
    }

    if(!wasLetterFound && chances > 0)
    {
        chances--;
    }

    
}

void GuessLetter(void)
{
    char letter;
    char buffer[3];

    printf("Guess a letter: \n");
    fgets(buffer, sizeof(buffer), stdin);

    if (buffer[1] != '\n') 
    {
        printf("Too many characters! Only first was accepted.\n");
    }

    if (strchr(buffer, '\n') == NULL) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
    }

    letter = toupper(buffer[0]);

    if (!isalpha(letter)) {
        fprintf(stderr, "Invalid input! Enter a single letter.\n");
        return;
    }

    if(!WasLetterUsed(usedLetters, usedCount, letter))
    {
        AddUsedLetter(&usedLetters, &usedCount, letter);
    }
 
    RevealLetterInWord(randomWord, hiddenWord, letter);
}

bool WasLetterUsed(const char *usedLetters, int numberOfUsedLetters, char letter)
{
    for(int i=0; i<numberOfUsedLetters; i++)
    {
        if(usedLetters[i] == letter)
        {
            return true;
        }
    }
    return false;
}

void AddUsedLetter(char **usedLetters, int *numberOfUsedLetters, char letter)
{
    char *newList = realloc(*usedLetters, (*numberOfUsedLetters+4) * sizeof(char));
    if(newList == NULL)
    {
        fprintf(stderr, "Error! Memory allocation faild");
        exit(1);
    }

    *usedLetters = newList;
    (*usedLetters)[*numberOfUsedLetters] = letter;
    (*numberOfUsedLetters)++;
    (*usedLetters)[*numberOfUsedLetters] = ',';
    (*numberOfUsedLetters)++;
    (*usedLetters)[*numberOfUsedLetters] = ' ';
    (*numberOfUsedLetters)++;
    (*usedLetters)[*numberOfUsedLetters] = '\0';
    
    
}

void GameOver(void)
{
    printf("GAME OVER!\n");
    printf("THE HIDDEN WORD WAS: %s\n", randomWord);
    printf("\n");
}

int main()
{
    
    while(true)
    {
        char option = DrawMenu();
        
        switch(option)
        {
            case '1':
                Play();
            break;

            case '2':
                SeeWordsList();
            break;

            case '3':
                AddNewWord();
            break;

            case '4':
                RemoveWord();
            break;

            case '5':
                printf("Bye, bye!\n");
                exit(0);
            break;

            default:
                fprintf(stderr, "There is no option such as: %c", option);
                WaitForEnter();
            break;

        }

    }
    
    return 0;
}