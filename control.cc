
#include "translator.h"
#include "fonts.h"

#ifdef JLAB
#define JLAB true
#else
#define JLAB false
#endif

using std::endl;
using std::cout;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::string;

extern fontInfoClass fi;
static char *fptr;

extern bool urgb;
extern bool retitle;

buttonclass::buttonclass(int attr)
{
	fill_attrs(attr);
	bclr = 0;
	open = 1;
}

buttonclass::~buttonclass()
{
}

// process the "choice button" object
int buttonclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{

	bool stacking = 0;

	colormode = 0;
	//outd << "In ChoiceButton "  << translator::line_ctr << endl;
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
			else if(!strcmp(s1,"chan")||!strcmp(s1,"ctrl"))  {
				chan = string(line, eq_pos+1, std::string::npos);;
			}
			else if(!strcmp(s1,"stacking"))  stacking = 1;
			else if(!strcmp(s1,"clrmod")) {
                if(strstr(s2, "static") != 0x0) colormode = 0;
                else if(strstr(s2, "alarm") != 0x0) colormode = 1;
                else if(strstr(s2, "discrete") != 0x0) colormode = 2;
            }
			else outd << "Choice Button Can't decode " << line << endl;
		}
    } while (open > 0);

	outf << endl;
    outf << "# (Choice Button)" << endl;
    outf << "object activeChoiceButtonClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << BTC_MAJOR_VERSION << endl;
    outf << "minor " << BTC_MINOR_VERSION << endl;
    outf << "release " << BTC_RELEASE << endl;
    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;
	if(urgb) {
		if(colormode == 1) {   
			// medm shows fg in green for alarm colormode
			// edm needs to have fgColor set to green for same behavior
			outf << "fgColor rgb " << "0 65535 0" << endl;
		} else {
			outf << "fgColor rgb " << cmap.getRGB(clr) << endl;
		}
		outf << "bgColor rgb " << cmap.getRGB(bclr) << endl;
		outf << "selectColor rgb " << cmap.getRGB(bclr) << endl;
		outf << "inconsistentColor rgb " << cmap.getRGB(clr) << endl;
		outf << "topShadowColor rgb " << cmap.getRGB(0) << endl;
		outf << "botShadowColor rgb " << cmap.getRGB(14) << endl;
	} else {
		if(colormode == 1) {   
			// medm shows fg in green for alarm colormode
			// edm needs to have fgColor set to green for same behavior
			outf << "fgColor index " << 15 << endl;
		} else {
			outf << "fgColor index " << clr << endl;
		}
		outf << "bgColor index " << bclr << endl;
		outf << "selectColor index " << bclr << endl;
		outf << "inconsistentColor index " << clr << endl;
		outf << "topShadowColor index " << 0 << endl;
		outf << "botShadowColor index " << 14 << endl;
	}
	if(colormode == 1) {
        outf << "fgAlarm" << endl;
    }

	outf << "controlPv " << chan <<  endl;
    //outf << "indicatorPv " << chan << endl;
    //outf << "labelType \"pvState\"" << endl;
    //outf << "3d" << endl;

	fptr = fi.bestFittingFont( 12 );
    if ( fptr ) {
    	outf << "font \"" << fptr << "\"" << endl;
    } else {
    	outf << "font \"" << "helvetica-bold-r-12.0" << "\"" << endl;
    }

    //outf << "objType \"controls\"" << endl;
	if(stacking)
    	outf << "orientation \"horizontal\""  << endl;
		
    outf << "endObjectProperties" << endl;

	return 1;
}

valclass::valclass(int attr)
{
	fill_attrs(attr);
}

valclass::~valclass()
{
}

