#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>


static void
make_window( Display *dpy, const char *name,
             int x, int y, int width, int height,
             Window *winRet, GLXContext *ctxRet, VisualID *visRet)
{
   int attribs[64];
   int i = 0;

   int scrnum;
   XSetWindowAttributes attr;
   unsigned long mask;
   Window root;
   Window win;
   GLXContext ctx;
   XVisualInfo *visinfo;

   /* Singleton attributes. */
   attribs[i++] = GLX_RGBA;
   attribs[i++] = GLX_DOUBLEBUFFER;

   /* Key/value attributes. */
   attribs[i++] = GLX_RED_SIZE;
   attribs[i++] = 1;
   attribs[i++] = GLX_GREEN_SIZE;
   attribs[i++] = 1;
   attribs[i++] = GLX_BLUE_SIZE;
   attribs[i++] = 1;
   attribs[i++] = GLX_DEPTH_SIZE;
   attribs[i++] = 1;

   attribs[i++] = None;

   scrnum = DefaultScreen( dpy );
   root = RootWindow( dpy, scrnum );

   visinfo = glXChooseVisual(dpy, scrnum, attribs);
   if (!visinfo) {
      printf("Error: couldn't get an RGB, Double-buffered visual\n");
      exit(1);
   }

   /* window attributes */
   attr.background_pixel = 0;
   attr.border_pixel = 0;
   attr.colormap = XCreateColormap( dpy, root, visinfo->visual, AllocNone);
   attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
   /* XXX this is a bad way to get a borderless window! */
   mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

   win = XCreateWindow( dpy, root, x, y, width, height,
		        0, visinfo->depth, InputOutput,
		        visinfo->visual, mask, &attr );

   /* set hints and properties */
   {
      XSizeHints sizehints;
      sizehints.x = x;
      sizehints.y = y;
      sizehints.width  = width;
      sizehints.height = height;
      sizehints.flags = USSize | USPosition;
      XSetNormalHints(dpy, win, &sizehints);
      XSetStandardProperties(dpy, win, name, name,
                              None, (char **)NULL, 0, &sizehints);
   }

   ctx = glXCreateContext( dpy, visinfo, NULL, True );
   if (!ctx) {
      printf("Error: glXCreateContext failed\n");
      exit(1);
   }

   *winRet = win;
   *ctxRet = ctx;
   *visRet = visinfo->visualid;

   XFree(visinfo);
}

#define NOP 0
#define EXIT 1
#define DRAW 2

static int
handle_event(Display *dpy, Window win, XEvent *event)
{
   (void) dpy;
   (void) win;

   switch (event->type) {
   case Expose:
      return DRAW;
   case KeyPress:
      {
         char buffer[10];
         int code;
         code = XLookupKeysym(&event->xkey, 0);

        XLookupString(&event->xkey, buffer, sizeof(buffer),
                        NULL, NULL);
        if (buffer[0] == 27) {
            /* escape */
            return EXIT;
        }
         return DRAW;
      }
   }
   return NOP;
}
static void
event_loop(Display *dpy, Window win)
{
   while (1) {
      int op;

      while (XPending(dpy) > 0) {
         XEvent event;
         XNextEvent(dpy, &event);
         op = handle_event(dpy, win, &event);
         if (op == EXIT)
            return;
         else if (op == DRAW)
            break;
      }
  }      
}


int
main(int argc, char *argv[])
{
   unsigned int winWidth = 300, winHeight = 300;
   int x = 0, y = 0;
   Display *dpy;
   Window win;
   GLXContext ctx;
   char *dpyName = NULL;
   VisualID visId;
   dpy = XOpenDisplay(dpyName);
   if (!dpy) {
      printf("Error: couldn't open display %s\n",
	     dpyName ? dpyName : getenv("DISPLAY"));
      return -1;
   }

   make_window(dpy, "glxgears", x, y, winWidth, winHeight, &win, &ctx, &visId);
   XMapWindow(dpy, win);
   glXMakeCurrent(dpy, win, ctx);

   event_loop(dpy, win);

   glXMakeCurrent(dpy, None, NULL);
   glXDestroyContext(dpy, ctx);
   XDestroyWindow(dpy, win);
   XCloseDisplay(dpy);

   return 0;
}