#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using std::vector;

using std::string;
using std::ostream;

#define MAXNAMELENGTH 14

class NameProcessor
{
private:
	string s_Name;

	//High score writing
	bool b_CapitalLetter;
	bool b_newHighScore;
	char c_CharToBeAdded;
	int i_NameCharCount;

	vector<char> vec_NameInput;

public:
	NameProcessor();
	~NameProcessor();

	//Get and Set Functions
	string GetName(void);
	int GetNameCharCount(void);
	void AddNameCharCount(void);
	void SubtractNameCharCount(void);

	bool GetCapitalLetter(void);
	void SetCapitalLetter(const bool newCapitalLetter);

	void SetCharToAdd(const char newChar);
	void SetCharToRemove(void);
	char GetCharToAdd(void);

	void SetNameInput(void);
	vector<char> GetNameChar(void);
	void SetName(string newName);
};

