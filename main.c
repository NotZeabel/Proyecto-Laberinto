#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdbool.h>
 
gboolean redOn,amberOn,greenOn = TRUE;
GtkWidget* window;



//================= Logica ==================

#define FRONTERA -1
#define NONE 0
#define LABERINTO 1
#define B_ARRIBA 0b0001
#define B_DERECHA 0b0010
#define B_ABAJO 0b0100
#define B_IZQUIERDA 0b1000

struct celda{
    int estado; 
    char barrera;
};

struct pos{
    int row;
    int col;
};




int agregarFronteras(struct celda** laberinto, int r, int c, struct pos* fronteras, int size){
    if(laberinto[r+1][c].estado == NONE){
        laberinto[r+1][c].estado = FRONTERA;
        fronteras[size].row = r+1;
        fronteras[size].col = c;
        size++;
    } 
    if(laberinto[r-1][c].estado == NONE){
        laberinto[r-1][c].estado = FRONTERA;
        fronteras[size].row = r-1;
        fronteras[size].col = c;
        size++;
    } 
    if(laberinto[r][c+1].estado == NONE){
        laberinto[r][c+1].estado = FRONTERA;
        fronteras[size].row = r;
        fronteras[size].col = c+1;
        size++;
    } 
    if(laberinto[r][c-1].estado == NONE){
        laberinto[r][c-1].estado = FRONTERA;
        fronteras[size].row = r;
        fronteras[size].col = c-1;
        size++;
    }     

    return size;
}

char direccionOpuesta(char direccion){
    switch (direccion){
     case B_ARRIBA:
        return B_ABAJO;        
    case B_ABAJO:
        return B_ARRIBA;
    case B_DERECHA:        
        return B_IZQUIERDA;        
    case B_IZQUIERDA:
        return B_DERECHA;    
     default:
        return 0;
     }    
}

bool eliminarBarrera(struct celda** laberinto, int row, int col, char direccion){
     if(row<0 || col<0) return false;

     int rowVecino, colVecino = 0;

     switch (direccion){
     case B_ARRIBA:
        rowVecino = row;
        colVecino = col-1;
        break;
    case B_ABAJO:
        rowVecino = row;
        colVecino = col+1;
        break;
    case B_DERECHA:
        rowVecino = row+1;
        colVecino = col;
        break;
    case B_IZQUIERDA:
        rowVecino = row-1;
        colVecino = col;
        break;
     default:
        return false;
    }

    if(rowVecino<0 || colVecino<0) return false;
    if(laberinto[rowVecino][colVecino].estado != LABERINTO) return false;
    
    laberinto[rowVecino][colVecino].barrera = laberinto[rowVecino][colVecino].barrera & direccionOpuesta(direccion);
    laberinto[row][col].barrera = laberinto[row][col].barrera & direccion;

    return true;
}

void generarLaberinto(struct celda** laberinto, int rows, int cols){
    //Inicializacion
    for(int i=0; i<rows; i++){
        struct celda row[cols];                      //Talves hay que usar malloc    
        for (int j = 0; j < cols; j++){
            struct celda c = {NONE, 0};
            row[j] = c;
        }
        laberinto[i] = row;        
    }

    struct pos fronteras[2/3*rows*cols]; 
    int fronterasSize = 0;
    
    laberinto[0][0].estado = LABERINTO;
    fronterasSize = agregarFronteras(laberinto, 0, 0, fronteras, fronterasSize);

    int r;
    while(fronterasSize != 0){
        //Escoger celda frontera random
        r = rand()%fronterasSize;
        int row = fronteras[r].row;
        int col = fronteras[r].row;

        //Eliminar frontera de lista
        fronteras[r] = fronteras[fronterasSize-1];
        fronterasSize--;
        laberinto[row][col].estado = LABERINTO;

        //Eliminar barrera aleatoria                    
        char direccion[] = {B_ARRIBA, B_DERECHA, B_ABAJO, B_IZQUIERDA};         
        int randDir = rand()%4;
        while(!eliminarBarrera(laberinto, row, col, direccion[randDir])){
            randDir = (randDir+1)%4;
        }
        
        //Agregar nuevas fronteras
        fronterasSize = agregarFronteras(laberinto, row, col, fronteras, fronterasSize);
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