#include <glib.h>
#include <gtk/gtk.h>

gdouble delta_x;
gdouble delta_y;

gchar pathToConfig[777];
gchar **allStrings = NULL;
const gchar	*configName = "/.config/diosearchtool/diosearchtool.conf";

gboolean isAppRunning = FALSE;

GPid		pid;
GPid		pid2;
GMenu	   *menu;
GMenuModel *menuModel;

GtkEntryBuffer *textBuff;
GtkEntryBuffer *pathBuff;

GtkStringList *iconsList;
GtkStringList *namesList;

GtkSingleSelection *singleSelection;

GtkWidget *popPref;
GtkWidget *menuPop;
GtkWidget *errorBar;
GtkWidget *openEntry;
GtkWidget *openWithPop;
GtkWidget *searchWindow;
GtkWidget *waitingPopup;
GtkWidget *waitingLabel;
GtkWidget *searchResults;
GtkWidget *openWithEntry;
GtkWidget *regFilesSwitch;
GtkWidget *searchTextEntry;
GtkWidget *searchPathEntry;
GtkWidget *openLocaionEntry;
GtkWidget *inclSubdirsSwitch;
GtkWidget *excludeHomeSwitch;
GtkWidget *caseSensitiveSwitch;
