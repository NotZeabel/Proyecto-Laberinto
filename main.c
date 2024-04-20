#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>


//================= Defintions ==================

//UI 
GtkWidget* window;
GtkWidget *saveWidget, *generateWidget, *solveWidget, *loadWidget, *exitWidget, *executeWidget, 
*stopWidget, *stopAnimationWidget;
GtkWidget *drawArea;
double zoom;
gboolean dragging = FALSE;
gdouble initialX, initialY, previousX, previousY, offsetX, offsetY = 0;
bool drawingAnimation = FALSE;
bool solving = FALSE;
int sleepTime = 700000;



//Maze
#define FRONTIER -1
#define NONE 0
#define LABYRINTH 1
#define B_UPPER 0b0001
#define B_RIGHT 0b0010
#define B_LOWER 0b0100
#define B_LEFT 0b1000

//Tremaux
#define UNMARKED 0
#define MARKED 1
#define F_MARKED 2

//Fattah
#define NORMAL 0
#define TRAVERSAL_LEFT 1
#define TRAVERSAL_RIGHT 2

//TODO: make all the structs typedef
//Labirynth
struct cell{
    int state; 
    char barrier;
    int visited;
};
struct maze{
    struct cell** matrix;
    int rows;
    int cols;
};
struct pos{
    int row;
    int col;
};


//LinkedList
struct node{
    struct node* next;    
    struct pos position;
};
void addNode(struct node* list, struct pos position){
    list->next = (struct node*)malloc(sizeof(struct node));
    list->next->position = position;
    list->next->next = NULL;
}


//Stack
struct stackNode{
    struct pos position;
    struct stackNode* next;
};
typedef struct {
    struct stackNode* top;
} stack;
stack* createStack() {
    stack* s = (stack*)malloc(sizeof(stack));
    s->top = NULL;
    return s;
}
bool isStackEmpty(stack* s) {
    return s->top == NULL;
}
void push(stack* s, struct pos position) {
    struct stackNode* newNode = (struct stackNode*)malloc(sizeof(struct stackNode));
    newNode->position = position;
    newNode->next = s->top;
    s->top = newNode;
}
struct pos pop(stack* s) {
    if (s->top == NULL) {
        return (struct pos){-1, -1};
    }

    struct stackNode* tempNode = s->top;
    struct pos position = tempNode->position;
    s->top = s->top->next;
    free(tempNode);

    return position;
}
struct pos popNext(stack* s){
    pop(s);

    if (s->top == NULL) {
        return (struct pos){-1, -1};
    }

    return s->top->position;
}
void freeStack(stack* s) {
    while (!isStackEmpty(s)) {
        struct stackNode* tempNode = s->top;
        s->top = s->top->next;
        free(tempNode);
    }
    free(s);
}

struct maze lab = {NULL, 1, 1};
//Solutions
const struct pos start = {0,0};
bool randomMouseEnable = false;
struct node randomMouseSolution = {NULL, {0,0}};
bool leftHandEnable = false;
struct node leftHandSolution = {NULL, {0,0}};
bool rightHandEnable = false;
struct node rightHandSolution = {NULL, {0,0}};
bool pledgeEnable = false;
struct node pledgeSolution = {NULL, {0,0}};
bool tremauxEnable = false;
struct node tremauxSolution = {NULL, {0,0}};
bool fattahEnable = false;
struct node fattahSolution = {NULL, {0,0}};





//================= LOGIC ==================

void freeLabyrinth(struct maze* lab){
    if(lab->matrix != NULL){
        for(int i=0; i<lab->rows; i++){
            if(lab->matrix[i] != NULL) free(lab->matrix[i]);             
        }
        free(lab->matrix);
    }    
}

void freeList(struct node* list){
    struct node* currentNode = list->next;
    while(currentNode != NULL){
        struct node* temp = currentNode->next;
        free(currentNode);
        currentNode = temp;
    }
    list->next = NULL;
}

void freeMemory(){
    freeLabyrinth(&lab);
    if(randomMouseEnable) freeList(&randomMouseSolution);
    if(leftHandEnable) freeList(&leftHandSolution);
    if(rightHandEnable) freeList(&rightHandSolution);
    if(pledgeEnable) freeList(&pledgeSolution);
    if(tremauxEnable) freeList(&tremauxSolution);
    if(fattahEnable) freeList(&fattahSolution);
}

void printLabyrinth(struct maze* lab){
    for (int j = 0; j < lab->cols; j++){            
        printf("XXXX");        
    }
    printf("XX\n");

    for(int i=0; i<lab->rows; i++){
        printf("XX");
        for (int j = 0; j < lab->cols; j++){            
            if((lab->matrix[i][j].barrier & B_RIGHT)>0){
                printf("    ");
            }else{
                printf("  XX");
            }
        }
        printf("\nXX");
        for (int j = 0; j < lab->cols; j++){            
            if((lab->matrix[i][j].barrier & B_LOWER)>0){
                printf("  XX");
            }else{
                printf("XXXX");
            }
        }
        printf("\n");
    }
}

char oppositeDirection(char direction){
    switch (direction){
     case B_UPPER:
        return B_LOWER;        
    case B_LOWER:
        return B_UPPER;
    case B_RIGHT:        
        return B_LEFT;        
    case B_LEFT:
        return B_RIGHT;    
     default:
        return 0;
     }    
}

char leftDirection(char direction){
    switch (direction){
     case B_UPPER:
        return B_LEFT;        
    case B_LOWER:
        return B_RIGHT;
    case B_RIGHT:     
        return B_UPPER;        
    case B_LEFT:
        return B_LOWER;    
     default:
        return 0;
     }    
}

char rightDirection(char direction){
    switch (direction){
     case B_UPPER:
        return B_RIGHT;        
    case B_LOWER:
        return B_LEFT;
    case B_RIGHT:     
        return B_LOWER;        
    case B_LEFT:
        return B_UPPER;    
     default:
        return 0;
     }    
}

char getDirectionFromPos(struct pos pos1, struct pos pos2){
    if(pos2.col > pos1.col){
        return B_RIGHT;
    }
    if(pos2.col < pos1.col){
        return B_LEFT;
    }
    if(pos2.row > pos1.row){
        return B_LOWER;
    }
    if(pos2.row < pos1.row){
        return B_UPPER;
    }
    return 0;
}