// process the "valuator button" object
int valclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
	string dPrecision;
	bool use_limits = false;

	string hoprSrc;
	string hoprDefault("-9");
	string loprSrc;
	string loprDefault("-9");

	string precSrc;
	string precDefault("1");

	int direction = 0;
	colormode = 0;

	//outd << "In Valuator "  << translator::line_ctr << endl;
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
			else if((!strcmp(s1,"chan")) || (!strcmp(s1,"ctrl")) )
				chan = string(line, eq_pos+1, std::string::npos);
			else if(!strcmp(s1,"dPrecision"))  dPrecision = s2;
			else if(!strcmp(s1,"hoprSrc"))  hoprSrc = s2;
			else if(!strcmp(s1,"hoprDefault"))  hoprDefault = s2;
			else if(!strcmp(s1,"loprSrc"))  loprSrc = s2;
			else if(!strcmp(s1,"loprDefault"))  loprDefault = s2;
			else if(!strcmp(s1,"precSrc"))  precSrc = s2;
			else if(!strcmp(s1,"precDefault"))  precDefault = s2;
			else if(!strcmp(s1,"label"))  {
				if(strstr(s2,"limits")) use_limits = true;
			}
			else if(!strcmp(s1,"direction"))  {
				if(!strcmp(s1,"right"))  direction = 0;
                else if(!strcmp(s1,"left"))  direction = 0;
                else if(!strcmp(s1,"up"))  direction = 1;
                else if(!strcmp(s1,"down"))  direction = 1;
			}
			else if(!strcmp(s1,"clrmod"))  {
				// JS Not clear on how to convert this.
				// What causes this to alarm in medm?
				if(strstr(s2,"alarm")) colormode = 1;
			}
			else outd << "Motif Slider Can't decode " << line << endl;
		}
	} while (open > 0);

    outf << endl;
    outf << "# (Motif Slider)" << endl;
    outf << "object activeMotifSliderClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << BTC_MAJOR_VERSION << endl;
    outf << "minor " << BTC_MINOR_VERSION << endl;
    outf << "release " << BTC_RELEASE << endl;
    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;

	if(urgb) {
		outf << "fgColor rgb " << cmap.getRGB(bclr+1) << endl;
		outf << "bgColor rgb " << cmap.getRGB(bclr) << endl;
		outf << "2ndBgColor rgb " << cmap.getRGB(bclr) << endl;
		outf << "topShadowColor rgb " << cmap.getRGB(2) << endl;
		outf << "botShadowColor rgb " << cmap.getRGB(12) << endl;

	} else {
		outf << "fgColor index " << "14" << endl;
		outf << "bgColor index " << "51" << endl;
		outf << "2ndBgColor index " << 4 << endl;
		outf << "topShadowColor index " << 2 << endl;
		outf << "botShadowColor index " << 12 << endl;
	}
	outf << "controlPv " << chan << endl;
	outf << "controlLabelType \"pvName\"" << endl;

	fptr = fi.bestFittingFont( hgt/4 );
	if ( fptr ) {
		outf << "font \"" << fptr << "\"" << endl;
	} else {
		outf << "font \"" << "helvetica-bold-r-12.0" << "\"" << endl;
	}

	if(direction)
		outf << "orientation \"vertical\"" << endl;
	if(loprDefault != "-9")
		outf << "scaleMin " << loprDefault << endl;
	if(hoprDefault != "-9")
		outf << "scaleMax " << hoprDefault << endl;
	else 
		outf << "limitsFromDb" << endl;
	if(use_limits)
		outf << "showLimits" << endl;

	string dot(".");
	int tpos;
    tpos = dPrecision.find(dot,0);
    if(tpos != -1){
		dPrecision = string(dPrecision, 0, tpos);
		outf << "increment " << dPrecision << endl;
	}
	outf << "endObjectProperties" << endl;
	return 1;
}

mbuttonclass::mbuttonclass(int attr)
{
	fill_attrs(attr);
	bclr = 0;
	open = 1;
}

mbuttonclass::~mbuttonclass()
{
}

