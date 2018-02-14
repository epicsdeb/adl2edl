#include <cstdio>
#include <string>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <iostream> 
#include <fstream> 
#ifdef HPUX
#include <ostream>
#endif
#include <list>
#include <vector>
#include <algorithm>


//shell command in shell_cmd.h
#define SHCMDC_MAJOR_VERSION 4
#define SHCMDC_MINOR_VERSION 0
#define SHCMDC_RELEASE 0

//message button in message_button.h
#define MSGBTC_MAJOR_VERSION 4
#define MSGBTC_MINOR_VERSION 0
#define MSGBTC_RELEASE 0

//choice button in button.h
#define BTC_MAJOR_VERSION 4
#define BTC_MINOR_VERSION 0
#define BTC_RELEASE 0

// byte monitor in pvFactory/byte.h
#define BYTE_MAJOR 4
#define BYTE_MINOR 0
#define BYTE_RELEASE 0

//related_display.h
#define RDC_MAJOR_VERSION 4
#define RDC_MINOR_VERSION 0
#define RDC_RELEASE 0

//xygraph.h xyplot
#define XYGC_MAJOR_VERSION 4
#define XYGC_MINOR_VERSION 0
#define XYGC_RELEASE 0

//bar.h bar
#define BARC_MAJOR_VERSION 4
#define BARC_MINOR_VERSION 0
#define BARC_RELEASE 0

//meter.h meter
#define METERC_MAJOR_VERSION 4
#define METERC_MINOR_VERSION 0
#define METERC_RELEASE 0

// text entry(adl) text control(edl)
// text update(adl) text monitor(edl)
//x_text_dsp_obj.h text monitor
#define XTDC_MAJOR_VERSION 4
#define XTDC_MINOR_VERSION 7
#define XTDC_RELEASE 0

//x_text_obj.h text
// for static text(edl)
#define AXTC_MAJOR_VERSION 4
#define AXTC_MINOR_VERSION 1
#define AXTC_RELEASE 1

//line_obj.h polyline
#define ALC_MAJOR_VERSION 4
#define ALC_MINOR_VERSION 0
#define ALC_RELEASE 0

//circle_obj.h circle
#define ACC_MAJOR_VERSION 2
#define ACC_MINOR_VERSION 1
#define ACC_RELEASE 0

//arc_obj.h arc
#define AAC_MAJOR_VERSION 2
#define AAC_MINOR_VERSION 1
#define AAC_RELEASE 0

// rectangle_obj.h 
#define ARC_MAJOR_VERSION 4
#define ARC_MINOR_VERSION 0
#define ARC_RELEASE 0

// menu_button.h 
#define MBTC_MAJOR_VERSION 4
#define MBTC_MINOR_VERSION 0
#define MBTC_RELEASE 0

//act_win.h header
#define AWC_MAJOR_VERSION 4
#define AWC_MINOR_VERSION 0
#define AWC_RELEASE 1

//group.h text
#define AGC_MAJOR_VERSION 4
#define AGC_MINOR_VERSION 0
#define AGC_RELEASE 0

//pip.h text
#define PIP_MAJOR_VERSION 4
#define PIP_MINOR_VERSION 1
#define PIP_RELEASE 0

struct battr {
		int clr;
		int style;
		int fill;
		int width;
};

struct dattr {
		int colormode;	// static, alarm, or discrete
		int vis;
		std::string calc;
		std::string chan;
		std::string chanB;
		std::string chanC;
		std::string chanD;
};

class parseclass
{
public:
	parseclass();
	virtual ~parseclass();
	void stripQs(std::string &str);
	std::string line;
    int pos;
    int eq_pos;
    int open;
    std::string bopen;
    std::string bclose;
    std::string eq;
    std::string space;
    std::string nil;
	std::string squote;
    int snum;
    char s1[80], s2[80];
};

class comclass
{
public:
	comclass();
	virtual ~comclass();
	void fill_attrs(int attr);
	int x;
    int y;
    int wid;
    int hgt;
	int clr;

	int style;
	int fill;
	int linewidth;
	int colormode;	// static, alarm, or discrete

	int vis;		// static, if not zero, if zero, calc
	int visInverted;

	std::string calc;
	std::string chan;
	std::string chanB;
	std::string chanC;
	std::string chanD;
};

class shellclass : public comclass, public parseclass
{
public:
	shellclass(int attr);
	virtual ~shellclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bclr;
};

class valclass : public comclass, parseclass
{
public:
	valclass(int attr);
	virtual ~valclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bclr;
};

class mbuttonclass : public comclass, parseclass
{
public:
	mbuttonclass(int attr);
	virtual ~mbuttonclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bclr;
	std::string label;
	std::string press_msg;
	std::string release_msg;
};

class buttonclass : public comclass, parseclass
{
public:
	buttonclass(int attr);
	virtual ~buttonclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bclr;
};

class byteclass : public comclass, public parseclass
{
public:
	byteclass(int attr);
	virtual ~byteclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bclr;
	int direction;
	int sbit;
	int ebit;
};

class shellnode 
{
public:
    shellnode();
    virtual ~shellnode();
    std::string name;
    std::string label;
    std::string args;
};

class relnode 
{
public:
    relnode();
    virtual ~relnode();
    std::string name;
    std::string label;
    std::string args;
	std::string policy;
};

