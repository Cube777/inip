#include "inip.h"

using namespace inip;

iniParser::iniParser(std::string filename) :
	_filename(filename),
	_alwaysApply(false),
	_parsed(false)
{
	this->Reparse();
}

bool iniParser::Reparse()
{
	std::ifstream fileRead(_filename.c_str());
	//Return false if there was an error reading the file
	if (!fileRead)
		return false;
	_properties.ClearAllLines();

	std::string tempRead;
	std::string currentSection;
	while (std::getline(fileRead, tempRead)) {
		if (tempRead[0] == '[') {
			//Return false if section syntax is incorrect
			if (tempRead.back() != ']')
				return false;

			currentSection.clear();
			for (int i = 1; (i + 1) < tempRead.size(); i++)
				currentSection += tempRead[i];
			if (!_properties.CreateSection(currentSection))
				return false;
			continue;
		}
		if ((tempRead[0] != INI_COMMENT) && (!tempRead.empty())) {
			FileProperties::Line newLine(iniParser::FileProperties::Key, tempRead);
			newLine._section = currentSection;;
			if (!_properties.AddRawLine(newLine))
				return false;
			continue;
		}
		if (!tempRead.empty()) {
			FileProperties::Line newLine(iniParser::FileProperties::Comment, tempRead);
			if (!_properties.AddRawLine(newLine))
				return false;
		} else {
			FileProperties::Line newLine(iniParser::FileProperties::BlankLine, tempRead);
			if (!_properties.AddRawLine(newLine))
				return false;
		}
	}
	_parsed = true;
	return true;
}

std::string iniParser::ReturnValue(std::string section, std::string key)
{
	return _properties.ReturnValue(section, key);
}

bool iniParser::CreateSection(std::string section)
{
	bool success = _properties.CreateSection(section);
	if (success && _alwaysApply)
		return this->ApplyChanges();
	return success;
}

bool iniParser::AddProperty(std::string section, std::string key, std::string value /* = "" */)
{
	bool success = _properties.AddProperty(section, key, value);
	if (success && _alwaysApply)
		return this->ApplyChanges();
	return success;
}

bool iniParser::ChangeProperty(std::string section, std::string key, std::string newValue /* = "" */)
{
	bool success = _properties.ChangeProperty(section, key, newValue);
	if (success && _alwaysApply)
		return this->ApplyChanges();
	return success;
}

bool iniParser::DeleteSection(std::string section)
{
	bool success = _properties.DeleteSection(section);
	if (success && _alwaysApply)
		return this->ApplyChanges();
	return success;
}

bool iniParser::DeleteProperty(std::string section, std::string key)
{
	bool success = _properties.DeleteProperty(section, key);
	if (success && _alwaysApply)
		return this->ApplyChanges();
	return success;
}

bool iniParser::ApplyChanges()
{
	std::ofstream fileOut(_filename.c_str());
	//Return false if there was an error opening the file
	if (!fileOut)
		return false;

	for (int i = 0; i < _properties.AmountOfLines(); i++) {
		fileOut << _properties.ReturnLine(i)->_rawString;
		if ((i + 1 != _properties.AmountOfLines()) &&
		        (_properties.ReturnLine(i)->_lineType != iniParser::FileProperties::BlankLine))
			fileOut << '\n';
	}
	return true;
}

void iniParser::SetAlwaysApply(bool value)
{
	this->_alwaysApply = value;
}

bool iniParser::IsParsed()
{
	return _parsed;
}

iniParser::FileProperties::Line::Line(LineType ltype, std::string rawline):
	_lineType(ltype),
	_rawString(rawline),
	_parsed(false)
{
	if (_lineType == iniParser::FileProperties::Section) {
		for (int i = 1; i + 1 < _rawString.size(); i++)
			this->_section += _rawString[i];
		_parsed = true;
		return;
	}
	if (_lineType == iniParser::FileProperties::Key) {
		int i = 0;
		for (; (i < _rawString.size()) && (_rawString[i] != INI_DELIMITER); i++)
			this->_key += _rawString[i];
		//Leave unparsed if key syntax is incorrect
		if (_rawString[i] != INI_DELIMITER)
			return;

		i++;
		for (; i < _rawString.size(); i++)
			this->_value += _rawString[i];

		_parsed = true;
		return;
	}
	if (_lineType == iniParser::FileProperties::BlankLine)
		_rawString = "\n";
	if ((_lineType == iniParser::FileProperties::Comment) ||
	        (_lineType == iniParser::FileProperties::BlankLine))
		_parsed = true;
}

bool iniParser::FileProperties::CreateSection(std::string section)
{
	//Return false if section name is empty
	if (section == "")
		return false;

	//Check if the section name exists and return false if it does
	if (DoesSectionExist(section))
		return false;

	section = "[" + section + "]";
	Line newSection(iniParser::FileProperties::Section, section);
	return this->AddRawLine(newSection);
}

