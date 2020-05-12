#pragma once
#include <Header.h>
#include "ParserEml.h"

#include "../FileOperations/FileOperations.hpp"
#include "../Strings/Strings.hpp"

#include "../Base64/Base64.h"

//#pragma comment (lib, "msvcrt.lib")

ParserEml::ParserEml(QWidget* parent)
	: QMainWindow(parent),
	mSettings{ "572", "ParserEml" }
{
	ui.setupUi(this);

	ui.TableResult->setColumnCount(COLUMN_LABELS.size());
	ui.TableResult->setHorizontalHeaderLabels(COLUMN_LABELS);
	ui.TableResult->setFocusPolicy(Qt::NoFocus);
	ui.TableResult->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.TableResult->horizontalHeader()->setStretchLastSection(true);

	if (mDb.getTable().isEmpty())
	{
		mDb.createTableMoreAndMore("mail");
		mDb.createTable(
			"date",
			std::make_pair(WEEK, QString{ "TEXT" }),
			std::make_pair(DAY, QString{ "TEXT" }),
			std::make_pair(MONTH, QString{ "TEXT" }),
			std::make_pair(YEAR, QString{ "TEXT" }),
			std::make_pair(TIME, QString{ "TEXT" })
		);
		mDb.createTable(
			"sender",
			std::make_pair(NAME, QString{ "TEXT" }),
			std::make_pair(MAIL, QString{ "TEXT" })
		);
		mDb.createTable(
			"recipient",
			std::make_pair(NAME, QString{ "TEXT" }),
			std::make_pair(MAIL, QString{ "TEXT" })
		);
	}

	connect(ui.ActDir, &QAction::triggered, this, &ParserEml::AddLst);

	connect(ui.ActStart, &QAction::triggered, this, &ParserEml::Parsing);
}

void ParserEml::AddLst()
{
	fops::reg::select_path_dir(mSettings, DIR);

	fs::path dir{ mSettings.value(DIR).toString().toStdString() };

	for (const auto& file : fs::directory_iterator{ dir }
		 | views::remove_if(fops::is_not_file))
	{
		ui.LstDir->addItem(QString{ file.path().filename().generic_string().c_str() });
	}

	ui.statusBar->showMessage("Добавлено: " + QString::number(ui.LstDir->count()) + " элемент(ов)");
}

std::optional<std::vector<std::string>>
ParserEml::date(const std::vector<char>& data)
{
	std::vector<std::string> out;
	if (auto it{ std::search(
		std::cbegin(data), std::cend(data),
		std::cbegin(GDATE), std::cend(GDATE)) };
		it != std::cend(data))
	{
		std::string str;
		std::copy(
			std::next(it, GDATE.size()), std::search(
				std::next(it, GDATE.size()), std::cend(data),
				std::cbegin(ENTER), std::cend(ENTER)),
			std::back_inserter(str));
		for (const auto& elm : str
			 | views::remove_if(str::is_comma)
			 | views::split(' ')
			 )
		{
			out.push_back(elm);
		}
		return out;
	}
	return {};
}
std::optional<std::vector<std::string>>
ParserEml::sender(const std::vector<char>& data)
{
	if (auto it{ std::search(
		std::cbegin(data), std::cend(data),
		std::cbegin(FROM), std::cend(FROM)) };
		it != std::cend(data))
	{
		std::string name;
		auto name_end = std::search(
			it, std::cend(data),
			std::cbegin(SCOB), std::cend(SCOB));
		std::copy(
			std::next(it, FROM.size()), name_end,
			std::back_inserter(name));

		std::string mail;
		auto mail_end{
			std::search(
				it, std::cend(data),
				std::cbegin(SCOB_RIGHT), std::cend(SCOB_RIGHT))
		};
		std::copy(
			name_end, mail_end,
			std::back_inserter(mail));

		return std::vector<std::string>{name, mail};
	}
	return {};
}
using namespace std;
string Ascii_To_Hex(string &Code) {
	string Result;

	for (uint32_t Index = 0; Index < Code.size(); Index++) {

		if (char Left, Right; Code[Index] >= 0x41) {
			Left = ((Code[Index++] - 0x31) + 0x0A) << 4;
			Right = Code[Index++] >= 0x41 ? ((Code[Index] - 0x31) + 0x0A) & 0x0F : Code[Index] & 0x0F;
			Result.push_back(Left ^ Right);
		}
		else {
			Left = (Code[Index++] << 4) & 0xF0;
			Right = Code[Index++] >= 0x41 ? ((Code[Index] - 0x31) + 0x0A) & 0x0F : Code[Index] & 0x0F;
			Result.push_back(Left ^ Right);
		}
	}

	return Result;
}

