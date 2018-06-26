#include "translator.h"
#include "fonts.h"

using std::vector;
using std::endl;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;

extern fontInfoClass fi;
static char *fptr;

extern bool urgb;

tnode::tnode()
{
}

tnode::~tnode()
{
}

byteclass::byteclass(int attr)
{
	fill_attrs(attr);
	direction = 0;
	sbit = 0;
	ebit = 15;
}

byteclass::~byteclass()
{
}

// process the "byte" object
int byteclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
	int mode = 0;
	int style = 1;
	int numBits = 1;
	direction = 1;
	sbit = -1;
	ebit = -1;

	//outd << "In Byte " << translator::line_ctr << endl;
	do {
		getline(inf,line);
		translator::line_ctr++;
		pos = line.find(bopen,0);
		if(pos != -1) { open++; }
		pos = line.find(bclose,0);
        if(pos != -1) { open--; }

        eq_pos = line.find(eq,0);
        if(eq_pos != -1){
			line.replace(eq_pos,1,space);
			snum = sscanf(line.c_str(), "%s %s", s1,s2);
			if     (!strcmp(s1,"x"))  x = atoi(s2); 
			else if(!strcmp(s1,"y"))  y = atoi(s2); 
			else if(!strcmp(s1,"width"))  wid = atoi(s2); 
			else if(!strcmp(s1,"height"))  hgt = atoi(s2); 
			else if(!strcmp(s1,"clr"))  clr = atoi(s2); 
			else if(!strcmp(s1,"bclr"))  bclr = atoi(s2); 
			else if(!strcmp(s1,"rdbk")) { 
				chan = string(line, eq_pos+1, std::string::npos);
			}
			else if(!strcmp(s1,"chan")) { 
				chan = string(line, eq_pos+1, std::string::npos);
			}
			else if(!strcmp(s1,"ebit"))  ebit = atoi(s2); 
			else if(!strcmp(s1,"sbit"))  sbit = atoi(s2); 
			else if(!strcmp(s1,"direction")) { 
				if(!strcmp(s1,"right"))  direction = 0;
				else if(!strcmp(s1,"left"))  direction = 1;
				else if(!strcmp(s1,"up"))  direction = 1;
				else if(!strcmp(s1,"down"))  direction = 0;
			}
		}
		else {
			if(!strstr(line.c_str(),"object")) ;
			else if(!strstr(line.c_str(),"monitor")) ;
			else outd << "BYTE Can't decode:  " << line << endl;
		}
    } while (open > 0);

	if(sbit == -1) sbit = ebit;

	outf << endl;
    outf << "# (Byte)" << endl;
    outf << "object ByteClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << BYTE_MAJOR << endl;
    outf << "minor " << BYTE_MINOR << endl;
    outf << "release " << BYTE_RELEASE << endl;
    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;
	outf << "controlPv " << chan <<  endl;
	if(urgb) {
		outf << "lineColor rgb " << cmap.getRGB(14) << endl;
		outf << "onColor rgb " << cmap.getRGB(clr) << endl;
		outf << "offColor rgb " << cmap.getRGB(bclr) << endl;
	} else {
		outf << "lineColor index " << 14 << endl;
		outf << "onColor index " << clr << endl;
		outf << "offColor index " << bclr << endl;
	}
	if(sbit > ebit ){
		outf << "endian \"big\"" << endl; 	
		outf << "numBits " << sbit - ebit + 1 << endl; 	
		outf << "shift " << ebit << endl; 	// shft
	} else {
		outf << "endian \"little\"" << endl; 	
		outf << "numBits " << ebit - sbit + 1 << endl; 	
		outf << "shift " << sbit << endl; 	// shft
	}
	outf << "endObjectProperties" << endl;
	return 1;
}

xyclass::xyclass(int attr)
{
	fill_attrs(attr);
	squote = "\"";
	nil = "";
	bgColor = 0;
	plotMode = 1; 	//0=PLOT_MODE_PLOT_N_STOP, 1=PLOT_MODE_PLOT_LAST_N
	border = 1;
	count = 1;
	updateTimerValue = 100;
	xAxis = 1; 		//0=don't show, 1= show
    xAxisStyle = 0; //0=linear, 1=log, 2=time, 3=timelog (0)
    xAxisSource = 1;
    xMin1 = 16;
    xMin2 = 0;
    xMax1 = 24;
    xMax2 = 0;
    xAxisTimeFormat = 0;
	symbol = 1;
}

