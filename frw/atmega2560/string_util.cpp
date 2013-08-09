//String Utility Source (Arduino Version)
//	Created By:	Mike Moss
//	Modified On:	03/12/2013

//Definitions for "string_util.h"
#include "string_util.h"

//String Stream Header
#include <sstream>

//Convert To Boolean Function
bool msl::to_bool(std::string value)
{
	for(unsigned int ii=0;ii<value.size();++ii)
		if(value[ii]>='A'&&value[ii]<='Z')
			value[ii]+=32;

	return !(value=="false"||(value.size()==1&&value=="0"));
}

//Convert To Double Function
double msl::to_double(const std::string& value)
{
	//Return Converted Value
	return atof(value.c_str());
}

//Convert To Int Function
int msl::to_int(const std::string& value)
{
	//Convert Variables
	int temp=0;
        int sign=1;

        //Get Sign
        std::string sign_value=value;

        if(sign_value.size()>0)
          if(sign_value[0]=='-')
            sign=-1;

        if(sign_value.size()>0&&(sign_value[0]=='-'||sign_value[0]=='+'))
          sign_value.erase(0,1);

	//Convert
        for(unsigned int ii=0;ii<sign_value.size();++ii)
        {
          if(sign_value[ii]<'0'||sign_value[ii]>'9')
            break;

          int place=1;

          for(unsigned int jj=0;jj<sign_value.size()-1-ii;++jj)
            place*=10;

          temp+=(sign_value[ii]-'0')*place;
        }

	//Return Converted Value
	return temp*sign;
}

//Convert To Char Function
char msl::to_char(const std::string& value)
{
	//Integers Will Work!
	return to_int(value);
}

//Convert To Unsigned Char Function
unsigned char msl::to_uchar(const std::string& value)
{
	//Integers Will Work!
	int temp=to_int(value);

	//Unsigned Conversion Factor
	while(temp<0)
		temp+=256;

	//Return Converted Value
	return temp;
}

//Starts With Function (Checks if string starts with another string)
bool msl::starts_with(const std::string& str,const std::string& start)
{
	//Check if Sizes are Comparable
	if(start.size()<=str.size())
	{
		//Check Each Character
		for(unsigned int ii=0;ii<start.size();++ii)
			if(str[ii]!=start[ii])
				return false;

		//If Match
		return true;
	}

	//Default False
	return false;
}

//Ends With Function (Checks if string ends with another string)
bool msl::ends_with(const std::string& str,const std::string& end)
{
	//Check if Sizes are Comparable
	if(end.size()<=str.size())
	{
		//Check Each Character
		for(unsigned int ii=1;ii<end.size();++ii)
			if(str[str.size()-ii]!=end[end.size()-ii])
				return false;

		//If Match
		return true;
	}

	//Default False
	return false;
}