// process the "message button" object
int mbuttonclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
	bool isNum = true;
	int ret;
	char *ptr;
	string squote;
	squote =  "\"";
	string tname;
	int tpos;

	colormode = 0;
	//outd << "In MessageButton " << translator::line_ctr  << endl;
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
			else if((!strcmp(s1,"chan")) || (!strcmp(s1,"ctrl")) )
				chan = string(line, eq_pos+1, std::string::npos);
			else if(!strcmp(s1,"label"))  
				label = string(line, eq_pos+1, std::string::npos);
			else if(!strcmp(s1,"press_msg"))  {
				press_msg = string(line, eq_pos+1, std::string::npos);
				tname = press_msg;
				while((tpos = tname.find(squote,0)) != -1)
                	tname.replace(tpos,1,nil);
				strtoul(tname.c_str(), &ptr,0);
				if(*ptr != 0x0) isNum = false;
			}
			else if(!strcmp(s1,"release_msg"))  {
				release_msg = string(line, eq_pos+1, std::string::npos);
				tname = release_msg;
				while((tpos = tname.find(squote,0)) != -1)
                	tname.replace(tpos,1,nil);
				strtoul(tname.c_str(), &ptr,0);
				if(*ptr != 0x0) isNum = false;
				}
			else if(!strcmp(s1,"clrmod"))  {
				// JS Not clear on how to convert this.
				// What causes this to alarm in medm?
				if(strstr(s2,"alarm")) colormode = 1;
			}
			else outd << "Message Button Can't decode " << line << endl;
		}
    } while (open > 0);

	outf << endl;
    outf << "# (Message Button)" << endl;
    outf << "object activeMessageButtonClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << MSGBTC_MAJOR_VERSION << endl;
    outf << "minor " << MSGBTC_MINOR_VERSION << endl;
    outf << "release " << MSGBTC_RELEASE << endl;
    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;
	if(urgb) {
		outf << "fgColor rgb " << cmap.getRGB(clr) << endl;
		outf << "onColor rgb " << cmap.getRGB(bclr) << endl;
		outf << "offColor rgb " << cmap.getRGB(bclr) << endl;
		outf << "topShadowColor rgb " << cmap.getRGB(0) << endl;
		outf << "botShadowColor rgb " << cmap.getRGB(14) << endl;
	} else {
		outf << "fgColor index " << clr << endl;
		outf << "onColor index " << bclr << endl;
		outf << "offColor index " << bclr << endl;
		outf << "topShadowColor index " << 0 << endl;
		outf << "botShadowColor index " << 14 << endl;
	}
    outf << "controlPv " << chan << endl;
    outf << "pressValue " << press_msg << endl;
    outf << "releaseValue " << release_msg << endl;
    outf << "onLabel " << label << endl;
    outf << "offLabel " << label << endl;
    outf << "3d" << endl;
	if (isNum)
		outf << "useEnumNumeric" << endl;

	fptr = fi.bestFittingFont( (int) (hgt-.3*hgt) );
	if ( fptr ) {
		outf << "font \"" << fptr << "\"" << endl;
	} else {
		outf << "font \"" << "helvetica-bold-r-12.0" << "\"" << endl;
	}
    outf << "endObjectProperties" << endl;
	return 1;
}

shellnode::shellnode()
{
}

shellnode::~shellnode()
{
}

shellclass::shellclass(int attr)
{
	fill_attrs(attr);
	bclr = 0;
}

shellclass::~shellclass()
{
}

// process the "shell command" object
int shellclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
	int mode = 0;
	int shell_ctr = 0;

	vector <shellnode> shelllist;
	shellnode *sh;

	colormode = 0;
	//outd << "In Shell " << translator::line_ctr << endl;
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

			else if(strstr(s1, "label")!= 0x0) {
				sh->label = string(line, eq_pos+1, std::string::npos);
			}
			else if(strstr(s1, "name")!= 0x0) {
				sh->name = string(line, eq_pos+1, std::string::npos);
			}
			else if( (strstr(s1, "args")!= 0x0)) {
				sh->args = string(line, eq_pos+1, std::string::npos);
			}
		} else { 
			if( (strstr(line.c_str(), "command")!= 0x0)) {
				sh = new shellnode();
				mode = 1;
			}
			else if(( mode == 1) && (strstr(line.c_str(), "}")!= 0x0)) {
				mode = 0;
				// Copy data to vector
				if(sh->name.length() > 2) { // empty item will be ""
					shelllist.push_back(*sh);
					shell_ctr++;
				}
				delete sh;
			}
		}
    } while (open > 0);

	outf << endl;
    outf << "# (Shell Command)" << endl;
    outf << "object shellCmdClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << SHCMDC_MAJOR_VERSION << endl;
    outf << "minor " << SHCMDC_MINOR_VERSION << endl;
    outf << "release " << SHCMDC_RELEASE << endl;
    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;

	if(urgb) {
		outf << "fgColor rgb " << cmap.getRGB(clr) << endl;
		outf << "bgColor rgb " << cmap.getRGB(bclr) << endl;
		outf << "topShadowColor rgb " << cmap.getRGB(2) << endl;
		outf << "botShadowColor rgb " << cmap.getRGB(12) << endl;
	} else {
		outf << "fgColor index " << clr << endl;
		outf << "bgColor index " << bclr << endl;
		outf << "topShadowColor index " << 2 << endl;
		outf << "botShadowColor index " << 12 << endl;
	}
   
	fptr = fi.bestFittingFont( (int) (hgt-.3*hgt) );
	if ( fptr ) {
		outf << "font \"" << fptr << "\"" << endl;
	} else {
		outf << "font \"" << "helvetica-bold-r-12.0" << "\"" << endl;
	}

	outf << "buttonLabel \"!\"" << endl;
    outf << "numCmds " << shell_ctr << endl;

    outf << "commandLabel {" << endl;
    for (int i=0; i<shell_ctr; i++)
        if(shelllist[i].label.length() > 0)
            outf << "  " << i << " " << shelllist[i].label << endl;
    outf << "}" << endl;

    outf << "command {" << endl;
    for (int i=0; i<shell_ctr; i++)
        if(shelllist[i].name.length() > 0){
			stripQs(shelllist[i].name);
			stripQs(shelllist[i].args);
            outf << "  " << i << " " << "\"" << shelllist[i].name << " " 
				<< shelllist[i].args << "\"" << endl;
		}
    outf << "}" << endl;

    outf << "endObjectProperties" << endl;
    return 1;
}