bool hasNeighbor(struct maze* lab, int row, int col, char direction){
    if(row<0 || col<0) return false;

     int rowNeighbor, colNeighbor = 0;

     switch (direction){
     case B_UPPER:
        rowNeighbor = row-1;
        colNeighbor = col;
        break;
    case B_LOWER:
        rowNeighbor = row+1;
        colNeighbor = col;
        break;
    case B_RIGHT:
        rowNeighbor = row;
        colNeighbor = col+1;
        break;
    case B_LEFT:
        rowNeighbor = row;
        colNeighbor = col-1;
        break;
     default:
        return false;
    }

    if(rowNeighbor<0 || colNeighbor<0 || rowNeighbor >= lab->rows || colNeighbor >= lab->cols) return false;

    return true;
}

struct pos neighborPosition(struct maze* lab, int row, int col, char direction){
     int rowNeighbor, colNeighbor = 0;

     switch (direction){
     case B_UPPER:
        rowNeighbor = row-1;
        colNeighbor = col;
        break;
    case B_LOWER:
        rowNeighbor = row+1;
        colNeighbor = col;
        break;
    case B_RIGHT:
        rowNeighbor = row;
        colNeighbor = col+1;
        break;
    case B_LEFT:
        rowNeighbor = row;
        colNeighbor = col-1;
        break;
    default:
    }

    struct pos neighborPosition = {0, 0};

    if(rowNeighbor<0 || colNeighbor<0 || rowNeighbor >= lab->rows || col >= lab->cols) return neighborPosition;

    neighborPosition.row = rowNeighbor;
    neighborPosition.col = colNeighbor;
    return neighborPosition;
}

int addFrontiers(struct maze* lab, int row, int col, struct pos* frontiers, int size){
    char direction[] = {B_UPPER, B_RIGHT, B_LOWER, B_LEFT};

    for(int i=0; i<4; i++){
        if(!hasNeighbor(lab, row, col, direction[i])) continue;
        
        struct pos neighbor = neighborPosition(lab, row, col, direction[i]);

        if(lab->matrix[neighbor.row][neighbor.col].state == NONE){
            lab->matrix[neighbor.row][neighbor.col].state = FRONTIER;
            frontiers[size].row = neighbor.row;
            frontiers[size].col = neighbor.col;
            size++;
        } 
    }

    return size;
}

bool deleteBarrier(struct maze* lab, int row, int col, char direction){
    if(row<0 || col<0 || row > lab->rows || col > lab->cols) return false;
    if(!hasNeighbor(lab, row, col, direction)) return false;
    
    struct pos neighbor = neighborPosition(lab, row, col, direction);

    if(lab->matrix[neighbor.row][neighbor.col].state != LABYRINTH) return false;
    
    lab->matrix[neighbor.row][neighbor.col].barrier = lab->matrix[neighbor.row][neighbor.col].barrier | oppositeDirection(direction);
    lab->matrix[row][col].barrier = lab->matrix[row][col].barrier | direction;
    return true;
}

void generateLabyrinth(struct maze* lab){
    lab->matrix = (struct cell**)malloc(lab->rows * sizeof(struct cell*));        
    for (int i=0; i<lab->rows; i++){
        lab->matrix[i] = (struct cell*)malloc(lab->cols * sizeof(struct cell));
    }
    for(int i=0; i<lab->rows; i++){                        
        for (int j = 0; j < lab->cols; j++){
            struct cell c = {NONE, 0, UNMARKED};
            lab->matrix[i][j] = c;
        }         
    }
    
    int size = ((lab->rows*lab->cols));
    size = size * 2/3;
    size = size * sizeof(struct pos);
    struct pos* frontiers = (struct pos*)malloc(size);    //Algorithm's formula  
    int frontierSize = 0;
    
    lab->matrix[0][0].state = LABYRINTH;
    frontierSize = addFrontiers(lab, 0, 0, frontiers, frontierSize);

    int r;
    while(frontierSize != 0){
        //Choose a random frontier cell
        r = rand()%frontierSize;
        int row = frontiers[r].row;
        int col = frontiers[r].col;

        //Delete frontier cell from array
        frontiers[r] = frontiers[frontierSize-1];
        frontierSize--;
        lab->matrix[row][col].state = LABYRINTH;

        //Delete random barrier                   
        char direction[] = {B_UPPER, B_RIGHT, B_LOWER, B_LEFT};         
        int randDir = rand()%4;
        while(!deleteBarrier(lab, row, col, direction[randDir])){
            randDir = (randDir+1)%4;
        }
        
        //Add new frontier
        frontierSize = addFrontiers(lab, row, col, frontiers, frontierSize);
    }
    free(frontiers);
    
    lab->matrix[0][0].barrier = lab->matrix[0][0].barrier | B_LEFT;
    lab->matrix[lab->rows-1][lab->cols-1].barrier = lab->matrix[lab->rows-1][lab->cols-1].barrier | B_RIGHT;        
    zoom = 1;
}



/////////////////////////////////////////////////////////
////////////           Solutions           //////////////
/////////////////////////////////////////////////////////


void disableSolutions(){
    randomMouseEnable = false;
    leftHandEnable = false;
    rightHandEnable = false;
    pledgeEnable = false;
    tremauxEnable = false;
    fattahEnable = false;
}

void drawPartialSolution() {
    if(drawingAnimation){
        gtk_widget_queue_draw(drawArea);
        while(gtk_events_pending()) gtk_main_iteration();
        usleep(sleepTime);     
    }
}


void solveRandomMouse(struct maze* lab, struct node* solutionList){
    struct node* currentNode = solutionList;
    struct pos currentPos = currentNode->position;    
    struct pos endPos = {lab->rows-1,lab->cols-1};
    lab->matrix[0][0].barrier = lab->matrix[0][0].barrier & ~(B_LEFT); //Insert wall in entrance
    char currentDir = B_RIGHT;

    while(currentPos.row != endPos.row || currentPos.col != endPos.col){                
        char directions[3];      
        int dirCount=0;

        //Save possible directions
        if((lab->matrix[currentPos.row][currentPos.col].barrier & currentDir) != 0){
            directions[dirCount] = currentDir;
            dirCount++;
        }
        if((lab->matrix[currentPos.row][currentPos.col].barrier & leftDirection(currentDir)) != 0){
            directions[dirCount] = leftDirection(currentDir);
            dirCount++;
        }
        if((lab->matrix[currentPos.row][currentPos.col].barrier & rightDirection(currentDir)) != 0){
            directions[dirCount] = rightDirection(currentDir);
            dirCount++;
        }

        char dir;
        if(dirCount != 0){
            //Select random direction from possible directions
            dir = directions[rand()%dirCount];
        }else{
            dir = oppositeDirection(currentDir);
        }

        struct pos nextPos = neighborPosition(lab, currentPos.row, currentPos.col, dir);
        addNode(currentNode, nextPos);
        currentNode = currentNode->next;   
        currentPos = currentNode->position;
        currentDir = dir;

        //Draw current solution
        drawPartialSolution();   
        if(!solving)return;
    }
    lab->matrix[0][0].barrier = lab->matrix[0][0].barrier | B_LEFT; //Remove wall in entrance
}

