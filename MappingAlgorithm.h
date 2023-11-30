#define ROW 13
#define COL 9

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
    struct Cell *curPos;
    struct Cell *prevPos;
    enum DIRECTION curDir;

} Robot;

bool visitedCells[ROW][COL];

char *dirToText();
void printRobotPos();

void setStartCell(Cell **Path);
bool isFrontPath();
void findPotentialPaths();

// Move forward relative to Robot's direction
void moveForward();

// Turning relative to Robot's direction
void turnLeft();
void turnRight();

// Move robot to currentNode
void moveRobot(Cell **currentNode);

// Move to next cell, taking into account the direction that robot is in
void moveToNextCell(Cell **currentNode, enum DIRECTION direction);
void moveRobotWithPath(Cell *startNode);
void goTo(struct Cell *targetCell);
void performBFS(Cell *startCell);
void printLinkedList(Cell *start);
void printVisitedCells();
void initRobot();