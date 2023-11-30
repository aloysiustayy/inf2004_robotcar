#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "MappingAlgorithm.h"

Robot robot;

bool visitedCells[ROW][COL];

// Initialize queue for BFS
struct Cell *queue[ROW * COL];
int front = 0, rear = 0;

// int mazeOnGround[ROW][COL] = {
//     {0, 0, 0, 1, 0, 0, 0, 0, 0},
//     {0, 1, 0, 1, 1, 1, 0, 1, 0},
//     {0, 1, 0, 1, 0, 1, 0, 1, 0},
//     {0, 1, 1, 1, 0, 1, 1, 1, 0},
//     {0, 1, 0, 0, 0, 1, 0, 1, 0},
//     {0, 1, 0, 1, 0, 1, 0, 1, 0},
//     {0, 1, 0, 1, 0, 1, 0, 0, 0},
//     {0, 1, 0, 1, 1, 1, 1, 1, 0},
//     {0, 1, 0, 1, 0, 0, 0, 1, 0},
//     {0, 1, 1, 1, 0, 1, 1, 1, 0},
//     {0, 1, 0, 0, 0, 1, 0, 1, 0},
//     {0, 1, 0, 1, 1, 1, 0, 1, 0},
//     {0, 0, 0, 0, 0, 1, 0, 0, 0}};
int mazeOnGround[ROW][COL] = {
    {0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 1, 1, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0}};

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
    printf("I am at (%d, %d) right now\n\n", robot.curPos->x, robot.curPos->y);
}

/// @brief Set the starting position of robot
/// @param Path Type Cell **, main path
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
}

/// @brief Determine if robot's facing a Wall or Path
/// @return Return true if is a Path, false otherwise
bool isFrontPath()
{
    int xToCheck = -1;
    int yToCheck = -1;
    switch (robot.curDir)
    {
    case NORTH:
        // x++
        xToCheck = robot.x + 1;
        yToCheck = robot.y;
        break;
    case EAST:
        // y++
        xToCheck = robot.x;
        yToCheck = robot.y + 1;
        break;
    case SOUTH:
        // x--
        xToCheck = robot.x - 1;
        yToCheck = robot.y;
        break;
    case WEST:
        // y--
        xToCheck = robot.x;
        yToCheck = robot.y - 1;
        break;
    }
    return mazeOnGround[xToCheck][yToCheck];
}

/// @brief Find potential paths (enqueue Cell that is a path)
void findPotentialPaths()
{

    Cell *newCell = (Cell *)malloc(sizeof(Cell));
    newCell->visited = 0;

    // Only enqueue IF is a path
    if (isFrontPath())
    {
        switch (robot.curDir)
        {
        case NORTH:
            // y++
            newCell->up = NULL;
            // newCell->down = (*Path); // set new cell's down Cell as current path
            newCell->down = robot.curPos; // set new cell's down Cell as current path
            newCell->left = NULL;
            newCell->right = NULL;
            newCell->x = robot.x + 1; // set new cell's position
            newCell->y = robot.y;     // set new cell's position
            break;
        case EAST:
            // x--
            newCell->up = NULL;
            newCell->down = NULL;
            newCell->left = NULL;
            newCell->right = robot.curPos;
            newCell->x = robot.x;
            newCell->y = robot.y + 1;
            break;
        case SOUTH:
            // y--
            newCell->up = robot.curPos;
            newCell->down = NULL;
            newCell->left = NULL;
            newCell->right = NULL;
            newCell->x = robot.x - 1;
            newCell->y = robot.y;
            break;
        case WEST:
            // x++
            newCell->up = NULL;
            newCell->down = NULL;
            newCell->left = robot.curPos;
            newCell->right = NULL;
            newCell->x = robot.x;
            newCell->y = robot.y - 1;
            break;
        }
        // Enqueue the newly discovered cell
        queue[rear++] = newCell;
    }
}

/// @brief Move forward relative to Robot's direction
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

