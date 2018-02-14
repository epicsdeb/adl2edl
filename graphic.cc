#include "translator.h"
#include "fonts.h"

using std::find;
using std::list;
using std::endl;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;

extern fontInfoClass fi;
static char *fptr;

extern bool urgb;

comclass::comclass()
{
}

comclass::~comclass()
{
}

parseclass::parseclass()
{
	open = 1;
    bopen = "{";
    bclose = "}";
    eq = "=";
    space = " ";
    nil = "";
	squote = "\"";
}

parseclass::~parseclass()
{
}

void parseclass::stripQs(string &str)
{
	int tpos;
	string quote;
	string nil;
	quote = "\"";
	nil = "";

	while((tpos = str.find(quote,0)) != -1)
		str.replace(tpos,1,nil);
}

polyclass::polyclass(int attrs)
{
	fill_attrs(attrs);
	close = 0;
	arrows = 0;
}

polyclass::~polyclass()
{
}

// process the "polygon or polyline" object
int polyclass::parse(ifstream &inf, ostream &outf, ostream &outd, int close)
{
    string popen("(");
    string pclose(")");
	int comma_pos;
	int p_pos;
	int tpos;
    string comma(",");
	int pt_ctr = 0;
    int mode = 0;
	list <int> lx;
	list <int> ly;

	if(close) {
		close = 1;
		fill = 1;
	} else {
		fill = 0;
	}
    //outd << "In Poly " << translator::line_ctr << endl;
    do {
        getline(inf,line);
		translator::line_ctr++;
        pos = line.find(bopen,0);
        if(pos != -1)  open++; 
        pos = line.find(bclose,0);
        if(pos != -1)  open--; 

        eq_pos = line.find(eq,0);
        if(eq_pos != -1){
            line.replace(eq_pos,1,space);
            snum = sscanf(line.c_str(), "%s %s", s1,s2);
            if(!strcmp(s1,"x"))  x = atoi(s2); 
            else if(!strcmp(s1,"y"))  y = atoi(s2); 
            else if(!strcmp(s1,"width")) {
                if(mode == 0) wid = atoi(s2);
                else  linewidth = atoi(s2);
            }
            else if(!strcmp(s1,"height"))  hgt = atoi(s2); 
            else if(!strcmp(s1,"clr")) {
                if(mode == 1) clr = atoi(s2);
                else if(strstr(s2, "static")!= 0x0) colormode = 0;
                else if(strstr(s2, "alarm")!= 0x0) colormode = 1;
                else if(strstr(s2, "discrete")!= 0x0) colormode = 2;
            }
            else if(!strcmp(s1,"fill")) {
                if(strstr(s2, "outline")!= 0x0) fill = 0;
				else fill = 1;
            }
            else if(!strcmp(s1,"style")) {
                if(strstr(s2, "solid")!= 0x0) style = 0;
                else style = 1;
			}
			    else if(strstr(s1, "vis")!= 0x0) {
                	if(strstr(line.c_str(), "static")!= 0x0) vis = 0;
                	else if(strstr(line.c_str(), "if not zero")!= 0x0) vis = 1;
                	else if(strstr(line.c_str(), "if zero")!= 0x0) vis = 2;
                	else if(strstr(line.c_str(), "calc")!= 0x0) vis = 3;
            }
            else if(!strcmp(s1,"calc")) calc = s2;
            else if(!strcmp(s1,"chan")) chan = s2;
            else if(!strcmp(s1,"chanB")) chanB = s2;
            else if(!strcmp(s1,"chanC")) chanC = s2;
            else if(!strcmp(s1,"chanD")) chanD = s2;
            else outd << "Lines Can't decode line " << line << endl;
        }
		else if(mode == 3) {
			/* points in adl file are in the form:
			points {
				(31,80)
				(60,55)
				(94,91)
				(142,49)
    		}
			*/
        	comma_pos = line.find(comma,0);
        	if(comma_pos != -1){
				line.replace(comma_pos,1,space);
	        	p_pos = line.find(popen,0);
        		if(p_pos != -1) line.replace(p_pos,1,nil);
	        	p_pos = line.find(pclose,0);
        		if(p_pos != -1) line.replace(p_pos,1,nil);
            	snum = sscanf(line.c_str(), "%s %s", s1,s2);
				lx.push_back(atoi(s1));
				ly.push_back(atoi(s2));
				pt_ctr++;
			}
		}
        else {
            //outd << "OTHER: " << line << endl;
            if(strstr(line.c_str(), "basic") != 0x0) mode = 1;
            else if(strstr(line.c_str(), "dynamic") != 0x0) mode = 2;
            else if(strstr(line.c_str(), "object") != 0x0) mode = 0;
            else if(strstr(line.c_str(), "}") != 0x0) mode = 0;
        	else if(strstr(line.c_str(),"points") != 0x0) mode = 3;
        }
    } while (open > 0);

    // Strip the quotes from the pv names when used for visibility pv
    if(vis) {
		stripQs(calc);
		stripQs(chan);
		stripQs(chanB);
		stripQs(chanC);
		stripQs(chanD);
		if(calc.length() == 0) calc = "A";
    }

    outf << endl;
    outf << "# (Lines)" << endl;
    outf << "object activeLineClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << ALC_MAJOR_VERSION << endl;
    outf << "minor " << ALC_MINOR_VERSION << endl;
    outf << "release " << ALC_RELEASE << endl;
    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;

	if(urgb) outf << "lineColor rgb " << cmap.getRGB(clr) << endl;
    else outf << "lineColor index " << clr << endl;
	if(fill) {
		outf << "fill" << endl;
	}
	if(urgb) outf << "fillColor rgb " << cmap.getRGB(clr) << endl;
	else outf << "fillColor index " << clr << endl;
    if(linewidth != 1) outf << "lineWidth " << linewidth << endl;
    if(style) outf << "lineStyle \"dash\"" << endl;

	if(close) outf << "closePolygon" << endl;
	outf << "numPoints " << pt_ctr << " {" << endl;
	list<int>::iterator p = lx.begin();
	list<int>::iterator q = ly.begin();
	outf << "xPoints " <<  " {" << endl;
	int i = 0;
	while(p != lx.end())
		outf << i++ << " " << *p++ << endl;
	outf << "}" << endl;

	outf << "yPoints " <<  " {" << endl;
	i = 0;
	while(q != ly.end())
		outf << i++ << " " << *q++ << endl;
	outf << "}" << endl;
    if(colormode == 1) {        // ALARM
        outf << "fillAlarm" << endl;
        outf << "lineAlarm" << endl;
        outf << "alarmPv " << chan << endl;
    }  
	if(vis != 0) {       //VISABILITY
        string tstr = "visPv \"CALC\\\\\\{(";
        string tstr2 = ")\\}(";
        outf <<  tstr << calc << tstr2 << chan ;
        if(chanB.length()) outf << ", " << chanB ;
        if(chanC.length()) outf << ", " << chanC ;
        if(chanD.length()) outf << ", " << chanD ;
        outf << ")\"" << endl;
        if(vis == 1 || vis == 3) outf << "visInvert" <<  endl;
        outf << "visMin 0" << endl;
        outf << "visMax 1" << endl;
    }

	outf << "endObjectProperties" << endl;
    return 1;
}