xyclass::~xyclass()
{
}

// process the "cartesian" object
int xyclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
	int open = 1;
	int mode = 0;
	int style = 1;
	int trace_ct = 0;

	vector <tnode> tracelist;
	tnode *tn;
	bool node = false;
	bool no_x = false;

	// axes stuff
	int num_axes;
	int range_x = 0;
	int range_y = 0;
	int range_y1 = 0;
	double minx = -9.0;
	double maxx = 9.0;
	double miny = -9.0;
	double maxy = 9.0;
	double miny1 = -999.0;
	double maxy1 = -999.0;
	string tstr;
	int i;
	string bclose("}");

	//outd << "In XY " << translator::line_ctr << endl;
	do {
		getline(inf,line);
		translator::line_ctr++;
		pos = line.find(bopen,0);
		if(pos != -1)  open++; 
		pos = line.find(bclose,0);
        if(pos != -1)  open--; 

        //outd << "L: " << line << endl;
        eq_pos = line.find(eq,0);
        if(eq_pos != -1){
			line.replace(eq_pos,1,space);
			snum = sscanf(line.c_str(), "%s %s", s1,s2);
			//outd << "\t" <<  s1 << " " << s2 << endl;
			if     (!strcmp(s1,"x"))  x = atoi(s2); 
			else if(!strcmp(s1,"y"))  y = atoi(s2); 
			else if(!strcmp(s1,"width"))  wid = atoi(s2); 
			else if(!strcmp(s1,"height"))  hgt = atoi(s2); 
			else if(!strcmp(s1,"clr"))  clr = atoi(s2); 
			else if(!strcmp(s1,"bclr"))  bgColor = atoi(s2); 
			else if(!strcmp(s1,"count")) { 
				int tpos;
				while((tpos = line.find(squote,0)) != -1) {
					line.replace(tpos,1,nil);
				}
				tstr = string(line, eq_pos+1, (std::string::npos));
				count = atoi(tstr.c_str());
			}
			else if(!strcmp(s1,"erase_oldest")) { 
				if(strstr(s2, "last") != 0x0) erase = 1;
				else erase = 0;
			}
			else if(!strcmp(s1,"style")) { 
				if(strstr(s2, "line") != 0x0) style = 1;
				else style = 0; 
			}
			// axis stuff

			else if(!strcmp(s1,"rangeStyle")) { 
				if(strstr(s2, "last") != 0x0) {
					if(mode == 1)range_x = 1;
					else if(mode == 2)range_y = 1;
					else range_y1 = 1;
				}
				else if(strstr(s2, "auto") != 0x0) {
					if(mode == 1)range_x = 2;
                       else if(mode == 2)range_y = 2;
                       else range_y1 = 2;
                   }
			}

			else if(!strcmp(s1,"minRange")) { 
				if(mode == 1)  minx = strtod(s2, (char **)NULL); 
				else if(mode == 2)  miny = strtod(s2, (char **)NULL); 
				else miny1 = strtod(s2, (char **)NULL); 
				//outd << "X mode: " << mode << " " << strtod(s2, (char **)NULL) << endl;
			}
			else if(!strcmp(s1,"maxRange")) { 
				if(mode == 1)  maxx = strtod(s2, (char **)NULL); 
				else if(mode == 2)  maxy = strtod(s2, (char **)NULL); 
				else maxy1 = strtod(s2, (char **)NULL); 
				//outd << "Y mode: " << mode << " " << strtod(s2, (char **)NULL) << endl;
			}

			else if(!strcmp(s1,"title"))  
				title = string(line, eq_pos+1, std::string::npos);
			else if(!strcmp(s1,"xlabel"))  
				xlabel = string(line, eq_pos+1, std::string::npos);
			else if(!strcmp(s1,"ylabel"))  
				ylabel = string(line, eq_pos+1, std::string::npos);

			//trace stuff
			else if(mode == 4 && !strcmp(s1,"xdata")) { 
				if(node == false){
					tn = new tnode();
					node = true;
				}
				tn->xdata = string(line, eq_pos+1, std::string::npos);
			}
			else if(mode == 4 && !strcmp(s1,"ydata")) { 
				if(node == false){
					tn = new tnode();
					node = true;
					no_x = true;
				}
				tn->ydata = string(line, eq_pos+1, std::string::npos);
			}
			else if(mode == 4 && !strcmp(s1,"data_clr")) { 
				tn->color = atoi(s2);
				tracelist.push_back(*tn);
				trace_ct++;
				delete tn;
				node = false;
			}
		} // end of has '=' sign section

		else if(strstr(line.c_str(), "x_axis") != 0x0) mode = 1;
		else if(strstr(line.c_str(), "y1_axis") != 0x0) mode = 2;
		else if(strstr(line.c_str(), "y2_axis") != 0x0) mode = 3;
		else if(strstr(line.c_str(), "trace") != 0x0) mode = 4;
		else if(strstr(line.c_str(), bclose.c_str()) != 0x0)  mode = 0; 
		else if(strstr(line.c_str(), "object") != 0x0) ; // ignore
		else if(strstr(line.c_str(), "plotcom") != 0x0) ; // ignore

		else outd << "cartesian X-Y Graph Can't decode:  " << line << endl;
    } while (open > 0);
	outf << endl;
    outf << "# (X-Y Graph)" << endl;
    outf << "object xyGraphClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << XYGC_MAJOR_VERSION << endl;
    outf << "minor " << XYGC_MINOR_VERSION << endl;
    outf << "release " << XYGC_RELEASE << endl;
    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;

    outf << "# Appearance" << endl;
    outf << "border" << endl;
    outf << "graphTitle " << title << endl;
    outf << "xLabel " << xlabel << endl;
    outf << "yLabel " << ylabel << endl;
	if(urgb) {
		outf << "fgColor rgb " << cmap.getRGB(clr) << endl;
		outf << "bgColor rgb " << cmap.getRGB(bgColor) << endl;
		outf << "gridColor rgb " << cmap.getRGB(clr) << endl;
	} else {
		outf << "fgColor index " << clr << endl;
		outf << "bgColor index " << bgColor << endl;
		outf << "gridColor index " << clr << endl;
	}

	fptr = fi.bestFittingFont( 12 );
    if ( fptr ) {
      outf << "font \"" << fptr << "\"" << endl;
    } else {
      outf << "font \"" << "helvetica-bold-r-12.0" << "\"" << endl;
    }

    outf << "# Operating Modes" << endl;
	outf << "plotMode \"plotLastNPts\"" << endl;
    outf << "nPts " << count << endl;

    outf << "#  X axis properties" << endl;
    outf << "showXAxis" << endl;
	outf << "xAxisSrc \"AutoScale\"" << endl;
	if(range_x == 0) {
    	outf << "xMin " << minx << endl;
    	outf << "xMax " << maxx << endl;
	}

	// spelling in edm?
    outf << "xLablePrecision 1" << endl;

    outf << "#  Y axis properties" << endl;
    outf << "showYAxis" << endl;
	outf << "yAxisSrc \"AutoScale\"" << endl;
	if(range_y == 0) {
		outf << "yMin " << miny << endl;
		outf << "yMax " << maxy << endl;
	}
    //outf << "yLabelPrecision 1" << endl;

	if(miny1 != -999 && maxy1 != -999) {
		outf << "#  Y2 axis properties" << endl;
		outf << "showY2Axis" << endl;
		outf << "y2AxisSrc \"AutoScale\"" << endl;
		if(range_y1 == 0) {
			outf << "y2Min " << miny1 << endl;
			outf << "y2Max " << maxy1 << endl;
		}
	}

    outf << "#  Trace Properties" << endl;
    outf << "numTraces " << trace_ct << endl;

	if(!no_x) {
		outf << "xPv {" << endl;
		for(i = 0; i < trace_ct; i++)
			outf << "  " << i << " " << tracelist[i].xdata << endl;
		outf << "}" << endl;
	}
	
	outf << "yPv {" << endl;
	for(i = 0; i < trace_ct; i++)
		outf << "  " << i << " " << tracelist[i].ydata << endl;
	outf << "}" << endl;
	
	outf << "plotStyle {" << endl;
	for(i = 0; i < trace_ct; i++)
		if(style == 0)
			outf << "  " << i << " " << "\"point\"" << endl;
		else if(style == 1)
			outf << "  " << i << " " << "\"line\"" << endl;
	outf << "}" << endl;

	outf << "plotUpdateMode {" << endl;
	for(i = 0; i < trace_ct; i++)
		outf << "  " << i << " " << "\"x\"" << endl;
	outf << "}" << endl;
	
	outf << "plotSymbolType {" << endl;
	for(i = 0; i < trace_ct; i++)
		outf << "  " << i << " " << "\"circle\"" << endl;
	outf << "}" << endl;
	
	outf << "plotColor {" << endl;
	for(i = 0; i < trace_ct; i++) {
		if(urgb) {
			string tstr = cmap.getRGB(tracelist[i].color);
			outf << "  " << i << " rgb " << tstr << endl;
		} else {
			outf << "  " << i << " index " << tracelist[i].color << endl;
		}
	}
	outf << "}" << endl;
	
	outf << "endObjectProperties" << endl;
	return 1;
}


