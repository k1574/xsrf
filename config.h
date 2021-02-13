/* modifier 0 means no modifier */
static int xsrfuseragent = 1;  /* Append Surf version to default WebKit user agent. */
static char *fulluseragent = ""; /* Or override the whole user agent string */
static char *scriptfile = "~/lib/xsrf/script.js";
static char *styledir = "~/lib/xsrf/styles/";
static char *certdir = "~/lib/xsrf/certificates/";
static char *cachedir = "~/lib/xsrf/cache/";
static char *cookiefile = "~/lib/xsrf/cookies.txt";
static char *start_uri = "about:blank" ;

/* Webkit default features */
/* Highest priority value will be used.
	Default parameters are priority 0;
	Per-uri parameters are priority 1;
	Command parameters are priority 2. */
static Parameter defconfig[ParameterLast] = {
	/* parameter                    Arg value       priority */
	[AcceleratedCanvas]   =       { { .i = 1 },     },
	[AccessMicrophone]    =       { { .i = 0 },     },
	[AccessWebcam]        =       { { .i = 0 },     },
	[Certificate]         =       { { .i = 0 },     },
	[CaretBrowsing]       =       { { .i = 0 },     },
	[CookiePolicies]      =       { { .v = "@Aa" }, },
	[DefaultCharset]      =       { { .v = "UTF-8" }, },
	[DiskCache]           =       { { .i = 1 },     },
	[DNSPrefetch]         =       { { .i = 0 },     },
	[FileURLsCrossAccess] =       { { .i = 0 },     },
	[FontSize]            =       { { .i = 12 },    },
	[FrameFlattening]     =       { { .i = 0 },     },
	[Geolocation]         =       { { .i = 0 },     },
	[HideBackground]      =       { { .i = 0 },     },
	[Inspector]           =       { { .i = 0 },     },
	[Java]                =       { { .i = 1 },     },
	[JavaScript]          =       { { .i = 1 },     },
	[KioskMode]           =       { { .i = 0 },     },
	[LoadImages]          =       { { .i = 1 },     },
	[MediaManualPlay]     =       { { .i = 1 },     },
	[Plugins]             =       { { .i = 1 },     },
	[PreferredLanguages]  =       { { .v = (char *[]){ NULL } }, },
	[RunInFullscreen]     =       { { .i = 0 },     },
	[ScrollBars]          =       { { .i = 0 },     },
	[ShowIndicators]      =       { { .i = 1 },     },
	[SiteQuirks]          =       { { .i = 1 },     },
	[SmoothScrolling]     =       { { .i = 0 },     },
	[SpellChecking]       =       { { .i = 0 },     },
	[SpellLanguages]      =       { { .v = ((char *[]){ "en_US", NULL }) }, },
	[StrictTLS]           =       { { .i = 1 },     },
	[Style]               =       { { .i = 1 },     },
	[WebGL]               =       { { .i = 0 },     },
	[ZoomLevel]           =       { { .f = 1.0 },   },
} ;

static UriParameters uriparams[] = {
	{ "(://|\\.)suckless\\.org(/|$)", {
	  [JavaScript] = { { .i = 0 }, 1 },
	  [Plugins]    = { { .i = 0 }, 1 },
	}, },
} ;

/* Default window size: width, height. */
static int winsize[] = { 800, 600 };

static WebKitFindOptions findopts = WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE |
                                    WEBKIT_FIND_OPTIONS_WRAP_AROUND;

#define PROMPT_GO   "Go:"
#define PROMPT_FIND "Find:"
#define PROMPT_SEARCH "Search:"

/* SETPROP(readprop, setprop, prompt)*/
#define SETPROP(r, s, p) { \
        .v = (const char *[]){ "/bin/env", "rc", "-c", \
		"prop=`{xprp $2 $3 | xmen -w $2 -p $5}" \
		"&& exec xprp $2 $4 $\"prop", \
             "xsrf-setprop", winid, r, s, p, NULL \
        } \
}

/* DOWNLOAD(URI, referer) */
#define DOWNLOAD(u, r) { \
	.v = (const char *[]){ "/bin/env", "rc", "-c",\
		"exec xtrm `{test -z $XEMBED || echo -w $XEMBED}-t $1':'$5 -e rc -c '"\
			"test -z $load || {mkdir -p $load && cd $load }"\
			"; curl -g -L -J -O -A $2 -b $3 -c $3 -e $4 $5"\
			"; read' $*", \
			"'"\
             "xsrf-download", useragent, cookiefile, r, u, NULL \
        } \
}

/* PLUMB(URI) */
/* This called when some URI which does not begin with "about:",
 * "http://" or "https://" should be opened.
 */
#define PLUMB(u) {\
        .v = (const char *[]){ "/bin/env", "rc", "-c", \
             "exec plumb $1", u, NULL \
        } \
}

/* VIDEOPLAY(URI) */
#define VIDEOPLAY(u) {\
        .v = (const char *[]){ "/bin/env", "rc", "-c",\
		"if(test -z $XEMBED)"\
			"exec mpv --really-quiet '--title='xsrf-video':'$1 $1;"\
		"if not "\
			"exec mpv --really-quiet '--title='xsrf-video':'$1 '--wid='$XEMBED $1;",\
		u, NULL \
        } \
}

