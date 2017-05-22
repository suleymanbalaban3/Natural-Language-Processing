/**
 * @author Suleyman Balaban - 121044014
 * @date 19 May 2017
 * @version 2
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

using namespace std;


vector<string> remainFiles;
vector<string> notRemainFiles;

class document {							//for one document's vectors
public:
    document();
    document(const string file);
    string getFile()const {return file;}
    void setFile(const string file){this->file = file;}
 	map<string, pair<int,double> >docs;		//term weighted verctor in it
private:
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

typedef enum{FALSE=0,TRUE=1}BOOL;

void grepFromFile(FILE *inputFile,char *fileName,int classification);
void grepFromDirectory(char *directoryName, int flag,int isRemain,int classification);
int findOccInGrams(int nGram, string target, char * fileName, int & allGramSize);
int findSizeOfFile(char * fileName, int nGram);
int termCountInDocument(const string fileName,const string term,document & currentDocument,int classification);
int maxTermCountInDocument(document currentDocument);
int dfi(const string term,int classification);
void tfIdf(document & tempDocs,int classification);
void calculatePrototypeVectors();
bool isNotInTheDataSet(const string fileName);
bool isInTheRemainDataSet(string & fileName);
document createVectorForInputFile(const string fileName);
void printDocumentVector(document tempDocument);
double calculateCosSim(document prototypeDoc, document inputDoc);
void calculateSimilarityForRocchio();
int findAssignedClass();
void printSimilarityValues();
int nearestNegighbour(const int k, map<int,int> &knnOccurrence);
void sortTheCosSimForKnn();
void printNearest(const int k, map<int, int> knnOccurrence);
BOOL isTxt(char *directoryName);

//Start Of Main
int main(int argc,char *argv[]) {
    int flag=0;
    char sentence[80];
    int nGram = 1;
    char line[25];
    double result;
    int threadCounter = 0;
    int classification = -1;
  	bool t = false;
   	string fileNameForTextCategorization, textCategorizationMode;
   	document forInputFileVectors;
   	string isContinue = "-1";

    if(argc!=2) {//arguman check
        fprintf(stderr,"Usage : %s [directory] \n",argv[0]);
        exit(0);
    }
    cout<<"################################# WELCOME #################################"<<endl;
    cerr<<"Processing files...";
    grepFromDirectory(argv[1],flag,0,classification);
    for (int i = 0; i < 5; ++i)
    	for (int j = 0; j < allDocuments[i].size(); ++j)
    		tfIdf(allDocuments[i][j],i);
    
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
		    cout<<"Enter the method:";
		    cin>>textCategorizationMode;
		    t = isInTheRemainDataSet(fileNameForTextCategorization);
		    if(t == false)
		    	cerr<<"You didn't enter file name from %5(remaining) data set!"<<endl;
		    if(textCategorizationMode != "Rocchio" && textCategorizationMode != "KNN3" && textCategorizationMode != "KNN5")
		    	cerr<<"You didn't enter true text categorization modes!"<<endl;

		}while(t == false || (textCategorizationMode != "Rocchio" && textCategorizationMode != "KNN3" && textCategorizationMode != "KNN5"));
		
		termWectorForInputFile = createVectorForInputFile(fileNameForTextCategorization);
    	tfIdf(termWectorForInputFile,0);

		if(textCategorizationMode == "Rocchio"){						//Rocchio
			calculatePrototypeVectors();
		    calculateSimilarityForRocchio();
		    int similarity = findAssignedClass();
		    cout<<"Assigned class is " <<similarity<<", similarity score is "<<cosSim[similarity-1]<<endl;
		    if(DEBUG){
			    cout<<"-------------------------------------------"<<endl;
			    printSimilarityValues();
			    cout<<"-------------------------------------------"<<endl;
			}
		    
		}else if(textCategorizationMode == "KNN3"){									//KNN3
			map<int,int> knnOccurrence;
			cout<<"Assigned class is " <<nearestNegighbour(3,knnOccurrence)<<endl;
			if(DEBUG){
			    cout<<"-------------------------------------------"<<endl;
			    printNearest(3,knnOccurrence);
			    cout<<"-------------------------------------------"<<endl;
			}
		}else{ 																		//KNN5
			map<int,int> knnOccurrence;
			cout<<"Assigned class is " <<nearestNegighbour(5,knnOccurrence)<<endl;
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
        str = line.substr(0,5);
        if(str.size() == 5){
        	if(tempDocs.docs.count(str)==0){
        		tempDocs.docs[str].first = 1;
        		tempDocs.docs[str].second = 0.0;
        	}else{
        		(tempDocs.docs[str].first)++;
        		tempDocs.docs[str].second = 0.0;
        	}
        }
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
int maxTermCountInDocument(document currentDocument){
	int res = -2;
	document d;
	
	for (map<string,pair<int, double> >::iterator it = currentDocument.docs.begin(); it!= currentDocument.docs.end(); ++it) {
		if((it->second.first) > res)
			res = it->second.first;
    }
    return res;
}
/*____________________________________________________________________________________________*/
void tfIdf(document & tempDocs,int classification){
	int maxF = maxTermCountInDocument(tempDocs);
	//cout<<" document :" <<tempDocs.getFile()<<endl;
	

	for (map<string,pair<int, double> >::iterator it = tempDocs.docs.begin(); it!= tempDocs.docs.end(); ++it) {
		int f = it->second.first;//termCountInDocument(tempDocs.getFile(),it->first,classification);
		double tfValue= (double)f / maxF;
		int dfiValueDown = dfi(it->first,classification);
		double tfIdfValue, dfiValue;

		if(dfiValueDown == 0){
			tfIdfValue = 0;
		}else {
			dfiValue = (double) 1095 / dfiValueDown;
			tfIdfValue = tfValue*log2(dfiValue);
		}
		tempDocs.docs[it->first].second = tfIdfValue;
		it->second.second = tfIdfValue;
    }
	

}
/*____________________________________________________________________________________________*/
int dfi(const string term,int classification){
	int counter = 0;
	for (int j = 0; j < 5; ++j)
		for (int i = 0; i < allDocuments[j].size(); ++i)
			if(allDocuments[j][i].docs.count(term)>0)
				counter++;
	return counter;	

}
/*____________________________________________________________________________________________*/
void calculatePrototypeVectors(){
	for (int i = 0; i < 5; ++i){
		document tempDocs;
		allDocumentsPrototypeVector[i].push_back(tempDocs);
		for (int j = 0; j < allDocuments[i].size(); ++j){			
			for (map<string,pair<int, double> >::iterator it = allDocuments[i][j].docs.begin(); it!= allDocuments[i][j].docs.end(); ++it) {
				if(allDocumentsPrototypeVector[i][0].docs.count(it->first)==0){
					allDocumentsPrototypeVector[i][0].docs[it->first].first = it->second.first;
					allDocumentsPrototypeVector[i][0].docs[it->first].second = it->second.second;
				}else{
					allDocumentsPrototypeVector[i][0].docs[it->first].first += it->second.first;
					allDocumentsPrototypeVector[i][0].docs[it->first].second += it->second.second;
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
	int letter = 0;
    int targetSize=5+1;
    string str;
    ifstream out;
    string line;

    out.open(fileName);
    document tempDocs(fileName);
    
    while(out>>line) {
        str = line.substr(0,5);
        if(str.size() == 5){
        	if(tempDocs.docs.count(str)==0){
        		tempDocs.docs[str].first = 1;
        		tempDocs.docs[str].second = 0.0;
        	}else{
        		(tempDocs.docs[str].first)++;
        		tempDocs.docs[str].second = 0.0;
        	}
        }
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
double calculateCosSim(document prototypeDoc, document inputDoc){
	double up = 0.0, prototypeDocDownAllSquare = 0.0, inputDocDownAllSquare = 0.0;
	for (map<string,pair<int, double> >::iterator it = inputDoc.docs.begin(); it!= inputDoc.docs.end(); ++it) {
		double inputDocWeightTerm = it->second.second;
		double prototypeDocWeightTerm = prototypeDoc.docs.count(it->first);
		up += inputDocWeightTerm * prototypeDocWeightTerm;
		inputDocDownAllSquare += inputDocWeightTerm * inputDocWeightTerm;
		prototypeDocDownAllSquare += prototypeDocWeightTerm * prototypeDocWeightTerm;
    }
    return up / pow((inputDocDownAllSquare * prototypeDocDownAllSquare),0.5);
}
/*____________________________________________________________________________________________*/
void calculateSimilarityForRocchio(){
	for (int i = 0; i < 5; ++i){
		cosSim.push_back(calculateCosSim(allDocumentsPrototypeVector[i][0], termWectorForInputFile));
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
int nearestNegighbour(const int k,map<int,int> &knnOccurrence){
	
	int max = -1, result = -1;

	if(k > 0 && (k%2 == 1)){
		for (int i = 0; i < 5; ++i){
			document tempDocs;
			for (int j = 0; j < allDocuments[i].size(); ++j){
				arg_struct temp;
				temp.arg1 = i;
				temp.arg2 = calculateCosSim(allDocuments[i][j], termWectorForInputFile);
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
			if(max < it->second){
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
			if(forKnn[i].arg2 < forKnn[j].arg2){
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
/*------------------------------------ END OF HW02_121044014_Suleyman_Balaban.cpp -----------------------------------*/