stripclass::stripclass(int attr)
{
	fill_attrs(attr);
	bgColor = 0;
	plotMode = 1; 	//0=PLOT_MODE_PLOT_N_STOP, 1=PLOT_MODE_PLOT_LAST_N
	border = 1;
	count = 200;
	updateTimerValue = 100;
	xAxis = 1; 		//0=don't show, 1= show
    xAxisStyle = 0; //0=linear, 1=log, 2=time, 3=timelog (0)
    xAxisSource = 1;
    xMin1 = 16;
    xMin2 = 0;
    xMax1 = 24;
    xMax2 = 0;
    xAxisTimeFormat = 0;
	symbol = 1;
}

stripclass::~stripclass()
{
}

// process the "cartesian" object
int stripclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
	int open = 1;
	int mode = 0;
	int style = 1;
	int trace_ct = 0;

	vector <tnode> tracelist;
	tnode *tn;

	// axes stuff
	int num_axes;
	int range_x = 0;
	int range_y = 0;
	int range_y1 = 0;
	double minx = 0.0;
	double maxx = 1.0;
	double miny = 0.0;
	double maxy = 1.0;
	double miny1 = 0.0;
	double maxy1 = 1.0;
	string tstr;
	int i;

	//outd << "In Strip " << translator::line_ctr << endl;
	do {
		getline(inf,line);
		//outd << "X" << line << "X" << endl;
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
			else if(!strcmp(s1,"width"))  wid = atoi(s2); 
			else if(!strcmp(s1,"height"))  hgt = atoi(s2); 
			else if(!strcmp(s1,"clr")) {
				if(mode == 0) {
					clr = atoi(s2); 
				} else {
					tn->color = atoi(s2);
					tracelist.push_back(*tn);
					trace_ct++;
					delete tn;
					mode = 0;
				}
			}
			else if(!strcmp(s1,"bclr"))  bgColor = atoi(s2); 

			// axis stuff
			else if(!strcmp(s1,"rangeStyle")) { 
				if(strstr(s2, "last") != 0x0) {
					if(mode == 1)range_x = 1;
					else if(mode == 2)range_x = 1;
					else range_y1 = 1;
				}
				else if(strstr(s2, "auto") != 0x0) {
					if(mode == 1)range_x = 2;
                       else if(mode == 2)range_y = 2;
                       else range_y1 = 2;
                }
			}
			else if(!strcmp(s1,"minRange")) { 
				if(mode == 1)  minx = strtod(s2, (char **)NULL); 
				else if(mode == 2)  miny = strtod(s2, (char **)NULL); 
				else miny1 = strtod(s2, (char **)NULL); 
			}
			else if(!strcmp(s1,"maxRange")) { 
				if(mode == 1)  maxy = strtod(s2, (char **)NULL); 
				else if(mode == 2)  maxy = strtod(s2, (char **)NULL); 
				else maxy1 = strtod(s2, (char **)NULL); 
			}

			else if(!strcmp(s1,"title"))  
				title = string(line, eq_pos+1, std::string::npos);
			else if(!strcmp(s1,"xlabel"))  
				xlabel = string(line, eq_pos+1, std::string::npos);
			else if(!strcmp(s1,"ylabel"))  
				ylabel = string(line, eq_pos+1, std::string::npos);

			else if(mode == 1 && !strcmp(s1,"chan")) { 
				tn->xdata = string(line, eq_pos+1, std::string::npos);
			}
		}// end of has '=' sign section

		else if(strstr(line.c_str(), "object") != 0x0) ;// ignore
		else if(strstr(line.c_str(), "plotcom") != 0x0) ;// ignore
		else if(strstr(line.c_str(), "}") != 0x0) ;// ignore
		else if(strstr(line.c_str(), "pen") != 0x0) {
			mode = 1;
			tn = new tnode();
		}
		else {
			outd << "strip X-Y Graph Can't decode:  " << line << endl;
		}
    } while (open > 0);

	outf << endl;
    outf << "# (X-Y Graph)" << endl;
    outf << "object xyGraphClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << XYGC_MAJOR_VERSION << endl;
    outf << "minor " << XYGC_MINOR_VERSION << endl;
    outf << "release " << XYGC_RELEASE << endl;

	outf << "# Geometry" << endl;
    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;

    outf << "# Appearance" << endl;
    outf << "border" << endl;
    outf << "plotAreaBorder" << endl;
    outf << "graphTitle " << title << endl;
    outf << "xLabel " << xlabel << endl;
    outf << "yLabel " << ylabel << endl;
	if(urgb) {
		outf << "fgColor rgb " << cmap.getRGB(clr) << endl;
		outf << "bColor rgb " << cmap.getRGB(bgColor) << endl;
		outf << "gridColor rgb " << cmap.getRGB(clr) << endl;
	} else {
		outf << "fgColor index " << clr << endl;
		outf << "bgColor index " << bgColor << endl;
		outf << "gridColor index " << clr << endl;
	}

	fptr = fi.bestFittingFont( 25 );
    if ( fptr ) {
      outf << "font \"" << fptr << "\"" << endl;
    }
    else {
      outf << "font \"" << "helvetica-medium-r-18.0" << "\"" << endl;
    }

    outf << "# Operating Modes" << endl;
	outf << "plotMode \"plotLastNPts\"" << endl;
    outf << "nPts " << count << endl;

    outf << "#  X axis properties" << endl;
    //outf << "showXAxis" << endl;
	outf << "xAxisSrc \"AutoScale\"" << endl;
    //outf << "xMin " << minx << endl;
    outf << "xMax " << maxx << endl;

	// spelling in edm?
    outf << "xLablePrecision 1" << endl;

    outf << "#  Y axis properties" << endl;
    outf << "showYAxis" << endl;
	outf << "yAxisSrc \"AutoScale\"" << endl;
    //outf << "yMin " << miny << endl;
    outf << "yMax " << maxy << endl;
    //outf << "yLabelPrecision 1" << endl;

    outf << "#  Y2 axis properties" << endl;
    outf << "showY2Axis" << endl;
	outf << "y2AxisSrc \"AutoScale\"" << endl;
    //outf << "y2Min " << miny << endl;
    outf << "y2Max " << maxy << endl;

    outf << "#  Trace Properties" << endl;
    outf << "numTraces " << trace_ct << endl;

	outf << "yPv {" << endl;

	for(i = 0; i < trace_ct; i++){
		outf << "  " << i << " " << tracelist[i].xdata << endl;
	}

	outf << "}" << endl;

	outf << "plotUpdateMode {" << endl;
	for(i = 0; i < trace_ct; i++)
		outf << "  " << i << " " << "\"y\"" << endl;
	outf << "}" << endl;
	
	outf << "opMode {" << endl;
	for(i = 0; i < trace_ct; i++)
		outf << "  " << i << " " << "\"plot\"" << endl;
	outf << "}" << endl;
	
	outf << "plotColor {" << endl;
	for(i = 0; i < trace_ct; i++) {
		if(urgb) {
			string tstr = cmap.getRGB(tracelist[i].color);
			outf << "  " << i << " rgb " << tstr << endl;
		} else {
			outf << "  " << i << " index " << tracelist[i].color << endl;
		}
	}
	outf << "}" << endl;
	
	outf << "endObjectProperties" << endl;
	return 1;
}

