
class Base64
{
public:
	Base64();
	~Base64();

	void Decode_Directory(fs::path Dir);
	bool Decode_File(fs::path File_Path);
	bool Decode_File(fs::path File_Path, fs::path Out_Dir);
	std::string Decode_String(const std::string_view in);

private:
	const uint32_t CHUNK = 16384;
	const bool EMPTY_FILE = false;
	const bool GOOD = true;
	const bool FAIL = false;
};

