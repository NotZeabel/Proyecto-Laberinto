#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <math.h>
 
GtkWidget* window;
GtkWidget* saveWidget;
double zoom;
gboolean dragging = FALSE;
gdouble initialX, initialY, previousX, previousY, offsetX, offsetY = 0;

//================= LOGIC ==================

#define FRONTIER -1
#define NONE 0
#define LABYRINTH 1
#define B_UPPER 0b0001
#define B_RIGHT 0b0010
#define B_LOWER 0b0100
#define B_LEFT 0b1000

struct cell{
    int state; 
    char barrier;
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

struct maze lab = {NULL, 1, 1};


void freeLabyrinth(struct maze* lab){
    if(lab->matrix != NULL){
        for(int i=0; i<lab->rows; i++){
            if(lab->matrix[i] != NULL) free(lab->matrix[i]);             
        }
        free(lab->matrix);
    }    
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
            struct cell c = {NONE, 0};
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
    //printLabyrinth(lab);   //Remove in final version

    zoom = 1;
}




//================= DRAW ==================

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
    int padding = 20;
    int mazeHeight = height-padding;
    int mazeWidth = width-padding;
    int cellHeight =  mazeHeight*zoom/rows;
    int cellWidth =  mazeWidth*zoom/cols;

    if(zoom==1){
        offsetX=offsetY=0;
    }else{
        int oppositeCorner = (rows * cellWidth + offsetX + padding) / (zoomFactor / zoom);
        offsetX = CLAMP(offsetX, -oppositeCorner, 0);
        offsetY = CLAMP(offsetY, -oppositeCorner, 0);
    }
    


    double strokeWidth = 1.0;
    cairo_set_line_width(cr,strokeWidth);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int xCord = j * cellWidth + offsetX + padding/2;
            int yCord = i * cellHeight + offsetY + padding/2;

            if((xCord<0 && xCord+cellWidth<0)  || (xCord>width && xCord+cellWidth>width) ||
                (yCord<0 && yCord+cellHeight<0) || (yCord>height && yCord+cellHeight>height)){
                continue;
            }

            if(!((lab.matrix[i][j].barrier & B_RIGHT) > 0)) {
                cairo_move_to(cr, xCord + cellWidth, yCord);
                cairo_line_to(cr, xCord + cellWidth, yCord + cellHeight);
            }

            if(!((lab.matrix[i][j].barrier & B_LEFT) > 0)) {
                cairo_move_to(cr, xCord, yCord);
                cairo_line_to(cr, xCord, yCord + cellHeight);
            }
            
            if(!((lab.matrix[i][j].barrier & B_LOWER) > 0)) {
                cairo_move_to(cr, xCord, yCord + cellHeight);
                cairo_line_to(cr, xCord + cellWidth, yCord + cellHeight);
            }

            if(!((lab.matrix[i][j].barrier & B_UPPER) > 0)) {
                cairo_move_to(cr, xCord, yCord);
                cairo_line_to(cr, xCord + cellWidth, yCord);
            }            
        }
    }
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
    freeLabyrinth(&lab);

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
            struct cell c = {NONE, 0};
            lab.matrix[i][j] = c;
        }         
    }

    //Read matrix
    for(int i=0; i<lab.rows; i++){
        fread(lab.matrix[i], sizeof(struct cell), lab.cols, file);
    }    

    //Assign Initial Zoom
    zoom = 1;
    
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
            freeLabyrinth(&lab);
            lab.rows = rows;
            lab.cols = cols;
            generateLabyrinth(&lab);    
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
    GtkWidget *dialogWindow;
    const gchar *message = "Not implemented";    

    dialogWindow = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);

    gtk_dialog_run(GTK_DIALOG(dialogWindow));
    gtk_widget_destroy(dialogWindow);
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
    GtkWidget *drawArea;

    gtk_init(&argc,&argv);


    builder = gtk_builder_new();
    if( gtk_builder_add_from_file (builder,"gui.glade" ,&error) == 0){
        printf("gtk_builder_add_from_file FAILED %s\n",error->message);
        return EXIT_FAILURE;
    }
    
    window  = GTK_WIDGET (gtk_builder_get_object (builder,"window"));
    saveWidget  = GTK_WIDGET (gtk_builder_get_object (builder,"saveButton"));
    gtk_builder_connect_signals (builder, NULL);

    drawArea = GTK_WIDGET(gtk_builder_get_object(builder, "labyrinthDrawingArea"));

    gtk_widget_add_events(drawArea, GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
    
    generateLabyrinth(&lab);
    gtk_widget_show_all (window);
    gtk_main();

    return EXIT_SUCCESS;
}