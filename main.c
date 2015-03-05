#include <stdlib.h>
#include <gtk/gtk.h>
#include "nestapi_core.h"


GtkWidget *main_window, *flist, *text_width, *text_height;
GtkListStore *list_store;
struct DxfFile *dxf_files;
struct DxfFileI *dxf_files_int;
char **f_names;
int max_files = 128, f_count = 0;
int quant_files = 4;

enum {
	COLUMN_ITEM_NUMBER,
	COLUMN_ITEM_PATH,
	COLUMN_ITEM_QUANT
};

void on_window_destroy(GtkWidget *widget, gpointer user_data)
{
	gtk_main_quit ();
	 /* quit main loop when windows closes */ 
}

void on_button_nest_click(GtkWidget *widget, gpointer user_data)
{
	const char *cwidth, *cheight;
	int width, height;
	
	cwidth = gtk_entry_get_text(GTK_ENTRY(text_width));
	cheight = gtk_entry_get_text(GTK_ENTRY(text_height));

	width = atoi(cwidth);
	height = atoi(cheight);

	if (height == 0 || width == 0) {
		printf("something wrong with width and height\n");
		return;
	}

	nest_dxf(dxf_files, f_count, width, height);
}


void on_subitem_nest_click (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *nest_options_window, *buffer_width, *buffer_height; 
	GtkWidget *nest_button; 
	GtkWidget *v_box, *h_box1, *h_box2, *lbl_width, *lbl_height;
	
	nest_options_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	text_width = gtk_entry_new();
	text_height = gtk_entry_new();
	lbl_width = gtk_label_new("width: ");
	lbl_height = gtk_label_new("height:");
	nest_button = gtk_button_new_with_label("Nest");

	v_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	h_box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	h_box2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	
	
	gtk_box_pack_start(GTK_BOX(h_box1), lbl_width, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(h_box1), text_width, 0, 0, 0);

	gtk_box_pack_start(GTK_BOX(h_box2), lbl_height, 0, 0, 0);	
	gtk_box_pack_start(GTK_BOX(h_box2), text_height, 0, 0, 0);

	gtk_box_pack_start(GTK_BOX(v_box), h_box1, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(v_box), h_box2, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(v_box), nest_button, 0, 0, 0);

		
	gtk_container_add(GTK_CONTAINER(nest_options_window), v_box);
	
	g_signal_connect(nest_button, "clicked", G_CALLBACK(on_button_nest_click), NULL);

	gtk_window_set_default_size(GTK_WINDOW(nest_options_window), 320, 240);
	gtk_widget_show_all(nest_options_window);
}

void on_subitem_nest1_click (GtkWidget *widget, gpointer user_data)
{
	start_nfp_nesting(dxf_files, f_count, 800, 800);
}

void on_subitem_nest2_click (GtkWidget *widget, gpointer user_data)
{
    start_nfp_nesting_int(dxf_files_int, f_count, 800, 800);
}


int check_if_opened(char *path)
{
    int i;
    for (i = 0; i < f_count; i++) {
        if (strcmp(dxf_files[i].path, path) == 0)
            return 1;
    }

    return 0;
}

void on_subitem_open_click (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *ofd;
	GtkFileChooserAction action;
	GtkTreeIter iter;
    GtkFileFilter *filter;
	struct DxfFile dxf_file;
    struct DxfFileI dxf_file_int;
	int res;
	
	action = GTK_FILE_CHOOSER_ACTION_OPEN;
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.dxf");
    gtk_file_filter_set_name(filter, "DXF Files | *.dxf");
	ofd = gtk_file_chooser_dialog_new("Open File", NULL, action, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(ofd), filter);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(ofd), TRUE);
	res = gtk_dialog_run(GTK_DIALOG(ofd));

	if (res == GTK_RESPONSE_ACCEPT)	{
        GSList* filenames_list;	
		char *filename;
		filenames_list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(ofd));
        
        while (filenames_list != NULL) {
            filename = filenames_list->data;
            if (check_if_opened(filename)) {
                 gtk_widget_destroy(ofd);
                 filenames_list = filenames_list->next;
                 continue;
            }
        
	    	f_names[f_count] = filename;
         
	    	dxf_file_new(&dxf_file, filename, quant_files);
            dxf_file_new_int(&dxf_file_int, filename, quant_files);
	    	dxf_files[f_count] = dxf_file;
            dxf_files_int[f_count] = dxf_file_int;
	    	f_count++;
	    	if (f_count == max_files) {
    
	    		max_files *= 2;
	    		f_names = (char**)realloc(f_names ,sizeof(char*) * max_files);
		    	dxf_files = (struct DxfFile*)realloc(dxf_files ,sizeof(struct DxfFile) * max_files);
                dxf_files_int = (struct DxfFileI*)realloc(dxf_files_int, sizeof(struct DxfFileI) * max_files);
    
	    	}
	    	gtk_list_store_append(list_store, &iter);
    		gtk_list_store_set(list_store, &iter, COLUMN_ITEM_NUMBER, f_count, COLUMN_ITEM_PATH, f_names[f_count - 1], COLUMN_ITEM_QUANT, quant_files, -1);
            filenames_list = filenames_list->next;
        }
	}
	gtk_widget_destroy(ofd);
}