bool iniParser::FileProperties::DeleteSection(std::string section)
{
	//Return false if section name is empty
	if (section == "")
		return false;

	//Returns false if section doesn't exist
	if (!DoesSectionExist(section))
		return false;

	auto itr = _lines.begin();

	while (true) {
		for (; itr->_lineType != iniParser::FileProperties::Section; itr++);
		if (itr->_section == section)
			break;
		itr++;
	}

	itr = _lines.erase(itr);
	while ((itr != _lines.end()) && (itr->_lineType != iniParser::FileProperties::Section))
		itr = _lines.erase(itr);
	return true;
}

bool iniParser::FileProperties::AddProperty(std::string section, std::string rawline)
{
	//Returns false if section doesn't exist
	if (!DoesSectionExist(section))
		return false;

	Line newKey(iniParser::FileProperties::Key, rawline);
	newKey._section = section;
	//Return false if key syntax is incorrect
	if (!newKey._parsed)
		return false;

	//Return false if key exists
	if (DoesKeyExist(newKey._section, newKey._key))
		return false;

	return this->AddRawLine(newKey);
}

bool iniParser::FileProperties::AddProperty(std::string section, std::string key, std::string value)
{
	return this->AddProperty(section, (key + INI_DELIMITER + value));
}

bool iniParser::FileProperties::ChangeProperty(std::string section, std::string key, std::string newValue)
{
	//Returns false if key does not exist
	if (!DoesKeyExist(section, key))
		return false;

	Line* targetKey = ReturnKeyRef(section, key);
	targetKey->_value = newValue;
	targetKey->_rawString = targetKey->_key + INI_DELIMITER + targetKey->_value;
	return true;
}

bool iniParser::FileProperties::DeleteProperty(std::string section, std::string key)
{
	//Return false if key doesn't exist
	if (!DoesKeyExist(section, key))
		return false;

	Line* target = ReturnKeyRef(section, key);
	auto itr = _lines.begin();
	for (; &(*itr) != target; itr++);
	_lines.erase(itr);
	return true;
}

std::string iniParser::FileProperties::ReturnValue(std::string section, std::string key)
{
	//Return null terminated string if key doesn't exist
	if (!DoesKeyExist(section, key))
		return "\0";

	return ReturnKeyRef(section, key)->_value;
}

bool iniParser::FileProperties::AddRawLine(Line newLine)
{
	//Return false if the line wasn't correctly parsed
	if (!newLine._parsed)
		return false;

	if (newLine._lineType == iniParser::FileProperties::Key) {
		auto itr = _lines.begin();
		while (true) {
			for (; itr->_lineType != iniParser::FileProperties::Section; itr++);
			if (itr->_section == newLine._section)
				break;
			itr++;
		}
		itr++;
		for (; (itr != _lines.end()) && (itr->_lineType != iniParser::FileProperties::Section); itr++);
		_lines.insert(itr, newLine);
		return true;
	}
	_lines.push_back(newLine);
	return true;
}

int iniParser::FileProperties::AmountOfLines()
{
	return _lines.size();
}

iniParser::FileProperties::Line* iniParser::FileProperties::ReturnLine(int line)
{
	Line* foo = &_lines[line];
	return foo;
}

bool iniParser::FileProperties::DoesSectionExist(std::string section)
{
	int i = 0;
	while (true) {
		for (; (i < _lines.size()) && (_lines[i]._lineType != iniParser::FileProperties::Section); i++);
		if (i == _lines.size())
			return false;
		if (_lines[i]._section == section)
			return true;
		i++;
	}
}

bool iniParser::FileProperties::DoesKeyExist(std::string section, std::string key)
{
	if (!DoesSectionExist(section))
		return false;

	auto itr = _lines.begin();
	while (true) {
		for (; itr->_lineType != iniParser::FileProperties::Section; itr++);
		if (itr->_section == section)
			break;
		itr++;
	}
	itr++;
	for (; (itr != _lines.end()) &&
	        (itr->_lineType != iniParser::FileProperties::Section) &&
	        (itr->_key != key); itr++);
	if (itr == _lines.end())
		return false;
	if (itr->_key == key)
		return true;
	else
		return false;
}

iniParser::FileProperties::Line* iniParser::FileProperties::ReturnKeyRef(std::string section, std::string key)
{
	if (!DoesKeyExist(section, key))
		return NULL;

	auto itr = _lines.begin();
	for (; (itr->_lineType != iniParser::FileProperties::Section) && (itr->_section != section); itr++);
	itr++;
	for (; itr->_key != key; itr++);
	return &(*itr);
}

void iniParser::FileProperties::ClearAllLines()
{
	_lines.clear();
	return;
}