circleclass::circleclass(int attrs)
{
	fill_attrs(attrs);
}

circleclass::~circleclass()
{
}

// process the "circle" object
int circleclass::parse(ifstream &inf, ostream &outf, ostream &outd )
{
    int mode = 0;
	int tpos;

    //outd << "In Circle " << translator::line_ctr << endl;
    do {
        getline(inf,line);
		translator::line_ctr++;
        pos = line.find(bopen,0);
        if(pos != -1)  open++; 
        pos = line.find(bclose,0);
        if(pos != -1)  open--; 

        eq_pos = line.find(eq,0);
        if(eq_pos != -1){
            line.replace(eq_pos,1,space);
            snum = sscanf(line.c_str(), "%s %s", s1,s2);
            if     (!strcmp(s1,"x"))  x = atoi(s2); 
            else if(!strcmp(s1,"y"))  y = atoi(s2); 
            else if(!strcmp(s1,"width")) {
                if(mode == 0) wid = atoi(s2);
                else  linewidth = atoi(s2);
            }
            else if(!strcmp(s1,"height"))  hgt = atoi(s2); 
            else if(!strcmp(s1,"clr")) {
                if(mode == 1) clr = atoi(s2);
                else if(strstr(s2, "static")!= 0x0) colormode = 0;
                else if(strstr(s2, "alarm")!= 0x0) colormode = 1;
                else if(strstr(s2, "discrete")!= 0x0) colormode = 2;
            }

            else if(!strcmp(s1,"fill")) {
                if(strstr(s2, "solid")!= 0x0) {
					fill = 1;
				//	linewidth = 0;
				} else fill = 0;
            }
            else if(!strcmp(s1,"style")) {
                if(strstr(s2, "solid")!= 0x0) style = 0;
                else style = 1; //dashed line
			}
            else if(strstr(s1, "vis")!= 0x0) {
                if(strstr(line.c_str(), "static")!= 0x0) vis = 0;
                else if(strstr(line.c_str(), "if not zero")!= 0x0) vis = 1;
                else if(strstr(line.c_str(), "if zero")!= 0x0) vis = 2;
                else if(strstr(line.c_str(), "calc")!= 0x0) vis = 3;
            }
            else if(!strcmp(s1,"calc")) calc = s2;
            else if(!strcmp(s1,"chan")) chan = s2;
            else if(!strcmp(s1,"chanB")) chanB = s2;
            else if(!strcmp(s1,"chanC")) chanC = s2;
            else if(!strcmp(s1,"chanD")) chanD = s2;
            else outd << "Circle Can't decode circle " << line << endl;
        }
        else {
            if(strstr(line.c_str(), "basic") != 0x0) mode = 1;
            else if(strstr(line.c_str(), "dynamic") != 0x0) mode = 2;
            else if(strstr(line.c_str(), "object") != 0x0) mode = 0;
            else if(strstr(line.c_str(), "}") != 0x0) mode = 0;
        }
    } while (open > 0);

    // Strip the quotes from the pv names when used for visibility pv
    if(vis) {
		stripQs(calc);
		stripQs(chan);
		stripQs(chanB);
		stripQs(chanC);
		stripQs(chanD);
		if(calc.length() == 0) calc = "A";
    }

	outf << endl;
    outf << "# (Circle)" << endl;
    outf << "object activeCircleClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << ARC_MAJOR_VERSION << endl;
    outf << "minor " << ARC_MINOR_VERSION << endl;
    outf << "release " << ARC_RELEASE << endl;

	if(linewidth) {
		x = x + (int)(linewidth/2);
		y = y + (int)(linewidth/2);
		wid = wid -linewidth;
		hgt = hgt -linewidth;
	}
	if(fill) {
		x = x +1;
		y = y +1;
		wid =  wid-2;
		hgt =  hgt-2;
	} 

    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;

	if(urgb) outf << "lineColor rgb " << cmap.getRGB(clr) << endl;
    else outf << "lineColor index " << clr << endl;
    if(fill)  outf << "fill" << endl;
	if(urgb) outf << "fillColor rgb " << cmap.getRGB(clr) << endl;
    else outf << "fillColor index " << clr << endl;
    if(linewidth != 1) outf << "lineWidth " << linewidth << endl;
    if(style) outf << "lineStyle \"dash\"" << endl;

    if(colormode == 1) {        // ALARM
        outf << "fillAlarm" << endl;
        outf << "lineAlarm" << endl;
        outf << "alarmPv " << chan << endl;
    } 
	if(vis != 0) {       //VISABILITY
        string tstr = "visPv \"CALC\\\\\\{(";
        string tstr2 = ")\\}(";
        outf <<  tstr << calc << tstr2 << chan ;
        if(chanB.length()) outf << ", " << chanB ;
        if(chanC.length()) outf << ", " << chanC ;
        if(chanD.length()) outf << ", " << chanD ;
        outf << ")\"" << endl;
        if(vis == 1 || vis == 3) outf << "visInvert" <<  endl;
        outf << "visMin 0" << endl;
        outf << "visMax 1" << endl;
    }
    outf << "endObjectProperties" << endl;
    return 1;
}

