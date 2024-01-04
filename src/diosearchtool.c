/* DioSearchTool
 * Functionality:
	 	Search entry;
	 	Directory chooser;
	 	Search button;
	 	Option to search for regular files only (switch, if switch is off then look for both)
	 	Option to include subdirectories (switch, if switch if on then search recursively)
	 	Option to exclude HOME directory
	 	Option to search case sensitive
	 	Search result numbers counter
	 	Left click on the search result options:
	 		Open
	 		Open with
	 		Open file location
	 		Close
	 	Settings:
	 		Open default command: xdg-open
	 	Buttons:
	 		Close
	 		Open results in Text Editor
 * Defaults:
 		All switches 'off'
 		Open - opens automatically with xdg-open
 		Open with - shows an entry to enter the application namo (buttons: close, open)
 * Implementation:
 		After typing text and click 'Search' all the result go first to a file and then is displayed
 */

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "configsgen.c"
#include "externvars.h"
#include "formatstring.h"
#include "getvaluefromconf.h"
#include "getnumberoflines.h"
#include "populateallstrings.h"
#include "searchallinsensitive.h"

static void setup_listitem(GtkListItemFactory *factory, GtkListItem *list_item);
static void bind_listitem(GtkListItemFactory *factory, GtkListItem *list_item);

/// freeing resources
static void on_close() {
	g_print("Closing\n");
}

gboolean on_search_click_cb(gpointer listView) {
	// cleaning the text file
	FILE *output = fopen("/tmp/.diosearchtool", "w+");
		fprintf(output, "%s", "");
	fclose(output);

	// opening the file for append
	output = fopen("/tmp/.diosearchtool", "a");

	textBuff = gtk_entry_get_buffer(GTK_ENTRY(searchTextEntry));

	gint caseSens = gtk_switch_get_state(GTK_SWITCH(caseSensitiveSwitch));
	gint regularFilesOnly = gtk_switch_get_state(GTK_SWITCH(regFilesSwitch));
	gint includeSubdirs = gtk_switch_get_state(GTK_SWITCH(inclSubdirsSwitch));
	gint excludeHome = gtk_switch_get_state(GTK_SWITCH(excludeHomeSwitch));

	const gchar *stringToSearch = gtk_entry_buffer_get_text(textBuff);
	const gchar *stringPath = gtk_entry_buffer_get_text(pathBuff);

	g_print("search for: %s\nin path: %s\n", stringToSearch, stringPath);
	g_print("Case sensitive: %d\n", caseSens);
	g_print("Regular files: %d\n", regularFilesOnly);
	g_print("Include subdirectories: %d\n", includeSubdirs);
	g_print("Exclude HOME: %d\n", excludeHome);

	// search all files and directories depending on switch on/off
	search_all_insensitive(stringToSearch, stringPath, "/tmp/.diosearchtool", caseSens,
									regularFilesOnly, includeSubdirs, excludeHome, output);
	fclose(output);

	// populate all strings
	populate_allstrings("/tmp/.diosearchtool", &allStrings);
	gint nrOfLines = get_number_of_lines("/tmp/.diosearchtool");
	g_print("Number of lines: %d\n", nrOfLines);

	// if key word not found
	if (nrOfLines == 0) {
		g_print("'%s' not found.\n", stringToSearch);
		gchar *error = g_strdup_printf("'%s' not found!", stringToSearch);
		gtk_label_set_label(GTK_LABEL(errorBar), error);
		g_free(error);
		
		gtk_list_view_set_factory(GTK_LIST_VIEW(listView), NULL);
		gtk_list_view_set_model(GTK_LIST_VIEW(listView), NULL);

		// set search text highlighted in search text entry
		gtk_editable_select_region(GTK_EDITABLE(searchTextEntry), 0, -1);
		gtk_label_set_markup(GTK_LABEL(waitingLabel),
						"<span size=\"20pt\" weight=\"bold\">Not Found!</span>");

		return G_SOURCE_REMOVE | FALSE;
	}

	gchar *icons[nrOfLines];
	gchar *filenames[nrOfLines];

	gint jNumbers = 0;
	gint i = 0;

	gint loopCount = nrOfLines / 2;

	for (i = 0; i < loopCount; i++) {
		icons[i] = allStrings[jNumbers];
		filenames[i] = allStrings[jNumbers + 1];
		jNumbers = jNumbers + 2;
	}

	icons[i] = NULL;
	filenames[i] = NULL;

	iconsList = gtk_string_list_new((const char * const *)icons);
	namesList = gtk_string_list_new((const char * const *)filenames);

	g_print("\nEnd of strings\n\n");

	GListModel *listModel;
	listModel = G_LIST_MODEL(namesList);

	GtkListItemFactory *listItemFactory;
	listItemFactory = gtk_signal_list_item_factory_new();

	singleSelection = gtk_single_selection_new(listModel);

	GtkSelectionModel *selectionModel;
	selectionModel = GTK_SELECTION_MODEL(singleSelection);

	g_signal_connect(listItemFactory, "setup", G_CALLBACK(setup_listitem), NULL);
	g_signal_connect(listItemFactory, "bind", G_CALLBACK(bind_listitem), NULL);

	gtk_list_view_set_factory(GTK_LIST_VIEW(listView), listItemFactory);
	gtk_list_view_set_model(GTK_LIST_VIEW(listView), selectionModel);

	// set search text highlighted in search text entry
	gtk_editable_select_region(GTK_EDITABLE(searchTextEntry), 0, -1);

	// showing total number of files found
	gint nrOfElements = g_list_model_get_n_items(listModel);
	gchar *error = g_strdup_printf("%d files found!", nrOfElements);
	gtk_label_set_label(GTK_LABEL(errorBar), error);
	g_free(error);

	// free memory
	for (int i = 0; allStrings[i] != NULL; i++) {
		free(allStrings[i]);
	}
	free(allStrings);

	gtk_popover_popdown(GTK_POPOVER(waitingPopup));

	return G_SOURCE_REMOVE | FALSE;
}