void solveLeftHand(struct maze* lab, struct node* solutionList){
    struct node* currentNode = solutionList; 
    struct pos currentPos = currentNode->position;    
    struct pos endPos = {lab->rows-1,lab->cols-1};
    char currentDir = B_RIGHT; 

    while(currentPos.row != endPos.row || currentPos.col != endPos.col){        
        
        char dir = 0;
        if((lab->matrix[currentPos.row][currentPos.col].barrier & leftDirection(currentDir)) != 0){
            dir = leftDirection(currentDir);
        } else if((lab->matrix[currentPos.row][currentPos.col].barrier & currentDir) != 0){
            dir = currentDir;
        } else if((lab->matrix[currentPos.row][currentPos.col].barrier & rightDirection(currentDir)) != 0){
            dir = rightDirection(currentDir);
        } else{
            dir = oppositeDirection(currentDir);
        }                

        struct pos nextPos = neighborPosition(lab, currentPos.row, currentPos.col, dir); //this method can be a parameter of addNode
        addNode(currentNode, nextPos);
        currentNode = currentNode->next;   
        currentPos = currentNode->position;    
        currentDir = dir;    

        drawPartialSolution();   
        if(!solving)return;       
    }    
}

void solveRightHand(struct maze* lab, struct node* solutionList){
    struct node* currentNode = solutionList;
    struct pos currentPos = currentNode->position;    
    struct pos endPos = {lab->rows-1,lab->cols-1};
    char currentDir = B_RIGHT;

    while(currentPos.row != endPos.row || currentPos.col != endPos.col){        
        
        char dir = 0;
        if((lab->matrix[currentPos.row][currentPos.col].barrier & rightDirection(currentDir)) != 0){
            dir = rightDirection(currentDir);
        } else if((lab->matrix[currentPos.row][currentPos.col].barrier & currentDir) != 0){
            dir = currentDir;
        } else if((lab->matrix[currentPos.row][currentPos.col].barrier & leftDirection(currentDir)) != 0){
            dir = leftDirection(currentDir);
        } else{
            dir = oppositeDirection(currentDir);
        }                

        struct pos nextPos = neighborPosition(lab, currentPos.row, currentPos.col, dir); //this method can be a parameter of addNode
        addNode(currentNode, nextPos);
        currentNode = currentNode->next;   
        currentPos = currentNode->position;    
        currentDir = dir;     

        drawPartialSolution();
        if(!solving)return;
    }    
}

void solvePledge(struct maze* lab, struct node* solutionList){
    struct node* currentNode = solutionList; 
    struct pos currentPos = currentNode->position;    
    struct pos endPos = {lab->rows-1,lab->cols-1};
    lab->matrix[0][0].barrier = lab->matrix[0][0].barrier & ~(B_LEFT); //Insert wall in entrance
    char currentDir = B_RIGHT; 
    bool handOnWall = FALSE;
    int angle = 0;

    while(currentPos.row != endPos.row || currentPos.col != endPos.col){        
        char dir = 0;
        
        if((lab->matrix[currentPos.row][currentPos.col].barrier & B_RIGHT) != 0 && !handOnWall){
            dir = B_RIGHT;
        }else{          
            if(!handOnWall){
                currentDir = rightDirection(currentDir);
                angle -= 90;    
            }
            handOnWall = TRUE;
            if((lab->matrix[currentPos.row][currentPos.col].barrier & leftDirection(currentDir)) != 0){
                dir = leftDirection(currentDir);
                angle += 90;
            } else if((lab->matrix[currentPos.row][currentPos.col].barrier & currentDir) != 0){
                dir = currentDir;
            } else if((lab->matrix[currentPos.row][currentPos.col].barrier & rightDirection(currentDir)) != 0){
                dir = rightDirection(currentDir);
                angle -= 90;
            } else{
                dir = oppositeDirection(currentDir);
                angle -= 180;
            }               
            printf("%d\n" , angle);
            if(angle == 0){
                handOnWall = FALSE;
            }            
        }

        struct pos nextPos = neighborPosition(lab, currentPos.row, currentPos.col, dir); //this method can be a parameter of addNode
        addNode(currentNode, nextPos);
        currentNode = currentNode->next;   
        currentPos = currentNode->position;    
        currentDir = dir;       
        
        drawPartialSolution();   
        if(!solving)return;
    }    
    lab->matrix[0][0].barrier = lab->matrix[0][0].barrier | B_LEFT; //Remove wall in entrance
}

//TREMAUX AUX
void unmarkAllNodes(struct maze* lab){
    for(int r = 0; r < lab->rows; r++){
        for(int c = 0; c < lab->cols; c++){
            lab->matrix[r][c].visited = UNMARKED;
        }
    }
}

bool isValidMove(struct maze *lab, int row, int col, char direction, int newRow, int newCol){
    if (newRow < 0 || newRow >= lab->rows || newCol < 0 || newCol >= lab->cols) {
        return false;
    }

    if (!(lab->matrix[row][col].barrier & direction)) {
        return false;
    }

    return true;
}

int validPaths(struct maze* lab, int row, int col) {
    int validPaths = 0;

    char directions[] = {B_UPPER, B_RIGHT, B_LOWER, B_LEFT};
    int dRow[] = {-1, 0, 1, 0};
    int dCol[] = {0, 1, 0, -1};

    for (int i = 0; i < 4; i++) {
        int newRow = row + dRow[i];
        int newCol = col + dCol[i];
        char direction = directions[i];

        if(isValidMove(lab, row, col, direction, newRow, newCol)){
            if(lab->matrix[newRow][newCol].visited != F_MARKED) validPaths++;
        }
    }

    if(row == 0 && col == 0) validPaths++;
    return validPaths;
}

void moveToNext(struct pos* nextCell, struct pos* currentPos){
    currentPos->row = nextCell->row;
    currentPos->col = nextCell->col;
}