void on_selection_changed(GtkWidget *widget, gpointer user_data)
{
	int i;
	for (i = 0; i < f_count; i++)
		show_dxf_file(&dxf_files[i]);
}

void cell_edited_callback(GtkCellRendererText *cell, char *path_string, char *new_text, gpointer user_data)
{
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    int column, num, quant;
    char *old;

    model = GTK_TREE_MODEL(list_store);
    path = gtk_tree_path_new_from_string(path_string);
    column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), "column"));
    gtk_tree_model_get_iter(model, &iter, path);

    switch (column) {
        case COLUMN_ITEM_QUANT: 
            gtk_tree_model_get(model, &iter, COLUMN_ITEM_NUMBER, &num, -1);
            gtk_tree_model_get(model, &iter, column, &quant, -1);

            if (atoi(new_text) > 0) {
                gtk_list_store_set(list_store, &iter, column, atoi(new_text), -1);
                dxf_files[num - 1].how_many = atoi(new_text);
                dxf_files_int[num - 1].how_many = atoi(new_text);
            }

            break;
    }
}

int main (int argc, char *argv[])
{
	GtkWidget *menu, *menu_item, *submenu, *submenu_item1, *submenu_item2, *submenu_item3, *submenu_item4,*v_box, *scroll_window;

	GtkCellRenderer *renderer;
    	char *item1 = "file", *subitem1 = "open", *subitem2 = "rect nest", *subitem3 = "nfp nest double", *subitem4 = "nfp nest int";
	
	dxf_files = (struct DxfFile*)malloc(sizeof(struct DxfFile) * max_files);
    dxf_files_int = (struct DxfFileI*)malloc(sizeof(struct DxfFileI) * max_files);

	f_names = (char**)malloc(sizeof(char*) * max_files);
	
	gtk_init (&argc,&argv);	
	
	 
	
	//create window
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	//creating menu
	v_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
	gtk_container_add(GTK_CONTAINER(main_window), v_box);
	menu = gtk_menu_bar_new(); 
	submenu = gtk_menu_new();
	menu_item = gtk_menu_item_new_with_label(item1); 
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item); 
	
	submenu_item1 = gtk_menu_item_new_with_label(subitem1); 	
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), submenu_item1);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), submenu);
	g_signal_connect(submenu_item1, "activate", G_CALLBACK(on_subitem_open_click), NULL); //menu open signal click

	submenu_item2 = gtk_menu_item_new_with_label(subitem2); 	
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), submenu_item2);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), submenu);
	g_signal_connect(submenu_item2, "activate", G_CALLBACK(on_subitem_nest_click), NULL); //menu open signal click

	submenu_item3 = gtk_menu_item_new_with_label(subitem3); 	
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), submenu_item3);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), submenu);
	g_signal_connect(submenu_item3, "activate", G_CALLBACK(on_subitem_nest1_click), NULL); //menu open signal click
    
    submenu_item4 = gtk_menu_item_new_with_label(subitem4); 	
	gtk_menu_shell_append(GTK_MENU_SHELL(submenu), submenu_item4);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), submenu);
	g_signal_connect(submenu_item4, "activate", G_CALLBACK(on_subitem_nest2_click), NULL); //menu open signal click



	gtk_box_pack_start(GTK_BOX(v_box), menu, 0, 0, 0);
	
	//create list box
	scroll_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll_window), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	list_store = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT);
	flist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));	
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(flist), 1);
	gtk_tree_selection_set_mode (gtk_tree_view_get_selection(GTK_TREE_VIEW (flist)), GTK_SELECTION_SINGLE);

	renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(cell_edited_callback), NULL);
    g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(COLUMN_ITEM_NUMBER));
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(flist), -1, "Number", renderer, "text", COLUMN_ITEM_NUMBER, NULL);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(cell_edited_callback), NULL);
    g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(COLUMN_ITEM_PATH));
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(flist), -2, "File", renderer, "text", COLUMN_ITEM_PATH, NULL);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(cell_edited_callback), NULL);
    g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(COLUMN_ITEM_QUANT));
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(flist), -3, "Quantity", renderer, "text", COLUMN_ITEM_QUANT, NULL);

	gtk_box_pack_start(GTK_BOX(v_box), scroll_window, TRUE, TRUE, FALSE);
	gtk_container_add (GTK_CONTAINER (scroll_window), flist);
	
	//connect selection changed signal
	g_signal_connect(GTK_TREE_VIEW(flist), "row-activated", G_CALLBACK(on_selection_changed), NULL);
	
	gtk_window_set_default_size(GTK_WINDOW(main_window), 640, 480);	

	gtk_widget_show_all(main_window);
	 /* display main_window and children */


	g_signal_connect (main_window, "destroy", G_CALLBACK(on_window_destroy), NULL);
	 /* connect the main_window closing event with the "on_window_destroy" function */

	gtk_main ();
	 /* MAIN LOOP */

	return 0;
}
