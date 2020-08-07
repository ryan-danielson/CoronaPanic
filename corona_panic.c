/* 
 * Author: Ryan Danielson
 * 
 * Description: A simple terminal based game about social distancing. Simply make your
 * 		way through the forest without encountering other people. Take some
 * 		trees with you along the way to achieve a high score.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BORDER 1 
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define RED "\033[1;31m"
#define BLUE "\033[1;33m"
#define CYAN "\033[1;36m"

typedef struct player player;
struct player {
	int x;
	int y;
};

typedef struct enemy enemy;
struct enemy {
	int x;
	int y;
	enemy *next;
};

int getDimension();
void intro(void);
void createBoard(int dimension, int boardArray[dimension][dimension]);
void printBoard(int dimension, int boardUpdate[dimension][dimension], player *playerLocation, enemy *enemyLocation);
player *input(player *playerLocation, int dimension, int boardArray[][dimension], int *score, enemy *enemyLocation);
void cleanBuf();
int boundaryCheck(int y, int x, int dimension, int boardArray[dimension][dimension]);
int stateCheck(player *playerLocation, enemy *enemyLocation, int dimension, int boardArray[][dimension]);
void treeRemoval(char *choice, player *playerLocation, int dimension, int boardArray[][dimension], char animate, int *score, enemy *enemyLocation);
void chopAnimate(player *playerLocation, int dimension, int dimensionY, int dimensionX, int boardArray[][dimension], enemy *enemyLocation);
void fireAnimate(player *playerLocation, int dimension, int dimensionY, int dimensionX, int boardArray[][dimension], int *score, enemy *enemyLocation);
void enemyMovement(int dimension, int boardArray[][dimension], player *playerLocation, enemy *enemyLocation);
enemy *enemyHead(int dimension);
void freeEnemy(enemy *head);

int 
main(void)
{
	/* import score file */
	FILE *ifp = fopen("score.txt", "r+");

	/* retrieve high score from score.txt, assign it to old score. activate new score pointer. */
	int over = 0;
	int *score = NULL;
	score = malloc(sizeof(int));
	*score = 0;
	int oldScore = 0;
	fscanf(ifp, "%d", &oldScore);
	rewind(ifp);

	/* define player pointer and initial location */
	player *playerLocation = NULL;
	playerLocation = malloc(sizeof(player));
	playerLocation->x = 1;
	playerLocation->y = 1;

	/* define new player location pointer */
	player *newPlayerLocation = NULL;
	newPlayerLocation = malloc(sizeof(player));

	/* get board dimension from menu, then append border size */
	int dimension = getDimension();
	dimension += BORDER;


	/* define enemy pointer and create a linked list */
	enemy *enemyLocation = enemyHead(dimension);

	/* set energy pointer */
	int *energy = NULL;
	energy = malloc(sizeof(int));
	*energy = dimension*2;

	/* define and populate board */
	int boardArray[dimension][dimension];
	createBoard(dimension, boardArray);
	
	/* intro text */
	intro();

	/* main game loop. will loop until energy is depleted or player is infected. */
	while (!over) {
		system("clear");
		printf("%sEnergy: %d/%d %10sHigh Score: %d %10sScore: %d\n", BLUE, *energy, dimension*2, BLUE, oldScore, BLUE, *score);	
		printBoard(dimension, boardArray, playerLocation, enemyLocation);
		newPlayerLocation = input(playerLocation, dimension, boardArray, score, enemyLocation);		/* retrieve new player location via input */
		if (boundaryCheck(newPlayerLocation->y, newPlayerLocation->x, dimension, boardArray)) {		/* check the new location against boundaries */
			playerLocation = newPlayerLocation;							/* set playerLocation to new location if allowed */
			--*energy;
		}
		
		enemyMovement(dimension, boardArray, playerLocation, enemyLocation);				/* update enemy movement */

		if (*energy <= 0) {										/* game over due to no energy */
			printf("You ran out of energy and died alone in the woods.\n");				
			++over;
		}
		if (stateCheck(playerLocation, enemyLocation, dimension, boardArray) == 1) {			/* check if infected */
			printf("You've failed to maintain a radius of six feet from infected hikers and will now suffer the consequences.\n");
			++over;
		} else if (stateCheck(playerLocation, enemyLocation, dimension, boardArray) == 2) { 		/* check if successfully exited stage */
			system("clear");
			printBoard(dimension, boardArray, playerLocation, enemyLocation);
			usleep(500000);
			system("clear");
			playerLocation->x += 1;
			printBoard(dimension, boardArray, playerLocation, enemyLocation);
			usleep(500000);
			system("clear");
			printf("You've managed to follow the safety guidelines. Humanity thanks you.\n");
			usleep(500000);
			++over;
		}
	}

	/* write high-score if it's greater than old score */
	if (*score > oldScore) 	
		fprintf(ifp, "%d", *score);
	
	/* free memory and close files */
	fclose(ifp);
	freeEnemy(enemyLocation);
	free(playerLocation);	
	free(energy);
	
	return 0;
}

