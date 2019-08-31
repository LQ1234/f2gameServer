#include <vector>
#include <string> 
#include <sstream>
#include <iostream>
#include "ListSerializer.h"


void ListSerializer::setClassAttributes(std::vector<ListSerializer::dataType> newCurrObjectAttributeTypes) {
	if (currObjectAttributeTypes.size() > 0)
		pushToBuffer();

	currObjectAttributeTypes = newCurrObjectAttributeTypes;
}
void ListSerializer::addObjectAttributes(void* a[]) {
	int ci = 0;
	for (auto i = currObjectAttributeTypes.begin(); i != currObjectAttributeTypes.end(); i++) {
		void* thisobj = a[ci];
		switch (*i) {
		case STRING: {
			std::string asStr = *static_cast<std::string*>(thisobj);
			size_t strSize = (asStr.size());
			currSerializedStream.write(reinterpret_cast<char const*>(&(strSize)), sizeof(size_t));//4
			currSerializedStream.write(asStr.data(), strSize);//unk

		}
					 break;
		case UINT:
			currSerializedStream.write(reinterpret_cast<char const*>(thisobj), sizeof(unsigned int));//4

			break;
		case FLOAT:
			currSerializedStream.write(reinterpret_cast<char const*>(thisobj), sizeof(float));//4

			break;
		case INTR:
			currSerializedStream.write(reinterpret_cast<char const*>(thisobj), sizeof(int));//4

			break;
		case BYTE://unsigned char
			currSerializedStream.write(reinterpret_cast<char const*>(thisobj), sizeof(char));//1

			break;
		case XYPAIRS: {
			std::vector<float> asArr = *static_cast<std::vector<float>*>(thisobj);
			size_t byteSze = asArr.size() * 4;

			currSerializedStream.write(reinterpret_cast<char const*>(&(byteSze)), sizeof(size_t));//4

			currSerializedStream.write(reinterpret_cast<char const*>(&asArr[0]), byteSze);//4

		}
					  break;

		}
		ci++;
	}
	delete[] a;
}
void ListSerializer::pushToBuffer() {

	serializedClasses.push_back(currSerializedStream.str());
	std::ostringstream().swap(currSerializedStream);
}

std::string ListSerializer::serialize() {

	pushToBuffer();
	/*
	std::cout << "szes:" <<  "\n";

	for (auto i = serializedClasses.begin(); i != serializedClasses.end(); ++i) {
		std::cout<<"sze:"<<(*i).size()<<"\n";
	}

	for (auto i = serializedClasses.begin(); i != serializedClasses.end(); ++i) {
		std::cout << "cont:";
		printStringBytes(*i);
		std::cout << "\n";
	}*/
	std::ostringstream fnal;
	for (auto i = serializedClasses.begin(); i != serializedClasses.end(); ++i) {
		size_t tsize = (*i).size();
		fnal.write(reinterpret_cast<char const*>(&tsize), sizeof(size_t));//4
	}
	for (auto i = serializedClasses.begin(); i != serializedClasses.end(); ++i) {
		fnal.write((*i).data(), (*i).size());//4
	}
	return(fnal.str());
}

