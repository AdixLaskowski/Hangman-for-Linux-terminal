# Hangman game in linux termial

Hangman game for linux terminal with SQLite database.

## Features

- Play a hangman game
- View all existing words in database.
- Add a new word to database.
- Remove a word from database.

## Requirements

- C compiler
- SQLite3

## Compilation

```bash
gcc main.c -lsqlite3 -o hangman
```

## Running

./hangman

## Screenshots

Menu:
![screenshots/Menu.png]

Game:
![screenshots/Game.png]

Guessed word:
![screenshots/Success.png]

Faild:
![screenshots/Failure.png]

View all words:
![screenshots/List.png]

Adding a word:
![screenshots/Add.png]

Deleting a word:
![screenshots/delete.png]