struct pos getPassageNext(struct maze* lab, struct pos* currentPos) {
    char directions[] = {B_UPPER, B_RIGHT, B_LOWER, B_LEFT};
    int dRow[] = {-1, 0, 1, 0};
    int dCol[] = {0, 1, 0, -1};

    for (int i = 0; i < 4; i++) {
        char direction = directions[i];
        int newRow = currentPos->row + dRow[i];
        int newCol = currentPos->col + dCol[i];
        
        if (isValidMove(lab, currentPos->row, currentPos->col, direction, newRow, newCol) && 
            lab->matrix[newRow][newCol].visited == UNMARKED) {
            struct pos nextCell = {newRow, newCol};
            lab->matrix[newRow][newCol].visited = MARKED;
            return nextCell;
        }
    }

    return (struct pos){-1, -1};
}

int junctionMarks(struct maze* lab, int row, int col) {
    int markedEntrances = 0;
    char directions[] = {B_UPPER, B_RIGHT, B_LOWER, B_LEFT};
    int dRow[] = {-1, 0, 1, 0};
    int dCol[] = {0, 1, 0, -1};

    if(validPaths(lab, row, col) == 1) return markedEntrances;

    for(int i = 0; i < 4; i++){
        char direction = directions[i];
        int newRow = row + dRow[i];
        int newCol = col + dCol[i];
        if(isValidMove(lab, row, col, direction, newRow, newCol) &&
            (lab->matrix[newRow][newCol].visited == MARKED || lab->matrix[newRow][newCol].visited == F_MARKED)) {
                markedEntrances++;
        }
    }

    if(row == 0 && col == 0) markedEntrances++;
    return markedEntrances;
}

void randomUnmarkedEntrance(struct maze* lab, struct pos* currentPos) {
    char directions[] = {B_UPPER, B_RIGHT, B_LOWER, B_LEFT};
    int dRow[] = {-1, 0, 1, 0};
    int dCol[] = {0, 1, 0, -1};
    bool exit = false;

    while(!exit){
        int newPos = rand() % 4;
        char direction = directions[newPos];
        int newRow = currentPos->row + dRow[newPos];
        int newCol = currentPos->col + dCol[newPos];
        if(isValidMove(lab, currentPos->row, currentPos->col, direction, newRow, newCol)
            && lab->matrix[newRow][newCol].visited == UNMARKED) {
            currentPos->row = newRow;
            currentPos->col = newCol;
            lab->matrix[newRow][newCol].visited = MARKED;
            exit = true;
        }
    }
}

void pickBestEntrance(struct maze* lab, struct pos* currentPos) {
    char directions[] = {B_UPPER, B_RIGHT, B_LOWER, B_LEFT};
    int dRow[] = {-1, 0, 1, 0};
    int dCol[] = {0, 1, 0, -1};

    struct pos bestUnmarked = {-1, -1};
    struct pos bestMarked = {-1, -1};

    for(int i = 0; i < 4; i++){
        char direction = directions[i];
        int newRow = currentPos->row + dRow[i];
        int newCol = currentPos->col + dCol[i];
        if(isValidMove(lab, currentPos->row, currentPos->col, direction, newRow, newCol)) {
            if(lab->matrix[newRow][newCol].visited == F_MARKED) continue;
            if(lab->matrix[newRow][newCol].visited == UNMARKED) {
                bestUnmarked = (struct pos){newRow, newCol};
                break;
            }
            if(lab->matrix[newRow][newCol].visited == MARKED) bestMarked = (struct pos){newRow, newCol};
        }
    }

    if(bestUnmarked.row != -1 && bestUnmarked.col != -1) {
        *currentPos = bestUnmarked;
        lab->matrix[currentPos->row][currentPos->col].visited = MARKED;
    } else if(bestMarked.row != -1 && bestMarked.col != -1) {
        *currentPos = bestMarked;
        lab->matrix[currentPos->row][currentPos->col].visited = F_MARKED;
    } else {
        *currentPos = bestUnmarked;
    }
}


//TREMAUX
void solveTremaux(struct maze* lab, struct node* solutionList) {
    unmarkAllNodes(lab);
    struct node* currentNode = solutionList;
    struct pos currentPos = currentNode->position;
    lab->matrix[currentPos.row][currentPos.col].visited = MARKED;

    stack* s = createStack();
    push(s, currentPos);

    while (!(currentPos.row == lab->rows - 1 && currentPos.col == lab->cols - 1)) {
        //WOOPSIE
        if (currentPos.row == -1 || currentPos.col == -1) {
            printf("THERE WAS A HUGE ERROR WHILE SOLVING\n");
            return;
        }

        int cellType = validPaths(lab, currentPos.row, currentPos.col); //1:DEADEND, 2:PASSAGE, >2:JUNCTION

        //PASSAGES
        if (cellType == 2) {
            struct pos nextCell = getPassageNext(lab, &currentPos);
            moveToNext(&nextCell, &currentPos);
            push(s, currentPos);
            addNode(currentNode, currentPos);
            currentNode = currentNode->next;
            drawPartialSolution();
            if(!solving)return;
            continue;
        }

        //JUNCTIONS
        if (cellType == 1 || cellType > 2) {
            //RULE 1: Arbitrary unmarked entrance
            if (junctionMarks(lab, currentPos.row, currentPos.col) == 1) {
                randomUnmarkedEntrance(lab, &currentPos);
                push(s, currentPos);
                addNode(currentNode, currentPos);
                currentNode = currentNode->next;
            } else if (cellType == 1) {
                lab->matrix[currentPos.row][currentPos.col].visited = F_MARKED;
                currentPos = popNext(s);
                addNode(currentNode, currentPos);
                currentNode = currentNode->next;
            } else {
                //RULE 3: Choose the best entrance
                pickBestEntrance(lab, &currentPos);
                push(s, currentPos);
                addNode(currentNode, currentPos);
                currentNode = currentNode->next;
            }
        }
        drawPartialSolution();
        if(!solving)return;
    }

    freeStack(s);
    s = NULL;
    addNode(currentNode, currentPos);
    currentNode = currentNode->next;
}

//FATTAH AUX
int MD(struct pos currentPos, struct pos endPos){
    return abs(currentPos.row - endPos.row) + abs(currentPos.col - endPos.col);
}