/// @brief Turn left according to Robot's relative direction
void turnLeft()
{
    switch (robot.curDir)
    {
    case NORTH:
        robot.curDir = WEST;
        break;
    case EAST:
        robot.curDir = NORTH;
        break;
    case SOUTH:
        robot.curDir = EAST;
        break;
    case WEST:
        robot.curDir = SOUTH;
        break;
    }
}
/// @brief Turn right according to Robot's relative direction
void turnRight()
{
    switch (robot.curDir)
    {
    case NORTH:
        robot.curDir = EAST;
        break;
    case EAST:
        robot.curDir = SOUTH;
        break;
    case SOUTH:
        robot.curDir = WEST;
        break;
    case WEST:
        robot.curDir = NORTH;
        break;
    }
}

/// @brief Update robot's position
/// @param currentNode Type Cell**, main path
void moveRobot(Cell **currentNode)
{
    robot.x = (*currentNode)->x;
    robot.y = (*currentNode)->y;
}

/// @brief Move to next cell
/// @param currentNode current position of robot
/// @param  direction enum DIRECTION, NSEW to move to
void moveToNextCell(Cell **currentNode, enum DIRECTION direction)
{
    switch (direction)
    {
    case NORTH:
        *currentNode = (*currentNode)->down;
        break;
    case EAST:
        *currentNode = (*currentNode)->left;
        break;
    case SOUTH:
        *currentNode = (*currentNode)->up;
        break;
    case WEST:
        *currentNode = (*currentNode)->right;
        break;
    default:
        break;
    }
}

/// @brief Move robot according to path
/// @param startNode Type Cell *, main path
void moveRobotWithPath(Cell *startNode)
{
    Cell *currentNode = startNode;

    while (currentNode != NULL)
    {
        // Move the robot to the current cell
        moveRobot(&currentNode);
        printRobotPos();

        // Move to the next cell based on the discovered path
        if (currentNode->down != NULL)
        {
            moveToNextCell(&currentNode, NORTH);
        }
        else if (currentNode->left != NULL)
        {
            moveToNextCell(&currentNode, EAST);
        }
        else if (currentNode->up != NULL)
        {
            moveToNextCell(&currentNode, SOUTH);
        }
        else if (currentNode->right != NULL)
        {
            moveToNextCell(&currentNode, WEST);
        }
        else
        {
            // No valid adjacent node found, exit loop
            break;
        }
    }
}

/// @brief Goes from robot's curPos to targetCell
/// @param targetCell Type struct Cell *, defines the Cell that robot should be at.
void goTo(struct Cell *targetCell)
{
    // go to robot's previous position
    // from there, see if targetCell is in robot's adjacent or not
    // determine to turn left, right, uturn or what

    struct Cell *tempPos = malloc(sizeof(struct Cell *));
    tempPos->x = targetCell->x;
    tempPos->y = targetCell->y;
    robot.x = tempPos->x;
    robot.y = tempPos->y;
    robot.curPos->x = robot.x;
    robot.curPos->y = robot.y;
    printRobotPos();
}