arcclass::arcclass(int attrs)
{
	fill_attrs(attrs);
}

arcclass::~arcclass()
{
}

// process the "arc" object
int arcclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
    int mode = 0;
	int tpos;

    //outd << "In Arc " << translator::line_ctr << endl;
    do {
        getline(inf,line);
		translator::line_ctr++;
        pos = line.find(bopen,0);
        if(pos != -1)  open++; 
        pos = line.find(bclose,0);
        if(pos != -1)  open--; 

        eq_pos = line.find(eq,0);
        if(eq_pos != -1){
            line.replace(eq_pos,1,space);
            snum = sscanf(line.c_str(), "%s %s", s1,s2);
            if(!strcmp(s1,"x"))  x = atoi(s2); 
            else if(!strcmp(s1,"y"))  y = atoi(s2); 
            else if(!strcmp(s1,"width")) {
                if(mode == 0) wid = atoi(s2);
                else  linewidth = atoi(s2);
            }
            else if(!strcmp(s1,"height"))  hgt = atoi(s2); 
            else if(!strcmp(s1,"clr")) {
                if(mode == 1) clr = atoi(s2);
                else if(strstr(s2, "static")!= 0x0) colormode = 0;
                else if(strstr(s2, "alarm")!= 0x0) colormode = 1;
                else if(strstr(s2, "discrete")!= 0x0) colormode = 2;
            }
            else if(!strcmp(s1,"fill")) {
                if(strstr(s2, "solid")!= 0x0) {
                    fill = 1;
                    linewidth = 0;
                } else fill = 0;
            }
            else if(!strcmp(s1,"style")) {
                if(strstr(s2, "solid")!= 0x0) style = 0;
                else style = 1; //dashed line
            }
            else if(!strcmp(s1,"begin")) {
				startAng = atoi(s2)/64;
			}
            else if(!strcmp(s1,"path")) {
				totalAng = atoi(s2)/64;
			}
            else if(strstr(s1, "vis")!= 0x0) {
                if(strstr(line.c_str(), "static")!= 0x0) vis = 0;
                else if(strstr(line.c_str(), "if not zero")!= 0x0) vis = 1;
                else if(strstr(line.c_str(), "if zero")!= 0x0) vis = 2;
                else if(strstr(line.c_str(), "calc")!= 0x0) vis = 3;
            }
            else if(!strcmp(s1,"calc"))  calc = s2; 
            else if(!strcmp(s1,"chan")) chan = s2;
            else if(!strcmp(s1,"chanB")) chanB = s2;
            else if(!strcmp(s1,"chanC")) chanC = s2;
            else if(!strcmp(s1,"chanD")) chanD = s2;
            else outd << "Arc Can't decode arc " << line << endl;
        }
        else {
            if(strstr(line.c_str(), "basic") != 0x0) mode = 1;
            else if(strstr(line.c_str(), "dynamic") != 0x0) mode = 2;
            else if(strstr(line.c_str(), "object") != 0x0) mode = 0;
            else if(strstr(line.c_str(), "}") != 0x0) mode = 0;
        }
    } while (open > 0);

    if(vis) {
		stripQs(calc);
		stripQs(chan);
		stripQs(chanB);
		stripQs(chanC);
		stripQs(chanD);
		if(calc.length() == 0) calc = "A";
    }

    outf << endl;
    outf << "# (Arc)" << endl;
    outf << "object activeArcClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << ARC_MAJOR_VERSION << endl;
    outf << "minor " << ARC_MINOR_VERSION << endl;
    outf << "release " << ARC_RELEASE << endl;
    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;

	if(urgb) outf << "lineColor rgb " << cmap.getRGB(clr) << endl;
    else outf << "lineColor index " << clr << endl;
    if(fill)  outf << "fill" << endl;
	if(urgb) outf << "fillColor rgb " << cmap.getRGB(clr) << endl;
    else outf << "fillColor index " << clr << endl;
    if(linewidth != 1) outf << "lineWidth " << linewidth << endl;
    if(style) outf << "lineStyle \"dash\"" << endl;

    if(colormode == 1) {
        outf << "fillAlarm" << endl;
        outf << "lineAlarm" << endl;
        outf << "alarmPv " << chan  << endl;
    } 
	if(vis != 0) {       
        string tstr = "visPv \"CALC\\\\\\{(";
        string tstr2 = ")\\}(";
        outf <<  tstr << calc << tstr2 << chan ;
        if(chanB.length()) outf << ", " << chanB ;
        if(chanC.length()) outf << ", " << chanC ;
        if(chanD.length()) outf << ", " << chanD ;
        outf << ")\"" << endl;
        if(vis == 1 || vis == 3) outf << "visInvert" <<  endl;
        outf << "visMin 0" << endl;
        outf << "visMax 1" << endl;
    }
    outf << "startAngle " << " " << startAng << endl;
    outf << "totalAngle " << " " << totalAng << endl;
    outf << "fillMode \"pie\"" << endl;
    outf << "endObjectProperties" << endl;
    return 1;
}

