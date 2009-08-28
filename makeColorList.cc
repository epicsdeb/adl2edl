#include <iostream>
#include <fstream>
#include <ostream>

using std::endl;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::string;

class cmap
{
public:
	cmap();
	~cmap();
	int buildColorList(ifstream&, ofstream&);
	int numColors;
};

cmap::cmap()
{
}

cmap::~cmap()
{
}


int main(int argc, char **argv)
{
	if(argc < 3) {
        cout << "Version 1.0" << endl;
        cout << "usage:  medm_file color_file" << endl;
        exit(1);
    }
	ifstream inf(argv[1]);
    if(!inf) {
        cout << "unable to open " << argv[1] << endl;
        exit(1);
    }

	ofstream outf(argv[2]);
    if(!outf) {
        cout << "unable to open " << argv[2] << endl;
        exit(1);
    }

	cmap mycmap;
	string line;
	while( inf ) {
        getline(inf,line);
		 if(strstr(line.c_str(), "color map")!= 0x0) {
			mycmap.buildColorList(inf, outf);
			break;
		}
	}
}


int cmap::buildColorList(ifstream &inf, ofstream &outf)
{
	int eq_pos;
	string eq("=");
	string space(" ");
	string bopen("{");
	string bclose("}");
	char t[4];
	int ctr = 0;
	int open = 1;
	int pos;
	char s1[100], s2[100];

	string line;
    do {
        getline(inf,line);
        pos = line.find(bopen,0);
        if(pos != -1)  {
			open++;
		}
        pos = line.find(bclose,0);
        if(pos != -1)  {
			open--;
		}

		eq_pos = line.find(eq,0);
        if(eq_pos != -1){
            line.replace(eq_pos,1,space);
            sscanf(line.c_str(), "%s %s", s1,s2);
            if (strstr(s1,"ncolors") != 0)  numColors = atoi(s2);
		}
        else if (strstr(line.c_str(),"colors") != 0)  {
			outf << "4 0 0" << endl;
			outf << endl;
			outf << "max=0x10000" << endl;
			outf << "columns=5" << endl;
			outf << endl;
			outf << endl;
			int done = 0;
    		do {
        		getline(inf,line);

				pos = line.find(bclose,0);
				if(pos != -1)  {
					done = 1;
					open --;
				}
				else {
					sscanf(line.c_str(), "%s", s1);
					string rgb;
					int r,g,b;
					char tstr[100];

					t[0] = s1[0];;
					t[1] = s1[1];;
					t[2] = 0x0;
					r = strtol(t, 0,16) << 8;

					t[0] = s1[2];;
					t[1] = s1[3];;
					g = strtol(t, 0,16) << 8;

					t[0] = s1[4];;
					t[1] = s1[5];;
					b = strtol(t, 0,16) << 8;

					outf << "static " << ctr << " \"a" << ctr << "\"" <<
						"\t{ " <<
						r << " " << g << " " << b << " } " << endl;
					ctr++;
				}
					
			} while (!done);
		}
	} while (open > 0);
	outf << endl;
	outf << "menumap {" << endl;
	ctr = 0;
	for(int i =0; i< numColors; i++) {
		outf << " \"a" << ctr << "\"" << endl;
		ctr++;
	}
	outf << " }" << endl;
	outf << "alarm {" << endl;
	outf << "  disconnected : \"a0\"" << endl;
	outf << "  invalid      : \"a0\"" << endl;
	outf << "  minor        : \"a0\"" << endl;
	outf << "  major        : \"a0\"" << endl;
	outf << "  noalarm      : \"a0\"" << endl;
	outf << " }" << endl;
	
	outf << endl;
	return 1;

}