relnode::relnode()
{
}

relnode::~relnode()
{
}

relatedclass::relatedclass(int attr)
{
	fill_attrs(attr);
	bclr = 0;
}

relatedclass::~relatedclass()
{
}

string relatedclass::look_for_file(ostream &outd, string tname)
{
	string vname;
    struct stat buf;
    char *dup0, *dup1, *dup2, *dup3;
    string temp, temp0, temp1, temp2, temp3;
    string dir, dir0, dir1, dir2, dir3;
	string slash("/");
	string slash2("//");
	int ret;
	int tpos;
	static int first = 1;

	outd << "LFF " << tname << endl;

	// "//" causes problems in dirname/basename
	while((tpos = tname.find(slash2,0)) != -1)
		tname.replace(tpos,1,nil);

	// look up the path from the calling adl file

	dup0 = strdup(translator::dir.c_str());
	dir0 = dirname(dup0);
	temp0 = dir0 + slash + tname;

	dup1 = strdup(dir0.c_str());
	dir1 = dirname(dup1);
	temp1 = dir1 + slash + tname;

	dup2 = strdup(dir1.c_str());
	dir2 = dirname(dup2);
	temp2 = dir2 + slash + tname;

	dup3 = strdup(dir2.c_str());
	dir3 = dirname(dup3);
	temp3 = dir3 + slash + tname;

	if(first){
		outd << "temp0 " << temp0 << endl;
		outd << "temp1 " << temp1 << endl;
		outd << "temp2 " << temp2 << endl;
		outd << "temp3 " << temp3 << endl;
		first = 0;
	}

	ret = stat(temp3.c_str(), &buf);
	if(!ret && S_ISREG(buf.st_mode)){
		temp = temp3;
		outd << "OK1 " << temp << endl;
	} else {
		ret = stat(temp2.c_str(), &buf);
		if(!ret && S_ISREG(buf.st_mode)){
			temp = temp2;
			outd << "OK2 " << temp << endl;
		} else {
			ret = stat(temp1.c_str(), &buf);
			if(!ret && S_ISREG(buf.st_mode)){
				temp = temp1;
				outd << "OK3 " << temp << endl;
			} else {
				ret = stat(temp0.c_str(), &buf);
				if(!ret && S_ISREG(buf.st_mode)){
					temp = temp0;
					outd << "OK4 " << temp << endl; 
				} else {
					temp = tname;
					outd << "FOUR STRIKES " << temp << endl;
				}
			}
		}
	}
	return temp;
}

