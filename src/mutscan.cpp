#include "mutscan.h"
#include "fastqreader.h"
#include <iostream>
#include "htmlreporter.h"
#include "sescanner.h"
#include "pescanner.h"
#include "util.h"

MutScan::MutScan(string mutationFile, string refFile, string read1File, string read2File, string html, int threadNum){
    mRead1File = read1File;
    mRead2File = read2File;
    mMutationFile = mutationFile;
    mRefFile = refFile;
    mHtmlFile = html;
    mThreadNum = threadNum;
}

bool MutScan::scan(){
    if(mRead2File != ""){
        //return scanPairEnd();
        PairEndScanner pescanner( mMutationFile, mRefFile, mRead1File, mRead2File, mHtmlFile, mThreadNum);
        return pescanner.scan();
    }
    else{
        //return scanSingleEnd();
        SingleEndScanner sescanner( mMutationFile, mRefFile, mRead1File, mHtmlFile, mThreadNum);
        return sescanner.scan();
    }
}

bool MutScan::scanPairEnd(){
    FastqReader reader1(mRead1File);
    FastqReader reader2(mRead2File);
    if(mMutationFile!=""){
        if(ends_with(mMutationFile, ".vcf") || ends_with(mMutationFile, ".VCF") || ends_with(mMutationFile, ".Vcf"))
            mutationList = Mutation::parseVcf(mMutationFile, mRefFile);
        else
            mutationList = Mutation::parseCsv(mMutationFile);
    }
    else
        mutationList = Mutation::parseBuiltIn();
    vector<Match*> *mutationMatches = new vector<Match*>[mutationList.size()];
    for(int i=0;i<mutationList.size();i++){
        mutationMatches[i] = vector<Match*>();
    }
    int processed = 0;
	while(true){
		Read* r1 = reader1.read();
		Read* r2 = reader2.read();
        if(!r1 || !r2)
            break;
        Read* rcr1 = r1->reverseComplement();
        Read* rcr2 = r2->reverseComplement();
        for(int i=0;i<mutationList.size();i++){
	        Match* matchR1 = mutationList[i].searchInRead(r1);
            if(matchR1)
                mutationMatches[i].push_back(matchR1);
	        Match* matchR2 = mutationList[i].searchInRead(r2);
            if(matchR2)
                mutationMatches[i].push_back(matchR2);
            Match* matchRcr1 = mutationList[i].searchInRead(rcr1);
            if(matchRcr1){
                matchRcr1->setReversed(true);
                mutationMatches[i].push_back(matchRcr1);
            }
            Match* matchRcr2 = mutationList[i].searchInRead(rcr2);
            if(matchRcr2){
                matchRcr2->setReversed(true);
                mutationMatches[i].push_back(matchRcr2);
            }
	    }
        delete r1;
        delete r2;
        delete rcr1;
        delete rcr2;

        processed += 1;
        if(processed % 1000000 == 0) {
            //cout<<"processed "<<processed<<" reads"<<endl;
        }
	}

    // sort the matches to make the pileup more clear
    for(int i=0;i<mutationList.size();i++){
        sort(mutationMatches[i].begin(), mutationMatches[i].end(), Match::greater); 
    }

    textReport(mutationList, mutationMatches);
    htmlReport(mutationList, mutationMatches);

    // free memory
    for(int i=0;i<mutationList.size();i++){
        mutationMatches[i].clear();
    }

	return true;
}


bool MutScan::scanSingleEnd(){
    FastqReader reader1(mRead1File);
    if(mMutationFile!=""){
        if(ends_with(mMutationFile, ".vcf") || ends_with(mMutationFile, ".VCF") || ends_with(mMutationFile, ".Vcf"))
            mutationList = Mutation::parseVcf(mMutationFile, mRefFile);
        else
            mutationList = Mutation::parseCsv(mMutationFile);
    }
    else
        mutationList = Mutation::parseBuiltIn();
    vector<Match*> *mutationMatches = new vector<Match*>[mutationList.size()];
    for(int i=0;i<mutationList.size();i++){
        mutationMatches[i] = vector<Match*>();
    }
    int processed = 0;
    while(true){
        Read* r1 = reader1.read();
        if(!r1)
            break;
        Read* rcr1 = r1->reverseComplement();
        for(int i=0;i<mutationList.size();i++){
            Match* matchR1 = mutationList[i].searchInRead(r1);
            if(matchR1)
                mutationMatches[i].push_back(matchR1);
            Match* matchRcr1 = mutationList[i].searchInRead(rcr1);
            if(matchRcr1){
                matchRcr1->setReversed(true);
                mutationMatches[i].push_back(matchRcr1);
            }
        }
        delete r1;
        delete rcr1;

        processed += 1;
        if(processed % 1000000 == 0) {
            //cout<<"processed "<<processed<<" reads"<<endl;
        }
    }

    // sort the matches to make the pileup more clear
    for(int i=0;i<mutationList.size();i++){
        sort(mutationMatches[i].begin(), mutationMatches[i].end(), Match::greater); 
    }

    textReport(mutationList, mutationMatches);
    htmlReport(mutationList, mutationMatches);

    // free memory
    for(int i=0;i<mutationList.size();i++){
        mutationMatches[i].clear();
    }

    return true;
}

void MutScan::textReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches) {
    //output result
    for(int i=0;i<mutationList.size();i++){
        vector<Match*> matches = mutationMatches[i];
        if(matches.size()>0){
            cout<<endl<<"---------------"<<endl;
            mutationList[i].print();
            for(int m=0; m<matches.size(); m++){
                cout<<m+1<<", ";
                matches[m]->print(mutationList[i].mLeft.length(), mutationList[i].mCenter.length(), mutationList[i].mRight.length());
            }
        }
    }
}

void MutScan::htmlReport(vector<Mutation>& mutationList, vector<Match*> *mutationMatches) {
    if(mHtmlFile == "")
        return;

    HtmlReporter reporter(mHtmlFile, mutationList, mutationMatches);
    reporter.run();
}
