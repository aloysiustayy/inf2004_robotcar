#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define ROW 13
#define COL 9

/*
    How does robot get Linked List maze without moving
*/
struct Cell;

typedef struct Cell
{
    int x, y;
    struct Cell *left;
    struct Cell *right;
    struct Cell *up;
    struct Cell *down;
    bool visited;
} Cell;

enum DIRECTION
{
    NORTH,
    EAST,
    SOUTH,
    WEST
};
typedef struct
{
    int x;
    int y;
    enum DIRECTION curDir;
} Robot;
Robot robot;

char *dirToText()
{
    char *text;

    switch (robot.curDir)
    {
    case NORTH:
        text = strdup("NORTH");
        break;
    case EAST:
        text = strdup("EAST");
        break;
    case SOUTH:
        text = strdup("SOUTH");
        break;
    case WEST:
        text = strdup("WEST");
        break;
    default:
        text = strdup(""); // Empty string for unknown direction
    }

    return text;
}
void printRobotPos()
{
    char *directionText = dirToText();

    printf("I am at (%d, %d) facing %s right now\n", robot.x, robot.y, directionText);
}

int mazeOnGround[ROW][COL] = {
    {0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 1, 0, 1, 1, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 1, 1, 0, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 0, 0},
    {0, 1, 0, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 1, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 0, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 1, 1, 0, 1, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0}};

void setStartCell(Cell **Path)
{
    // Allocate memory for the Cell structure
    *Path = (Cell *)malloc(sizeof(Cell));

    // Check if memory allocation was successful
    if (*Path == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the starting cell
    for (int y = 0; y < COL; y++)
    {
        // Detect where is the starting point
        if (mazeOnGround[0][y] == 1)
        {
            (*Path)->x = 0;
            (*Path)->y = y;
            break;
        }
    }
    // Path.left
    (*Path)->left = NULL;
    (*Path)->right = NULL;
    (*Path)->up = NULL;
    (*Path)->down = NULL;
    // (*Path)->left = (*Path);

    // (*Path)->left = (Cell *)malloc(sizeof(Cell));
    // (*Path)->right = (Cell *)malloc(sizeof(Cell));
    // (*Path)->up = (Cell *)malloc(sizeof(Cell));
    // (*Path)->down = (Cell *)malloc(sizeof(Cell));
}

void checkLeft()
{
}
void checkRight()
{
}
bool isFrontWall()
{
    // get robot x and y
    // get robot dir
    // see if it is in mazeonground
    // for (int x = 0; x < ROW; x++)
    // {
    //     for (int y = 0; y < COL; y++)
    //     {
    //         if(x == )
    //     }
    // }
    // if(mazeOnGround[robot.x][robot.y])
    switch (robot.curDir)
    {
    case NORTH:
        // y++
        int xToCheck = robot.x + 1;
        int yToCheck = robot.y;

        if (mazeOnGround[xToCheck][yToCheck] == 1)
        {
            // means is path
            printf("(%d, %d) is a path\n", xToCheck, yToCheck);
        }
        else
        {
            // means is wall
            printf("(%d, %d) is a wall\n", xToCheck, yToCheck);
        }
        break;
    case EAST:
        // x--
        robot.y++;
        break;
    case SOUTH:
        // y--
        robot.x--;
        break;
    case WEST:
        // x++
        robot.y--;
        break;
    }
    return false;
}
void checkFront(Cell **Path)
{
    // check infront of robot
    // if robot direction is North, get current position(x,y)
    // if current position(y++), check against mazeOnGround, if mazeOnGround[x][y] is 1, means BLACK, set to 0
    // printf("I am facing %d right now\n", robot.curDir);
    Cell *newCell = (Cell *)malloc(sizeof(Cell));
    newCell->visited = 0;
    printRobotPos();
    isFrontWall();
    switch (robot.curDir)
    {
    case NORTH:
        // y++
        newCell->down = (*Path);     // set new cell's down Cell as current path
        newCell->x = (*Path)->x;     // set new cell's position
        newCell->y = (*Path)->y + 1; // set new cell's position
        (*Path)->up = newCell;       // Set current path's cell
        break;
    case EAST:
        // x--
        newCell->right = (*Path);
        newCell->x = (*Path)->x - 1;
        newCell->y = (*Path)->y;
        (*Path)->left = newCell;
        break;
    case SOUTH:
        // y--
        newCell->up = (*Path);
        newCell->x = (*Path)->x;
        newCell->y = (*Path)->y - 1;
        (*Path)->down = newCell;
        break;
    case WEST:
        // x++
        newCell->left = (*Path);
        newCell->x = (*Path)->x + 1;
        newCell->y = (*Path)->y;
        (*Path)->right = newCell;
        break;
    }

    // printf("I am facing %d right now\n", robot.curDir);
}

// Move forward relative to Robot's direction
void moveForward()
{
    switch (robot.curDir)
    {
    case NORTH:
        // y++
        robot.x++;
        break;
    case EAST:
        // x--
        robot.y++;
        break;
    case SOUTH:
        // y--
        robot.x--;
        break;
    case WEST:
        // x++
        robot.y--;
        break;
    }
}

// Turning relative to Robot's direction
void turnLeft()
{
    switch (robot.curDir)
    {
    case NORTH:
        // y++
        robot.curDir = WEST;
        break;
    case EAST:
        // x--
        robot.curDir = NORTH;
        break;
    case SOUTH:
        // y--
        robot.curDir = EAST;
        break;
    case WEST:
        // x++
        robot.curDir = SOUTH;
        break;
    }
}
void turnRight()
{
    switch (robot.curDir)
    {
    case NORTH:
        // y++
        robot.curDir = EAST;
        break;
    case EAST:
        // x--
        robot.curDir = SOUTH;

        break;
    case SOUTH:
        // y--
        robot.curDir = WEST;

        break;
    case WEST:
        // x++
        robot.curDir = NORTH;

        break;
    }
}

void Mapping()
{
    //
}

int main()
{

    // Declare a pointer to Path and initialize it to NULL
    Cell *Path = NULL;

    // Call the function to set the start cell
    setStartCell(&Path);

    // Initialise Robot
    robot.x = Path->x;
    robot.y = Path->y;
    robot.curDir = NORTH;

    // Print the values of the starting cell
    // printf("Start cell is (%d, %d)\n", (Path->left)->x, Path->y);
    // checkFront(&Path);
    moveForward();
    printRobotPos();

    // turnLeft();
    turnLeft();

    printRobotPos();
    // turnRight();
    // // turnRight();
    // printRobotPos();
    // moveForward();
    // turnLeft();
    // moveForward();
    // Free the allocated memory
    free(Path);

    return 0;
}