// process the "related display" object
int relatedclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
	bool jlab = JLAB;

	int adl_pos;
	string echar(".edl");
	string adl(".adl");
	string medm("medm");
	string slash("/");
	string tstring;
	string title;
	string squote;
	string saved_name;
	squote =  "\"";
	string nil = "";
	int slash_pos;
	int mode = 0;
	int rel_ctr = 0;
	int tpos;
	int ret;
	bool invis = false;
	string tname;
	string vname;
	string temp;
	struct stat buf;

	vector <relnode> rellist;
	relnode *rel;

	//outd << "In Related " << translator::line_ctr << endl;
	//outd << "retitle:" << retitle << endl;
	do {
		getline(inf,line);
		translator::line_ctr++;
		//outd << line << endl;
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

			else if(!strcmp(s1,"visual"))  {
				if(strstr(s2, "invisible")!= 0x0) 
					invis = true;
			}
			else if(strstr(s1, "label")!= 0x0) {
				tstring = string(line, eq_pos+1, std::string::npos);
				while((tpos = tstring.find(squote,0)) != -1)
                    tstring.replace(tpos,1,nil);
				if(mode) rel->label = tstring;
				else title = tstring;
			} 
			else if(strstr(s1, "policy")!= 0x0) {
				tstring = string(line, eq_pos+1, std::string::npos);
				while((tpos = tstring.find(squote,0)) != -1)
                    tstring.replace(tpos,1,nil);
				if(mode) rel->policy = tstring;
			}

			else if(strstr(s1, "name")!= 0x0) {
				tname = s2;
				while((tpos = tname.find(squote,0)) != -1)
                	tname.replace(tpos,1,nil);
				// outd << "START " << tname << endl;
				saved_name = tname;

				if(tname.length() != 0) { // medm ignores blank names. edm does not.
					adl_pos = tname.find(adl,0);
					if(adl_pos != -1){
						tname.replace(adl_pos,4,echar);
					}
					rel->name = tname;
				}

				else if(!retitle && jlab) {
					temp = "";
                    if ((tpos = tname.find("//")) == 0) tname.replace(tpos,1,nil);
                        
					// make absolute paths relative.
					if ((tpos = tname.find("/usr/user4/mccops/medm/")) == 0) tname.replace(0, 23,nil);
					else if ((tpos = tname.find("/usr/user2/mccops/medm/")) == 0) tname.replace(0, 23,nil);
					else if ((tpos = tname.find("/cs/opshome/medm/")) == 0) tname.replace(0, 17,nil);
					else if ((tpos = tname.find("./")) == 0) tname.replace(tpos,2,nil);

					ret = stat(tname.c_str(), &buf);		// look in given path from medm dir
					if(!ret && S_ISREG(buf.st_mode)){
						outd << "FINAL 1 " << tname << endl;
						temp = tname;
					} else {
						vname = translator::dir + slash + tname;
						ret = stat(vname.c_str(), &buf);	// look in the calling dir
						if(!ret && S_ISREG(buf.st_mode)){
							if ((tpos = vname.find("/usr/user4/mccops/medm/")) == 0) vname.replace(0, 23,nil);
							outd << "FINAL 2 " << vname << endl;
							temp = vname;
						}
                    }
					adl_pos = temp.find(adl,0);
					if(adl_pos != -1){
						temp.replace(adl_pos,4,echar);
						rel->name = temp;
					}

					if(temp.length() == 0) {
						if(translator::dir == "/tmp") {
							temp =  saved_name;
							outd << "Web file " << saved_name << endl;
							// If caller was a web file, So is this one. Don't change adl to edl!
							rel->name = tname;
						} else {
							// This next call may not be needed once files stabilize.
							temp = look_for_file(outd, tname);
							if ((tpos = temp.find("/usr/user4/mccops/medm/")) == 0) {
								temp.replace(0, 23,nil);
								outd << "FINAL 3 " << temp << endl;

								adl_pos = temp.find(adl,0);
								if(adl_pos != -1)
									temp.replace(adl_pos,4,echar);
								rel->name = tname;

							} else {
								rel->name = tname;
								// Don't change adl to edl!
							}
						} 
					} // end if temp length = 0

					rel->name = temp;

				} // end if jlab

				else if (retitle) {			// Not jlab
					adl_pos = tname.find(adl,0);
					if(adl_pos != -1){
						tname.replace(adl_pos,4,echar);
						rel->name = tname;
					}
				}

				// outd << "RelatedDisplay name " << rel->name << endl;

			} // end if name


			else if( (strstr(s1, "args")!= 0x0)) {
				rel->args = string(line, eq_pos, std::string::npos);
				// outd << "RelatedDisplay args " << rel->args << endl;
				// get rid of leading spaces . See below.
				while((tpos = rel->args.find(space,0)) != -1)
   	               	rel->args.replace(tpos,1,nil);
			}
		} // end if eqpos

		else { 
			// Create a new node
			if( (strstr(line.c_str(), "display")!= 0x0)) {
				rel = new relnode();
				mode = 1;
				// outd << "RelatedDisplay new node" << endl;
			} else if(( mode == 1) && (strstr(line.c_str(), bclose.c_str())!= 0x0)) {
				mode = 0;
				// Some programatically created screens leave rel.name and the rest
				// of the fields as "". This is ok for medm. Not for edm.
				// So, if the name is "" or blank after stripping quotes, 
				// we don't push this onto the list.
				//outd << "PB? " << rel->name.length() << endl;
				if(rel->name.length() > 0) {
					rellist.push_back(*rel);
					rel_ctr++;
				}
				delete rel;
			}
		}
    } while (open > 0);

	outf << endl;
	outf << "# (Related Display)" << endl;
	outf << "object relatedDisplayClass" << endl;
	outf << "beginObjectProperties" << endl;
	outf << "major " << RDC_MAJOR_VERSION << endl;
	outf << "minor " << RDC_MINOR_VERSION << endl;
	outf << "release " << RDC_RELEASE << endl;
	outf << "x " << x << endl;
	outf << "y " << y << endl;
	outf << "w " << wid<< endl;
	outf << "h " << hgt << endl;

	if(urgb) {
		outf << "fgColor rgb " << cmap.getRGB(clr) << endl;
		outf << "bgColor rgb " << cmap.getRGB(bclr) << endl;
		outf << "topShadowColor rgb " << cmap.getRGB(2) << endl;
		outf << "botShadowColor rgb " << cmap.getRGB(12) << endl;
	} else {
		outf << "fgColor index " << clr << endl;
		outf << "bgColor index " << bclr << endl;
		outf << "topShadowColor index " << 2 << endl;
		outf << "botShadowColor index " << 12 << endl;
	}

	if ( title.length() ) {
		fptr = fi.bestFittingFont( (int) (hgt-.3*hgt) );
	} else {
		fptr = fi.bestFittingFont( (int) (hgt-.5*hgt) );
	}

	if ( fptr ) {
		outf << "font \"" << fptr << "\"" << endl;
	} else {
		outf << "font \"" << "helvetica-bold-r-12.0" << "\"" << endl;
	}

	// A minus sign in front of the title means don't show the lil boxes.
	if(title.length()){
		string minus("-");
        int minus_pos = title.find(minus,0);
			if(minus_pos == 0){
				title.replace(minus_pos,1,nil);
				outf << "buttonLabel \"" << title << "\""<< endl;
			} else {
				//outf << "buttonLabel \"[ ] " << title << "\""<< endl;
				outf << "icon" << endl;
				outf << "buttonLabel \"" << title << "\""<< endl;
			}
	} else {
		//outf << "buttonLabel \"[ ]\"" << endl;
		outf << "icon" << endl;
		//outf << "buttonLabel \"[ ]\"" << endl;
	}

	// T'is a conundrum! 
	// To medm, invisible means there is no indication the button exists.
	// Visible means that the control widgets rise to the top.

	// To edm, invisible is the same as medm.
	// Visible means it may or may not appear, depending on whether
	// something is on top of it. 

	// The adl2edl default is that nothing is invisible unless medm makes it so.
	// Perhaps the only good solution is to raise control widgets to the top by 
	// writing them to the file last. This hasn't been done.
	// This could be a big problem because of composites, etc.

	// One can bring the file into the editor, select each gray
	// mouseoveer box, and "raise" them as desired. Side by side adl/edl 
	// compare helps find 'em.

	// Or one can "lower" whatever is obscuring the widgets.

	// One can bring the screen into the editor and check invisible to fix
	// the ones that do show and shouldn't.
	
	if (invis)
		outf << "invisible" << endl;
	outf << "numPvs " << 2*rel_ctr << endl;
	outf << "numDsps " << rel_ctr << endl;

	outf << "displayFileName {" << endl;
	for (int i=0; i<rel_ctr; i++) {
		outf << "  " << i << " " << rellist[i].name << endl;  
		//outd << "Rel Disp: " << rellist[i].name << endl;  
	}
	outf << "}" << endl;				

	outf << "menuLabel {" << endl;
	for (int i=0; i<rel_ctr; i++) 
		outf << "  " << i << " " << rellist[i].label << endl;  
	outf << "}" << endl;				

	// Write 'symbol open curly brace' only if there are args
	int once = 0;
	for (int i=0; i<rel_ctr; i++) {
		//if(rellist[i].args.length() > 0){
		//outd << "len " << rellist[i].args.length() << "<" << rellist[i].args << ">" << endl;
		if(rellist[i].args.length() == 2){
			//outd << "changed" << endl;
			rellist[i].args = "\" \"";
		}
			if(!once) {
				outf << "symbols {" << endl;
				once++;
			}
			outf << "  " << i << " " << rellist[i].args << endl;  
			//outd << "  " << i << "< " << rellist[i].args << ">" << endl;  
		//}
	}
	if (once) outf << "}" << endl;				

	once = 0;
	for (int i=0; i<rel_ctr; i++) {
		//outd << "  " << i << "< " << rellist[i].policy << ">" << endl;  
		if(strstr(rellist[i].policy.c_str(), "replace")){
			if(!once) {
				outf << "closeAction {" << endl;
				once++;
			}
			outf << "  " << i << " 1" << endl;  
		}
	}
	if (once) outf << "}" << endl;

    // Simulate medm behaviour by replacing the parent macros
    once = 0;
    for (int i=0; i<rel_ctr; i++) {
        if(!once) {
            outf << "replaceSymbols {" << endl;
            once++;
        }
        outf << "  " << i << " 1" << endl;
    }
    if (once) outf << "}" << endl;

	once = 0;
	for (int i=0; i<rel_ctr; i++) {
		if(strstr(rellist[i].policy.c_str(), "replace")){
			if(!once) {
				outf << "closeDisplay {" << endl;
				once++;
			}
			outf << "  " << i << " 1" << endl;  
		}
	}
	if (once) outf << "}" << endl;

	outf << "endObjectProperties" << endl;
	return 1;
}