bool hasProductivePath(struct maze* lab, int bestMD, struct pos currentPos, struct pos endPos) {
    char directions[] = {B_RIGHT, B_LOWER};
    int dRow[] = {0, 1};
    int dCol[] = {1, 0};

    
    for(int i = 0; i < 2; i++){
        char direction = directions[i];
        int newRow = currentPos.row + dRow[i];
        int newCol = currentPos.col + dCol[i];
        if(isValidMove(lab, currentPos.row, currentPos.col, direction, newRow, newCol) &&
            bestMD > MD((struct pos){newRow, newCol}, endPos)) {
            return true;
        }
    }

    return false;
}

void getRandomProductive(struct maze* lab, int bestMD, char* currentDir, struct pos* currentPos, struct pos endPos) {
    char directions[] = {B_RIGHT, B_LOWER};
    int dRow[] = {0, 1};
    int dCol[] = {1, 0};
    struct pos positions[] = {(struct pos){-1,-1}, (struct pos){-1,-1}};
    int validPositions = 0;
    int randomNumber = rand() % 2;

    for(int i = 0; i < 2; i++){
        char direction = directions[i];
        int newRow = currentPos->row + dRow[i];
        int newCol = currentPos->col + dCol[i];
        if(isValidMove(lab, currentPos->row, currentPos->col, direction, newRow, newCol)) {
            positions[i] = (struct pos){newRow, newCol};
            validPositions++;
        }
    }

    struct pos temp;
    if(validPositions == 1) {
        temp = (positions[0].row == -1 && positions[0].col == -1) ? positions[1] : positions[0];
        *currentDir = (positions[0].row == -1 && positions[0].col == -1) ? directions[1] : directions[0];
    } else {
        temp = positions[randomNumber];
        *currentDir = directions[randomNumber];
    }
    currentPos->row = temp.row;
    currentPos->col = temp.col;
}

struct pos nextTraversalPath(struct maze* lab, char mode, char* currentDir, struct pos currentPos) {
    struct pos nextCell = {-1, -1};
    char dir = 0;

    if(mode == TRAVERSAL_LEFT){
        if((lab->matrix[currentPos.row][currentPos.col].barrier & leftDirection(*currentDir)) != 0){
            dir = leftDirection(*currentDir);
        } else if((lab->matrix[currentPos.row][currentPos.col].barrier & *currentDir) != 0){
            dir = *currentDir;
        } else if((lab->matrix[currentPos.row][currentPos.col].barrier & rightDirection(*currentDir)) != 0){
            dir = rightDirection(*currentDir);
        } else {
            dir = oppositeDirection(*currentDir);
        }
    } else {
        if((lab->matrix[currentPos.row][currentPos.col].barrier & rightDirection(*currentDir)) != 0){
            dir = rightDirection(*currentDir);
        } else if((lab->matrix[currentPos.row][currentPos.col].barrier & *currentDir) != 0){
            dir = *currentDir;
        } else if((lab->matrix[currentPos.row][currentPos.col].barrier & leftDirection(*currentDir)) != 0){
            dir = leftDirection(*currentDir);
        } else {
            dir = oppositeDirection(*currentDir);
        }
    }
    *currentDir = dir;
    nextCell = neighborPosition(lab, currentPos.row, currentPos.col, dir);

    return nextCell;
}

//FATTAH
void solveFattah(struct maze* lab, struct node* solutionList){
    struct node* currentNode = solutionList;
    struct pos currentPos = currentNode->position;
    struct pos endPos = {lab->rows - 1, lab->cols - 1};
    int bestMD = MD(currentPos, endPos);
    char mode = NORMAL;
    char tModes[] = {TRAVERSAL_LEFT, TRAVERSAL_RIGHT};
    int modeIndex = rand() % 2;
    char currentDir = B_RIGHT;
    lab->matrix[0][0].barrier = lab->matrix[0][0].barrier & ~(B_LEFT);

    while(!(currentPos.row == endPos.row && currentPos.col == endPos.col)) {
        if(bestMD == MD(currentPos, endPos) && hasProductivePath(lab, bestMD, currentPos, endPos)) {
            bestMD--;
            getRandomProductive(lab, bestMD, &currentDir, &currentPos, endPos);
            addNode(currentNode, currentPos);
            currentNode = currentNode->next;
            mode = NORMAL;
        } else if(mode == TRAVERSAL_LEFT || mode == TRAVERSAL_RIGHT) {
            currentPos = nextTraversalPath(lab, mode, &currentDir, currentPos);
            addNode(currentNode, currentPos);
            currentNode = currentNode->next;
        } else {
            mode = tModes[modeIndex];
            currentPos = nextTraversalPath(lab, mode, &currentDir, currentPos);
            addNode(currentNode, currentPos);
            currentNode = currentNode->next;
        }

        drawPartialSolution();
        if(!solving)return;
    }

    lab->matrix[0][0].barrier = lab->matrix[0][0].barrier | B_LEFT;
}


void printSolution(struct node* solutionList){    
    struct node* currentNode = solutionList;
    printf("[");
    while(currentNode != NULL){
        printf("(%d, %d),", currentNode->position.row, currentNode->position.col);
        currentNode = currentNode->next;
    }
    printf("]");
}










//================= DRAW ==================

void drawSolution(cairo_t *cr, struct node* list, int cellWidth, int cellHeight, int width, int height, int padding, int track){    
    struct node* currentNode = list;
    char lastDir = B_RIGHT;
    int trackWidth = cellWidth - (cellWidth/7 * track);
    int trackHeigth = cellHeight/7 * track;


    while(currentNode != NULL){             
        int xCord = currentNode->position.col * cellWidth + offsetX + padding/2;
        int yCord = currentNode->position.row * cellHeight + offsetY + padding/2;
        
        //if cell is out of the view it is not rendered
        if((xCord<0 && xCord+cellWidth<0)  || (xCord>width && xCord+cellWidth>width) ||
            (yCord<0 && yCord+cellHeight<0) || (yCord>height && yCord+cellHeight>height)){
            currentNode = currentNode->next;
            continue;
        }            

        //Draw Line
        //Cairo Lineto is in all cases, simplify 
        switch (lastDir){
            case B_UPPER:
                cairo_move_to(cr, xCord + trackWidth, yCord + cellHeight);
                cairo_line_to(cr, xCord + trackWidth, yCord + trackHeigth);                            
                break;
            case B_LOWER:
                cairo_move_to(cr, xCord + trackWidth, yCord);
                cairo_line_to(cr, xCord + trackWidth, yCord + trackHeigth);        
                break;
            case B_RIGHT:                                
                cairo_move_to(cr, xCord, yCord + trackHeigth);
                cairo_line_to(cr, xCord + trackWidth, yCord + trackHeigth);   
                break;
            case B_LEFT:
                cairo_move_to(cr, xCord + cellWidth, yCord + trackHeigth);
                cairo_line_to(cr, xCord + trackWidth, yCord + trackHeigth);                       
                break;
        }    
                    
        char nextDir = B_RIGHT;

        if(currentNode->next != NULL){                
            nextDir = getDirectionFromPos(currentNode->position, currentNode->next->position);    
        }

        if(nextDir == oppositeDirection(lastDir)){
            //track +=1;  going back, doesnt work.
        }

        switch (nextDir){
            case B_UPPER:
                cairo_move_to(cr, xCord + trackWidth, yCord + trackHeigth);
                cairo_line_to(cr, xCord + trackWidth, yCord);                            
                break;
            case B_LOWER:
                cairo_move_to(cr, xCord + trackWidth, yCord + trackHeigth);
                cairo_line_to(cr, xCord + trackWidth, yCord + cellHeight);        
                break;
            case B_RIGHT:                                
                cairo_move_to(cr, xCord + trackWidth, yCord + trackHeigth);
                cairo_line_to(cr, xCord + cellWidth, yCord + trackHeigth);   
                break;
            case B_LEFT:
                cairo_move_to(cr, xCord + trackWidth, yCord + trackHeigth);
                cairo_line_to(cr, xCord, yCord + trackHeigth);                       
                break;
        }    
                    
        lastDir = nextDir;
        currentNode = currentNode->next;                    
    }       
}