string Ascii_To_Hex(string &Code, char Delim) {
	string Result;

	for (uint32_t Index = 0; Index < Code.size(); Index++) {

		if (Code[Index] == Delim) {

			if (char Left, Right; Code[++Index] >= 0x41) {
				Left = ((Code[Index] - 0x31) + 0x0A) << 4;
				Right = Code[++Index] >= 0x41 ? ((Code[Index] - 0x31) + 0x0A) & 0x0F : Code[Index] & 0x0F;
				Result.push_back(Left ^ Right);
			}
			else {
				Left = (Code[Index] << 4) & 0xF0;
				Right = Code[++Index] >= 0x41 ? ((Code[Index] - 0x31) + 0x0A) & 0x0F : Code[Index] & 0x0F;
				Result.push_back(Left ^ Right);
			}
		}
		else if (Code[Index] == ';') {

		}
		else if (Code[Index] == 0x0D) {

		}
		else {
			Result.push_back(Code[Index]);
		}
	}

	return Result;
}

string Get_Filename_Start(string Buffer, size_t Start, const string Str_Lenght) {

	std::remove_if(Buffer.begin(), Buffer.end(), [&](char &Symbol) {
		bool Result;

		if (Symbol == ';') {
			Result = REMOVE;
		}
		else {
			Result = NO_REMOVE;
		}

		return Result;
			  });

	return Buffer.substr(Start + Str_Lenght.size(),
						 Buffer.size() - (Start + Str_Lenght.size()));
}

string Get_Filename_Continue(ifstream &Source, uint32_t Offset, const string Code) {
	string Result;
	string Buffer;
	Base64 o_Base;

	do {
		getline(Source, Buffer);
		string Filename_Code = Buffer.substr(Offset, Buffer.size() - Code.size() + Offset);
		Result += o_Base.Decode_String(Filename_Code);
	} while (Buffer.find("\"") == string::npos);

	return Result;
}

string Get_Filename_Continue(ifstream &Source, char Delim) {
	string Result;
	string Buffer;

	do {
		getline(Source, Buffer);

		if (Buffer.size() >= SIZE_FILENAME) {
			string Code_Name = Buffer.substr(SIZE_FILENAME, Buffer.size() - SIZE_FILENAME);
			Result += Ascii_To_Hex(Code_Name, Delim);
		}
		else {

		}

	} while (Buffer.find(";") != string::npos);

	return Result;
}

size_t Is_Finded_Filename(string &Buffer, const string Code) {
	return Buffer.size() >= Code.size() ? Buffer.find(Code) : string::npos;
}

void Delete_End_1251Q(string &Filename_Code) {
	size_t First = Filename_Code.find("?=\"");

	if (First != string::npos) {
		Filename_Code = Filename_Code.substr(0, First);
	}
	else {

	}
}

void Delete_End_1251Q_2(string &Filename_Code) {
	size_t First = Filename_Code.find("?=");

	if (First != string::npos) {
		Filename_Code = Filename_Code.substr(0, First);
	}
	else {

	}
}

void Delete_Forbidden_Symbol_Base64(string &Filename_Code) {

	Filename_Code.erase(std::remove_if(Filename_Code.begin(), Filename_Code.end(), [&](char Symbol) {
		bool Result;

		if (Symbol == '?' || Symbol == '=' ||
			Symbol == '\r' || Symbol == '"') {

			Result = REMOVE;
		}
		else {
			Result = NO_REMOVE;
		}

		return Result;
								  }), Filename_Code.end());

	Filename_Code.shrink_to_fit();
}

void Replace_Forbidden_Symbol(string &Buffer) {

	std::replace_if(Buffer.begin(), Buffer.end(),
			   [&](char &Symbol) {
				   bool Result;

				   if (Symbol == '\\' || Symbol == '/' ||
					   Symbol == ':' || Symbol == '*' ||
					   Symbol == '?' || Symbol == '"' ||
					   Symbol == '<' || Symbol == '>' ||
					   Symbol == '|' || Symbol == '+') {

					   Result = REMOVE;
				   }
				   else {
					   Result = NO_REMOVE;
				   }

				   return Result;

			   }, ' ');

}