/// on search button click
static void on_search_click(GtkWidget *listView) {
	// checks if nothing provided as a search term
	textBuff = gtk_entry_get_buffer(GTK_ENTRY(searchTextEntry));
	if (strcmp(gtk_entry_buffer_get_text(textBuff), "") == 0) {
		printf("no search term provided\n");
		gtk_label_set_label(GTK_LABEL(errorBar), "No search text provided!");
		return;
	}

	gtk_label_set_markup(GTK_LABEL(waitingLabel),
						"<span size=\"20pt\" weight=\"bold\">Please Wait...</span>");
	gtk_popover_popup(GTK_POPOVER(waitingPopup));
	gtk_label_set_label(GTK_LABEL(errorBar), "Searching...");
	gtk_list_view_set_factory(GTK_LIST_VIEW(listView), NULL);
	gtk_list_view_set_model(GTK_LIST_VIEW(listView), NULL);

 	g_timeout_add_seconds(1, on_search_click_cb, listView);
}

/// callbacks for open file/folder actions
static void open_cb() {
	gboolean result;
	GError *error = NULL;

	GtkStringObject *selectedLabel;
	selectedLabel = gtk_single_selection_get_selected_item(singleSelection);
	guint position = gtk_single_selection_get_selected(singleSelection);

	const gchar *selectedSearchText = gtk_string_object_get_string(selectedLabel);
	const gchar *iconType = gtk_string_list_get_string(iconsList, position);
	const gchar *openFile = get_char_value_from_conf(pathToConfig, "open");
	const gchar *fileManager = get_char_value_from_conf(pathToConfig, "filemanager");

	printf("position: %d\n", position);
	printf("selectedSearchText %s\n", selectedSearchText);
	printf("iconType: %s\n", iconType);

	gtk_widget_set_can_target(searchResults, TRUE);

	// if it's a folder
	if (strcmp(iconType, "folder") == 0) {
		// if no filemanager specified open preferences
		if (strcmp(fileManager, "") == 0) {
			gtk_popover_popup(GTK_POPOVER(popPref));
		}
		else {
			// opens directory
			char *openDirCMD[] = { (gchar *)fileManager, (gchar *)selectedSearchText, NULL };
			result = g_spawn_async(
						NULL,
						openDirCMD,
						NULL,
						G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
						NULL,
						NULL,
						&pid,
						&error);
			if (!result) {
				g_print("Error: %s\n", error->message);
				g_error_free(error);
			}
		}
	}
	// if it's a file
	else {
		// if no open file specified open preferences
		if (strcmp(openFile, "") == 0) {
			gtk_popover_popup(GTK_POPOVER(popPref));
		}
		else {
			// opens the file
			char *openFileCMD[] = { (gchar *)openFile, (gchar *)selectedSearchText, NULL };
			result = g_spawn_async(
						NULL,
						openFileCMD,
						NULL,
						G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
						NULL,
						NULL,
						&pid2,
						&error);
			if (!result) {
				g_print("Error: %s\n", error->message);
				g_error_free(error);
			}
		}
	}
}