#define zoomFactor 0.25

gboolean on_drawingarea_draw(GtkWidget *widget, cairo_t *cr, __attribute__((unused)) gpointer data) {
    //Initialize drawing area
    //Taken from http://www.peteronion.org.uk/GtkExamples/GladeTutorials.html    
    guint width, height;

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

    
    // Make background a solid black color
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
    cairo_rectangle(cr,0,0,width,height);
    cairo_fill(cr);
    
    // Add white border
    cairo_set_line_width(cr, 5);
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
    cairo_rectangle(cr,0,0,width,height);
    cairo_stroke(cr);


    //Draw actual labyrinth
    int rows = lab.rows;
    int cols = lab.cols;
    int padding = 40;
    int mazeHeight = height-padding;
    int mazeWidth = width-padding;
    int cellHeight =  mazeHeight*zoom/rows;
    int cellWidth =  mazeWidth*zoom/cols;

    if(cellHeight < 1) cellHeight=1;
    if(cellWidth < 1) cellWidth=1;

    if(zoom==1){
        offsetX=offsetY=0;
    }else{
        int oppositeCornerX = (cols * cellWidth + offsetX + padding) / (zoomFactor / zoom);
        int oppositeCornerY = (rows * cellHeight + offsetY + padding) / (zoomFactor / zoom);
        offsetX = CLAMP(offsetX, -oppositeCornerX, 0);
        offsetY = CLAMP(offsetY, -oppositeCornerY, 0);
    }
    

    double strokeWidth = 1.0;
    cairo_set_line_width(cr,strokeWidth);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

    //Draw cells    
    //TODO: Draws repeated walls, must be fixed.    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int xCord = j * cellWidth + offsetX + padding/2;
            int yCord = i * cellHeight + offsetY + padding/2;
            
            //if cell is out of the view it is not rendered
            if((xCord<0 && xCord+cellWidth<0)  || (xCord>width && xCord+cellWidth>width) ||
                (yCord<0 && yCord+cellHeight<0) || (yCord>height && yCord+cellHeight>height)){
                continue;
            }

            //Draw Cell
            if(!((lab.matrix[i][j].barrier & B_RIGHT) > 0)) {
                cairo_move_to(cr, xCord + cellWidth, yCord);
                cairo_line_to(cr, xCord + cellWidth, yCord + cellHeight);
            }
            if(!((lab.matrix[i][j].barrier & B_LEFT) > 0) && j==0) {
                cairo_move_to(cr, xCord, yCord);
                cairo_line_to(cr, xCord, yCord + cellHeight);
            }            
            if(!((lab.matrix[i][j].barrier & B_LOWER) > 0)) {
                cairo_move_to(cr, xCord, yCord + cellHeight);
                cairo_line_to(cr, xCord + cellWidth, yCord + cellHeight);
            }
            if(!((lab.matrix[i][j].barrier & B_UPPER) > 0) && i==0) {
                cairo_move_to(cr, xCord, yCord);
                cairo_line_to(cr, xCord + cellWidth, yCord);
            }                                    
        }
    }

    cairo_stroke(cr);
    
    //Draw Solutions
    //TODO draw each solution with a different color
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    if(randomMouseEnable) drawSolution(cr, &randomMouseSolution, cellWidth, cellHeight, width, height, padding, 1);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    if(leftHandEnable) drawSolution(cr, &leftHandSolution, cellWidth, cellHeight, width, height, padding, 2);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    if(rightHandEnable) drawSolution(cr, &rightHandSolution, cellWidth, cellHeight, width, height, padding, 3);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    if(pledgeEnable) drawSolution(cr, &pledgeSolution, cellWidth, cellHeight, width, height, padding, 4);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
    if(tremauxEnable) drawSolution(cr, &tremauxSolution, cellWidth, cellHeight, width, height, padding, 5);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    if(fattahEnable) drawSolution(cr, &fattahSolution, cellWidth, cellHeight, width, height, padding, 6);

    cairo_stroke(cr);

    return FALSE;
}



//Zoom In/Out

gboolean zoomEvent(GtkWidget *widget, GdkEventScroll *event, gpointer data){
    if (event->direction == GDK_SCROLL_UP) {
        zoom += (zoomFactor * zoom);
    } else if(event->direction == GDK_SCROLL_DOWN) {
        zoom -= (zoomFactor * (zoom / 2)); 
    }

    if(zoom > MIN(lab.cols, lab.rows)) zoom = MIN(lab.cols, lab.rows);
    if(zoom<1) zoom=1;
    
    gtk_widget_queue_draw(widget);

    return TRUE;
}

//Drag and move around
gboolean buttonPressed(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if(event->button == GDK_BUTTON_PRIMARY) {
        dragging = TRUE;
        initialX = event->x;
        initialY = event->y;
        previousX = initialX;
        previousY = initialY;
    }

    return TRUE;
}

gboolean buttonReleased(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    if(event->button == GDK_BUTTON_PRIMARY) {
        dragging = FALSE;
        double curentOffsetX = offsetX;
        double curentOffsetY = offsetY;

        curentOffsetX = curentOffsetX + curentOffsetY;

    }

    return TRUE;
}

