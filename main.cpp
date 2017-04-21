/**
 * @author Suleyman Balaban - 121044014
 * @date 19 April 2017
 * @version 1
 */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdio.h>//libraries
#include <string.h>
#include <string>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

#define oneGramFile "oneGrams.txt"
#define twoGramFile "twoGrams.txt"
#define threeGramFile "threeGrams.txt"
#define fourGramFile "fourGrams.txt"
#define fiveGramFile "fiveGrams.txt"
#define oneGramRemainingFile "remainingOneGrams.txt"
#define twoGramRemainingFile "remainingTwoGrams.txt"
#define threeGramRemainingFile "remainingThreeGrams.txt"
#define fourGramRemainingFile "remainingFourGrams.txt"
#define fiveGramRemainingFile "remainingFiveGrams.txt"

class gram {
public:
    gram();
    gram(const string word, const int count);
    string getWord()const{return word;}
    int getCount()const{return count;}
    void setWord(const string word){this->word = word;}
    void setCount(const int count){this->count= count;}
    void increaseCount(){count++;}
    int equal(vector<gram> allGram);
private:
    string word;
    int count;
    
};
#define SIZE 256 //maksimum dosya ismi boyutu

typedef enum{FALSE=0,TRUE=1}BOOL;//typedef for BOOL type

void grepFromFile(FILE *inputFile,char *fileName,int nGram,vector<gram>&grams);
void grepFromDirectory(char *directoryName,int nGram, int flag, vector<gram>&grams,int isRemain);
bool findGrams(char *directoryName,int nGram, int flag, vector<gram>grams, int isRemain);
double gramsChainMarkov(string sentence, int isRemain, int nGram);
double chainMarkovOneGram(string subSentence, int isRemain, int nGram);
int findOccInGrams(int nGram, string target, char * fileName, int & allGramSize);
int findSizeOfFile(char * fileName, int nGram);
BOOL isTxt(char *directoryName);