barclass::barclass(int attr)
{
	fill_attrs(attr);
}

barclass::~barclass()
{
}

// process the "bar" object
int barclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
	bool vert = false;
	bool center = false;
	int label_type;
	string prec;
	string max;
	string min;

	//outd << "In Bar " << translator::line_ctr << endl;

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
			else if(!strcmp(s1,"width"))  wid = atoi(s2); 
			else if(!strcmp(s1,"height"))  hgt = atoi(s2); 
			else if(!strcmp(s1,"clr"))  clr = atoi(s2); 
			else if(!strcmp(s1,"bclr"))  bgColor = atoi(s2); 
			else if(!strcmp(s1,"label")) { 
				if(strstr(line.c_str(), "none")!= 0x0) label_type = 0;
				else if(strstr(line.c_str(), "no decorations")!= 0x0) label_type = 0;
				else if(strstr(line.c_str(), "outline")!= 0x0) label_type = 1;
				else if(strstr(line.c_str(), "limits")!= 0x0) label_type = 2;
				else if(strstr(line.c_str(), "channel")!= 0x0) label_type = 3;
				//label = string(line, eq_pos+1, std::string::npos);
			}
			else if(!strcmp(s1,"chan")||!strcmp(s1,"rdbk")) { 
				chan = string(line, eq_pos+1, std::string::npos);
			}
			else if(!strcmp(s1,"direction")) {
				if(strstr(line.c_str(), "up")!= 0x0) vert = true;
				else if(strstr(line.c_str(), "down")!= 0x0) vert = true;
				else if(strstr(line.c_str(), "right")!= 0x0) vert = false;
				else if(strstr(line.c_str(), "left")!= 0x0) vert = false;
			}
            else if(!strcmp(s1,"clrmod")) { 
            	if(strstr(s2, "static") != 0x0) colormode = 0;
            	else if(strstr(s2, "alarm") != 0x0) colormode = 1;
            	else if(strstr(s2, "discrete") != 0x0) colormode = 2;
			}
			else if(!strcmp(s1,"fillmod")) {
				if(strstr(line.c_str(), "center")!= 0x0) center = true;
			}
			else if(!strcmp(s1,"precSrc")) {
                ; // ignore
            }
			else if(!strcmp(s1,"hoprSrc")) {
                ; // ignore
            }
			else if(!strcmp(s1,"loprSrc")) {
                ; // ignore
            }
			else if(!strcmp(s1,"loprDefault")) {
                min = s2;; 
            }
			else if(!strcmp(s1,"hoprDefault")) {
                max = s2;; 
            }
            else if(!strcmp(s1,"precDefault")) {
                prec = s2;
            }

			else {
				outd << "Bar Can't decode text " << line << endl;
			}
		} else {
			if(strstr(line.c_str(), "object")!= 0x0) ; //ignore
			else if(strstr(line.c_str(), "monitor")!= 0x0) ; //ignore
		}

	} while (open > 0);

	outf << "# (Bar)" << endl;
	outf << "object activeBarClass" << endl;
	outf << "beginObjectProperties" << endl;
	outf << "major " << BARC_MAJOR_VERSION << endl;
	outf << "minor " << BARC_MINOR_VERSION << endl;
	outf << "release " << BARC_RELEASE << endl;
	outf << "x " << x << endl;
	outf << "y " << y << endl;
	outf << "w " << wid<< endl;
	outf << "h " << hgt << endl;

	if(center)
		outf << "origin 0" << endl;
	if(urgb) {
		outf << "indicatorColor rgb " << cmap.getRGB(clr) << endl;
		outf << "fgColor rgb " << cmap.getRGB(14) << endl;
		outf << "bgColor rgb " << cmap.getRGB(bgColor) << endl;
	} else {
		outf << "indicatorColor index " << clr << endl;
		outf << "fgColor index " << 14 << endl;
		outf << "bgColor index " << bgColor << endl;
	}
    if(colormode == 1) {   
        outf << "fgAlarm" << endl;
        outf << "indicatorAlarm" << endl;
	}
	outf << "indicatorPv " << chan << endl;	
	if(label_type == 1 || label_type == 2)
		outf << "showScale" << endl;

	fptr = fi.bestFittingFont( 8 );
	if ( fptr ) {
		outf << "font \"" << fptr << "\"" << endl;
	} else {
		outf << "font \"" << "helvetica-bold-r-10.0" << "\"" << endl;
	}

	outf << "labelTicks 1" << endl;
	outf << "majorTicks 10" << endl;
	outf << "minorTicks 0" << endl;
	// It would be nice to use "border" in some cases when type=0;
	if(label_type == 1)
		outf << "border" << endl;
	if(label_type == 1)
		outf << "limitsFromDb" << endl;
	if(prec.length())
		outf << "precision " << prec << endl;
	if(min.length())
		outf << "min " << min << endl;
	if(max.length())
		outf << "max " << max << endl;
	outf << "border" << endl;
	outf << "scaleFormat \"FFloat\"" << endl;
	if(vert)
		outf << "orientation \"vertical\"" << endl;
	else
		outf << "orientation \"horizontal\"" << endl;
	outf << "endObjectProperties" << endl;
	return 1;
}