/* getDimension prompts for board dimensions within a range of 10-50 */

int 
getDimension()
{
	int validate = -1;	
	int dimension;

	printf("Please enter a dimension for the board (10-50): ");
	validate = scanf("%d", &dimension);
	cleanBuf();	

	while (dimension < 10 || dimension > 50 || validate != 1) {	
		puts("Sorry, you've entered a number outside of the range.");
		printf("Please enter a dimension for the board (0-50): ");
		validate = scanf("%d", &dimension);	/* returns successful number of reads from input */
		cleanBuf();
	}
	return dimension;
}

/* intro displays the intro text */

void 
intro(void)
{
	system("clear");
	printf("%sStir crazy from shelter-in-place orders, you decide to go for a walk in the woods.\n", BLUE);
	printf("%sThere are a few infectious like-minded individuals in the forest on this day.\n", BLUE);
	printf("%sExit the forest while maintaining social distancing. You can gain points by removing trees.\n", BLUE);
	usleep(5555000);
}

/*
 * createBoard creates the initial board. The board array is randomly generated based on rand() % 2, which will 
 * result in 0 or 1 (tree or ground). An exit is randomly generated at the end.
 */

void 
createBoard(int dimension, int boardArray[][dimension])
{
	srand(time(NULL));

	/* top and left border */

	for (int i = 0; i < BORDER; ++i) {
		for (int j = 0; j < dimension; ++j) {
			boardArray[j][i] = 2;
			boardArray[i][j] = 2;
		}
	}

	/* bottom and right border */

	for (int i = 0; i < dimension; ++i) {
		for (int j = dimension-1; j < dimension; ++j) {	
			boardArray[i][j] = 2;
			boardArray[j][i] = 2;
		}
	}
	
	/* trees and ground */

	for (int i = BORDER; i < dimension-1; ++i) {			/* y dimension */
		for (int j = BORDER; j < dimension-1; ++j) {		/* x dimension */
			boardArray[i][j] = rand() % 2;
		}
	}
	
	boardArray[2 + rand() % (dimension - 3)][dimension-1] = 4;	/* randomly generates exit */
	boardArray[1][1] = 1; 						/* set object behind player at start to ground */
}

/* printBoard takes care of printing the boardArray to the screen. It checks for values in the array
 * and translates them into their respective characters. It also prints player and enemies.
 */

void 
printBoard(int dimension, int boardUpdate[][dimension], player *playerLocation, enemy *enemyLocation)
{
	for (int y = 0; y < dimension; ++y) {
		for (int x = 0; x < dimension; ++x) {
			/* print player location */
			if (x == playerLocation->x && y == playerLocation->y) {
				printf("%s@ ", RED);
			}

			/* print enemy locations */
			else if ((x == enemyLocation->x && y == enemyLocation->y) ||
			         (x == enemyLocation->next->x && y == enemyLocation->next->y ) ||
				 (x == enemyLocation->next->next->x && y == enemyLocation->next->next->y) ||
				 (x == enemyLocation->next->next->next->x && y == enemyLocation->next->next->next->y)) 
			{
				printf("%sX ", BLUE);
			}
			
			/* print environment */
			else if (boardUpdate[y][x] == 0) {
				printf("%s^ ", GREEN);
			}
			else if (boardUpdate[y][x] == 1) {
				printf("%s. ", YELLOW);
			}
			else if (boardUpdate[y][x] == 2) {
				printf("%s# ", CYAN);
			}
			else if (boardUpdate[y][x] == 3) {
				printf("%s^ ", RED);
			}
			else if (boardUpdate[y][x] == 4) {
				printf("  ");
			}		
		}
		printf("\n");
	}
}


/* 
 * input takes care of receiving and analyzing input from the keyboard. If it receives a new
 * location it returns the potential new location as a player struct. The returned location is
 * then bounds checked in boundaryCheck. If the move is allowed then the new location is applied
 */