/// @brief Run BFS on cell, to map out the maze using BFS
/// @param startCell Type of Cell, main path.
void performBFS(Cell *startCell)
{

    // Mark the starting cell as visited and enqueue it
    startCell->visited = true;
    queue[rear++] = startCell;

    // Traverse the queue until reach the rear
    while (front != rear)
    {
        printf("Queue: ");
        for (int i = front; i < rear; ++i)
        {
            printf("(%d, %d) ", queue[i]->x, queue[i]->y);
        }
        printf("\n");

        // Dequeue to go to next cell
        struct Cell *currentCell = queue[front++];

        // Move robot to the position
        goTo(currentCell);

        // Define directions for adjacent cells
        int dx[] = {0, 0, -1, 1}; // x coordinate changes for up, down
        int dy[] = {-1, 1, 0, 0}; // y coordinate changes for left, right

        // Explore adjacent cells (left, right, up, down)
        for (int i = 0; i < 4; ++i)
        {
            int newX = currentCell->x + dx[i];
            int newY = currentCell->y + dy[i];

            // Check if the new cell is within bounds and not visited
            // Adjust the conditions based on the maze structure
            if (newX >= 0 && newY >= 0 && newX < ROW && newY < COL &&
                mazeOnGround[newX][newY] == 1 && !visitedCells[newX][newY])
            {

                // Mark the cell as visited
                visitedCells[newX][newY] = true;

                // Create a new cell for the linked list
                Cell *newCell = (Cell *)malloc(sizeof(Cell));
                newCell->x = newX;
                newCell->y = newY;
                newCell->visited = true;
                newCell->left = NULL;
                newCell->right = NULL;
                newCell->up = NULL;
                newCell->down = NULL;

                // Update the linked list representation based on direction
                switch (i)
                {
                case 0:                           // Moving left
                    newCell->right = currentCell; // Set the new cell's right pointer to the current cell
                    currentCell->left = newCell;  // Set the current cell's left pointer to the new cell
                    break;
                case 1:                           // Moving right
                    newCell->left = currentCell;  // Set the new cell's left pointer to the current cell
                    currentCell->right = newCell; // Set the current cell's right pointer to the new cell
                    break;
                case 2:                          // Moving up
                    newCell->down = currentCell; // Set the new cell's down pointer to the current cell
                    currentCell->up = newCell;   // Set the current cell's up pointer to the new cell
                    break;
                case 3:                          // Moving down
                    newCell->up = currentCell;   // Set the new cell's up pointer to the current cell
                    currentCell->down = newCell; // Set the current cell's down pointer to the new cell
                    break;
                default:
                    break;
                }

                // Enqueue the newly discovered cell
                queue[rear++] = newCell;

                // Print visited cell at every update.
                printVisitedCells();
            }
        }
    }
    printf("Maze traversed successfully!\n");
}

/// @brief Print linked list
/// @param start Type of Cell, main Path
void printLinkedList(Cell *start)
{
    Cell *current = start;

    // Traverse the linked list
    while (current != NULL)
    {
        printf("Visited node: (%d, %d)\n", current->x, current->y);

        // Print connections
        if (current->left != NULL)
        {
            printf("Left: (%d, %d)\n", current->left->x, current->left->y);
        }
        if (current->right != NULL)
        {
            printf("Right: (%d, %d)\n", current->right->x, current->right->y);
        }
        if (current->up != NULL)
        {
            printf("Up: (%d, %d)\n", current->up->x, current->up->y);
        }
        if (current->down != NULL)
        {
            printf("Down: (%d, %d)\n", current->down->x, current->down->y);
        }

        printf("\n");

        // Move to the next node (Adjust this based on your linked list structure)
        // Assuming rightward traversal, modify this logic as per your structure
        if (current->right != NULL)
        {
            current = current->right;
        }
        else if (current->down != NULL)
        {
            current = current->down;
        }
        else
        {
            // You might need additional logic based on your linked list structure
            break; // Exit loop if no valid adjacent node found
        }
    }
}
/// @brief Print all visited cells
void printVisitedCells()
{
    printf("Visited Cells:\n");
    for (int i = 0; i < ROW; ++i)
    {
        for (int j = 0; j < COL; ++j)
        {
            if (visitedCells[i][j])
            {
                printf("X ");
            }
            else
            {
                printf(". ");
            }
        }
        printf("\n");
    }
}
/// @brief This method will initialise Robot object
/// @param Path Type of Cell, main path
void initRobot(Cell **Path)
{
    robot.x = (*Path)->x;
    robot.y = (*Path)->y;
    struct Cell *newPos = (struct Cell *)malloc(sizeof(struct Cell *));
    newPos->x = robot.x;
    newPos->y = robot.y;
    robot.curPos = (struct Cell *)malloc(sizeof(struct Cell *));
    robot.prevPos = (struct Cell *)malloc(sizeof(struct Cell *));
    robot.prevPos = newPos;
    robot.curPos = newPos;
}
int main()
{

    // Declare a pointer to Path and initialize it to NULL
    Cell *Path = NULL;

    // Call the function to set the start cell
    setStartCell(&Path);

    // Initialise Robot
    initRobot(&Path);

    // Perform BFS traversal and build linked list
    visitedCells[0][3] = true;
    performBFS(Path);

    printLinkedList(Path); // Print the linked list nodes and connections
    printVisitedCells();

    free(Path);
    return 0;
}