meterclass::meterclass(int attr)
{
	fill_attrs(attr);
}

meterclass::~meterclass()
{
}

// process the "meter" object
int meterclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
	string min;
	string max;
	//outd << "In Meter " << translator::line_ctr << endl;
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
			else if(!strcmp(s1,"width"))  wid = atoi(s2); 
			else if(!strcmp(s1,"height"))  hgt = atoi(s2); 
			else if(!strcmp(s1,"clr"))  clr = atoi(s2); 
			else if(!strcmp(s1,"bclr"))  bgColor = atoi(s2); 
			else if(!strcmp(s1,"label"))  
			label = string(line, eq_pos+1, std::string::npos);
			else if(!strcmp(s1,"chan"))  
				chan = string(line, eq_pos+1, std::string::npos);
			else if(!strcmp(s1,"loprDefault")) {
                min = s2;;
            }
            else if(!strcmp(s1,"hoprDefault")) {
                max = s2;;
            }
			else if(!strcmp(s1,"hoprSrc"))  ; // ignore 
			else if(!strcmp(s1,"loprSrc"))  ; // ignore 

			else 
				outd << "Meter Can't decode text " << line << endl;
		}
	} while (open > 0);

    outf << "# (Meter)" << endl;
    outf << "object activeMeterClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << METERC_MAJOR_VERSION << endl;
    outf << "minor " << METERC_MINOR_VERSION << endl;
    outf << "release " << METERC_RELEASE << endl;
    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;

	if(urgb) {
		outf << "caseColor rgb " << cmap.getRGB(bgColor) << endl;
		outf << "sccaleColor rgb " << cmap.getRGB(clr) << endl;
		outf << "labelColor rgb " << cmap.getRGB(clr) << endl;
		outf << "fgColor rgb " << cmap.getRGB(clr) << endl;
		outf << "bgColor rgb " << cmap.getRGB(bgColor) << endl;
		outf << "topShadowColor rgb " << cmap.getRGB(clr) << endl;
		outf << "botShadowColor rgb " << cmap.getRGB(clr) << endl;
	} else {
		outf << "caseColor index " << bgColor << endl;
		outf << "scaleColor index " << clr << endl;
		outf << "labelColor index " << clr << endl;
		outf << "fgColor index " << clr << endl;
		outf << "bgColor index " << bgColor << endl;
		outf << "topShadowColor index " << clr << endl;
		outf << "botShadowColor index " << clr << endl;
	}
	outf << "readPv " << chan << endl;
	outf << "showScale " << endl;
	outf << "scaleFormat \"FFloat\"" << endl;
	if(min != "" || max != "") {
		outf << "scaleMin " << min << endl;
		outf << "scaleMax " << max << endl;
	} else {
		outf << "scaleLimitsFromDb " << endl;
	}
	outf << "useDisplayBg " << endl;
	outf << "labelIntervals " << 2 << endl;
	outf << "majorIntervals " << 2 << endl;
	outf << "minorIntervals " << 0 << endl;
	outf << "complexNeedle " << endl;
	outf << "3d " << endl;

	fptr = fi.bestFittingFont( 25 );
	if ( fptr ) {
		outf << "labelFontTag \"" << fptr << "\"" << endl;
	} else {
		outf << "labelFontTag \"" << "helvetica-medium-r-18.0" << "\"" << endl;
	}

	fptr = fi.bestFittingFont( 8 );
	if ( fptr ) {
		outf << "scaleFontTag \"" << fptr << "\"" << endl;
	} else {
		outf << "scaleFontTag \"" << "helvetica-bold-r-10.0" << "\"" << endl;
	}

	outf << "endObjectProperties" << endl;
	return 1;
}