gboolean motionNotify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
    if(dragging) {
        gdouble dirX = (event->x - previousX);
        gdouble dirY = (event->y - previousY) ;

        offsetX += dirX;
        offsetY += dirY;

        previousX = event->x;
        previousY = event->y;

        gtk_widget_queue_draw(widget);
    } 

    return TRUE;
}






//================= SAVE & LOAD FUNCTIONS ==================


void enableSave() {
    gtk_widget_set_sensitive(saveWidget, TRUE);
}

void disableLowerButtons(){
    gtk_widget_set_sensitive(executeWidget, FALSE);
    gtk_widget_set_sensitive(stopWidget, FALSE);
    gtk_widget_set_sensitive(stopAnimationWidget, FALSE);
}

void enableSideButtons(){
    gtk_widget_set_sensitive(saveWidget, TRUE);
    gtk_widget_set_sensitive(loadWidget, TRUE);
    gtk_widget_set_sensitive(generateWidget, TRUE);
    gtk_widget_set_sensitive(exitWidget, TRUE);
    gtk_widget_set_sensitive(solveWidget, TRUE);
}

//disables all the buttons that may cause problems while the maze is being solve
void disableSideButtons(){
    gtk_widget_set_sensitive(saveWidget, FALSE);
    gtk_widget_set_sensitive(loadWidget, FALSE);
    gtk_widget_set_sensitive(generateWidget, FALSE);
    gtk_widget_set_sensitive(exitWidget, FALSE);
    gtk_widget_set_sensitive(solveWidget, FALSE);

} 

void saveFile(char* path){
    FILE* file;

    file = fopen(path, "wb");

    fwrite(&(lab.rows), sizeof(int), 1, file);
    fwrite(&(lab.cols), sizeof(int), 1, file);

    for(int i=0; i<lab.rows; i++){
        fwrite(lab.matrix[i], sizeof(struct cell), lab.cols, file);
    }    
        
    fclose(file); 
}

void loadFile(char* path){
    FILE* file;
    file = fopen(path, "rb");


    if(file == NULL) {
        GtkWidget *dialogWindow;
        const gchar *message = "Error. Unable to read file";

        dialogWindow = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);

        gtk_dialog_run(GTK_DIALOG(dialogWindow));

        gtk_widget_destroy(dialogWindow);
        return;
    }

    //Free used memory
    freeMemory();

    //Read rows and cols 
    fread(&(lab.rows), sizeof(int), 1, file);
    fread(&(lab.cols), sizeof(int), 1, file);

    //Initialize new labyrinth
    lab.matrix = (struct cell**)malloc(lab.rows * sizeof(struct cell*));        
    for (int i=0; i<lab.rows; i++){
        lab.matrix[i] = (struct cell*)malloc(lab.cols * sizeof(struct cell));
    }
    for(int i=0; i<lab.rows; i++){                        
        for (int j = 0; j < lab.cols; j++){
            struct cell c = {NONE, 0, UNMARKED};
            lab.matrix[i][j] = c;
        }         
    }

    //Read matrix
    for(int i=0; i<lab.rows; i++){
        fread(lab.matrix[i], sizeof(struct cell), lab.cols, file);
    }    

    //Assign Initial Zoom
    zoom = 1;
    
    disableSolutions();
    disableLowerButtons();
    enableSave();
    fclose(file);   
}




//================= BUTTON EVENTS ==================
//Taken from GTK API



void generateButton(__attribute__((unused)) GtkWidget *widget, 
                       __attribute__((unused)) gpointer   data){
    //Taken from https://stackoverflow.com/questions/48177396/signal-handlers-for-message-dialogs-in-gtk-using-glade-and-c
    GtkWidget  *dialog;
    GtkWidget  *rowsInput;
    GtkWidget  *colsInput;
    GtkBuilder *builder;

    g_return_if_fail (window != NULL);

    builder = gtk_builder_new_from_file("inputDialog.glade");

    dialog = GTK_WIDGET(gtk_builder_get_object (builder, "inputDialog"));
    gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(window));
    rowsInput = GTK_WIDGET(gtk_builder_get_object (builder, "rowsInput"));
    colsInput = GTK_WIDGET(gtk_builder_get_object (builder, "colsInput"));

    gtk_widget_show_all(dialog);

    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    const gchar* rowsText = gtk_entry_get_text(GTK_ENTRY(rowsInput));
    const gchar* colsText = gtk_entry_get_text(GTK_ENTRY(colsInput));
    
    if(response == 1){
        int rows = atoi(rowsText);
        int cols = atoi(colsText);    
        if(0<rows && rows<=2048 && 0<cols && cols<=2048){            
            freeMemory();
            lab.rows = rows;
            lab.cols = cols;
            generateLabyrinth(&lab);    
            disableSolutions();
            disableLowerButtons();
            enableSave();
        }else{
            GtkWidget *dialogWindow;
            const gchar *message = "Error. Input must be a number between 0 and 2048.";    
            dialogWindow = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", message);
            gtk_dialog_run(GTK_DIALOG(dialogWindow));
            gtk_widget_destroy(dialogWindow);
        }        
    }
    
    gtk_widget_destroy(dialog);
    g_object_unref(G_OBJECT(builder));
}

void solveButton(__attribute__((unused)) GtkWidget *widget, 
                       __attribute__((unused)) gpointer   data){

    GtkWidget  *dialog;
    GtkBuilder *builder;

    g_return_if_fail (window != NULL);

    builder = gtk_builder_new_from_file("solveDialog.glade");

    dialog = GTK_WIDGET(gtk_builder_get_object (builder, "solveDialog"));
    gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(window));

    gtk_widget_show_all(dialog);

    int response = gtk_dialog_run(GTK_DIALOG(dialog));

    if(response == 1) {
        if(randomMouseEnable || leftHandEnable || rightHandEnable || pledgeEnable || tremauxEnable || fattahEnable){
            if(randomMouseEnable) freeList(&randomMouseSolution);
            if(leftHandEnable) freeList(&leftHandSolution);
            if(rightHandEnable) freeList(&rightHandSolution);
            if(pledgeEnable) freeList(&pledgeSolution);
            if(tremauxEnable) freeList(&tremauxSolution);
            if(fattahEnable) freeList(&fattahSolution);
            gtk_widget_set_sensitive(executeWidget, TRUE);
        } else {
            GtkWidget *dialogWindow;
            const gchar *message = "Error. No algorithm selected.";    
            dialogWindow = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", message);
            gtk_dialog_run(GTK_DIALOG(dialogWindow));
            gtk_widget_destroy(dialogWindow);
            gtk_widget_set_sensitive(executeWidget, FALSE);
        }
    }
    ///Claen active solutions
    

    // //Test -- 
    // randomMouseEnable = false;
    // leftHandEnable = true;
    // rightHandEnable = true;
    // pledgeEnable = true;
    // tremauxEnable = true;
    // fattahEnable = true;
    // // -- 

    gtk_widget_destroy(dialog);
    g_object_unref(G_OBJECT(builder));
    return;    
}