rectclass::rectclass(int attr)
{
	fill_attrs(attr);
}

rectclass::~rectclass()
{
}

// process the "rectangle" object
int rectclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
    int mode = 0; 	// braced subtopics in medm: 0=normal, 1=basic, 2=dynamic
	squote = "\"";
	int tpos;


    //outd << "In Rectangle " << translator::line_ctr << endl;
    do {
        getline(inf,line);
		translator::line_ctr++;
        pos = line.find(bopen,0);
        if(pos != -1)  open++; 
        pos = line.find(bclose,0);
        if(pos != -1)  open--; 

        //If found, replace = with a space
        eq_pos = line.find(eq,0);
        if(eq_pos != -1){
            line.replace(eq_pos,1,space);
            snum = sscanf(line.c_str(), "%s %s", s1,s2);
            if (!strcmp(s1,"x"))  x = atoi(s2); 
            else if(!strcmp(s1,"y"))  y = atoi(s2); 
			// If width is within the basic attribs braces, it's linewidth
            else if(!strcmp(s1,"width")) {
                if(mode == 0) wid = atoi(s2); 	//normal mode
                else  linewidth = atoi(s2);		// basic mode
            }
            else if(!strcmp(s1,"height"))  hgt = atoi(s2); 

			// If clr is within the dyn attribs braces, it's colormode
        	else if(!strcmp(s1,"clr")) {
                if(mode == 1) clr = atoi(s2);	//basic mode
                else if(strstr(s2, "static")!= 0x0) colormode = 0;
                else if(strstr(s2, "alarm")!= 0x0) colormode = 1;
                else if(strstr(s2, "discrete")!= 0x0) colormode = 2;
            }
            else if(!strcmp(s1,"fill")) {
                if(strstr(s2, "solid")!= 0x0) {
					fill = 1;
					linewidth = 0;
				} else fill = 0;
            }
            else if(!strcmp(s1,"style")) {
                if(strstr(s2, "solid")!= 0x0) style = 0;
                else style = 1; //dashed line
			}
            else if(strstr(s1, "vis")!= 0x0) {
                if(strstr(line.c_str(), "static")!= 0x0) vis = 0;
                else if(strstr(line.c_str(), "if not zero")!= 0x0) vis = 1;
                else if(strstr(line.c_str(), "if zero")!= 0x0) vis = 2;
                else if(strstr(line.c_str(), "calc")!= 0x0) vis = 3;
            }
            else if(!strcmp(s1,"calc")) calc = s2;
            else if(!strcmp(s1,"chan")) chan = s2;
            else if(!strcmp(s1,"chanB")) chanB = s2;
            else if(!strcmp(s1,"chanC")) chanC = s2;
            else if(!strcmp(s1,"chanD")) chanD = s2;
            else  outd << "Rectangle Can't decode rect " << line << endl; 
        }
        else {
            if(strstr(line.c_str(), "basic") != 0x0) mode = 1;
            else if(strstr(line.c_str(), "dynamic") != 0x0) mode = 2;
            else if(strstr(line.c_str(), "object") != 0x0) mode = 0;
            else if(strstr(line.c_str(), "}") != 0x0) mode = 0;
        }
    } while (open > 0);

	if(vis) {
		stripQs(calc);
		stripQs(chan);
		stripQs(chanB);
		stripQs(chanC);
		stripQs(chanD);
		if(calc.length() == 0) calc = "A";
	}

	// Fix for rf SRF zone screens
	if(wid==2 && hgt==2 && linewidth==3) {
		outd << "bad rectangle " << translator::line_ctr << endl;
		return 1;
	}

    outf << endl;
    outf << "# (Rectangle)" << endl;
    outf << "object activeRectangleClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << ARC_MAJOR_VERSION << endl;
    outf << "minor " << ARC_MINOR_VERSION << endl;
    outf << "release " << ARC_RELEASE << endl;

	if(linewidth) {
		x = x + (int)(linewidth/2);
		y = y + (int)(linewidth/2);
		wid = wid -linewidth;
		hgt = hgt -linewidth;
	}
	if(fill) {
		wid =  wid-1;
		hgt =  hgt-1;
	} 

    outf << "x " << x << endl;
    outf << "y " << y << endl;
	outf << "w " << wid << endl;
	outf << "h " << hgt << endl;
	if(urgb) outf << "lineColor rgb " << cmap.getRGB(clr) << endl;
	else outf << "lineColor index " << clr << endl;

	if(fill)  outf << "fill" << endl;

	if(urgb) outf << "fillColor rgb " << cmap.getRGB(clr) << endl;
	else outf << "fillColor index " << clr << endl; 

	//outf << "lineWidth " << linewidth << endl;
	if(linewidth != 1) outf << "lineWidth " << linewidth << endl;
	if(style) outf << "lineStyle \"dash\"" << endl;

	if(colormode == 1) { 	
		outf << "fillAlarm" << endl;
        outf << "lineAlarm" << endl;
		outf << "alarmPv " << chan  << endl; 
	} 
	if(vis != 0) { 
		string tstr = "visPv \"CALC\\\\\\{(";
		string tstr2 = ")\\}(";
		outf <<  tstr << calc << tstr2 << chan ;
		if(chanB.length()) outf << ", " << chanB ;
		if(chanC.length()) outf << ", " << chanC ;
		if(chanD.length()) outf << ", " << chanD ;
		outf << ")\"" << endl;
		//outd << "vis " << vis << endl;
		//outd <<  tstr << calc << tstr2 << chan << endl;
		if(vis == 1 || vis == 3) {
			outf << "visInvert" <<  endl;
			outf << "visMin 0" << endl;
			outf << "visMax 1" << endl;
		} else {
			//outf << "visMin 1" << endl;
			//outf << "visMax 10000" << endl;
			outf << "visMin 0" << endl;
			outf << "visMax 1" << endl;
		}
	}
    outf << "endObjectProperties" << endl; 
    return 1;
}

