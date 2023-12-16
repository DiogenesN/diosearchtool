#include <glib.h>
#include <gtk/gtk.h>

#ifndef EXTERNVARS_H_
#define EXTERNVARS_H_

	extern gdouble delta_x;
	extern gdouble delta_y;

	extern gchar **allStrings;
	extern const gchar *configName;
	extern gchar pathToConfig[777];

	extern gboolean isAppRunning;
	
	extern GPid			pid;
	extern GPid			pid2;
	extern GMenu	   *menu;
	extern GMenuModel *menuModel;

	extern GtkEntryBuffer *textBuff;
	extern GtkEntryBuffer *pathBuff;

	extern GtkStringList *iconsList;
	extern GtkStringList *namesList;

	extern GtkSingleSelection *singleSelection;

	extern GtkWidget *popPref;
	extern GtkWidget *menuPop;
	extern GtkWidget *errorBar;
	extern GtkWidget *openEntry;
	extern GtkWidget *openWithPop;
	extern GtkWidget *waitingPopup;
	extern GtkWidget *waitingLabel;
	extern GtkWidget *searchWindow;
	extern GtkWidget *openWithEntry;
	extern GtkWidget *searchResults;
	extern GtkWidget *regFilesSwitch;
	extern GtkWidget *searchTextEntry;
	extern GtkWidget *searchPathEntry;
	extern GtkWidget *openLocaionEntry;
	extern GtkWidget *inclSubdirsSwitch;
	extern GtkWidget *excludeHomeSwitch;
	extern GtkWidget *caseSensitiveSwitch;

#endif
