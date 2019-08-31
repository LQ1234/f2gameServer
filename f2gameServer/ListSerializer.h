#pragma once
#include <vector>
#include <string> 
#include <sstream>
#include <iostream>

class ListSerializer {
public:

	enum dataType {
		STRING,
		UINT,
		FLOAT,
		INTR,
		BYTE,
		XYPAIRS
	};

	void setClassAttributes(std::vector<dataType> newCurrObjectAttributeTypes);
	void addObjectAttributes(void* a[]);
	std::string serialize();


private:
	std::vector<dataType> currObjectAttributeTypes;
	std::vector<std::string> serializedClasses;
	std::ostringstream currSerializedStream;
	void pushToBuffer();
};

class Serializable {
public:
	virtual void** getAttributes() = 0;

};