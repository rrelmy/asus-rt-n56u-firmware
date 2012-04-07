/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkentry.h>
#include <gtk/gtksignal.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

int outfd;
int err;

static void okpressed(void *widget, void *clientdata)
{
  GtkWidget *answer=clientdata;
  gchar *pass;
  int passlen;
  ssize_t wrote;
  (void)widget;

  pass=gtk_entry_get_text(GTK_ENTRY(answer));

  passlen=strlen(pass);
  if(!passlen)
    return;

  if((wrote=write(outfd, pass, passlen))!=passlen) {
    if(wrote<0)
      syslog(LOG_ERR, "write error on outpipe: %m");
    else
      syslog(LOG_ERR, "short write on outpipe");
    err=1;
  }
  gtk_main_quit();
}

int main(int argc, char **argv)
{
  GtkWidget *mainwindow, *vbox, *question, *answer, *ok;
  char buf[1024];
  gtk_init(&argc, &argv);

  openlog(argv[0], LOG_PID, LOG_DAEMON);
  if(argc!=4) {
    syslog(LOG_WARNING, "Usage error");
    return 1;
  }
  outfd=atoi(argv[3]);
  mainwindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(mainwindow), "pppgetpass");
  gtk_signal_connect(GTK_OBJECT(mainwindow), "destroy",
                     GTK_SIGNAL_FUNC(gtk_main_quit), 0);

  vbox=gtk_vbox_new(FALSE, 5);
  gtk_container_add(GTK_CONTAINER(mainwindow), vbox);
  gtk_widget_show(vbox);

  if(argv[1][0] && argv[2][0])
    snprintf(buf, sizeof buf, "Password for PPP client %s on server %s: ", argv[1], argv[2]);
  else if(argv[1][0] && !argv[2][0])
    snprintf(buf, sizeof buf, "Password for PPP client %s: ", argv[1]);
  else if(!argv[1][0] && argv[2][0])
    snprintf(buf, sizeof buf, "Password for PPP on server %s: ", argv[2]);
  else
    snprintf(buf, sizeof buf, "Enter PPP password: ");
  question=gtk_label_new(buf);
  gtk_box_pack_start(GTK_BOX(vbox), question, FALSE, TRUE, 0);
  gtk_widget_show(question);

  answer=gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(answer), 0);
  gtk_box_pack_start(GTK_BOX(vbox), answer, FALSE, TRUE, 0);
  gtk_widget_show(answer);

  ok=gtk_button_new_with_label("OK");
  gtk_box_pack_start(GTK_BOX(vbox), ok, FALSE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(ok), "clicked",
                     GTK_SIGNAL_FUNC(okpressed), answer);
  gtk_widget_show(ok);

  gtk_widget_show(mainwindow);
  gtk_main();

  return err;
}