void Delete_Forbidden_Symbol(string &Buffer) {

	Buffer.erase(std::remove_if(Buffer.begin(), Buffer.end(),
						   [&](char &Symbol) {
							   bool Result;

							   if (Symbol == '\\' || Symbol == '/' ||
								   Symbol == ':' || Symbol == '*' ||
								   Symbol == '?' || Symbol == '"' ||
								   Symbol == '<' || Symbol == '>' ||
								   Symbol == '|' || Symbol == '+' ||
								   Symbol == '\r' || Symbol == '\n' ||
								   Symbol == '=') {

								   Result = REMOVE;
							   }
							   else {
								   Result = NO_REMOVE;
							   }

							   return Result;

						   }), Buffer.end());

	Buffer.shrink_to_fit();
}
std::optional<std::vector<std::string>>
ParserEml::recipient(const std::vector<char>& data)
{
	if (auto it{ std::search(
		std::cbegin(data), std::cend(data),
		std::cbegin(TO), std::cend(TO)) };
		it != std::cend(data))
	{
		std::string Result;
		std::string name;
		auto name_end = std::search(
			it, std::cend(data),
			std::cbegin(SCOB), std::cend(SCOB));
		std::copy(
			std::next(it, TO.size()), name_end,
			std::back_inserter(name));

		// filename*=UTF-8''
		if (size_t First = Is_Finded_Filename(name, M_UTF8_ONE);
			First != string::npos) {

			string Filename_Code = Get_Filename_Start(name, First, M_UTF8_ONE);

			Result = Ascii_To_Hex(Filename_Code, '%');
			Replace_Forbidden_Symbol(Result);
		}
		// filename*=windows-1251''
		else if (First = Is_Finded_Filename(name, M_1251_ONE);
				 First != string::npos) {

			string Filename_Code = Get_Filename_Start(name, First, M_1251_ONE);

			Result = Ascii_To_Hex(Filename_Code, '%');
			Replace_Forbidden_Symbol(Result);
		}
		// filename*0*=UTF-8''
		/*else if (First = Is_Finded_Filename(name, M_UTF8_MULTI_U);
				 First != string::npos) {

			string Filename_Code = Get_Filename_Start(Buffer, First, M_UTF8_MULTI_U);
			Filename_Code += Get_Filename_Continue(Source, '%');

			Result = Ascii_To_Hex(Filename_Code, '%');
			Replace_Forbidden_Symbol(Result);
		}*/
		// filename*0*=utf-8''
		/*else if (First = Is_Finded_Filename(name, M_UTF8_MULTI_D);
				 First != string::npos) {

			string Filename_Code = Get_Filename_Start(Buffer, First, M_UTF8_MULTI_D);
			Filename_Code += Get_Filename_Continue(Source, '%');

			Result = Ascii_To_Hex(Filename_Code, '%');
			Replace_Forbidden_Symbol(Result);
		}*/
		// filename*0*=windows-1251''
		/*else if (First = Is_Finded_Filename(name, M_1251_MULTI);
				 First != string::npos) {

			string Filename_Code = Get_Filename_Start(Buffer, First, M_1251_MULTI);
			Filename_Code += Get_Filename_Continue(Source, '%');

			Result = Ascii_To_Hex(Filename_Code, '%');
			Replace_Forbidden_Symbol(Result);
		}*/
		// filename=\"=?windows-1251?Q?
		else if (First = Is_Finded_Filename(name, M_1251Q);
				 First != string::npos) {

			string Filename_Code = Get_Filename_Start(name, First, M_1251Q);
			Delete_End_1251Q(Filename_Code);
			Result = Ascii_To_Hex(Filename_Code, '=');
			Replace_Forbidden_Symbol(Result);
		}
		// filename==?windows-1251?Q?
		else if (First = Is_Finded_Filename(name, M_1252Q_2);
				 First != string::npos) {

			string Filename_Code = Get_Filename_Start(name, First, M_1252Q_2);
			Delete_End_1251Q_2(Filename_Code);
			Result = Ascii_To_Hex(Filename_Code, '=');
			Replace_Forbidden_Symbol(Result);
		}
		// filename=\"=?windows-1251?B?
		/*else if (First = Is_Finded_Filename(Buffer, M_1251B);
				 First != string::npos) {

			string Filename_Code = Get_Filename_Start(Buffer, First, M_1251B);
			Base64 o_Base;

			if (Filename_Code.find("\"") == string::npos) {
				Delete_Forbidden_Symbol_Base64(Filename_Code);
				Result = o_Base.Decode_String(Filename_Code);
				Result += Get_Filename_Continue(Source, SIZE_1251, M_1251B);
			}
			else {
				Result = o_Base.Decode_String(Filename_Code);
			}

			Replace_Forbidden_Symbol(Result);
		}*/
		// filename==?windows-1251?B?
		else if (First = Is_Finded_Filename(name, M_1251B_2);
				 First != string::npos) {

			string Filename_Code = Get_Filename_Start(name, First, M_1251B_2);
			Base64 o_Base;
			Result = o_Base.Decode_String(Filename_Code);
			Replace_Forbidden_Symbol(Result);
		}
		// filename=\"=?koi8-r?B?
		/*else if (First = Is_Finded_Filename(Buffer, M_KOI8B);
				 First != string::npos) {

			string Filename_Code = Get_Filename_Start(Buffer, First, M_KOI8B);
			Base64 o_Base;

			if (Filename_Code.find("\"") == string::npos) {
				Delete_Forbidden_Symbol_Base64(Filename_Code);
				Result = o_Base.Decode_String(Filename_Code);
				Result += Get_Filename_Continue(Source, SIZE_KOI8B, M_KOI8B);
			}
			else {
				Result = o_Base.Decode_String(Filename_Code);
			}

			Replace_Forbidden_Symbol(Result);
		}*/
		// filename=\"=?koi8-r
		/*else if (First = Is_Finded_Filename(Buffer, M_KOI8);
				 First != string::npos) {

			string Filename_Code = Get_Filename_Start(Buffer, First, M_KOI8);
			Filename_Code += Get_Filename_Continue(Source, '%');

			Result = Ascii_To_Hex(Filename_Code, '%');
			Replace_Forbidden_Symbol(Result);
		}*/
		// filename=\"
		else if (First = Is_Finded_Filename(name, M_EMPTY);
				 First != string::npos) {

			Result = Get_Filename_Start(name, First, M_EMPTY);
			Delete_Forbidden_Symbol(Result);
		}
		// 
		else {

		}

		std::string mail;
		auto mail_end{
			std::search(
				it, std::cend(data),
				std::cbegin(SCOB_RIGHT), std::cend(SCOB_RIGHT))
		};
		std::copy(
			name_end, mail_end,
			std::back_inserter(mail));

		return std::vector<std::string>{name, mail};
	}
	return {};
}
constexpr auto _msg = [](const auto& data)
{
	return data;
};

