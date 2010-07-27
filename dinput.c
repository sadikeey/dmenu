/* See LICENSE file for copyright and license details. */
#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "dmenu.h"

/* forward declarations */
static void cleanup(void);

/* variables */
static size_t cursor = 0;

void
cleanup(void) {
	cleanupdraw(&dc);
	XDestroyWindow(dpy, win);
	XUngrabKeyboard(dpy, CurrentTime);
	XCloseDisplay(dpy);
}

void
drawbar(void)
{
	dc.x = 0;
	dc.y = 0;
	dc.w = mw;
	dc.h = mh;
	drawbox(&dc, normcol);
	/* print prompt? */
	if(prompt) {
		dc.w = promptw;
		drawtext(&dc, prompt, selcol);
		dc.x += dc.w;
	}
	dc.w = mw - dc.x;
	drawtext(&dc, text, normcol);
	drawcursor(&dc, text, cursor, normcol);
	commitdraw(&dc, win);
}

void
kpress(XKeyEvent *e) {
	char buf[sizeof text];
	int num;
	unsigned int i, len;
	KeySym ksym;

	len = strlen(text);
	num = XLookupString(e, buf, sizeof buf, &ksym, NULL);
	if(ksym == XK_KP_Enter)
		ksym = XK_Return;
	else if(ksym >= XK_KP_0 && ksym <= XK_KP_9)
		ksym = (ksym - XK_KP_0) + XK_0;
	else if(IsFunctionKey(ksym) || IsKeypadKey(ksym)
	|| IsMiscFunctionKey(ksym) || IsPFKey(ksym)
	|| IsPrivateKeypadKey(ksym))
		return;
	/* first check if a control mask is omitted */
	if(e->state & ControlMask) {
		switch(tolower(ksym)) {
		default:
			return;
		case XK_a:
			ksym = XK_Home;
			break;
		case XK_b:
			ksym = XK_Left;
			break;
		case XK_c:
			ksym = XK_Escape;
			break;
		case XK_e:
			ksym = XK_End;
			break;
		case XK_f:
			ksym = XK_Right;
			break;
		case XK_h:
			ksym = XK_BackSpace;
			break;
		case XK_j:
		case XK_m:
			ksym = XK_Return;
			break;
		case XK_k:
			text[cursor] = '\0';
			break;
		case XK_u:
			memmove(text, text + cursor, sizeof text - cursor + 1);
			cursor = 0;
			break;
		case XK_w:
			if(cursor > 0) {
				i = cursor;
				while(i-- > 0 && text[i] == ' ');
				while(i-- > 0 && text[i] != ' ');
				memmove(text + i + 1, text + cursor, sizeof text - cursor + 1);
				cursor = i + 1;
			}
			break;
		case XK_y:
			{
				FILE *fp;
				char *s;
				if(!(fp = popen("sselp", "r")))
					eprint("cannot popen sselp\n");
				s = fgets(buf, sizeof buf, fp);
				pclose(fp);
				if(s == NULL)
					return;
			}
			num = strlen(buf);
			if(num && buf[num-1] == '\n')
				buf[--num] = '\0';
			break;
		}
	}
	switch(ksym) {
	default:
		num = MIN(num, sizeof text - cursor);
		if(num && !iscntrl((int) buf[0])) {
			memmove(text + cursor + num, text + cursor, sizeof text - cursor - num);
			memcpy(text + cursor, buf, num);
			cursor += num;
		}
		break;
	case XK_BackSpace:
		if(cursor == 0)
			return;
		for(i = 1; cursor - i > 0 && !IS_UTF8_1ST_CHAR(text[cursor - i]); i++);
		memmove(text + cursor - i, text + cursor, sizeof text - cursor + i);
		cursor -= i;
		break;
	case XK_Delete:
		if(cursor == len)
			return;
		for(i = 1; cursor + i < len && !IS_UTF8_1ST_CHAR(text[cursor + i]); i++);
		memmove(text + cursor, text + cursor + i, sizeof text - cursor);
		break;
	case XK_End:
		cursor = len;
		break;
	case XK_Escape:
		exit(EXIT_FAILURE);
	case XK_Home:
		cursor = 0;
		break;
	case XK_Left:
		if(cursor == 0)
			return;
		while(cursor-- > 0 && !IS_UTF8_1ST_CHAR(text[cursor]));
		break;
	case XK_Return:
		fprintf(stdout, "%s", text);
		fflush(stdout);
		exit(EXIT_SUCCESS);
	case XK_Right:
		if(cursor == len)
			return;
		while(cursor++ < len && !IS_UTF8_1ST_CHAR(text[cursor]));
		break;
	}
	drawbar();
}

int
main(int argc, char *argv[]) {
	unsigned int i;

	/* command line args */
	progname = "dinput";
	for(i = 1; i < argc; i++)
		if(!strcmp(argv[i], "-i"))
			;  /* ignore flag */
		else if(!strcmp(argv[i], "-b"))
			topbar = False;
		else if(!strcmp(argv[i], "-l"))
			i++;  /* ignore flag */
		else if(!strcmp(argv[i], "-fn")) {
			if(++i < argc) font = argv[i];
		}
		else if(!strcmp(argv[i], "-nb")) {
			if(++i < argc) normbgcolor = argv[i];
		}
		else if(!strcmp(argv[i], "-nf")) {
			if(++i < argc) normfgcolor = argv[i];
		}
		else if(!strcmp(argv[i], "-p")) {
			if(++i < argc) prompt = argv[i];
		}
		else if(!strcmp(argv[i], "-sb")) {
			if(++i < argc) selbgcolor = argv[i];
		}
		else if(!strcmp(argv[i], "-sf")) {
			if(++i < argc) selfgcolor = argv[i];
		}
		else if(!strcmp(argv[i], "-v")) {
			printf("dinput-"VERSION", © 2006-2010 dmenu engineers, see LICENSE for details\n");
			exit(EXIT_SUCCESS);
		}
		else if(!*text) {
			strncpy(text, argv[i], sizeof text);
			cursor = strlen(text);
		}
		else {
			fputs("usage: dinput [-b] [-fn <font>] [-nb <color>] [-nf <color>]\n"
			      "              [-p <prompt>] [-sb <color>] [-sf <color>] [-v] [<text>]\n", stderr);
			exit(EXIT_FAILURE);
		}
	if(!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fprintf(stderr, "dinput: warning: no locale support\n");
	if(!(dpy = XOpenDisplay(NULL)))
		eprint("cannot open display\n");
	if(atexit(&cleanup) != 0)
		eprint("cannot register cleanup\n");
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	grabkeyboard();
	setup(0);
	run();
	return 0;
}