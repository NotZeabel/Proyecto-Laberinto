#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdbool.h>
 
gboolean redOn,amberOn,greenOn = TRUE;
GtkWidget* window;



//================= Logic ==================

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

struct pos{
    int row;
    int col;
};




int addFrontiers(struct cell** labyrinth, int r, int c, struct pos* frontiers, int size){
    if(labyrinth[r+1][c].state == NONE){
        labyrinth[r+1][c].state = FRONTIER;
        frontiers[size].row = r+1;
        frontiers[size].col = c;
        size++;
    } 
    if(labyrinth[r-1][c].state == NONE){
        labyrinth[r-1][c].state = FRONTIER;
        frontiers[size].row = r-1;
        frontiers[size].col = c;
        size++;
    } 
    if(labyrinth[r][c+1].state == NONE){
        labyrinth[r][c+1].state = FRONTIER;
        frontiers[size].row = r;
        frontiers[size].col = c+1;
        size++;
    } 
    if(labyrinth[r][c-1].state == NONE){
        labyrinth[r][c-1].state = FRONTIER;
        frontiers[size].row = r;
        frontiers[size].col = c-1;
        size++;
    }     

    return size;
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

bool deleteBarrier(struct cell** labyrinth, int row, int col, char direction){
     if(row<0 || col<0) return false;

     int neighborRow, neighborCol = 0;

     switch (direction){
     case B_UPPER:
        neighborRow = row;
        neighborCol = col-1;
        break;
    case B_LOWER:
        neighborRow = row;
        neighborCol = col+1;
        break;
    case B_RIGHT:
        neighborRow = row+1;
        neighborCol = col;
        break;
    case B_LEFT:
        neighborRow = row-1;
        neighborCol = col;
        break;
     default:
        return false;
    }

    if(neighborRow<0 || neighborCol<0) return false;
    if(labyrinth[neighborRow][neighborCol].state != LABYRINTH) return false;
    
    labyrinth[neighborRow][neighborCol].barrier = labyrinth[neighborRow][neighborCol].barrier & oppositeDirection(direction);
    labyrinth[row][col].barrier = labyrinth[row][col].barrier & direction;

    return true;
}

void generateLabyrinth(struct cell** labyrinth, int rows, int cols){
    //Initialization
    for(int i=0; i<rows; i++){
        struct cell row[cols];                      //Talves hay que usar malloc    
        for (int j = 0; j < cols; j++){
            struct cell c = {NONE, 0};
            row[j] = c;
        }
        labyrinth[i] = row;        
    }

    struct pos frontiers[2/3*rows*cols]; 
    int frontierSize = 0;
    
    labyrinth[0][0].state = LABYRINTH;
    frontierSize = addFrontiers(labyrinth, 0, 0, frontiers, frontierSize);

    int r;
    while(frontierSize != 0){
        //Choose a random frontier cell
        r = rand()%frontierSize;
        int row = frontiers[r].row;
        int col = frontiers[r].row;

        //Delete frontier cell from array
        frontiers[r] = frontiers[frontierSize-1];
        frontierSize--;
        labyrinth[row][col].state = LABYRINTH;

        //Delete random barrier                    
        char direction[] = {B_UPPER, B_RIGHT, B_LOWER, B_LEFT};         
        int randDir = rand()%4;
        while(!deleteBarrier(labyrinth, row, col, direction[randDir])){
            randDir = (randDir+1)%4;
        }
        
        //Add new frontier
        frontierSize = addFrontiers(labyrinth, row, col, frontiers, frontierSize);
    }
}




















//================= Dibujo ==================


//Taken from http://www.peteronion.org.uk/GtkExamples/GladeTutorials.html

gboolean on_drawingarea_draw(GtkWidget *widget, cairo_t *cr, __attribute__((unused)) gpointer data)
{
    guint width, height;
    double diameter;

    // Get the size of the drawing area
    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);
    

    // Define points for drawing the regular trapezium for the white boarder
    int borderPoints[][2] = {
        {10,10},{0,0},
        {width,0},{width-10,10},
        {width-10,height-10},{width,height},
        {0,height},{10,height-10},
        {10,10},{0,0},
    };
    
    // Fill whole area to black
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
    cairo_rectangle(cr,0,0,width,height);
    cairo_fill(cr);

    // Set fill colour to white
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr,1);
    
    // Draw the white boarder with four regular trapeziums
    for(int start=0; start<=6; start+=2)
    {
        for(int n=start; n<=start+3; n++)
        {
            cairo_line_to(cr,borderPoints[n][0],borderPoints[n][1]);
        }
        
        // Draw and fill the trapezium
        cairo_close_path(cr);
        cairo_stroke_preserve(cr);
        cairo_fill(cr);
    }

    // Draw Red circle
    diameter = (width < height/3) ? 0.35 * width : 0.35 * height / 3.0; 
    cairo_set_source_rgba (cr, redOn ? 1.0 : 0.5, 0.0, 0.0, 1.0);
    cairo_arc(cr, 0.5*width, 0.17*height, diameter, 0.0 ,2.0 * G_PI);
    cairo_fill(cr);

    // Draw Amber circle
    cairo_set_source_rgba (cr, amberOn ? 1.0 : 0.5, amberOn ? 1.0 : 0.5, 0.0, 1.0);
    cairo_arc(cr, 0.5*width, 0.5*height,diameter, 0.0, 2.0 * G_PI);
    cairo_fill(cr);

    // Draw Green circle
    cairo_set_source_rgba (cr, 0.0,greenOn ? 1.0 : 0.5, 0.0, 1.0);
    cairo_arc(cr, 0.5*width, 0.83*height, diameter, 0.0, 2.0 * G_PI);
    cairo_fill(cr);
  
    return FALSE;
}