player 
*input(player *playerLocation, int dimension, int boardArray[][dimension], int *score, enemy *enemyLocation)
{
	/* create player pointer for new location */
	player *newLocation = malloc(sizeof(playerLocation));
	newLocation->x = playerLocation->x;
	newLocation->y = playerLocation->y;

	printf("%s", BLUE);
	
	/* create pointers for choice and tree direction */
	char *choice = NULL;
	choice = malloc(sizeof(char));
	char *treeDirection = NULL;
	treeDirection = malloc(sizeof(char));

	*choice = 'r'; /* to enter loop */
	while (*choice != 'h' && *choice != 'j' && *choice != 'k' && *choice != 'l' 
		              && *choice != 'c' && *choice != 'f') {
		
		printf("%s", BLUE);
		printf("Please enter an action (h=left, j=up, k=down, l=right, c=chop, f=fire): \n");
		printf("Current location (x, y): (%d, %d)\n", playerLocation->x, playerLocation->y);
		scanf(" %c", choice);
		cleanBuf();	
		if (*choice == 'h') {
			newLocation->x -= 1; 
			printf("New location: (%d,%d)\n", newLocation->x, newLocation->y);
		}
		else if (*choice == 'j') 
			newLocation->y -= 1;
		else if (*choice == 'k')
			newLocation->y += 1;
       		else if (*choice == 'l')
			newLocation->x += 1;
		else if (*choice == 'c') {
			printf("Enter a direction to chop: ");
			scanf(" %c", treeDirection);
			treeRemoval(treeDirection, playerLocation, dimension, boardArray, *choice, score, enemyLocation);
		}
		else if (*choice == 'f') {
			printf("Enter a direction to spark: ");
			scanf(" %c", treeDirection);
			treeRemoval(treeDirection, playerLocation, dimension, boardArray, *choice, score, enemyLocation);
		}
		else {
			system("clear");
			printBoard(dimension, boardArray, playerLocation, enemyLocation);
			printf("%s", RED);
			printf("Invalid choice.\n");	
		}
	}

	/* free memory and return new location */
	free(choice);
	free(treeDirection);
	return newLocation;
}

/* cleanBuf takes care of any leftover characters in the input buffer */

void 
cleanBuf()
{
	int ch;
	while (ch = getchar() != '\n' && ch != EOF);
}

/* 
 * boundaryCheck takes in x and y coordinates and checks it against the board dimensions 
 * and trees. If the move is allowed, 1 is returned
 */

int 
boundaryCheck(int y, int x, int dimension, int boardArray[dimension][dimension])
{	
	/* if exit, allow movement */
	if (boardArray[y][x] == 4)
		return 1;

	/*if tree or border, deny movement */
	else if (boardArray[y][x] == 0 || y < 1 || y > dimension-2 || x < 1 || x > dimension-2) 
		return 0;
	
	/* otherwise it's ground and allowed */
	else
		return 1;
}

/* 
 * stateCheck checks the state of the game. It tests to see if the player has been infected or if the player 
 * has made it to the exit.
 */

int 
stateCheck(player *playerLocation, enemy *enemyLocation, int dimension, int boardArray[][dimension])
{
	enemy *node = enemyLocation;
	while (node->next != NULL) {
		if (node->y == playerLocation->y && node->x == playerLocation->x) 
			return 1;	/* end game due to infection */
		
		node = node->next;
	}
	if (boardArray[playerLocation->y][playerLocation->x] == 4) 
		return 2; 		/* end game due to victory */

	return 0;			/* game continues */
}

/* treeRemoval takes care of the directing input for tree elimination purposes (such as chop and fire). It takes the direction and action from
 * input and passes it to the appropriate function for animating the action
 *  */

void 
treeRemoval(char *choice, player *playerLocation, int dimension, int boardArray[][dimension], char animate, int *score, enemy *enemyLocation)
{
	int directionY = 0;
	int directionX = 0;
	
	switch (*choice) {
	case 'h' :
	        directionX = -1;
       		break;	
	case 'j' :
		directionY = -1;
		break;
	case 'k' :
		directionY = 1;
		break;
	case 'l' :
		directionX = 1;
		break;
	default  :
		printf("error, invalid choice received\n");
	}	
	
	/* ensure that there is a tree where you're trying to perform an action */
	if (boardArray[playerLocation->y+directionY][playerLocation->x+directionX] == 0){
		if (animate == 'c') {
			chopAnimate(playerLocation, dimension, playerLocation->y+directionY, playerLocation->x+directionX, boardArray, enemyLocation);
			++*score;
		}
		if (animate == 'f')
			fireAnimate(playerLocation, dimension, playerLocation->y+directionY, playerLocation->x+directionX, boardArray, score, enemyLocation);
	}	
}

/* chopAnimate takes care of the animation aspect of the chop action */

void 
chopAnimate(player *playerLocation, int dimension, int dimensionY, int dimensionX, int boardArray[][dimension], enemy *enemyLocation)
{
	system("clear");
	for (int i = 0; i < 4; ++i) {
		for (int k = 0; k < i; ++k) {
			printf("%sChop! ", YELLOW);
		}
		printf("\n");
		boardArray[dimensionY][dimensionX] = i%2;
		printBoard(dimension, boardArray, playerLocation, enemyLocation);
		usleep(500000);
		system("clear");
	}
}

