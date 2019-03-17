#include <conio.h>
#include <iostream>
#include <fstream>
#include <filesystem>
using namespace std;

struct DOSHeader
{
	unsigned char mz[2];
	unsigned char dnc[58];
	unsigned long peloc;
};

struct PEHeader
{
	unsigned char pe[4];
	unsigned char dnc[18];
	unsigned short int characteristics;
	unsigned short int magic;
	unsigned char dnc2[66];
	unsigned short int subsystem;
};

string IsPE(string filepath)
{
	string details;
	ifstream fobj;
	fobj.open(filepath.c_str(), ios::in | ios::binary);
	DOSHeader mz;
	fobj.read((char*)&mz, sizeof(mz));
	
	if (!fobj.good() || memcmp(mz.mz, "MZ", 2))
	{
		details.append("DOS_SIGNATURE_MISSING");
		return details;
	}
	
	details.append("DOS_SIGNATURE_PRESENT");
	
	fobj.seekg(mz.peloc);
	PEHeader pe;
	fobj.read((char*)&pe, sizeof(pe));
	unsigned char pesig[] = { 'P', 'E', 0x00, 0x00 };
	if (!fobj.good() || memcmp(pe.pe, pesig, 4))
	{
		details.append("|PE_SIGNATURE_MISSING");
		return details;
	}
	details.append("|PE_SIGNATURE_PRESENT");
	unsigned short int exp32bit = 0x10b, exp64bit = 0x20b;
	if (exp32bit == pe.magic)
		details.append("|X86");
	else if (exp64bit == pe.magic)
		details.append("|X64");
	else
	{
		details.append("|INVALID_CPU_ARCH");
		return details;
	}

	if (pe.characteristics & 0x2000 && (pe.subsystem >1 && pe.subsystem <=3))
		details.append("|FILE_TYPE_DLL");
	else if (pe.characteristics & 0x2 && (pe.subsystem > 1 && pe.subsystem <= 3))
		details.append("|FILE_TYPE_EXE");
	else
		details.append("|FILE_TYPE_UNKNOWN");

	//cout << " pe subsystem value = " << pe.subsystem << " path = "<<filepath.c_str()<<endl;
	return details;
}

int EnumerateFiles(string path,string criteria)
{
	int count = 0;
	for (auto &p : filesystem::recursive_directory_iterator(path.c_str()))
	{

		if (p.is_regular_file())
		{
			string result = IsPE(p.path().string());
			if (result.find(criteria) != string::npos)
			{
				cout << "filepath = " << p.path().string().c_str() << " type = " << IsPE(p.path().string()) << endl;
				count++;
			}
		}
	}
	return count;
}
int main(int argc, char**argv)
{
	if (argc < 3)
	{
		cout << "Usage = " << endl;
		cout << " PEParser.exe <directory path> <filter>" << endl;
		cout << "E.g. Filters are  'X86|FILE_TYPE_EXE', 'X64|FILE_TYPE_DLL', 'FILE_TYPE_EXE'" << endl;
		return 1;
	}
	string criteria = argv[2];
	cout<<"Identifying file characteristics in directory : "<<argv[1]<<endl<<endl;
	int count = EnumerateFiles(argv[1], criteria);
	cout<<"Total Files Matching criteria "<<criteria.c_str()<< " = " <<count<<endl;
	return 0;
}