/// open with action
static void open_with_cb() {
	gboolean result;
	GError *error = NULL;

	GtkStringObject *selectedLabel;
	selectedLabel = gtk_single_selection_get_selected_item(singleSelection);
	guint position = gtk_single_selection_get_selected(singleSelection);

	GtkEntryBuffer *openWithBuff;
	openWithBuff = gtk_entry_get_buffer(GTK_ENTRY(openWithEntry));

	const gchar *appName = gtk_entry_buffer_get_text(openWithBuff);
	const gchar *selectedSearchText = gtk_string_object_get_string(selectedLabel);
	const gchar *iconType = gtk_string_list_get_string(iconsList, position);

	printf("position: %d\n", position);
	printf("selectedSearchText %s\n", selectedSearchText);
	printf("iconType: %s\n", iconType);
	printf("appName: %s\n", appName);

	gtk_widget_set_can_target(searchResults, TRUE);

	// opens the file
	char *openFileCMD[] = { (gchar *)appName, (gchar *)selectedSearchText, NULL };
	result = g_spawn_async(
						NULL,
						openFileCMD,
						NULL,
						G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
						NULL,
						NULL,
						&pid,
						&error);
	if (!result) {
		g_print("Error: %s\n", error->message);
		g_error_free(error);
	}
}
static void openwith_cb() {
	gtk_popover_popup(GTK_POPOVER(openWithPop));
}

// open file location action
static void filelocation_cb() {
	gchar fullPathToSelDir[2048];
	memset(fullPathToSelDir, 0, sizeof(fullPathToSelDir));

	gboolean result;
	GError *error = NULL;

	GtkStringObject *selectedLabel;
	selectedLabel = gtk_single_selection_get_selected_item(singleSelection);
	guint position = gtk_single_selection_get_selected(singleSelection);

	const gchar *fileManager = get_char_value_from_conf(pathToConfig, "filemanager");
	const gchar *selectedSearchText = gtk_string_object_get_string(selectedLabel);
	strncpy(fullPathToSelDir, selectedSearchText, strlen(selectedSearchText) + 1);
	const gchar *iconType = gtk_string_list_get_string(iconsList, position);

	printf("position: %d\n", position);
	printf("directory %s\n", fullPathToSelDir);
	printf("iconType: %s\n", iconType);

	gtk_widget_set_can_target(searchResults, TRUE);

	// if it's a folder
	if (strcmp(iconType, "folder") == 0) {
		// if no filemanager specified open preferences
		if (strcmp(fileManager, "") == 0) {
			gtk_popover_popup(GTK_POPOVER(popPref));
		}
		else {
			// opens directory
			char *openDirCMD[] = { (gchar *)fileManager, (gchar *)selectedSearchText, NULL };
			result = g_spawn_async(
						NULL,
						openDirCMD,
						NULL,
						G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
						NULL,
						NULL,
						&pid,
						&error);
			if (!result) {
				g_print("Error: %s\n", error->message);
				g_error_free(error);
			}
		}
	}
	// if it's a file
	else {
		// if no open file specified open preferences
		if (strcmp(fileManager, "") == 0) {
			gtk_popover_popup(GTK_POPOVER(popPref));
		}
		else {

			// If it's a file then get the path. Find the last occurrence of '/' (last slash)
			char *lastSlash = strrchr(fullPathToSelDir, '/');

			// Null-terminate the string at the last slash
			if (lastSlash != NULL) {
				*lastSlash = '\0';

				strcat((char *)fullPathToSelDir, "/");
			}

			// opens the file path
			char *openFileCMD[] = { (gchar *)fileManager, (gchar *)fullPathToSelDir, NULL };
			result = g_spawn_async(
						NULL,
						openFileCMD,
						NULL,
						G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
						NULL,
						NULL,
						&pid2,
						&error);
			if (!result) {
				g_print("Error: %s\n", error->message);
				g_error_free(error);
			}
		}
	}
}

// close popup menu action
static void close_cb() {
	gtk_popover_popdown(GTK_POPOVER(menuPop));
	gtk_widget_set_can_target(searchResults, TRUE);
}

/// actions structure
const GActionEntry app_actions[] = {
	{
		.name = "open",
		.activate = open_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	},
	{
		.name = "openwith",
		.activate = openwith_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	},
	{
		.name = "filelocation",
		.activate = filelocation_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	},
	{
		.name = "close",
		.activate = close_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	}
};

