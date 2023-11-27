// typedef enum
// {
//     NORTH,
//     SOUTH,
//     EAST,
//     WEST
// } Direction;
// int cur_direction = NORTH;

// typedef struct RobotPos
// {
//     int row;
//     int col;
// } RobotPos;

// typedef enum
// {
//     EMPTY,
//     PATH,
//     START,
//     END
// } CellData;

// typedef struct CellContent
// {
//     int row;
//     int col;
//     CellData cellData;
// } CellContent;

// typedef struct MazeCell
// {
//     int row;
//     int col;
//     CellContent
// } MazeCell;

// void set_direction(int side);
// void update_maze();

// Enum to represent the possible contents of a maze cell
typedef enum
{
    EMPTY,
    OBSTACLE,
    PATH,
    START,
    END,
    // Add more content types as needed
} CellContent;

// Enum to represent the possible directions
typedef enum
{
    NORTH,
    EAST,
    SOUTH,
    WEST
} Direction;

// Structure to represent a maze cell
typedef struct MazeCell
{
    int row;
    int col;
    CellContent content;
    struct MazeCell *next;
} MazeCell;

// Structure to represent the robot's position
typedef struct RobotPosition
{
    int row;
    int col;
    Direction direction;
} RobotPosition;

RobotPosition **getRobot();
MazeCell **getMaze();
void insertMazeCell(MazeCell **maze, int row, int col, CellContent content);
void updateMaze(MazeCell **maze, RobotPosition *robot, int side);
void mapping_thread();
void mapping_main();

// This is my grid.
// #define grid [10][10];