textmonclass::textmonclass(int attr)
{
	fill_attrs(attr);
	format = "decimal";
}

textmonclass::~textmonclass()
{
}


// process the "text entry" and "text update" object
// adl: text entry, edl: control   editable
// adl: text update, edl: monitor  
int textmonclass::parse(ifstream &inf, ostream &outf, ostream &outd, int edit)
{
    int mode = 0;
	string align = "";
	string prec;

	if ( ! edit ) {
		// TextMonitor's should default to alarm sensitive if not specified
		colormode = 1;
	}

	//outd << "In Textmoner & Textupdte " << translator::line_ctr << endl;
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
            else if(!strcmp(s1,"width"))  wid = atoi(s2);
            else if(!strcmp(s1,"height"))  hgt = atoi(s2); 
            else if(!strcmp(s1,"clr"))  clr = atoi(s2); 
            else if(!strcmp(s1,"bclr"))  bclr = atoi(s2); 
			else if(!strcmp(s1,"align")) {
				if(strstr(line.c_str(), "horiz")!= 0x0) {
					if(strstr(line.c_str(), "center")!= 0x0) align = "center";
					else if(strstr(line.c_str(), "right")!= 0x0) align = "right";
					else if(strstr(line.c_str(), "left")!= 0x0) align = "left";
					}
				}
            else if(!strcmp(s1,"chan") || !strcmp(s1,"ctrl") || !strcmp(s1,"rdbk")) { 
                chan = string(line, eq_pos+1, std::string::npos);
			}
            else if(!strcmp(s1,"clrmod")) { 
            	if(strstr(s2, "static") != 0x0) colormode = 0;
            	else if(strstr(s2, "alarm") != 0x0) colormode = 1;
            	else if(strstr(s2, "discrete") != 0x0) colormode = 2;
			}
            else if(!strcmp(s1,"format")) { 
				format = s2;
			}
            else if(!strcmp(s1,"precSrc")) { 
				; // ignore
			}
            else if(!strcmp(s1,"precDefault")) { 
				prec = s2;
			}
            else 
                outd << "Text Control Can't decode line " << line << endl;
        }
        else {
            if(strstr(line.c_str(), "basic") != 0x0) mode = 1;
            else if(strstr(line.c_str(), "dynamic") != 0x0) mode = 2;
            else if(strstr(line.c_str(), "object") != 0x0) mode = 0;
            else if(strstr(line.c_str(), "}") != 0x0) mode = 0;
        }
    } while (open > 0);

	outf << endl;
	if ( edit ) {
		outf << "# (Text Control)" << endl;
		outf << "object activeXTextDspClass" << endl;
	}
	else {
		outf << "# (Text Monitor)" << endl;
		outf << "object activeXTextDspClass:noedit" << endl;
	}
	outf << "beginObjectProperties" << endl;

    outf << "major " << XTDC_MAJOR_VERSION  << endl;
    outf << "minor " << XTDC_MINOR_VERSION  << endl;
    outf << "release " << XTDC_RELEASE << endl;
    outf << "x " << x << endl;


    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;
    outf << "controlPv " << chan << endl;
	if(format == "\"truncated\""){
    	outf << "format \"decimal\"" << endl;
	}
	else if(format == "\"hexadecimal\""){
    	outf << "format \"hex\"" << endl;
	}
	else if(format == "\"compact\""){
    	outf << "format \"decimal\"" << endl;
	}
	else if ( format.length() >= 2 && format[0] == '"' && format[format.length()-1] == '"' ) {
    	outf << "format " << format << endl;
	} else {
    	outf << "format \"" << format << "\"" << endl;
	}

	if(!edit) {
		fptr = fi.bestFittingFont( hgt );
		if ( fptr ) {
			outf << "font \"" << fptr << "\"" << endl;
		} else {
			outf << "font \"" << "helvetica-bold-r-12.0" << "\"" << endl;
		}
	} else {
		fptr = fi.bestFittingFont( (int) (hgt-.3*hgt) );
		if ( fptr ) {
			outf << "font \"" << fptr << "\"" << endl;
		} else {
			outf << "font \"" << "helvetica-bold-r-12.0" << "\"" << endl;
		}
	}
	if(align.length())
        outf << "fontAlign \"" << align << "\"" << endl;
	else
        outf << "fontAlign \"center\"" << endl;

	if(!urgb) {
		if(edit) {
			clr  = 25;
			bclr = 5;
		} else {
			clr  = 15;
			bclr = 12;
		}
	}
	if(urgb) outf << "fgColor rgb " << cmap.getRGB(clr) << endl;
    else outf << "fgColor index " << clr << endl;
    if(colormode == 1) {   
        outf << "fgAlarm" << endl;
	}
	if(urgb) outf << "bgColor rgb " << cmap.getRGB(bclr) << endl;
    else outf << "bgColor index " << bclr << endl;
	if(edit) outf << "editable" << endl;
	// outf << "autoHeight" << endl;
	if(edit) outf << "motifWidget" << endl;
	if(format == "\"truncated\""){
		outf << "precision 0" << endl;
	}
	else if(format == "\"compact\""){
		outf << "precision 2" << endl;
	} else if(format != "\"hexadecimal\"" && prec.length()) {
		outf << "precision " << prec << endl;
	} else {
		outf << "limitsFromDb" << endl;
	}
	if(urgb) outf << "nullColor rgb " << cmap.getRGB(32) << endl;
	else outf << "nullColor index 32" << endl;
	// outf << "useHexPrefix" << endl;
	outf << "smartRefresh" << endl;
	outf << "fastUpdate" << endl;
	outf << "newPos" << endl;
	outf << "objType \"controls\"" << endl;
	outf << "endObjectProperties" << endl;
    return 1;
}

