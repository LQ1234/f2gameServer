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
	ListSerializer::dataType* adarr = currObjectAttributeTypes.data();
	for (int in = 0,  sz = currObjectAttributeTypes.size(); in < sz;in++) {
		void* thisobj = a[ci];
		switch (adarr[in]) {
		case STRING: {
			std::string asStr = *static_cast<std::string*>(thisobj);

			uint32_t strSize = (asStr.size());
			currSerializedStream.write(reinterpret_cast<char const*>(&(strSize)), sizeof(uint32_t));//4
			currSerializedStream.write(asStr.data(), strSize);//unk

		}
			break;
		case UINT:
		{
			uint32_t as32Bit=*reinterpret_cast<unsigned int const*>(thisobj);
			currSerializedStream.write(reinterpret_cast<char const*>(&as32Bit), sizeof(uint32_t));//4
			}
			break;
		case FLOAT:

			currSerializedStream.write(reinterpret_cast<char const*>(thisobj), sizeof(float));//4

			break;
		case INTR:
		{
			int32_t as32Bit=*reinterpret_cast<int const*>(thisobj);

			currSerializedStream.write(reinterpret_cast<char const*>(&as32Bit), sizeof(int32_t));//4
		}
			break;
		case BYTE://unsigned char
			currSerializedStream.write(reinterpret_cast<char const*>(thisobj), sizeof(char));//1

			break;
		case XYPAIRS: {
			std::vector<float> asArr = *static_cast<std::vector<float>*>(thisobj);
			uint32_t byteSze = asArr.size() * 4;

			currSerializedStream.write(reinterpret_cast<char const*>(&(byteSze)), sizeof(uint32_t));//4

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
		std::string str=*i;
		char const *cstr = str.c_str();

		for(int i=0;i<str.length();i++){
			std::cout<<(cstr[i]%256+256)%256<<" ";
		}
		std::cout << "\n";
	}*/
	std::ostringstream fnal;
	for (auto i = serializedClasses.begin(); i != serializedClasses.end(); ++i) {
		uint32_t tsize = (*i).size();
		fnal.write(reinterpret_cast<char const*>(&tsize), sizeof(uint32_t));//4
	}
	for (auto i = serializedClasses.begin(); i != serializedClasses.end(); ++i) {
		fnal.write((*i).data(), (*i).size());//4
	}
	return(fnal.str());
}
