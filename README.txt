UCSC Game 230 Final Project - Watermelon Game

Start the game by opening "\x64\Release\ucsc-game-230-final-suika-game.exe"

- Once it's gameover, you can start the game again by pressing Enter
- ....may have memory leaks as I was not as diligent in checking for this assignment.

- Highscores are saved to highscore.txt file
- Physics may feel a bit extra slippery
- Fruit combinations happen almost instantly so it doesn't have the extra delay like the actual game. (I meant to add it if I had time)
- "Fruit position" check is not perfect and may break while playing the game normally.
  - Ideally I would have additionally checked if the fruit has stopped moving.
  - Currently it only checks if it's above a certain Y axis and it has a grace period before it starts counting as a gameover.