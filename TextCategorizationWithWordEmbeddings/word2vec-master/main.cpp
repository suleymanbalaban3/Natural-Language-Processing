/**
 * @author Suleyman Balaban - 121044014
 * @date 5 June 2017
 * @version 3
 */
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
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
#include <thread>
#include <map>
#include <cmath>
#include <locale.h>

using namespace std;


vector<string> remainFiles;
vector<string> notRemainFiles;
map<string, double[3]>word2Vectors;
class document {							//for one document's vectors
public:
    document();
    document(const string file);
    string getFile()const {return file;}
    void setFile(const string file){this->file = file;}
    map<string, double[3]> map1;
 	map<string, pair<int,double> >docs;		//term weighted verctor in it
	string file;
    
};
struct arg_struct {
    int arg1;
    double arg2;
};

#define SIZE 256 //maksimum dosya ismi boyutu
#define DEBUG TRUE

vector<document>allDocuments[5];					//%95 news data set's term weighted vectors in it
vector<document>allDocumentsPrototypeVector[5];		//for 5 classification prototype vectors in it
document termWectorForInputFile;					//input document's term weighted vectors in it
vector<double>cosSim;								//cosSim(d,pi) results in it
vector<arg_struct> forKnn;							//KNN results for all documents in it 
int trueClass = 0;

typedef enum{FALSE=0,TRUE=1}BOOL;

void grepFromFile(FILE *inputFile,char *fileName,int classification);
void grepFromDirectory(char *directoryName, int flag,int isRemain,int classification);
int findOccInGrams(int nGram, string target, char * fileName, int & allGramSize);
int findSizeOfFile(char * fileName, int nGram);
void calculatePrototypeVectors();
bool isNotInTheDataSet(const string fileName);
bool isInTheRemainDataSet(string & fileName);
document createVectorForInputFile(const string fileName);
void printDocumentVector(document tempDocument);
double calculateCosSim(document prototypeDoc, document inputDoc,const string mode);
void calculateSimilarityForRocchio(const string mode);
int findAssignedClass();
void printSimilarityValues();
int nearestNegighbour(const int k, map<int,int> &knnOccurrence,const string mode);
void sortTheCosSimForKnn();
void printNearest(const int k, map<int, int> knnOccurrence);
BOOL isTxt(char *directoryName);

void readWord2Vec();