/// get mouse position deltas axes x and y
static void get_axes(GtkEventController *event) {
	// get mouse location
	GdkEvent *eventControllerX;
	eventControllerX = gtk_event_controller_get_current_event(event);
	gdk_event_get_axis(eventControllerX, GDK_AXIS_X, &delta_x);

	GdkEvent *eventControllerY;
	eventControllerY = gtk_event_controller_get_current_event(event);
	gdk_event_get_axis(eventControllerY, GDK_AXIS_Y, &delta_y);
}

/// clicking on a search item
static void left_click_menu_item() {
	gtk_popover_set_offset(GTK_POPOVER(menuPop), delta_x - 444, delta_y - 915);
	gtk_popover_popup(GTK_POPOVER(menuPop));
	gtk_widget_set_can_target(searchResults, FALSE);
}

/// creating labels to fill up the listview
static void setup_listitem(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory;

	GtkWidget *icon;
	icon = gtk_image_new();
	gtk_image_set_icon_size(GTK_IMAGE(icon), GTK_ICON_SIZE_NORMAL);

	GtkWidget *labelName;
	labelName = gtk_label_new(NULL);
	//gtk_label_set_selectable(GTK_LABEL(labelName), TRUE);

	// defining the size of text in label
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(17 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoLayout *layout = gtk_widget_create_pango_layout(labelName, NULL);
	pango_layout_set_attributes(layout, attr_list);
	gtk_label_set_attributes(GTK_LABEL(labelName), attr_list);

	GtkWidget *box;
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_prepend(GTK_BOX(box), icon);
	gtk_box_append(GTK_BOX(box), labelName);

	gtk_list_item_set_child(list_item, box);

	// freing resources
	g_object_unref(layout);
	//pango_attribute_destroy(attr);
	pango_attr_list_unref(attr_list);
}

/// binding the items to the list, this function runs in a loop
static void bind_listitem(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory; // Marking the parameter as unused

	// Get the GtkBox widget
	GtkWidget *box;
	box = gtk_list_item_get_child(list_item);

	// Find the GtkImage widget within the GtkBox
	GtkWidget *icon;
	icon = gtk_widget_get_first_child(box);

	// Find the next child after GtkImage whoch is a label 
	GtkWidget *labelName;
	labelName = gtk_widget_get_next_sibling(icon);

	// get searching term from entry
	textBuff = gtk_entry_get_buffer(GTK_ENTRY(searchTextEntry));

	// gets the strings
	guint listitemPos = gtk_list_item_get_position(list_item);
	const gchar *s_icons = gtk_string_list_get_string(iconsList, listitemPos);
	//const gchar *s_names = gtk_string_list_get_string(namesList, listitemPos);
	// g_markup_escape_text is extremely important to escape ampersand or it breaks search results
	const gchar *s_names = g_markup_escape_text(gtk_string_list_get_string(namesList, listitemPos), -1);

	// check if strings aren't NULL otherwise it will crash
	if (s_icons != NULL && s_names != NULL) {
		// set markup style to highlight search term
		const gchar *stFormat = "<span size='13pt' weight='bold' bgcolor='#A4EA00' foreground='red'>";
		const gchar *endFormat = "</span>";
		const gchar *key = gtk_entry_buffer_get_text(textBuff);
		const gchar *resultName = format_string(s_names, key, stFormat, endFormat);

		// sets number, icon and name label
		gtk_image_set_from_icon_name(GTK_IMAGE(icon), s_icons);
		gtk_label_set_markup(GTK_LABEL(labelName), resultName);
		g_free((void *)resultName);
		g_free((void *)s_icons);
		g_free((void *)s_names);
	}
	else {
		// NULL string reached
		g_print("NULL string\n");
		return;
	}
}

/// on key press popover
static void popdown_cb() {
	gtk_popover_popdown(GTK_POPOVER(waitingPopup));
	gtk_entry_buffer_delete_text(textBuff, 0, -1);
}

/// close/save preferences
static void save_close_pref(GtkWidget *poppref) {
	gtk_popover_popdown(GTK_POPOVER(poppref));
	gtk_widget_set_can_target(searchResults, TRUE);

	GtkEntryBuffer *openBuff;
	openBuff = gtk_entry_get_buffer(GTK_ENTRY(openEntry));

	GtkEntryBuffer *filemanagerBuff;
	filemanagerBuff = gtk_entry_get_buffer(GTK_ENTRY(openLocaionEntry));

	const gchar *openText = gtk_entry_buffer_get_text(openBuff);
	const gchar *filemanagerText = gtk_entry_buffer_get_text(filemanagerBuff);

	FILE *config = fopen(pathToConfig, "w+");

	fprintf(config, "open=%s\n", openText);
	fprintf(config, "filemanager=%s\n", filemanagerText);

	fclose(config);
}

/// opening preferences popup make list view can't target
static void can_target_no() {
	gtk_widget_set_can_target(searchResults, FALSE);
}

/// only one instance running
static void activate() {
	if (isAppRunning) {
		g_print("Already running\n");
		return;
	}
	else {
		g_print("Welcome to Dio Search Tool!\n");
	}
	isAppRunning = TRUE;
}

/// startup signal handle
static void startup(GtkApplication *app) {
	// building menu items
	// Create items in the menu
	GMenuItem *item1 = g_menu_item_new("Open", "app.open");
	GMenuItem *item2 = g_menu_item_new("Open with", "app.openwith");
	GMenuItem *item3 = g_menu_item_new("Open file location", "app.filelocation");
	GMenuItem *item4 = g_menu_item_new("Close", "app.close");

	// menu model Append items to the menu
	menu = g_menu_new();
	g_menu_append_item(menu, item1);
	g_menu_append_item(menu, item2);
	g_menu_append_item(menu, item3);
	g_menu_append_item(menu, item4);

	// GMenuModel *menuModel Create a GMenuModel from the GMenu
	menuModel = G_MENU_MODEL(menu);

	////////////////////////////////////// widgets ////////////////////////////////////////////
	searchWindow = gtk_application_window_new(GTK_APPLICATION(app));
	gtk_window_set_title(GTK_WINDOW(searchWindow), "Dio Search Tool");
	gtk_window_set_icon_name(GTK_WINDOW(searchWindow), "system-search-symbolic");
	g_signal_connect_swapped(searchWindow, "close-request", G_CALLBACK(on_close), searchWindow);

	// popup menu when clicking on a directory in directory browser
	menuPop = gtk_popover_menu_new_from_model(menuModel);
	gtk_popover_set_has_arrow(GTK_POPOVER(menuPop), FALSE);
	//gtk_widget_set_size_request(menuPop, 290, 100);
	//gtk_popover_set_offset(GTK_POPOVER(menuPop), 0, -500);

	GtkWidget *labelSearch;
	labelSearch = gtk_label_new("Search Text");

	GtkWidget *labelPath;
	labelPath = gtk_label_new("Search Path");

	searchTextEntry = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(searchTextEntry), "Type text to search");
	gtk_widget_set_halign(searchTextEntry, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(searchTextEntry, GTK_ALIGN_CENTER);
	gtk_widget_set_size_request(searchTextEntry, 500, 0);

	pathBuff = gtk_entry_buffer_new(getenv("PWD"), -1);

	searchPathEntry = gtk_entry_new_with_buffer(pathBuff);
	gtk_widget_set_halign(searchPathEntry, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(searchPathEntry, GTK_ALIGN_CENTER);
	gtk_widget_set_size_request(searchPathEntry, 500, 0);

	GtkWidget *labelRegFiles;
	labelRegFiles = gtk_label_new("Regular Files Only");
	gtk_widget_set_halign(labelRegFiles, GTK_ALIGN_START);
	gtk_widget_set_hexpand(labelRegFiles, FALSE);

	GtkWidget *labelInclSubdirs;
	labelInclSubdirs = gtk_label_new("Include Subdirectories");
	gtk_widget_set_halign(labelInclSubdirs, GTK_ALIGN_START);
	gtk_widget_set_hexpand(labelInclSubdirs, FALSE);

	GtkWidget *labelExcludeHome;
	labelExcludeHome = gtk_label_new("Exclude HOME");
	gtk_widget_set_halign(labelExcludeHome, GTK_ALIGN_START);
	gtk_widget_set_hexpand(labelExcludeHome, FALSE);

	GtkWidget *labelCaseSensitive;
	labelCaseSensitive = gtk_label_new("Case sensitive");
	gtk_widget_set_halign(labelCaseSensitive, GTK_ALIGN_START);
	gtk_widget_set_hexpand(labelCaseSensitive, FALSE);

	caseSensitiveSwitch = gtk_switch_new();
	gtk_widget_set_hexpand(caseSensitiveSwitch, FALSE);
	gtk_widget_set_halign(caseSensitiveSwitch, GTK_ALIGN_END);

	regFilesSwitch = gtk_switch_new();
	gtk_widget_set_hexpand(regFilesSwitch, FALSE);
	gtk_widget_set_halign(regFilesSwitch, GTK_ALIGN_END);

	inclSubdirsSwitch = gtk_switch_new();
	gtk_widget_set_hexpand(inclSubdirsSwitch, FALSE);
	gtk_widget_set_halign(inclSubdirsSwitch, GTK_ALIGN_END);

	excludeHomeSwitch = gtk_switch_new();
	gtk_widget_set_hexpand(excludeHomeSwitch, FALSE);
	gtk_widget_set_halign(excludeHomeSwitch, GTK_ALIGN_END);

	GtkWidget *searchButton;
	searchButton = gtk_button_new_with_label("Search");
	gtk_widget_set_halign(searchButton, GTK_ALIGN_CENTER);
	gtk_widget_set_size_request(searchButton, 300, 0);

	errorBar = gtk_label_new(NULL);
	gtk_widget_set_valign(errorBar, GTK_ALIGN_CENTER);
	gtk_widget_set_halign(errorBar, GTK_ALIGN_CENTER);

	// waiting popup label
	waitingLabel = gtk_label_new(NULL);

	// waiting popup dialog
	waitingPopup = gtk_popover_new();
	gtk_popover_set_offset(GTK_POPOVER(waitingPopup), 0, -400);
	gtk_popover_set_has_arrow(GTK_POPOVER(waitingPopup), FALSE);
	gtk_popover_set_child(GTK_POPOVER(waitingPopup), waitingLabel);

	// adding key event for waitingPopup so it's closed on any key activity
	GtkEventController *keyController;
	keyController = gtk_event_controller_key_new();
	gtk_widget_add_controller(waitingPopup, keyController);
	g_signal_connect_swapped(keyController, "key-pressed", G_CALLBACK(popdown_cb), NULL);

	// motion controller for list view to get mouse position
	GtkEventController *motionHoverEvent;
	motionHoverEvent = gtk_event_controller_motion_new();

	// list view that holds the results
	searchResults = gtk_list_view_new(NULL, NULL);
	gtk_widget_add_controller(searchResults, motionHoverEvent);
	gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(searchResults), TRUE);
	g_signal_connect(searchResults, "activate", G_CALLBACK(left_click_menu_item), NULL);
	g_signal_connect_swapped(searchTextEntry, "activate", G_CALLBACK(on_search_click), searchResults);
	g_signal_connect_swapped(searchPathEntry, "activate", G_CALLBACK(on_search_click), searchResults);
	g_signal_connect_swapped(searchButton, "clicked", G_CALLBACK(on_search_click), searchResults);
	g_signal_connect_swapped(motionHoverEvent, "motion", G_CALLBACK(get_axes), motionHoverEvent);

	GtkWidget *searchScrolled;
	searchScrolled = gtk_scrolled_window_new();
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(searchScrolled), searchResults);
	gtk_widget_set_size_request(searchScrolled, 0, 700);

	GtkWidget *closeButton;
	closeButton = gtk_button_new_with_label("Close");
	gtk_widget_set_valign(closeButton, GTK_ALIGN_END);
	gtk_widget_set_halign(closeButton, GTK_ALIGN_START);
	g_signal_connect_swapped(closeButton, "clicked", G_CALLBACK(gtk_window_close), searchWindow);

	// preferences window
	const gchar *openFile = get_char_value_from_conf(pathToConfig, "open");
	const gchar *fileManager = get_char_value_from_conf(pathToConfig, "filemanager");

	GtkWidget *openLabel;
	openLabel = gtk_label_new("Default open:");

	openEntry = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(openEntry), "e.g. xdg-open");
	gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(openEntry)), openFile, -1);

	GtkWidget *openLocaionLabel;
	openLocaionLabel = gtk_label_new("Default file manager:");

	openLocaionEntry = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(openLocaionEntry), "e.g. thunar");
	gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(openLocaionEntry)), fileManager, -1);

	GtkWidget *closeSave;
	closeSave = gtk_button_new_with_label("Close/Save");
	gtk_widget_set_valign(closeSave, GTK_ALIGN_CENTER);
	gtk_widget_set_halign(closeSave, GTK_ALIGN_CENTER);

	GtkWidget *prefBox;
	prefBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(prefBox), TRUE);
	gtk_box_prepend(GTK_BOX(prefBox), openLabel);
	gtk_box_append(GTK_BOX(prefBox), openLocaionLabel);

	GtkWidget *prefBox2;
	prefBox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(prefBox2), TRUE);
	gtk_box_prepend(GTK_BOX(prefBox2), openEntry);
	gtk_box_append(GTK_BOX(prefBox2), openLocaionEntry);

	GtkWidget *prefBoxes;
	prefBoxes = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(prefBoxes), TRUE);
	gtk_box_prepend(GTK_BOX(prefBoxes), prefBox);
	gtk_box_append(GTK_BOX(prefBoxes), prefBox2);

	GtkWidget *prefBoxesVer;
	prefBoxesVer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(prefBoxesVer), TRUE);
	gtk_box_prepend(GTK_BOX(prefBoxesVer), prefBoxes);
	gtk_box_append(GTK_BOX(prefBoxesVer), closeSave);

	/////////////////////////// popover for preferences /////////////////////////////////////
	popPref = gtk_popover_new();
	gtk_popover_set_child(GTK_POPOVER(popPref), prefBoxesVer);
	gtk_popover_set_has_arrow(GTK_POPOVER(popPref), FALSE);
	gtk_popover_set_autohide(GTK_POPOVER(popPref), FALSE);
	gtk_popover_set_offset(GTK_POPOVER(popPref), 0, -400);
	g_signal_connect_swapped(popPref, "closed", G_CALLBACK(gtk_widget_set_can_target), searchResults);
	g_signal_connect_swapped(closeSave, "clicked", G_CALLBACK(save_close_pref), popPref);
	g_signal_connect_swapped(popPref, "show", G_CALLBACK(can_target_no), NULL);

	GtkWidget *prefButton;
	prefButton = gtk_button_new_with_label("Preferences");
	gtk_widget_set_valign(prefButton, GTK_ALIGN_CENTER);
	gtk_widget_set_halign(prefButton, GTK_ALIGN_END);
	g_signal_connect_swapped(prefButton, "clicked", G_CALLBACK(gtk_popover_popup), popPref);

	/////////////////////////// popover for open with /////////////////////////////////////
	openWithEntry = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(openWithEntry), "Type application name");

	GtkWidget *openWithBtn;
	openWithBtn = gtk_button_new_with_label("Open");
	gtk_widget_set_valign(openWithBtn, GTK_ALIGN_CENTER);
	gtk_widget_set_halign(openWithBtn, GTK_ALIGN_CENTER);

	GtkWidget *openWithBox;
	openWithBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(openWithBox), TRUE);
	gtk_box_prepend(GTK_BOX(openWithBox), openWithEntry);
	gtk_box_append(GTK_BOX(openWithBox), openWithBtn);

	openWithPop = gtk_popover_new();
	gtk_popover_set_child(GTK_POPOVER(openWithPop), openWithBox);
	gtk_popover_set_has_arrow(GTK_POPOVER(openWithPop), FALSE);
	gtk_popover_set_autohide(GTK_POPOVER(openWithPop), FALSE);
	gtk_popover_set_offset(GTK_POPOVER(openWithPop), 0, -400);
	g_signal_connect_swapped(openWithPop, "closed", G_CALLBACK(open_with_cb), NULL);
	g_signal_connect_swapped(openWithBtn, "clicked", G_CALLBACK(gtk_popover_popdown), openWithPop);

	///////////////////////////////////// boxes ////////////////////////////////////////////////
	// label box
	GtkWidget *labelBox;
	labelBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(labelBox), TRUE);
	gtk_box_prepend(GTK_BOX(labelBox), labelSearch);
	gtk_box_append(GTK_BOX(labelBox), labelPath);
	gtk_widget_set_halign(labelBox, GTK_ALIGN_START);

	// entries box
	GtkWidget *entriesBox;
	entriesBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(entriesBox), TRUE);
	gtk_box_prepend(GTK_BOX(entriesBox), searchTextEntry);
	gtk_box_append(GTK_BOX(entriesBox), searchPathEntry);
	gtk_widget_set_halign(entriesBox, GTK_ALIGN_CENTER);

	// switches labels box
	GtkWidget *switchesLabelsBox;
	switchesLabelsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(switchesLabelsBox), TRUE);
	gtk_box_prepend(GTK_BOX(switchesLabelsBox), labelCaseSensitive);
	gtk_box_append(GTK_BOX(switchesLabelsBox), labelRegFiles);
	gtk_box_append(GTK_BOX(switchesLabelsBox), labelInclSubdirs);
	gtk_box_append(GTK_BOX(switchesLabelsBox), labelExcludeHome);
	gtk_widget_set_halign(switchesLabelsBox, GTK_ALIGN_END);

	// switches box
	GtkWidget *switchesBox;
	switchesBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(switchesBox), TRUE);
	gtk_box_prepend(GTK_BOX(switchesBox), caseSensitiveSwitch);
	gtk_box_append(GTK_BOX(switchesBox), regFilesSwitch);
	gtk_box_append(GTK_BOX(switchesBox), inclSubdirsSwitch);
	gtk_box_append(GTK_BOX(switchesBox), excludeHomeSwitch);
	gtk_widget_set_halign(switchesBox, GTK_ALIGN_END);

	// labels, entries, switches labels, switches boxes
	GtkWidget *upperBoxes;
	upperBoxes = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
	gtk_box_prepend(GTK_BOX(upperBoxes), labelBox);
	gtk_box_append(GTK_BOX(upperBoxes), entriesBox);
	gtk_box_append(GTK_BOX(upperBoxes), switchesLabelsBox);
	gtk_box_append(GTK_BOX(upperBoxes), switchesBox);
	gtk_widget_set_halign(upperBoxes, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(upperBoxes, GTK_ALIGN_CENTER);
	gtk_widget_set_margin_start(upperBoxes, 20);
	gtk_widget_set_margin_end(upperBoxes, 20);
	gtk_widget_set_margin_top(upperBoxes, 10);
	gtk_widget_set_margin_bottom(upperBoxes, 10);

	// previous boxes and search button
	GtkWidget *prevBoxes;
	prevBoxes = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_prepend(GTK_BOX(prevBoxes), upperBoxes);
	gtk_box_append(GTK_BOX(prevBoxes), searchButton);
	gtk_box_append(GTK_BOX(prevBoxes), errorBar);
	gtk_box_append(GTK_BOX(prevBoxes), menuPop);
	gtk_box_append(GTK_BOX(prevBoxes), waitingPopup);
	gtk_box_append(GTK_BOX(prevBoxes), popPref);
	gtk_box_append(GTK_BOX(prevBoxes), openWithPop);
	gtk_box_append(GTK_BOX(prevBoxes), searchScrolled);

	// buttons box
	GtkWidget *buttons_box;
	buttons_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(buttons_box), TRUE);
	gtk_box_prepend(GTK_BOX(buttons_box), closeButton);
	gtk_box_append(GTK_BOX(buttons_box), prefButton);
	gtk_widget_set_margin_start(buttons_box, 20);
	gtk_widget_set_margin_end(buttons_box, 20);
	gtk_widget_set_margin_top(buttons_box, 10);
	gtk_widget_set_margin_bottom(buttons_box, 10);

	// all boxes
	GtkWidget *allBoxes;
	allBoxes = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
	gtk_paned_set_start_child(GTK_PANED(allBoxes), prevBoxes);
	gtk_paned_set_end_child(GTK_PANED(allBoxes), buttons_box);
	gtk_paned_set_resize_start_child(GTK_PANED(allBoxes), FALSE);
	gtk_paned_set_shrink_start_child(GTK_PANED(allBoxes), FALSE);
	gtk_paned_set_resize_end_child(GTK_PANED(allBoxes), FALSE);
	gtk_paned_set_shrink_end_child(GTK_PANED(allBoxes), FALSE);

	// free resources
	g_object_unref(menu);
	g_object_unref(item1);
	g_object_unref(item2);
	g_object_unref(item3);
	g_object_unref(item4);

	gtk_window_set_child(GTK_WINDOW(searchWindow), allBoxes);
	gtk_window_present(GTK_WINDOW(searchWindow));
}

int main() {
	// create initial configs
	create_configs();

	// setting config file
	snprintf((char *)pathToConfig, sizeof(pathToConfig), "%s%s", getenv("HOME"), configName);

	gint status;

	GtkApplication *app;
	app = gtk_application_new("com.github.DiogenesN.diosearchtool", G_APPLICATION_DEFAULT_FLAGS);
	g_action_map_add_action_entries(G_ACTION_MAP(app), app_actions, G_N_ELEMENTS(app_actions), app);

	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	g_signal_connect(app, "startup", G_CALLBACK(startup), NULL);

	status = g_application_run(G_APPLICATION(app), 0, NULL);

	g_spawn_close_pid(pid);
	g_spawn_close_pid(pid2);
	g_object_unref(app);
	return status;
}