/* fireAnimate is a recursive function that continually checks for trees to set on fire. Each time it's called a new
 * boardArray dimension is sent to the function. */

void 
fireAnimate(player *playerLocation, int dimension, int dimensionY, int dimensionX, int boardArray[][dimension], int *score, enemy *enemyLocation)
{
	system("clear");
	printBoard(dimension, boardArray, playerLocation, enemyLocation);
	usleep(100000);

	for (int y = -1; y < 2; ++y) {
		for (int x = -1; x < 2; ++x) {
			if (boardArray[dimensionY+y][dimensionX+x] == 0) {
				boardArray[dimensionY+y][dimensionX+x] = 3;
				fireAnimate(playerLocation, dimension, dimensionY+y, dimensionX+x, boardArray, score, enemyLocation);
				boardArray[dimensionY+y][dimensionX+x] = 1;
				++*score;
				system("clear");
				printBoard(dimension, boardArray, playerLocation, enemyLocation);
				usleep(100000);
			}
		}
	}
	boardArray[dimensionY][dimensionX] = 1;
}

/* the purpose of enemyMovement is to check bounds and collisions with other enemies, then allow the move if all is clear */

void 
enemyMovement(int dimension, int boardArray[][dimension], player *playerLocation, enemy *enemyLocation)
{
	enemy *enemyNode = enemyLocation;

	/* iterate through all enemy nodes and move them if allowed */
	while (enemyNode->next != NULL) {
		if (playerLocation->x > enemyNode->x) {
			if (boundaryCheck(enemyNode->y, enemyNode->x+1, dimension, boardArray)) {
				int collisionCtr = 0;
				enemy *enemyCollision = enemyNode->next;
				while (enemyCollision->next != NULL) {
					if (enemyCollision->y == enemyNode->y && enemyCollision->x == enemyNode->x+1) 
						++collisionCtr;
					enemyCollision = enemyCollision->next;
				}
				if (collisionCtr == 0)
					enemyNode->x += 1;
			}
		}
		else if (playerLocation->y > enemyNode->y) {
			if (boundaryCheck(enemyNode->y+1, enemyNode->x, dimension, boardArray)) {
				int collisionCtr = 0;
				enemy *enemyCollision = enemyNode->next;
				while (enemyCollision->next != NULL) {
					if (enemyCollision->y == enemyNode->y+1 && enemyCollision->x == enemyNode->x)
						++collisionCtr;
					enemyCollision = enemyCollision->next;
				}
				if (collisionCtr == 0)
					enemyNode->y += 1;
			}
		}
		else if (playerLocation->x < enemyNode->x) {
			if (boundaryCheck(enemyNode->y, enemyNode->x-1, dimension, boardArray)) {
				int collisionCtr = 0;
				enemy *enemyCollision = enemyNode->next;
				while (enemyCollision->next != NULL) {
					if (enemyCollision->y == enemyNode->y && enemyCollision->x == enemyNode->x-1)
						++collisionCtr;
					enemyCollision = enemyCollision->next;
				}
				if (collisionCtr == 0)
					enemyNode->x -= 1;
			}
		}
		else if (playerLocation->y < enemyNode->y) {
			if (boundaryCheck(enemyNode->y-1, enemyNode->x, dimension, boardArray)) {
				int collisionCtr = 0;
				enemy *enemyCollision = enemyNode->next;
				while (enemyCollision->next != NULL) {
					if (enemyCollision->y == enemyNode->y-1 && enemyCollision->x == enemyNode->x)
						++collisionCtr;
					enemyCollision = enemyCollision->next;
				}
				if (collisionCtr == 0)
					enemyNode->y -= 1;
			}
		}
		enemyNode = enemyNode->next;
	}
}

/* enemyHead creates a linked list for enemy structs. It populates the node members (x,y,next) and returns the head of the list. */

enemy 
*enemyHead(int dimension)
{
	srand(time(NULL));
	enemy *head = malloc(sizeof(enemy));
	
	/* set random enemy locations */
	head->x = 2+rand() % (dimension-3); 
	head->y = 2+rand() % (dimension-3);
	
	enemy *last = head;
		
	for (int i = 0; i < 4; ++i) {
		enemy *new = malloc(sizeof(enemy));
		new->x = 2 + rand() % (dimension - 3);
		new->y = 2 + rand() % (dimension - 3); 
		new->next  = NULL;
		while (last->next != NULL) {
			last = last->next;
		}
		last->next = new;
		last = head;
	}
	return head;
}

/* freeEnemy takes care of freeing the memory of enemy linked-list at the end of the game */

void 
freeEnemy(enemy *head)
{
	enemy *node = head;
	while (node->next != NULL) {
		enemy *temp = node;
		node = node->next;
		free(temp);
	}
}