//Start Of Main
int main(int argc,char *argv[]) {
	setlocale(LC_ALL, "Turkish");
    int flag=0;
    char sentence[80];
    int nGram = 1;
    char line[25];
    double result;
    int threadCounter = 0;
    int classification = -1;
  	bool t = false;
   	string fileNameForTextCategorization, textCategorizationMode, textCategorizationMethod;
   	document forInputFileVectors;
   	string isContinue = "-1";

    if(argc!=2) {//arguman check
        fprintf(stderr,"Usage : %s [directory] \n",argv[0]);
        exit(0);
    }
    cout<<"################################# WELCOME #################################"<<endl;
    cerr<<"Processing files...";
    readWord2Vec();
    grepFromDirectory(argv[1],flag,0,classification);
    
   	cout<<endl<<endl;
   	do{
	   	do{
	   		string printRemainFiles;
	   		cout<<"If you want, i can show you remain files. For that enter 1 :";
	   		cin>>printRemainFiles;
	   		cout<<"_______________________________________________________"<<endl;
	   		if(printRemainFiles == "1"){
	   			for (int i = 0; i < remainFiles.size(); ++i)   	
	   				cout<<remainFiles[i]<<endl;
	   		}
		   	
		   	cout<<"Enter the file name for categorization:";
		   	cin>>fileNameForTextCategorization;
		    cout<<"Enter the classificaiton method:";
		    cin>>textCategorizationMode;
		    cout<<"Enter Method: ";
		    cin>>textCategorizationMethod;
		    t = isInTheRemainDataSet(fileNameForTextCategorization);
		    if(t == false)
		    	cerr<<"You didn't enter file name from %5(remaining) data set!"<<endl;
		    if(textCategorizationMode != "Rocchio" && textCategorizationMode != "KNN3" && textCategorizationMode != "KNN5")
		    	cerr<<"You didn't enter true text categorization modes!"<<endl;

		}while(t == false || (textCategorizationMode != "Rocchio" && textCategorizationMode 
			!= "KNN3" && textCategorizationMode != "KNN5" && textCategorizationMethod != "MIN" && 
			textCategorizationMethod != "MAX" && textCategorizationMethod != "AVG"));
		
		termWectorForInputFile = createVectorForInputFile(fileNameForTextCategorization);

		if(textCategorizationMode == "Rocchio"){						//Rocchio
			calculatePrototypeVectors();
		    calculateSimilarityForRocchio(textCategorizationMethod);
		    int similarity = findAssignedClass();
		    cout<<"Assigned class is " <<similarity<<", true class is "<<trueClass<<endl;
		    if(DEBUG){
			    cout<<"-------------------------------------------"<<endl;
			    printSimilarityValues();
			    cout<<"-------------------------------------------"<<endl;
			}
		    
		}else if(textCategorizationMode == "KNN3"){									//KNN3
			map<int,int> knnOccurrence;
			cout<<"Assigned class is " <<nearestNegighbour(3,knnOccurrence,textCategorizationMethod);
			cout<<", true class is "<<trueClass<<endl;
			if(DEBUG){
			    cout<<"-------------------------------------------"<<endl;
			    printNearest(3,knnOccurrence);
			    cout<<"-------------------------------------------"<<endl;
			}
		}else{ 																		//KNN5
			map<int,int> knnOccurrence;
			cout<<"Assigned class is " <<nearestNegighbour(5,knnOccurrence,textCategorizationMethod);
			cout<<", true class is "<<trueClass<<endl;
			if(DEBUG){
			    cout<<"-------------------------------------------"<<endl;
			    printNearest(5,knnOccurrence);
			    cout<<"-------------------------------------------"<<endl;
			}
		}
	    cout<<"_______________________________________________________"<<endl;
		cout<<"If you want to exit enter the -1 :";
	    cin>>isContinue;

	    if(isContinue != "-1"){
	    	if(textCategorizationMode == "Rocchio"){
				for (int i = 0; i < 5; ++i)
	    			cosSim.pop_back();
	    		for (int i = 0; i < 5; ++i)
			    	for (int j = 0; j < allDocumentsPrototypeVector[i].size(); ++j)
						allDocumentsPrototypeVector[i].pop_back();
	    	}else{
				for(int i = 0; i < 1095; i++)
					forKnn.pop_back();
			}
	    }
	}while(isContinue != "-1");
	cerr<<"Program closing...";
	sleep(2);
	cout<<endl<<"################################# THE END #################################"<<endl;
    return 0;
}//End Of Main
/*____________________________________________________________________________________________*/
void grepFromDirectory(char *directoryName,int flag,int isReamin,int classification) {  
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
            if((isTxt(fileName)==TRUE)) {//if filename is txt
            	
                
            	if(isReamin == 0 && countTxt < 219){
            		notRemainFiles.push_back(fileName);
            		grepFromFile(inputFile,fileName,classification);
            	}
            	else 
            		remainFiles.push_back(fileName);
                countTxt++;
            }else if(countTxt < 12){ //if directory : call self so,recursive
            	++classification;
                grepFromDirectory(fileName,flag,isReamin,classification);     
            }    
            free(fileName);//free dinamic filenName string
        } 
    }
    closedir(dirPointer);//close directory
}
/*____________________________________________________________________________________________*/
void grepFromFile(FILE *inputFile,char *fileName,int classification) {
    int letter = 0;
    int targetSize=5+1;
    int status;
    string str;
    ifstream out;
    string line;

    out.open(fileName);
    document tempDocs(fileName);
    
    while(out>>line) {
		tempDocs.map1[line][0] = word2Vectors[line][0];
		tempDocs.map1[line][1] = word2Vectors[line][1];
		tempDocs.map1[line][2] = word2Vectors[line][2];
        //}
    }   
    allDocuments[classification].push_back(tempDocs);
	out.close();
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
document::document() {
    
}
/*____________________________________________________________________________________________*/
document::document(const string file) {
	this->file = file;
}
int termCountInDocument(const string fileName,const string term,int classification) {
	for (int i = 0; i < allDocuments[classification].size(); ++i) {
		if(allDocuments[classification][i].getFile() == fileName){
			if(allDocuments[classification][i].docs.count(term)>0)
				return allDocuments[classification][i].docs[term].first;
			else
				return 0;
		}
	}
	return -1;
}
/*____________________________________________________________________________________________*/
void calculatePrototypeVectors(){
	for (int i = 0; i < 5; ++i){
		document tempDocs;
		allDocumentsPrototypeVector[i].push_back(tempDocs);
		for (int j = 0; j < allDocuments[i].size(); ++j){			
			for (map<string,double[3]>::iterator it = allDocuments[i][j].map1.begin(); it!= allDocuments[i][j].map1.end(); ++it) {
				if(allDocumentsPrototypeVector[i][0].map1.count(it->first)==0){
					allDocumentsPrototypeVector[i][0].map1[it->first][0] = it->second[0];
					allDocumentsPrototypeVector[i][0].map1[it->first][1] = it->second[1];
					allDocumentsPrototypeVector[i][0].map1[it->first][2] = it->second[2];
				}else{
					allDocumentsPrototypeVector[i][0].map1[it->first][0] += it->second[0];
					allDocumentsPrototypeVector[i][0].map1[it->first][1] += it->second[1];
					allDocumentsPrototypeVector[i][0].map1[it->first][2] += it->second[2];
				}	
		    }		
		}
	}
}
/*____________________________________________________________________________________________*/
bool isInTheRemainDataSet(string &fileName){
	for (int i = 0; i < remainFiles.size(); ++i) {
		int index = remainFiles[i].rfind("/");
		string str = remainFiles[i].substr(index+1,remainFiles[i].size());
		if(str == fileName){
			fileName = remainFiles[i];
			return true;
		}
	}
	return false;
}
/*____________________________________________________________________________________________*/
document createVectorForInputFile(const string fileName){
    string str;
    ifstream out;
    string line;

    out.open(fileName);
    document tempDocs(fileName);

    if (fileName.rfind("magazin") != -1){
    	trueClass = 1;
    }else if (fileName.rfind("siyasi") != -1){
    	trueClass = 2;
    }else if (fileName.rfind("spor") != -1){
    	trueClass = 3;
    }else if (fileName.rfind("saglik") != -1){
    	trueClass = 4;
    }else {
    	trueClass = 5;
    }
    while(out>>line) {
        tempDocs.map1[line][0] = word2Vectors[line][0];
		tempDocs.map1[line][1] = word2Vectors[line][1];
		tempDocs.map1[line][2] = word2Vectors[line][2];
    }   
	out.close();
	return tempDocs;
}
/*____________________________________________________________________________________________*/
void printDocumentVector(document tempDocument){
	for (map<string,pair<int, double> >::iterator it = tempDocument.docs.begin(); it!= tempDocument.docs.end(); ++it) {
		cout<<it->first<<" / "<<it->second.first<<" / "<<it->second.second<<endl;
    }
}
/*____________________________________________________________________________________________*/
double calculateCosSim(document prototypeDoc, document inputDoc,const string mode){
	double up = 0.0, prototypeDocDownAllSquare = 0.0, inputDocDownAllSquare = 0.0;
	for (map<string,double[3] >::iterator it = inputDoc.map1.begin(); it!= inputDoc.map1.end(); ++it) {
		if(mode =="MIN"){
			double inputDocWeightTerm = it->second[0];
			double prototypeDocWeightTerm = prototypeDoc.map1[it->first][0];
			up += inputDocWeightTerm * prototypeDocWeightTerm;
			inputDocDownAllSquare += inputDocWeightTerm * inputDocWeightTerm;
			prototypeDocDownAllSquare += prototypeDocWeightTerm * prototypeDocWeightTerm;
		}else if(mode == "MAX"){
			double inputDocWeightTerm = it->second[1];
			double prototypeDocWeightTerm = prototypeDoc.map1[it->first][1];
			up += inputDocWeightTerm * prototypeDocWeightTerm;
			inputDocDownAllSquare += inputDocWeightTerm * inputDocWeightTerm;
			prototypeDocDownAllSquare += prototypeDocWeightTerm * prototypeDocWeightTerm;
		}else{
			double inputDocWeightTerm = it->second[2];
			double prototypeDocWeightTerm = prototypeDoc.map1[it->first][2];
			up += inputDocWeightTerm * prototypeDocWeightTerm;
			inputDocDownAllSquare += inputDocWeightTerm * inputDocWeightTerm;
			prototypeDocDownAllSquare += prototypeDocWeightTerm * prototypeDocWeightTerm;
		}
    }
    return up / pow((inputDocDownAllSquare * prototypeDocDownAllSquare),0.5);
}
/*____________________________________________________________________________________________*/
void calculateSimilarityForRocchio(const string mode){
	for (int i = 0; i < 5; ++i){
		cosSim.push_back(calculateCosSim(allDocumentsPrototypeVector[i][0], termWectorForInputFile,mode));
	}
}
/*____________________________________________________________________________________________*/
int findAssignedClass(){
	double similarity = -2.0;
	int result = -1;
	for (int i = 0; i < cosSim.size(); ++i){
		if(cosSim[i] > similarity){
			similarity = cosSim[i];
			result = i+1;
		}
	}
	return result;
}
/*____________________________________________________________________________________________*/
void printSimilarityValues(){
	char classifications[5][10] = {"Magazin", "Siyasi", "Spor", "Sağlık", "Ekonomi"};
	for (int i = 0; i < cosSim.size(); ++i)
		cout<<"Similarity to ("<<classifications[i]<<")class "<<i+1<<" : "<<cosSim[i]<<endl;
}
/*____________________________________________________________________________________________*/
int nearestNegighbour(const int k,map<int,int> &knnOccurrence,const string mode){
	
	int max = -1, result = -1;

	if(k > 0 && (k%2 == 1)){
		for (int i = 0; i < 5; ++i){
			document tempDocs;
			for (int j = 0; j < allDocuments[i].size(); ++j){
				arg_struct temp;
				temp.arg1 = i;
				temp.arg2 = calculateCosSim(allDocuments[i][j], termWectorForInputFile,mode);
				forKnn.push_back(temp);
			}
		}
		sortTheCosSimForKnn();
		for (int i = 0; i < k; ++i){
			if(knnOccurrence.count(forKnn[i].arg1)==0){
        		knnOccurrence[forKnn[i].arg1] = 1;
        	}else{
        		knnOccurrence[forKnn[i].arg1]++;
        	}
		}
		for (map<int, int>::iterator it = knnOccurrence.begin(); it!= knnOccurrence.end(); ++it) {
			if(max <= it->second){
				max = it->second;
				result = it->first + 1;
			}
		}
	}else 
		cerr<<"Can't find KNN-"<<k<<" !"<<endl;
	return result;
}
/*____________________________________________________________________________________________*/
void sortTheCosSimForKnn(){
	for(int i = 0; i < forKnn.size()-1; i++){
		for(int j = i+1; j < forKnn.size()-1; j++){
			if(forKnn[i].arg2 <= forKnn[j].arg2){
				arg_struct tempForSwap; 
				tempForSwap.arg1 = forKnn[i].arg1;
				tempForSwap.arg2 = forKnn[i].arg2;

				forKnn[i].arg1 = forKnn[j].arg1;
				forKnn[i].arg2 = forKnn[j].arg2;

				forKnn[j].arg1 = tempForSwap.arg1;
				forKnn[j].arg2 = tempForSwap.arg2;
			}
		}
	}
}
/*____________________________________________________________________________________________*/
void printNearest(const int k, map<int, int> knnOccurrence){
	char classifications[5][10] = {"Magazin", "Siyasi", "Spor", "Sağlık", "Ekonomi"};
	for (map<int, int>::iterator it = knnOccurrence.begin(); it!= knnOccurrence.end(); ++it) 
		cout<<"Similarity to ("<<classifications[it->first]<<")class "<<it->first+1<<" : "<<it->second<<endl;	
}
void readWord2Vec(){
	string str;
    ifstream input;
    ofstream output;
    string line;
    string word;
    int i = 0, j = 0;
    double min = 2.0, max = -2.0, average = 0.0;
    double numWD = 0.0;
    int vectorSize = 0;
    output.open("allWordNums.txt");
    input.open("trwikivector.txt");
    
    while(input>>line) {
    	if(i == 1){
    		vectorSize = atoi(line.c_str());
    	}
    	else if(atof(line.c_str()) == 0.0 && line != "0"){
    		word = line;
    		while(input>>line && j < vectorSize){
    			numWD = atof(line.c_str());
    			if(numWD > max)
    				max = numWD;
    			if(numWD < min)
    				min = numWD;
    			average+=numWD;
    			j++;
    		}
    		j = 0;
    		word2Vectors[word][0] = min;
    		word2Vectors[word][1] = max;
    		word2Vectors[word][2] = average / vectorSize;
    		min = 2.0;
    		max = -2.0;
    		average = 0.0;
    		
    	}
    	i++;
    }   
    output.close();
	input.close();
}
/*------------------------------------ END OF HW02_121044014_Suleyman_Balaban.cpp -----------------------------------*/