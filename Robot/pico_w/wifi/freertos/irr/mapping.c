#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
#include "queue.h"
#include "hardware/i2c.h"
#include "math.h"
#include "stdint.h"
#include "mapping.h"

// void set_direction(int side)
// {
//     if (side == 0)
//     {
//         // Left
//         switch (cur_direction)
//         {
//         case NORTH:
//             cur_direction = WEST;
//             break;
//         case SOUTH:
//             cur_direction = EAST;
//             break;
//         case EAST:
//             cur_direction = NORTH;
//             break;
//         case WEST:
//             cur_direction = SOUTH;
//             break;
//         default:
//             break;
//         }
//     }
// else if (side == 1)
// {
//     // Right
//     switch (cur_direction)
//     {
//     case NORTH:
//         cur_direction = EAST;
//         break;
//     case SOUTH:
//         cur_direction = WEST;
//         break;
//     case EAST:
//         cur_direction = SOUTH;
//         break;
//     case WEST:
//         cur_direction = WEST;
//         break;
//     default:
//         break;
//     }
// }
// }

// void update_maze()
// {
//     switch (cur_direction)
//     {
//     case NORTH:
//         RobotPos.col++;
//         break;
//     case SOUTH:
//         RobotPos.col--;
//         break;
//     case EAST:
//         RobotPos.row++;
//         break;
//     case WEST:
//         RobotPos.row--;
//         break;
//     default:
//         break;
//     }
// }

// void mapping_thread(__unused void *params)
// {

//     while (true)
//     {
//         vTaskDelay(100);
//         printf("Cur Direction = %d\n", cur_direction);
//     }
// }

// void mapping_main()
// {

//     TaskHandle_t mapping_task;
//     xTaskCreate(mapping_thread,
//                 "MappingThread",
//                 configMINIMAL_STACK_SIZE,
//                 NULL,
//                 8,
//                 &mapping_task);
// }

RobotPosition *robot;
MazeCell *maze;
RobotPosition **getRobot()
{
    return &robot;
}
MazeCell **getMaze()
{
    return &maze;
}
// Function to create a new maze cell
MazeCell *createMazeCell(int row, int col, CellContent content)
{
    MazeCell *cell = (MazeCell *)malloc(sizeof(MazeCell));
    cell->row = row;
    cell->col = col;
    cell->content = content;
    cell->next = NULL;
    return cell;
}

// Function to insert a new cell at the end of the maze list
void insertMazeCell(MazeCell **maze, int row, int col, CellContent content)
{
    MazeCell *newCell = createMazeCell(row, col, content);
    if (*maze == NULL)
    {
        *maze = newCell;
    }
    else
    {
        MazeCell *current = *maze;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = newCell;
    }
}

// Function to find a cell in the maze linked list based on row and column
MazeCell *findCellInList(MazeCell *maze, int row, int col)
{
    MazeCell *current = maze;
    while (current != NULL)
    {
        if (current->row == row && current->col == col)
        {
            return current;
        }
        current = current->next;
    }
    return NULL; // Cell not found
}

// Function to calculate the next cell in a given direction
MazeCell *getNextCell(MazeCell *currentCell, Direction currentDirection)
{
    int nextRow = currentCell->row;
    int nextCol = currentCell->col;

    switch (currentDirection)
    {
    case NORTH:
        nextRow--;
        break;
    case EAST:
        nextCol++;
        break;
    case SOUTH:
        nextRow++;
        break;
    case WEST:
        nextCol--;
        break;
    }

    return findCellInList(currentCell, nextRow, nextCol);
}

