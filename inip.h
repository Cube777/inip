/*
_________  ____ __________________________________________________
\_   ___ \|    |   \______   \_   _____|______  \______  \______  \
/    \  \/|    |   /|    |  _/|    __)_    /    /   /    /   /    /
\     \___|    |  / |    |   \|        \  /    /   /    /   /    /
\________/|______/  |________/_________/ /____/   /____/   /____/

Developer: Kobus van Schoor alias Cube777
Email: pbscube@gmail.com

You are free to use or modify any part of this code as long as you acknowledge
the original developer.
*/

/*
This is a simple but feature rich INI file parser and editor. It supports parsing
INI files created with the standard syntax as follows:

#This a comment
[section1]
	key=value

The delimiter as well as the comment identifier can be changed be editing the value of
'INI_DELIMITER' and 'INI_COMMENT' defined below.
*/
#pragma once
#include <string>
#include <vector>
#include <fstream>

//Defines the delimiter char
#define INI_DELIMITER '='

//Defines the comment char
#define INI_COMMENT '#'

namespace inip
{
class iniParser
{
public:
	iniParser(std::string filename);

	//Reparse the current INI file, returns false if
	//the INI file could not be found or if there was an error
	//in the file or if the file could not be read
	bool Reparse();

	//Returns the value of a key, returns an empty string if
	//the section or property you specified does not exist
	std::string ReturnValue(std::string section, std::string key);

	//Create a new section in the INI file, returns false if
	//section name is already used
	bool CreateSection(std::string section);

	//Add a property to the INI file, returns false if
	//the key name is already used in the section or if
	//the section you specified does not exist
	bool AddProperty(std::string section, std::string key,
	                 std::string value = "");

	//Change the value of an already existing key, returns
	//false if the section or the key you specified does not exist
	bool ChangeProperty(std::string section, std::string key,
	                    std::string newValue = "");

	//Delete a section, also deletes any properties and comments that
	//the section contains. Returns false if the section specified does
	//not exist
	bool DeleteSection(std::string section);

	//Deletes a property, returns false if the section or property
	//you specified does not exist
	bool DeleteProperty(std::string section, std::string key);

	//This will write any changes made to the INI file, returns false
	//if there was an error writing to the file
	bool ApplyChanges();

	//If AlwaysApply is set to true, any changes made to the INI file
	//will be immediately saved, otherwise ApplyChanges() have to be
	//called first before the changes will be written to the file.
	//AlwaysApply defaults to false.
	void SetAlwaysApply(bool value);

	//Returns true if the file was correctly parsed and there was no errors.
	//Possible reasons for parse errors are:
	//-Empty section names
	//-Section names don't have 2 brackets
	//-Keys have no delimiter
	bool IsParsed();

private:
	//Container for all the properties, some of the iniParser
	//functions are merely wrappers for the functions defined
	//in the FileProperties class
	class FileProperties
	{
	public:
		enum LineType { Section, Key, Comment, BlankLine };
		struct Line;

		bool CreateSection(std::string section);
		bool DeleteSection(std::string section);

		bool AddProperty(std::string section, std::string rawline);
		//Overload should the you want to add a new property that
		//is not inside the INI file
		bool AddProperty(std::string section, std::string key,
		                 std::string value);
		bool ChangeProperty(std::string section, std::string key,
		                    std::string newValue);
		bool DeleteProperty(std::string section, std::string key);

		std::string ReturnValue(std::string section, std::string key);
		bool AddRawLine(Line newLine);

		int AmountOfLines();
		Line* ReturnLine(int line);

		bool DoesSectionExist(std::string section);
		bool DoesKeyExist(std::string section, std::string key);
		Line* ReturnKeyRef(std::string section, std::string key);
		void ClearAllLines();

		struct Line {
		public:
			Line(LineType ltype, std::string rawline);

			LineType _lineType;
			std::string _rawString;
			std::string _key;
			std::string _value;
			std::string _section;
			bool _parsed;
		};

	private:
		std::vector<Line> _lines;
	};

	FileProperties _properties;

	std::string _filename;
	bool _alwaysApply;
	bool _parsed;
};
}
