#include "NameProcessor.h"


NameProcessor::NameProcessor()
: b_CapitalLetter(false)
, b_newHighScore(false)
, c_CharToBeAdded(' ')
, i_NameCharCount(0)
{
}


NameProcessor::~NameProcessor()
{
}

string NameProcessor::GetName(void)
{
	return s_Name;
}

void NameProcessor::SetName(string newName)
{
	this->s_Name = newName;
}

int NameProcessor::GetNameCharCount(void)
{
	return i_NameCharCount;
}

void NameProcessor::AddNameCharCount(void)
{
	++this->i_NameCharCount;
}

void NameProcessor::SubtractNameCharCount(void)
{
	--this->i_NameCharCount;
}

bool NameProcessor::GetCapitalLetter(void)
{
	return b_CapitalLetter;
}

void NameProcessor::SetCapitalLetter(const bool newCapitalLetter)
{
	this->b_CapitalLetter = newCapitalLetter;
}

char NameProcessor::GetCharToAdd(void)
{
	return c_CharToBeAdded;
}

void NameProcessor::SetCharToAdd(const char newInputChar)
{
	if (vec_NameInput.size() < MAXNAMELENGTH)
		this->c_CharToBeAdded = newInputChar;
}

void NameProcessor::SetCharToRemove(void)
{
	if (vec_NameInput.size() > 0)
		vec_NameInput.pop_back();
}

void NameProcessor::SetNameInput(void)
{
	vec_NameInput.push_back(this->c_CharToBeAdded);
	this->c_CharToBeAdded = ' ';
}

vector<char> NameProcessor::GetNameChar(void)
{
	return vec_NameInput;
}