//Start Of Main
int main(int argc,char *argv[]) {
    int flag=0;
    char sentence[80];
    int nGram = 1;
    double result;

    if(argc!=2) {//arguman check
        fprintf(stderr,"Usage : %s [directory] \n",argv[0]);
        exit(0);
    }
    //////////////////////////////%95 news set////////////////////////////////
    /*vector<gram>grams;
    for (int i = 1; i < 6; ++i) {
    	if(findGrams(argv[1],i,flag,grams, 1) == false) {
    		cerr<<"Error in corpora. Couldn't calculate "<<i<<"-gram !"<<endl;
    		return 0;
    	}
    }*/
    //////////////////////////////%5 news set/////////////////////////////////
    /*for (int i = 1; i < 6; ++i) {
    	if(findGrams(argv[1],i,flag,grams, 1) == false) {
    		cerr<<"Error in corpora. Couldn't calculate "<<i<<"-gram !"<<endl;
    		return 0;
    	}
    }*/
    ////////////////////////////Remaining set example/////////////////////////
    /*gramsChainMarkov("Benzin ve beyaz eşya satışı arttı",1, 4);
    result = gramsChainMarkov(sentence,0, nGram);*/
    //////////////////////All set example with user///////////////////////////
    cout<<"Use interpolation: N"<<endl;
    cout<<"N-gram: ";
    cin>>nGram;
    cout << "Enter sentence: ";
    cin.get();
   	cin.get(sentence, 80);
    result = gramsChainMarkov(sentence,0, nGram);
    printf("The probability of sentence is: %.40f\n", result);
    //////////////////////////////////////////////////////////////////////////
    return 0;
}//End Of Main
/*____________________________________________________________________________________________*/
void grepFromDirectory(char *directoryName,int nGram,int flag, vector<gram>&grams,int isReamin) {  
    DIR *dirPointer;//directory pointer
    struct dirent *dirInfo;
    FILE *inputFile;
    int occuarence=0;
    char *fileName;
    pid_t pid;
    int countTxt = 0;

    dirPointer=opendir(directoryName);//open directory
    if (dirPointer==NULL) {
        perror("open directory");
        exit(1);
    }
    while ((dirInfo=readdir(dirPointer)) != NULL) {
        if(strcmp(dirInfo->d_name,".")!=0 && strcmp(dirInfo->d_name,"..")!=0) {
            flag++;//malloc for each file increase this because of recursive
            fileName=(char*)malloc(flag*SIZE*sizeof(char));
            if(fileName==NULL) {
                fprintf(stderr,"Memory error\n");
                exit(0);
            }
            strcpy(fileName,directoryName);//add tools
            strcat(fileName,"/");
            strcat(fileName,dirInfo->d_name);
            if((isTxt(fileName)==TRUE && isReamin == 0) || (isReamin !=0 && countTxt < 12 && 
            		isTxt(fileName)==TRUE)) {//if filename is txt
                grepFromFile(inputFile,fileName,nGram,grams);
                countTxt++;
            }else if(countTxt < 12) //if directory : call self so,recursive
                grepFromDirectory(fileName,nGram,flag,grams,isReamin);         
            free(fileName);//free dinamic filenName string
        } 
    }
    closedir(dirPointer);//close directory
}
/*____________________________________________________________________________________________*/
void grepFromFile(FILE *inputFile,char *fileName,int nGram,vector<gram>&grams) {
    int letter = 0;
    int targetSize=nGram+1;
    char * foundedTarget=(char*)malloc(nGram*sizeof(char));
    if((inputFile=fopen(fileName,"r"))==NULL) {//if input file couldn't open
        printf("Dosya Hatasi : %s\n",strerror(errno));
        exit(0);
    }
    //cout<<fileName<<endl;
    while(fgets(foundedTarget,targetSize,inputFile)!=NULL) {
        letter++;//for input,fseek()
        gram gram1;
        gram1.setWord(foundedTarget);
        //cout<<gram1.getWord()<<endl;
        int inIt = gram1.equal(grams);
        if(inIt!=-1){
            grams[inIt].increaseCount();
            
        }else {
            gram1.increaseCount();
            grams.push_back(gram1);
        }
        fseek(inputFile,letter,SEEK_SET);
    }   
    fclose(inputFile);//close the file
    free(foundedTarget);//free string
}
/*____________________________________________________________________________________________*/
BOOL isTxt(char *directoryName) {
    DIR *directoryPointer;
    directoryPointer=opendir(directoryName);//open directory
    if (directoryPointer==NULL) {//if txt
        closedir(directoryPointer);
        return TRUE;
    }
    closedir(directoryPointer);//else directory
    return FALSE;
}
/*____________________________________________________________________________________________*/
gram::gram() {
    this->word = ' ';
    this->count = 0;
}
/*____________________________________________________________________________________________*/
gram::gram(const string word, const int count) {
    this->word = word;
    this->count = count;
}
/*____________________________________________________________________________________________*/
int gram::equal(vector<gram> allGram) {
    for (int i = 0; i < allGram.size(); ++i) {
        if(allGram[i].getWord() == this->getWord())
            return i;
    }
    return -1;
}
/*____________________________________________________________________________________________*/
bool findGrams(char *directoryName,int nGram, int flag, vector<gram>grams, int isRemain) {
	ofstream myfile;

	grepFromDirectory(directoryName,nGram,flag,grams,isRemain);

	switch(nGram) {
		case 1:
			if(isRemain == 0)
				myfile.open (oneGramFile);
			else
				myfile.open (oneGramRemainingFile);
			break;
		case 2 :
			if(isRemain == 0)
				myfile.open (twoGramFile);
			else
				myfile.open (twoGramRemainingFile);
			break;
		case 3:
			if(isRemain == 0)
				myfile.open (threeGramFile);
			else
				myfile.open (threeGramRemainingFile);
			break;
		case 4:
			if(isRemain == 0)
				myfile.open (fourGramFile);
			else
				myfile.open (fourGramRemainingFile);
			break;
		case 5:
			if(isRemain == 0)
				myfile.open (fiveGramFile);
			else
				myfile.open (fiveGramRemainingFile);
			break;
		default :
			return false;
	}
    
    for (int i = 0; i < grams.size(); ++i) {
        myfile<<grams[i].getWord()<<grams[i].getCount()<<endl;
        cout<<grams[i].getWord()<<" / "<<grams[i].getCount()<<endl;
    }
    myfile.close();
    return true;

}
/*____________________________________________________________________________________________*/
double gramsChainMarkov(string sentence, int isRemain, int nGram) {
	double result = 0, multiply = 1;

	if(sentence.size() < nGram)
		return -1.0;
	for (int i = 0; i <= sentence.size()-nGram; ++i) {
		string left;
		for (int j = 0; j < nGram; ++j) 
			left.insert(j,1,sentence[i+j]);
		//cout<<"-----------------------------------------------------------------"<<endl;
		multiply*=chainMarkovOneGram(left,isRemain,nGram);
	}
	return multiply;
}
/*____________________________________________________________________________________________*/
double chainMarkovOneGram(string subSentence, int isRemain, int nGram) {
	ifstream myfile1, myfile2, myfile3, myfile4, myfile5;
	double result = 0, upResult = 0, downResult = 0;
	string up, down;
	char fileName1[30] = "template.txt";
	char fileName2[30] = "template.txt";

	up = subSentence;
	for (int j = 0; j < nGram; ++j) {
		if(j<nGram-1) {
			down.insert(j,1,subSentence[j]);
		}
	}
	if(isRemain != 1) {
		if(nGram == 1)
			strcpy(fileName1,oneGramFile);
		else if(nGram == 2){
			strcpy(fileName1,twoGramFile);
			strcpy(fileName2,oneGramFile);
		}
		else if(nGram == 3){
			strcpy(fileName1,threeGramFile);
			strcpy(fileName2,twoGramFile);
		}
		else if(nGram == 4){
			strcpy(fileName1,fourGramFile);
			strcpy(fileName2,threeGramFile);
		}
		else if(nGram == 5){
			strcpy(fileName1,fiveGramFile);
			strcpy(fileName2,fourGramFile);
		}
	}else {
		if(nGram == 1)
			strcpy(fileName1,oneGramRemainingFile);
		else if(nGram == 2){
			strcpy(fileName1,twoGramRemainingFile);
			strcpy(fileName2,oneGramRemainingFile);
		}
		else if(nGram == 3){
			strcpy(fileName1,threeGramRemainingFile);
			strcpy(fileName2,twoGramRemainingFile);
		}
		else if(nGram == 4){
			strcpy(fileName1,fourGramRemainingFile);
			strcpy(fileName2,threeGramRemainingFile);
		}
		else if(nGram == 5){
			strcpy(fileName1,fiveGramRemainingFile);
			strcpy(fileName2,fourGramRemainingFile);
		}
	}
	if(nGram == 1) {
		int sizeFile = 0;
		int occ = findOccInGrams(nGram,up,fileName1, sizeFile);

		if(occ == 0)
			return 1;
		return (double)occ/sizeFile;
	}
	else {
		int upSizeFile = 0, downSizeFile = 0;
		int upOcc = findOccInGrams(nGram,up,fileName1,upSizeFile);
		int downOcc = findOccInGrams(nGram-1,down,fileName2,downSizeFile);
		upResult = (double)upOcc / upSizeFile;
		downResult = (double)downOcc / downSizeFile;
	}
	if(upResult == 0)
		return 1;
	return upResult / downResult;
}
/*____________________________________________________________________________________________*/
int findOccInGrams(int nGram, string target, char * fileName, int & allGramSize) {
	//ifstream input;
	ifstream input( fileName);
	int count = 0, countChars = 0, occ = 0, occTemp = 0;
	string var;

	while(getline(input,var)) {
		for (int i = 0; i < nGram; ++i) {
			if(target[i] == var[i])
				countChars++;
		}
		if(var.size()>=nGram) {
			if(input!=0){
				count++;
				string str = var.substr(nGram);
				occTemp = atoi(str.c_str());
				allGramSize += occTemp;
			}
			if(input!=0 && countChars == nGram) {
				occ = occTemp;
			}
		}
		
		countChars = 0;
	}
	input.close();	
	return occ;
}
/*____________________________________________________________________________________________*/
int findSizeOfFile(char * fileName, int nGram) {
	ifstream input;
	int gramCount = 0, allGramCount = 0;
	string var;

	input.open(fileName,ifstream::in);

	while(input!=0) {
		input>>var;

		if(input!=0){
			string str = var.substr(nGram);
			gramCount = atoi(str.c_str());
			allGramCount += gramCount;
		}
	}
	input.close();	
	return allGramCount;
}
/*__________________________ END OF HW01_121044014_Suleyman_Balaban.cpp _______________________*/
