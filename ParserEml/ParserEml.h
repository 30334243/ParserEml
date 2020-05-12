#pragma once
#include "ui_ParserEml.h"

#include <db.hpp>

static const QString GDATE{ "Date: " };
static const QString TO{ "To: " };
static const QString FROM{ "From: " };
static const QString ENTER{ "\n" };

const bool REMOVE = true;
const bool NO_REMOVE = false;
const bool GOOD = true;
const bool FAIL = false; 
const uint32_t CHUNK = 16384;

const size_t SIZE_FILENAME = 13; // Size filename*1*=
const size_t SIZE_1251 = 18; // =?windows-1251?B?
const size_t SIZE_KOI8B = 12;

class ParserEml : public QMainWindow
{
	Q_OBJECT

public:
	ParserEml(QWidget *parent = Q_NULLPTR);

private:
	Ui::ParserEmlClass ui;

	const QStringList COLUMN_LABELS{ QStringList() << "Время" << "Отправитель" << "Получатель" };
	QSettings mSettings;
	db mDb{ "emails" };

	const QString DIR{ "/Directory" };

	const QString TIME{ "time" };
	const QString DAY{ "day" };
	const QString WEEK{ "week" };
	const QString YEAR{ "year" };
	const QString MONTH{ "month" };

	const QString NAME{ "name" };
	const QString MAIL{ "mail" };

	const QString SCOB{ "<" };
	const QString SCOB_RIGHT{ ">" };

	std::optional<std::vector<std::string>> date(const std::vector<char>& date);
	std::optional<std::vector<std::string>> sender(const std::vector<char>& data);
	std::optional<std::vector<std::string>> recipient(const std::vector<char>& data);

private:

	const std::string M_UTF8_ONE { "filename*=UTF-8''" }; // ascii to hex delim %
	const std::string M_1251_ONE { "filename*=windows-1251''" }; // ascii to hex delim %
	const std::string M_UTF8_MULTI_U { "filename*0*=UTF-8''" }; // ascii to hex delim %
	const std::string M_UTF8_MULTI_D { "filename*0*=utf-8''" }; // ascii to hex delim %
	const std::string M_1251_MULTI { "filename*0*=windows-1251''" }; // ascii to hex delim %
	const std::string M_1251Q { "filename=\"=?windows-1251?Q?" }; // ascii to hex delim =
	const std::string M_1252Q_2 { "filename==?windows-1251?Q?" }; // ascii to hex delim =
	const std::string M_1251B { "filename=\"=?windows-1251?B?" }; // base64
	const std::string M_1251B_2 { "filename==?windows-1251?B?" }; // base64
	const std::string M_KOI8B { "filename=\"=?koi8-r?B?" }; // base64
	const std::string M_KOI8 { "filename*0*=koi8-r''" }; // ascii to hex delim %
	const std::string M_EMPTY { "filename=\"" }; // empty simple name

private:
	
public slots:
	void AddLst();
	void Parsing();
};
