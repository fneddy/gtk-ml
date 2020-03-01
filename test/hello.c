#include <gtk/gtk.h>
#include "gtk-ml.h"

static char *GUI = " \
  (Application \"de.walterpi.example\" flags-none { \
    :activate (lambda (app) \
      (new-window app \"gtk-ml example\" 640 480))})";

int main() {
    const char *err;

    GtkMl_Context *ctx = gtk_ml_new_context();

    GtkMl_S *gui;
    if (!(gui = gtk_ml_loads(ctx, &err, GUI))) {
        gtk_ml_del_context(ctx);
        fprintf(stderr, "%s\n", err);
        return 1;
    }

    gtk_ml_push(ctx, gui);

    GtkMl_S *app;
    if (!(app = gtk_ml_exec(ctx, &err, gui))) {
        gtk_ml_del_context(ctx);
        fprintf(stderr, "%s\n", err);
        return 1;
    }

    gtk_ml_push(ctx, app);

    int status = g_application_run(G_APPLICATION(app->value.s_userdata.userdata), 0, NULL);

    gtk_ml_del_context(ctx);

    return status;
}