void ParserEml::Parsing()
{
	fs::path dir{ mSettings.value(DIR).toString().toStdString() };

	std::vector<std::vector<char>> datas{
		fs::directory_iterator{ dir }
		| views::remove_if(fops::is_not_file)
		| views::transform(fops::read_file)
	};

	std::vector<std::vector<std::vector<std::string>>> out(datas.size());

	std::transform(
		std::execution::par,
		std::cbegin(datas),
		std::cend(datas),
		std::begin(out),
		[this](const auto& data)
		{
			auto d1{ date(data).value() };

			auto d2{ sender(data).value() };

			auto d3{ recipient(data).value() };

			return std::vector<std::vector<std::string>>{d1, d2, d3};
		}
	);

	for (const auto& elm : out)
	{
		if (!mDb.find("date", TIME, elm[0][4].c_str()))
		{
			mDb.insert(
				"date",
				std::make_pair(WEEK, QString{ elm[0][0].c_str() }),
				std::make_pair(DAY, QString{ elm[0][1].c_str() }),
				std::make_pair(MONTH, QString{ elm[0][2].c_str() }),
				std::make_pair(YEAR, QString{ elm[0][3].c_str() }),
				std::make_pair(TIME, QString{ elm[0][4].c_str() })
			);
		}
		if (!mDb.find("sender", NAME, elm[1][0].c_str()))
		{
			mDb.insert(
				"sender",
				std::make_pair(NAME, QString{ elm[1][0].c_str() }),
				std::make_pair(MAIL, QString{ elm[1][1].c_str() })
			);
		}
		if (!mDb.find("recipient", NAME, elm[2][0].c_str()))
		{
			mDb.insert(
				"recipient",
				std::make_pair(NAME, QString{ elm[2][0].c_str() }),
				std::make_pair(MAIL, QString{ elm[2][1].c_str() })
			);
		}

		auto e1{ mDb.get_element2("date", "time", elm[0][4].c_str()) };
		auto e2{ mDb.get_element2("sender", "name", elm[1][0].c_str()) };
		auto e3{ mDb.get_element2("recipient", "name", elm[2][0].c_str()) };
		mDb.insert2(
			"mail",
			e1, e2, e3
		);

		int row{ ui.TableResult->rowCount() };
		ui.TableResult->insertRow(row);
		ui.TableResult->setItem(row, 0, new QTableWidgetItem{ elm[0][4].c_str() });
		ui.TableResult->setItem(row, 1, new QTableWidgetItem{ elm[1][0].c_str() });
		ui.TableResult->setItem(row, 2, new QTableWidgetItem{ elm[2][0].c_str() });
	}
}