void executeButton(__attribute__((unused)) GtkWidget *widget, 
                       __attribute__((unused)) gpointer   data){

    ///Claen active solutions
    if(randomMouseEnable) freeList(&randomMouseSolution);
    if(leftHandEnable) freeList(&leftHandSolution);
    if(rightHandEnable) freeList(&rightHandSolution);
    if(pledgeEnable) freeList(&pledgeSolution);
    if(tremauxEnable) freeList(&tremauxSolution);
    if(fattahEnable) freeList(&fattahSolution);                        
                        
    disableSideButtons();
    gtk_widget_set_sensitive(executeWidget, FALSE);
    gtk_widget_set_sensitive(stopWidget, TRUE);
    gtk_widget_set_sensitive(stopAnimationWidget, TRUE);

    drawingAnimation = TRUE;
    solving = TRUE;

    sleepTime = 7000000/(lab.rows * lab.cols);    
    if(randomMouseEnable && solving) solveRandomMouse(&lab, &randomMouseSolution);
    if(leftHandEnable && solving) solveLeftHand(&lab, &leftHandSolution);
    if(rightHandEnable && solving) solveRightHand(&lab, &rightHandSolution);
    if(pledgeEnable && solving) solvePledge(&lab, &pledgeSolution);
    if(tremauxEnable && solving) solveTremaux(&lab, &tremauxSolution);
    if(fattahEnable && solving) solveFattah(&lab, &fattahSolution);
    drawingAnimation=FALSE;
    solving = FALSE;
    
    gtk_widget_queue_draw(drawArea);
    enableSideButtons();
    gtk_widget_set_sensitive(executeWidget, TRUE);
    gtk_widget_set_sensitive(stopWidget, FALSE);
    gtk_widget_set_sensitive(stopAnimationWidget, FALSE);

}

void stopAnimationButton(__attribute__((unused)) GtkWidget *widget, 
                       __attribute__((unused)) gpointer   data){

    drawingAnimation = FALSE;
}

void stopButton(__attribute__((unused)) GtkWidget *widget, 
                       __attribute__((unused)) gpointer   data){

    drawingAnimation = FALSE;
    solving = FALSE;
}

void saveButton(GtkWidget *widget, __attribute__((unused)) gpointer   data) {
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    GtkFileFilter *filter;
    gint res;

    dialog = gtk_file_chooser_dialog_new ("Save File",
                                        GTK_WINDOW(window),
                                        action,
                                        "_Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        "_Save",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
    chooser = GTK_FILE_CHOOSER (dialog);

    filter = gtk_file_filter_new ();
    gtk_file_filter_add_pattern (filter, "*.maze");
    gtk_file_filter_set_name (filter, "Maze files");
    gtk_file_chooser_add_filter (chooser, filter);

    gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        char *extendedFilename;

        filename = gtk_file_chooser_get_filename (chooser);

        if (!g_str_has_suffix(filename, ".maze")) {
            extendedFilename = g_strconcat(filename, ".maze", NULL);
            g_free(filename);
            filename = extendedFilename;
        }

        printf("%s\n", filename);
        saveFile(filename);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
}

void loadButton(GtkWidget *widget, __attribute__((unused)) gpointer   data) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    GtkFileFilter *filter;
    gint res;
    
    dialog = gtk_file_chooser_dialog_new ("Open File",
                                        GTK_WINDOW(window),
                                        action,
                                        "_Cancel",
                                        GTK_RESPONSE_CANCEL,
                                        "_Open",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);

    filter = gtk_file_filter_new ();
    gtk_file_filter_add_pattern (filter, "*.maze");
    gtk_file_filter_set_name (filter, "Maze files");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
                                        
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename (chooser);
        loadFile(filename);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
}

void exitButton(__attribute__((unused)) GtkWidget *widget, 
                       __attribute__((unused)) gpointer   data){
        gtk_main_quit();
}
  
gboolean windowDelete(__attribute__((unused)) GtkWidget *widget, 
                      __attribute__((unused)) GdkEvent  *event,
                      __attribute__((unused)) gpointer   data){
    gtk_main_quit();    
    return FALSE;
}




//================= MAIN ==================

int main(int argc, char **argv)
{
    srand(time(NULL));
    GtkBuilder *builder;
    GError *error = NULL;
    

    gtk_init(&argc,&argv);


    builder = gtk_builder_new();
    if( gtk_builder_add_from_file (builder,"gui.glade" ,&error) == 0){
        printf("gtk_builder_add_from_file FAILED %s\n",error->message);
        return EXIT_FAILURE;
    }
    
    window  = GTK_WIDGET (gtk_builder_get_object (builder,"window"));
    saveWidget  = GTK_WIDGET (gtk_builder_get_object (builder,"saveButton"));
    generateWidget  = GTK_WIDGET (gtk_builder_get_object (builder,"generateButton"));
    solveWidget  = GTK_WIDGET (gtk_builder_get_object (builder,"solveButton"));
    loadWidget  = GTK_WIDGET (gtk_builder_get_object (builder,"loadButton"));
    exitWidget  = GTK_WIDGET (gtk_builder_get_object (builder,"exitButton"));
    executeWidget  = GTK_WIDGET (gtk_builder_get_object (builder,"executeButton"));
    stopWidget  = GTK_WIDGET (gtk_builder_get_object (builder,"stopButton"));
    stopAnimationWidget  = GTK_WIDGET (gtk_builder_get_object (builder,"stopAnimationButton"));
    gtk_builder_connect_signals (builder, NULL);

    drawArea = GTK_WIDGET(gtk_builder_get_object(builder, "labyrinthDrawingArea"));

    gtk_widget_add_events(drawArea, GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    
    generateLabyrinth(&lab);
    disableLowerButtons();
    gtk_widget_show_all (window);
    gtk_main();

    return EXIT_SUCCESS;
}
