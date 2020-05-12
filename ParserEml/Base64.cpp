#pragma once
#include <Header.h>
#include "Base64.h"



Base64::Base64()
{
}


Base64::~Base64()
{
}
using namespace std;
static fs::path File_Info(const fs::directory_entry &Entry) {
	const auto Type(status(Entry));
	return Entry.path();
}
void Base64::Decode_Directory(fs::path Dir) {
	std::list<fs::path> Files;
	std::transform(fs::directory_iterator{ Dir }, {},
		std::back_inserter(Files), File_Info);

	uint32_t Count = 0;
	fs::path Out_Dir(Dir / "Out");
	fs::create_directory(Out_Dir);
	for (const auto &In_File : Files) {
		Decode_File(In_File, Out_Dir);
		Count++;
	}

	cout << "Файлов обработано " << Count << endl;
}

bool Base64::Decode_File(fs::path File_Path) {
	bool Result = GOOD;
	string Full_Name = File_Path.generic_string();

	ifstream Source(Full_Name, ios::binary);
	uint32_t Size = fs::file_size(Full_Name);
	if (Size != EMPTY_FILE) {
		string Extension = ".bin";
		string In_File = File_Path.filename().generic_string();
		fs::path Out_Dir = canonical(File_Path).remove_filename() / ("Out" + In_File);
		fs::path Out_File = (Out_Dir / ("Base64" + Extension));
		if (fs::create_directory(Out_Dir)) {
			vector<char> Buf(CHUNK);
			uint32_t Reading = 0;
			uint32_t Pointer = 0;
			uint32_t Counter = 0;
			ofstream Dest(Out_File.generic_string(), ios::binary);
			while (Pointer < Size) {
				do {
					Source.read(&Buf[0], CHUNK);
					Dest << Decode_String(&Buf[0]);
					memset(&Buf[0], 0, CHUNK);
				} while (!(Source.fail()));
				Dest << endl << "#################################" << endl;

				Source.clear();
				Source.seekg(Pointer++, ios::beg);
			}
			uint32_t Size_Out = file_size(Out_File);
			if (Size_Out != 0) {
				cout << "Резльтат в файле " << Out_File << endl;
				cout << "Размер файла " << Size_Out << " байт." << endl;
			}
			else {
				fs::remove(Out_File);
				cout << "Base64 в файле " << Out_File << " не найден." << endl;

				Result = FAIL;
			}
		}
		else {
			cout << "Ошибка создания диретории выхода: " << Out_Dir << endl;
			Result = FAIL;
		}
	}
	else {
		Result = FAIL;
	}
	return Result;
}

bool Base64::Decode_File(fs::path File_Path, fs::path Out_Dir) {
	bool Result = GOOD;

	ifstream Source(File_Path.generic_string(), ios::binary);
	uint32_t Size = file_size(File_Path);
	if (Size != EMPTY_FILE) {
		string Extension = ".bin";
		fs::path Out_File(Out_Dir / File_Path.filename());
		vector<char> Buf(CHUNK);
		uint32_t Reading = 0;
		uint32_t Pointer = 0;
		uint32_t Counter = 0;
		ofstream Dest(Out_File.generic_string(), ios::binary);
		while (Pointer < Size) {
			do {
				Source.read(&Buf[0], CHUNK);
				Dest << Decode_String(&Buf[0]);
				memset(&Buf[0], 0, CHUNK);
			} while (!(Source.fail()));
			Dest << endl << "#################################" << endl;

			Source.clear();
			Source.seekg(Pointer++, ios::beg);
		}
		uint32_t Size_Out = file_size(Out_File);
		if (Size_Out != 0) {
			cout << "Резльтат в файле " << Out_File << endl;
			cout << "Размер файла " << Size_Out << " байт." << endl;
		}
		else {
			fs::remove(Out_File);
			cout << "Base64 в файле " << Out_File << " не найден." << endl;

			Result = FAIL;
		}
	}
	else {
		Result = FAIL;
	}
	return Result;
}

string Base64::Decode_String(const string_view in) {
	// table from '+' to 'z'
	const uint8_t lookup[] = {
		62,  255, 62,  255, 63,  52,  53, 54, 55, 56, 57, 58, 59, 60, 61, 255,
		255, 0,   255, 255, 255, 255, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
		10,  11,  12,  13,  14,  15,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
		255, 255, 255, 255, 63,  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
		36,  37,  38,  39,  40,  41,  42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };
	static_assert(sizeof(lookup) == 'z' - '+' + 1);

	string out;
	int val = 0, valb = -8;
	for (uint8_t c : in) {
		if (c < '+' || c > 'z') {
			break;
		}
		c -= '+';
		if (lookup[c] >= 64) {
			break;
		}
		else {
			val = (val << 6) + lookup[c];
			valb += 6;
			if (valb >= 0) {
				out.push_back(char((val >> valb) & 0xFF));
				valb -= 8;
			}
		}
	}
	return out;
}