/* Styles. */
/* The iteration will stop at the first match, beginning at the beginning of
	the list. */
static SiteSpecific styles[] = {
	/* Regexp, file in $styledir. */
	{ ".*",                 "default.css" },
} ;

/* Certificates. */
/* Provide custom certificate for urls. */
static SiteSpecific certs[] = {
	/* Regexp, file in $certdir. */
	{ "://suckless\\.org/", "suckless.org.crt" },
} ;

/* Search engines.
	Provide custom search engines. Prefix should be one word without spaces.
	It works like "g something to find".
	It'll do nothing if input string is wrong. */
static SearchEngine sengines[] = {
	/* Prefix, URI. */
	{  "g", "https://www.google.com/search?q=%s" },
	{  "ddg", "https://duckduckgo.com/?q=%s"},
	{  "y", "https://search.yahoo.com/search?p=%s"},
	{  "wpe", "https://en.wikipedia.org/wiki/%s"},
	{  "wde", "https://en.wiktionary.org/wiki/%s"},
	{  "wpr", "https://ru.wikipedia.org/wiki/%s"},
	{  "wdr", "https://ru.wiktionary.org/wiki/%s"},
} ;

#define MODKEY GDK_CONTROL_MASK
/* Hotkeys. */
/* If you use anything else but MODKEY and GDK_SHIFT_MASK, don't forget to
	edit the CLEANMASK() macro. */
static Key keys[] = {
	/* Mod key, key, func, arg. */
	{ MODKEY, GDK_KEY_g, spawn, SETPROP("_SURF_URI", "_SURF_GO", PROMPT_GO) },
	{ MODKEY, GDK_KEY_s, spawn, SETPROP("_SURF_SEARCH", "_SURF_SEARCH", PROMPT_SEARCH) },
	{ MODKEY, GDK_KEY_f, spawn, SETPROP("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },
	{ MODKEY, GDK_KEY_slash,  spawn, SETPROP("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },

	{ 0, GDK_KEY_Escape, stop, { 0 } },
	{ MODKEY, GDK_KEY_c, stop, { 0 } },

	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_r, reload, { .i = 1 } },
	{ MODKEY, GDK_KEY_r, reload, { .i = 0 } },

	{ MODKEY, GDK_KEY_l, navigate, { .i = +1 } },
	{ MODKEY, GDK_KEY_h, navigate, { .i = -1 } },

	/* vertical and horizontal scrolling, in viewport percentage */
	{ MODKEY, GDK_KEY_j, scrollv, { .i = +10 } },
	{ MODKEY, GDK_KEY_k, scrollv, { .i = -10 } },
	{ MODKEY, GDK_KEY_space, scrollv, { .i = +50 } },
	{ MODKEY, GDK_KEY_b, scrollv, { .i = -50 } },
	{ MODKEY, GDK_KEY_i, scrollh, { .i = +10 } },
	{ MODKEY, GDK_KEY_u, scrollh, { .i = -10 } },


	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_j, zoom, { .i = -1 } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_k, zoom, { .i = +1 } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_q, zoom, { .i = 0  } },
	{ MODKEY, GDK_KEY_minus, zoom, { .i = -1 } },
	{ MODKEY, GDK_KEY_plus, zoom, { .i = +1 } },

	{ MODKEY, GDK_KEY_p, clipboard, { .i = 1 } },
	{ MODKEY, GDK_KEY_y, clipboard, { .i = 0 } },

	{ MODKEY, GDK_KEY_n, find, { .i = +1 } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_n, find, { .i = -1 } },

	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_p, print, { 0 } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_t, showcert, { 0 } },

	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_a, togglecookiepolicy, { 0 } },
	{ 0, GDK_KEY_F11, togglefullscreen, { 0 } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_o, toggleinspector, { 0 } },

	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_c, toggle, { .i = CaretBrowsing } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_f, toggle, { .i = FrameFlattening } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_g, toggle, { .i = Geolocation } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_s, toggle, { .i = JavaScript } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_i, toggle, { .i = LoadImages } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_v, toggle, { .i = Plugins } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_b, toggle, { .i = ScrollBars } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_t, toggle, { .i = StrictTLS } },
	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_m, toggle, { .i = Style } },
} ;

/* Button definitions. */
/* Target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny. */
static Button buttons[] = {
	/* Target, event mask, button, function, argument, stop event. */
	{ OnLink, 0, 2, clicknewwindow, { .i = 0 }, 1 },
	{ OnLink, MODKEY, 2, clicknewwindow, { .i = 1 }, 1 },
	{ OnLink, MODKEY, 1, clicknewwindow, { .i = 1 }, 1 },
	{ OnAny, 0, 8, clicknavigate, { .i = -1 }, 1 },
	{ OnAny, 0, 9, clicknavigate, { .i = +1 }, 1 },
	{ OnMedia, MODKEY, 1, mediaOpenExternPlayer, { 0 }, 1 },
	{ OnLink, GDK_SHIFT_MASK, 2, linkOpenExternPlayer, { 0 }, 1},
} ;