textclass::textclass(int attrs)
{
	fill_attrs(attrs);
}

textclass::~textclass()
{
}

// process the "text" object
int textclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
	int newWid;
	int mode = 0;
	int tpos;
	hgt = 0;
	textstr = " ";
	string align = "";

	//outd << "In Text " << translator::line_ctr << endl;
	do {
		getline(inf,line);
		//outd << line << endl;
		translator::line_ctr++;
		pos = line.find(bopen,0);
		if(pos != -1)  open++; 
		pos = line.find(bclose,0);
        if(pos != -1)  open--; 

		eq_pos = line.find(eq,0);
        if(eq_pos != -1){
			line.replace(eq_pos,1,space);
			//outd << "Line after eq sub: " << line << endl;
			snum = sscanf(line.c_str(), "%s %s", s1,s2);
			if     (!strcmp(s1,"x"))  x = atoi(s2); 
			else if(!strcmp(s1,"y"))  y = atoi(s2); 

			// Why would medm write "wid = 2" in text within basic attrs? 	
			// For other graphic objects, it means linewidth.
			// We are ignoring it!
			else if(!strcmp(s1,"width"))  { 
				if(mode == 0)wid = atoi(s2);
			}
			// Likewise ignoring "fill" for text
			else if(!strcmp(s1,"fill"))  ; 
			else if(!strcmp(s1,"height"))  hgt = atoi(s2); 
			else if(!strcmp(s1,"clr")) {
				if(mode == 1)  clr = atoi(s2); 
				else if(strstr(s2, "static")!= 0x0) colormode = 0;
				else if(strstr(s2, "alarm")!= 0x0) colormode = 1;
				else if(strstr(s2, "discrete")!= 0x0) colormode = 2;
			}
			else if(!strcmp(s1,"textix")) { 
				textstr = string(line, eq_pos, std::string::npos);
			}

			// edm offers no alignment interactively on static text!
			// but fontAlign "center" works!
			else if(!strcmp(s1,"align")) {
               	if(strstr(line.c_str(), "horiz")!= 0x0) {
               		if(strstr(line.c_str(), "center")!= 0x0) align = "center"; 
               		else if(strstr(line.c_str(), "right")!= 0x0) align = "right";
               		else if(strstr(line.c_str(), "left")!= 0x0) align = "left";
				}
			}
			else if(strstr(s1, "vis")!= 0x0) {
               	if(strstr(line.c_str(), "static")!= 0x0) vis = 0;
               	else if(strstr(line.c_str(), "if not zero")!= 0x0) vis = 1;
               	else if(strstr(line.c_str(), "if zero")!= 0x0) vis = 2;
               	else if(strstr(line.c_str(), "calc")!= 0x0) vis = 3;
            }
            else if(!strcmp(s1,"calc")) calc = s2;
            else if(!strcmp(s1,"chan")) chan = s2;
			else if(!strcmp(s1,"chanB")) chanB = s2;
			else if(!strcmp(s1,"chanC")) chanC = s2;
			else if(!strcmp(s1,"chanD")) chanD = s2;
			else {
				outd << "Static Text Can't decode line: " << translator::line_ctr << endl;
				outd << line << endl;
			}
		} // end eq_pos
		else {
			if(strstr(line.c_str(), "basic") != 0x0) mode = 1;
			else if(strstr(line.c_str(), "dynamic") != 0x0) mode = 2;
			else if(strstr(line.c_str(), "object") != 0x0) mode = 0;
			else if(strstr(line.c_str(), bclose.c_str()) != 0x0) mode = 0;
		} 
	} while (open > 0);

    // Strip the quotes from the pv names when used for visibility pv
    if(vis) {
		stripQs(calc);
		stripQs(chan);
		stripQs(chanB);
		stripQs(chanC);
		stripQs(chanD);
		if(calc.length() == 0) calc = "A";
    }

	fptr = fi.bestFittingFont( hgt );
    if ( fptr ) {
        //outd << wid << endl;
        //outd << fi.textWidth( fptr, (char *) textstr.c_str() ) << endl;
        //outd << textstr << endl;

        if ( wid < fi.textWidth( fptr, (char *) textstr.c_str() ) ) {
            newWid = fi.textWidth( fptr, (char *) textstr.c_str() );
            if ( align == "center" ) {
                x = x - (int) ( ( newWid - wid ) * 0.5 ); 
            }
            else if ( align == "right" ) {
                x = x - newWid + wid;
            }
			wid = newWid; 
        }
        //outd << "\t" << wid << endl;
        //outd << "\t" << fi.textWidth( fptr, (char *) textstr.c_str() ) << endl;
        //outd << "\t" << textstr << endl;
    }

	outf << endl;
	outf << "# (Static Text)" << endl;
	outf << "object activeXTextClass" << endl;
	outf << "beginObjectProperties" << endl;
	outf << "major " << AXTC_MAJOR_VERSION << endl;
	outf << "minor " << AXTC_MINOR_VERSION << endl;
	outf << "release " << AXTC_RELEASE << endl;
	outf << "x " << x << endl;
	outf << "y " << y << endl;
	outf << "w " << wid<< endl;
	outf << "h " << hgt << endl;

	fptr = fi.bestFittingFont( hgt );
	if ( fptr ) 
		outf << "font \"" << fptr << "\"" << endl;
    else 
		outf << "font \"" << "helvetica-bold-r-12.0" << "\"" << endl;

	if(align.length())
		outf << "fontAlign \"" << align << "\"" << endl;

	if(!urgb) {
		clr = 14;	// Black-14
	}

	if(urgb) outf << "fgColor rgb " << cmap.getRGB(clr) << endl;
	else outf << "fgColor index " << clr << endl;
    if(colormode == 1) {        		
		outf << "fgAlarm" << endl;
	}
	outf << "bgColor index " << 3 << endl;	//Not available in medm
	outf << "useDisplayBg" << endl;

	outf << "value {" << endl;
	if ( textstr == " " ) 						// Handle empty text string
		outf << "  \"\"" << endl;
	 else 
		outf << " " << textstr << endl;
	outf << "}" << endl;

    if(colormode == 1) {        		
        outf << "alarmPv " << chan << endl;
        if(vis == 1)  outf << "visInvert" <<  endl;
	}
    if(vis != 0) {       			
        string tstr = "visPv \"CALC\\\\\\{(";
        string tstr2 = ")\\}(";
        outf <<  tstr << calc << tstr2 << chan ;
        if(chanB.length()) outf << ", " << chanB ;
        if(chanC.length()) outf << ", " << chanC ;
        if(chanD.length()) outf << ", " << chanD ;
        outf << ")\"" << endl;
        if(vis == 1 || vis == 3) outf << "visInvert" <<  endl;
        outf << "visMin 0" << endl;
        outf << "visMax 1" << endl;
    }

	// autoSize doesn't prevent clipping until the text is edited in edm.
	//outf << "autoSize" << endl;
	outf << "endObjectProperties" << endl;
	return 1;
}