// Function to update the maze based on sensor readings and robot position
void updateMaze(MazeCell **maze, RobotPosition *robot, int side)
{
    // // Assuming the robot turned left
    // robot->direction = (robot->direction -) % 4; // Rotate left
    if (side == 0)
    {
        // Left
        switch (robot->direction)
        {
        case NORTH:
            robot->direction = EAST;
            break;
        case SOUTH:
            robot->direction = WEST;
            break;
        case EAST:
            robot->direction = SOUTH;
            break;
        case WEST:
            robot->direction = NORTH;
            break;
        default:
            break;
        }
    }
    else if (side == 1)
    {
        // Left
        switch (robot->direction)
        {
        case NORTH:
            robot->direction = WEST;
            break;
        case SOUTH:
            robot->direction = EAST;
            break;
        case EAST:
            robot->direction = SOUTH;
            break;
        case WEST:
            robot->direction = NORTH;
            break;
        default:
            break;
        }
    }

    // Calculate the next cell based on the current direction
    MazeCell *nextCell = getNextCell(findCellInList(*maze, robot->row, robot->col), robot->direction);

    // Update the maze based on the new robot position and the next cell
    if (nextCell != NULL)
    {
        // Update the content of the next cell (e.g., mark it as the path the robot has taken)
        nextCell->content = PATH;
        // Update the robot's position
        robot->row = nextCell->row;
        robot->col = nextCell->col;
    }
    insertMazeCell(maze, robot->row, robot->col, PATH);
}

// Function to move the robot in a given direction and update its position
void moveRobot(RobotPosition *robot)
{
    // Implement code to move the robot forward and update its position
}

// Function to print the maze from the linked list
void printMaze(MazeCell *maze)
{
    MazeCell *current = maze;
    while (current != NULL)
    {
        switch (current->content)
        {
        case EMPTY:
            printf("  ");
            break;
        case OBSTACLE:
            printf("X ");
            break;
        case PATH:
            printf(". ");
            break;
        case START:
            printf("S ");
            break;
        case END:
            printf("E ");
            break;
            // Add more cases as needed
        }
        current = current->next;
    }
    printf("\n");
}

void mapping_thread(__unused void *params)
{
    // stdio_init_all();

    // Initialize GPIO pins
    // (Assuming you have MOTOR and SENSOR pins, similar to previous examples)

    // Initialize an empty maze

    // Insert cells into the maze to represent the layout
    // insertMazeCell(&maze, 0, 0, START);
    // insertMazeCell(&maze, 0, 1, PATH);
    // insertMazeCell(&maze, 0, 2, PATH);
    // insertMazeCell(&maze, 1, 2, PATH);
    // insertMazeCell(&maze, 1, 1, OBSTACLE);
    // insertMazeCell(&maze, 1, 0, PATH);
    // insertMazeCell(&maze, 2, 0, PATH);
    // insertMazeCell(&maze, 2, 1, OBSTACLE);
    // insertMazeCell(&maze, 2, 2, PATH);
    // insertMazeCell(&maze, 3, 2, PATH);
    // insertMazeCell(&maze, 3, 1, PATH);
    // insertMazeCell(&maze, 3, 0, PATH);
    // insertMazeCell(&maze, 4, 0, PATH);
    // insertMazeCell(&maze, 4, 1, PATH);
    // insertMazeCell(&maze, 4, 2, END);

    // robot = {{0, 0, NORTH}}; // Initialize the robot's position and direction
    robot->col = 0;
    robot->row = 0;
    robot->direction = NORTH;

    insertMazeCell(0, 0, START);
    while (1)
    {
        vTaskDelay(100);
        // Update maze based on sensor readings and robot position
        // updateMaze(&maze, &robot, );

        // Move the robot forward and update its position
        // moveRobot(&robot);

        // Print the maze to see the updated layout
        printMaze(maze);

        // Implement web server code to display maze data
        // This might involve using a library like lwIP or other networking libraries
    }

    // Free allocated memory when done
    while (maze != NULL)
    {
        MazeCell *temp = maze;
        maze = maze->next;
        free(temp);
    }
}

void mapping_main()
{
    TaskHandle_t mapping_task;
    xTaskCreate(mapping_thread,
                "MappingThread",
                configMINIMAL_STACK_SIZE,
                NULL,
                8,
                &mapping_task);
}