class tnode{
public:
    tnode();
    virtual ~tnode();
    std::string xdata;
    std::string ydata;
    int color;
};


class relatedclass : public comclass, public parseclass
{
public:
	relatedclass(int attr);
	virtual ~relatedclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	std::string look_for_file(std::ostream&, std::string tname);
	int bclr;
};

class xyclass :public comclass, public parseclass
{
public:
	xyclass(int attr);
	virtual ~xyclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	//int clr;
	int bgColor;
	int plotMode;
	int border;
	int count;
	int updateTimerValue;
	int xAxis;
	int xAxisStyle;
	int xAxisSource;
	int xMin1;
	int xMin2;
	int xMax1;
	int xMax2;
	int xAxisTimeFormat;
	int symbol;

	std::string title;
	std::string xlabel;
	std::string ylabel;
	std::string squote;
	int erase;

	// Axes:
	// if range is "from pv", that value is not in adl file
	int xrangeStyle;
	int y1rangeStyle;
	int y2rangeStyle;
	// if min = 0 or max = 1, that value is not in adl file
	double xmin;
	double xmax;
	double y1min;
	double y1max;
	double y2min;
	double y2max;
};

class stripclass :public comclass, public parseclass
{
public:
	stripclass(int attr);
	virtual ~stripclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	//int clr;
	int bgColor;
	int plotMode;
	int border;
	int count;
	int updateTimerValue;
	int xAxis;
	int xAxisStyle;
	int xAxisSource;
	int xMin1;
	int xMin2;
	int xMax1;
	int xMax2;
	int xAxisTimeFormat;
	int symbol;

	std::string title;
	std::string xlabel;
	std::string ylabel;
	std::string squote;
	int erase;

	// Axes:
	// if range is "from pv", that value is not in adl file
	int xrangeStyle;
	int y1rangeStyle;
	int y2rangeStyle;
	// if min = 0 or max = 1, that value is not in adl file
	double xmin;
	double xmax;
	double y1min;
	double y1max;
	double y2min;
	double y2max;
};

class barclass : public comclass, public parseclass
{
public:
	barclass(int attr);
	virtual ~barclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bgColor;
	std::string label;
};

class meterclass :public comclass, public parseclass
{
public:
	meterclass(int attr);
	virtual ~meterclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);

	int meterColorMode;
	int scaleColor;
	int scaleColorMode;
	int labelColor;
	int fgColor;
	int bgColor;
	int tsColor;
	int bsColor;

	std::string controlPvExpStr;
	std::string readPvExpStr;
	std::string literalLabel;
	std::string label;
	int labelType;
	int showScale;
	std::string scaleFormat;
	int scalePrecision;
	int scaleLimitsFromDb;
	int useDisplayBg;
	int majorIntervals;
	int minorIntervals;
	int needleType;
	int shadowMode;
	int scaleMin;
	int scaleMax;
	std::string labelFontTag;
	std::string scaleFontTag;
	int meterAngle;
};

class menuclass :public comclass, public parseclass
{
public:
    menuclass(int attr);
    virtual ~menuclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int bclr;
};

class textmonclass :public comclass, public parseclass
{
public:
    textmonclass(int attr);
    virtual ~textmonclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&, int noedit);
	int bclr;
	std::string format;
};

class polyclass : public comclass, public parseclass
{
public:
	polyclass(int attr);
	virtual ~polyclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&, int close);
	int fillColor;
	int close;
	int arrows;
};

class circleclass : public comclass, public parseclass
{
public:
	circleclass(int attr);
	virtual ~circleclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int width;
	int fillColor;
};

class arcclass : public comclass, public parseclass
{
public:
	arcclass(int attr);
	virtual ~arcclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	int fillColorMode;
	int startAng;
	int startAng2;
	int totalAng;
	int totalAng2;
	int fillMode;
};


class cmapclass : public comclass, public parseclass
{
public:
	cmapclass();
	virtual ~cmapclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
	std::string& getRGB(int index) ;
private:
	std::vector <std::string> rgbvec;
	int numColors;
};

class rectclass : public comclass, public parseclass
{
public:
	rectclass(int attr);
	virtual ~rectclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);
};

class textclass : public comclass, public parseclass
{
public:
	textclass(int attrs);
	virtual ~textclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);

	int bkClr;
	std::string textstr;
};
	
class headclass :public comclass, public parseclass
{
public:
	headclass();
	virtual ~headclass();
	int parse(std::ifstream&, std::ostream&, std::ostream&);

	int fgClr;
	int bkClr;
	int gridShow; 		// gridShow 0, Active 0, Spacing 10, orthogonal 0
	int gridSpacing; 	//5
	int gridOn; 		//0
	int snapToGrid; 	//0
};

class translator
{
public:
	translator();
	~translator();
	int processFile (char *in, char *out);
	int parseComposite(std::ifstream&, std::ostream&, 
		std::ostream&, std::ostream&);
	int parseCompositeClose(std::ifstream&, std::ostream&, 
		std::ostream&, std::ostream&);
	int read_battr(std::ifstream&, std::ostream&);
	int read_dattr(std::ifstream&, std::ostream&);
	std::ifstream inf;
	std::ofstream toutf;
	static std::string dir;
	static int line_ctr;
	static bool debug;
	static struct battr ba;
	static struct dattr da;
private:
	std::string medmfile;
	std::string edmfile;
};

static cmapclass cmap;