//Save & Load Functions

void saveFile(char* path){
    FILE* file;

    file = fopen(path, "w");

    // char data[81];

    // for(int i=0; i<9; i++){
    //     for(int j=0; j<9; j++){
    //         data[i*9 +j] = readGridButton(i,j);
    //     }
    // }

    // fprintf(file, "%s", data);
    fclose(file); 
}

void loadFile(char* path){
    FILE* file;
    file = fopen(path, "r");


    if(file == NULL) {
        GtkWidget *dialogWindow;
        const gchar *message = "Error. Unable to read file";

        dialogWindow = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);

        gtk_dialog_run(GTK_DIALOG(dialogWindow));

        gtk_widget_destroy(dialogWindow);
        return;
    }

    // char data[81];
    // fgets(data, 81, file);

    // for(int i=0; i<9; i++){
    //     for(int j=0; j<9; j++){
    //         if(data[i*9 +j]<'0' || data[i*9 +j]>'9'){
    //             writeGridButton(' ', i, j);
    //         }else{
    //             writeGridButton(data[i*9 +j], i, j);
    //         }
            
    //     }
    // }

    fclose(file);   
}

//From GTK API

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
    gtk_file_filter_add_pattern (filter, "*.sudoku");
    gtk_file_filter_set_name (filter, "Sudoku files");
    gtk_file_chooser_add_filter (chooser, filter);

    gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        char *extendedFilename;

        filename = gtk_file_chooser_get_filename (chooser);

        if (!g_str_has_suffix(filename, ".sudoku")) {
            extendedFilename = g_strconcat(filename, ".sudoku", NULL);
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
    gtk_file_filter_add_pattern (filter, "*.sudoku");
    gtk_file_filter_set_name (filter, "Sudoku files");
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

int main(int argc, char **argv)
{
    srand(time(NULL));
    GtkBuilder *builder;
    GError *error = NULL;

    gtk_init(&argc,&argv);

    builder = gtk_builder_new ();
    if( gtk_builder_add_from_file (builder,"gui.glade" ,&error) == 0)
    {
        printf("gtk_builder_add_from_file FAILED %s\n",error->message);
        return EXIT_FAILURE;
    }

    window  = GTK_WIDGET (gtk_builder_get_object (builder,"window"));
    gtk_builder_connect_signals (builder, NULL);

    // Turn all the lights ON
    redOn = amberOn = greenOn = TRUE;
    
    gtk_widget_show_all (window);
    gtk_main();

    return EXIT_SUCCESS;
}