#ifndef BASE64_H
#define BASE64_H

#include <string>
#include <string.h>
#include <bitset>



// Function take an integer value and convert into alphabet 
// according to the RFC 4648 Table used for Base64 Encoding
char rfc4648Convertor(int indx){
	if(indx<26){
		return 65+indx;
	}else if(indx<52){
		return 97+(indx-26);
	}else if(indx<62){
		return indx-4;
	}else if(indx == 62) return '+';
	else if (indx == 63) return '/';
	else return -1;
}

std::string getBase64(char* input, int length){
	std::string tbytes, enString = "";
	char bytes[7];
	

	int bindx =0, remain_bits = 0;


	// Create sextets from the bits of data and convert into base64
	for(int idx = 0; idx < length; idx++){
		int iidx =0;
		if(remain_bits > 0){
			//std::cout<<tbytes;
			iidx = 8-remain_bits;
			while (iidx < 8 && bindx < 6) {
				bytes[bindx++] = tbytes[iidx++];
			}

			remain_bits = 8-iidx;
		}

		if(bindx < 6){
			tbytes = std::bitset<8>(input[idx]).to_string();
			//std::cout<<" "<<input[idx]<<"="<<tbytes<<" "<<"\n";


			iidx= 0;
			int rem= 8-bindx;
			while (iidx < rem && bindx < 6) {
				bytes[bindx++] = tbytes[iidx++];
				
			}

			remain_bits = 8-iidx;
		}else {
			idx--;
		}


		if(bindx == 6){
			bytes[6] = '\0';
			int sextet = std::stoi(bytes, nullptr, 2);
			//std::cout<<bytes<<" "<<sextet<<" "<<rfc4648Convertor(sextet)<<std::endl<<"\n\n";
			enString.insert(enString.size(), 1, rfc4648Convertor(sextet));
			bindx =0;
		}
	}

	// Use the Remaining Bits;
	while(remain_bits != 0) {
		int iidx = 8 - remain_bits;
		while(bindx < 6){
			if(remain_bits !=0){
				bytes[bindx++] = tbytes[iidx++];
				remain_bits--;
			}else bytes[bindx++] = '0';
		}


		bytes[bindx] = '\0';
		int sextets = std::stoi(bytes, nullptr, 2);
		enString.insert(enString.size(), 1, rfc4648Convertor(sextets));
	}
	
	
	while (enString.size() % 4 != 0) {
		enString.append("=");
	}

	return enString;
}

#endif