menuclass::menuclass(int attr)
{
	fill_attrs(attr);
	bclr = 0;
}

menuclass::~menuclass()
{
}

// process the "menu_button" object
int menuclass::parse(ifstream &inf, ostream &outf, ostream &outd)
{
	colormode = 0;
	//outd << "In menubutton " << translator::line_ctr << endl;
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
			snum = sscanf(line.c_str(), "%s %s", s1,s2);
			if     (!strcmp(s1,"x"))  x = atoi(s2); 
			else if(!strcmp(s1,"y"))  y = atoi(s2); 
			else if(!strcmp(s1,"width"))  wid = atoi(s2); 
			else if(!strcmp(s1,"height"))  hgt = atoi(s2); 
			else if(!strcmp(s1,"clr"))  clr = atoi(s2); 
			else if(!strcmp(s1,"bclr"))  bclr = atoi(s2); 
			else if(!strcmp(s1,"chan")|| !strcmp(s1,"ctrl")) { 
               	chan = string(line, eq_pos+1, std::string::npos);
			}
			else if(!strcmp(s1,"clrmod")) {
                if(strstr(s2, "static") != 0x0) colormode = 0;
                else if(strstr(s2, "alarm") != 0x0) colormode = 1;
                else if(strstr(s2, "discrete") != 0x0) colormode = 2;
            }
			else outd << "Menu Button Can't decode " << line << endl;
		}
    } while (open > 0);

	outf << endl;
    outf << "# (Menu Button)" << endl;
    outf << "object activeMenuButtonClass" << endl;
    outf << "beginObjectProperties" << endl;
    outf << "major " << MBTC_MAJOR_VERSION << endl;
    outf << "minor " << MBTC_MINOR_VERSION << endl;
    outf << "release " << MBTC_RELEASE << endl;
    outf << "x " << x << endl;
    outf << "y " << y << endl;
    outf << "w " << wid<< endl;
    outf << "h " << hgt << endl;

	if(urgb) {
		outf << "fgColor rgb " << cmap.getRGB(clr) << endl;
		outf << "bgColor rgb " << cmap.getRGB(bclr) << endl;
		outf << "inconsistentColor rgb " << cmap.getRGB(12) << endl;
		outf << "topShadowColor rgb " << cmap.getRGB(2) << endl;
		outf << "botShadowColor rgb " << cmap.getRGB(12) << endl;

	} else {
		outf << "fgColor index " << clr << endl;
		outf << "bgColor index " << bclr << endl;
		outf << "inconsistentColor index " << 12 << endl;
		outf << "topShadowColor index " << 2 << endl;
		outf << "botShadowColor index " << 12 << endl;
	}
    outf << "controlPv " << chan << endl;
    outf << "indicatorPv " << chan << endl;
    
	fptr = fi.bestFittingFont( (int) (hgt-.3*hgt) );
    if ( fptr ) {
      outf << "font \"" << fptr << "\"" << endl;
    } else {
      outf << "font \"" << "helvetica-bold-r-12.0" << "\"" << endl;
    }

    outf << "endObjectProperties" << endl;
    return 1;
}

