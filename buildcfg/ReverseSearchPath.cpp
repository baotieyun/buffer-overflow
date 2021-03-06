#include "ReverseSearchPath.h"
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
using namespace llvm;


std::vector<ReverseSinglePath*> PathSet;
std::vector<ReverseSinglePath*> AdjustedMainToEntryPathSet;
std::vector<ReverseSinglePath*> AdjustedEntryToExitPathSet;
std::vector<ReverseSinglePath*> CompletePathSet;

bool info_tag=false;
std::vector<ReverseSinglePath*> TemporaryPathSet;

std::map<BasicBlock*, bool> *visited;
std::map<BasicBlock*, bool> *repeatvisit;
std::map<BasicBlock*, bool> *containdes;

extern std::map<pair<CFGNode*,CFGNode*>,std::vector<CFGNode*> >* FortifyPath;
ReverseSinglePath* singlePath;


void ChooseOptimalPath(CFGNode* begin,CFGNode* end,int mode){
	if(mode == 1){
		ReverseSinglePath* rpath=new ReverseSinglePath();
		for(unsigned int i=0;i<PathSet.at(0)->path.size();i++)
			rpath->PathAddNode(PathSet.at(0)->path.at(i));
		AdjustedMainToEntryPathSet.push_back(rpath);
		PathSet.clear();
	}else if(mode == 2){
		
		//do nothing
	}
}

//鏍规嵁鍩烘湰鍧楁壘鍒版帶鍒舵祦鍥剧殑鑺傜偣
CFGNode* ConvertBBToNode(BasicBlock* block){
        return &(*(ProgramCFG::nodes))[block];
}


void DetectRepeatedPath(ReverseSinglePath* rath){
	int index=0;
	for(unsigned int i=0;i<PathSet.size();i++){
		if(rath->path.size()!=PathSet.at(i)->path.size()) break;
		else{
			index=0;
			for(unsigned int j=0;j<PathSet.at(i)->path.size();j++)
				if(rath->path.at(j)==PathSet.at(i)->path.at(j)) index++;
			if(index==PathSet.at(i)->path.size()) errs()<<"repeated path!!!"<<"\n";
		}
	}
	
}

void DetectCircleInOnePath(ReverseSinglePath* rath){
	for(unsigned int i=0;i<rath->path.size();i++)
		for(unsigned int j=0;j<rath->path.size();j++)
			if(i!=j && rath->path.at(i)==rath->path.at(j))
				errs()<<"there exits circle in one path !!!"<<"\n";		
}

int Path_Total_Number=0;


void getReversePathSet(ReverseSinglePath* singlePath){
	Path_Total_Number++;
	unsigned int length=0;
	ReverseSinglePath* rpath=new ReverseSinglePath();
	for(unsigned int i=0;i<singlePath->path.size();i++){
		rpath->PathAddNode(singlePath->path.at(i));
		//errs() <<singlePath->path.at(i)<<"->";
		(*containdes)[singlePath->path.at(i)->bb]=true;
	}
	length=singlePath->path.size();
	singlePath->length=length;
	//errs() <<length<<'\n';
	rpath->length=length;
	PathSet.push_back(rpath);
	//errs()<<Path_Total_Number<<"\n";		
}


//鏍规嵁璺緞鐢熸垚鍒跺淇℃伅
void getGuideInfo(int path_number){
	for(unsigned int i=0;i<CompletePathSet.size();i++){
		for(unsigned int j=0;j<CompletePathSet.at(i)->path.size();j++){
			BasicBlock* bb=CompletePathSet.at(i)->path.at(j)->bb;	
			BasicBlock::iterator i = bb->begin();
			int number=getLineNumber(i);
			//errs() <<getFilename(i)<<"  "<<number<<"->";
		}
		//errs() <<'\n';
	}
	std::stringstream t_sstream;
	if(info_tag==false){
		t_sstream<<"Info:"<<std::endl;
		info_tag=true;
	}
	for(unsigned int j=0;j<CompletePathSet.at(0)->path.size();j++){
			BasicBlock* bb=CompletePathSet.at(0)->path.at(j)->bb;	
			BasicBlock::iterator it = bb->begin();
			int number=getLineNumber(it);
			if(number!=0){
				t_sstream<<getFilename(it)<<'\t'<<number;
				t_sstream<<std::endl;
			}
	}
	char filename[256]="GuideSrc";
	char tmp[16];
	//sprintf(tmp,"%d",path_number);
	//strcat(filename,tmp);
	strcat(filename,".txt");
	std::ofstream fileout(filename,ios::app);
	fileout<<t_sstream.str()<<std::endl;				
}

//鏍规嵁index瀵绘壘瀵瑰簲鐨刢fg鑺傜偣
CFGNode* index_Prev(int index,CFGNode* node){
	int number=0;
	if(index==0)	return node->firstPrev->v;
	else{
		Prev* p=node->firstPrev;
		while(p!=NULL){
			if(number==index) return p->v;
			p=p->nextPrev;
			number++;			
		}
	}
	return NULL;
}

//CFG涓墠椹辫妭鐐圭殑鏁扮洰
int PrevNumber(CFGNode* node){
	int number=0;
	if(node->firstPrev==NULL) return number;
	else{
		Prev* p=node->firstPrev;
		while(p!=NULL){
			number++;
			p=p->nextPrev;
			
		}
		return number;
	}
}



void DetectSameNode(CFGNode* node){	
	std::vector<CFGNode*>* CFGVec=new std::vector<CFGNode*>();
	if(node->firstPrev!=NULL){
		Prev* q=NULL;
		Prev* p=node->firstPrev;
		CFGVec->push_back(p->v);
			while(p!=NULL){
				q=p;
				p=p->nextPrev;	
				for(int i=0;i<CFGVec->size();i++){
					if(p!=NULL){
						if( p->v==CFGVec->at(i)){
							//errs() <<"Error!!!!"<<'\n';
							q->nextPrev=p->nextPrev;
							p->nextPrev=NULL;
							p=q;
						}
					}
				}
				if(p!=NULL)CFGVec->push_back(p->v);		
			}
	}
}

//------------------------------------------------
//鏍规嵁index瀵绘壘瀵瑰簲鐨刢fg鑺傜偣
CFGNode* index_Succ(int index,CFGNode* node){
	int number=0;
	if(index==0)	return node->firstSucc->v;
	else{
		Succ* p=node->firstSucc;
		while(p!=NULL){
			if(number==index) return p->v;
			p=p->nextSucc;
			number++;
			
		}
	}
	return NULL;
}

//CFG涓悗缁ц妭鐐圭殑鏁扮洰
int SuccNumber(CFGNode* node){
	int number=0;
	if(node->firstSucc==NULL) return number;
	else{
		Succ* p=node->firstSucc;
		while(p!=NULL){
			number++;
			p=p->nextSucc;
			
		}
		return number;
	}
}

//CFG涓煇涓や釜鑺傜偣鏄惁鐩存帴杩為€氬彲杈?
bool IsConnective(CFGNode* sNode,CFGNode* dNode){ 
	if(sNode->firstSucc==NULL) {  return false;}
	else if(dNode == sNode->firstSucc->v) {  return true;}
	else{ 
		Succ* cNode=sNode->firstSucc;
		while(cNode!=NULL){
			
			if(cNode->v==dNode) { return true;}
			cNode=cNode->nextSucc;
		}
		return false;
	}
}

Succ* FindFirstUnvisited(Succ* v_succ){
	if(v_succ->v->firstSucc==NULL)
		return NULL;
	else if(!(*visited)[v_succ->v->firstSucc->v->bb])
		return v_succ->v->firstSucc;
	else{
		
		Succ* temp=v_succ->v->firstSucc;
		while(temp!=NULL){
			(*visited)[temp->v->bb]= true;
			if(temp->nextSucc!=NULL){
				if(!(*visited)[temp->nextSucc->v->bb])
					return temp->nextSucc;
			}
			else temp=temp->nextSucc;
		}
		return NULL;		
	}
}

void SearchPathToExit(CFGNode* v_node){
	(*visited)[v_node->bb]= true;
	singlePath->PathAddNode(v_node);
	/*errs()<<"6*"<<"\n";Succ* v_succ=v_node->firstSucc;errs()<<"4*"<<"\n";
	if(v_succ!=NULL){
		(*visited)[v_succ->v->bb]= true;
		while(v_succ!=NULL){errs()<<"2*"<<"\n";
			singlePath->PathAddNode(v_succ->v);
			v_succ=FindFirstUnvisited(v_succ);	
			if(v_succ!=NULL) (*visited)[v_succ->v->bb]= true;	
		}errs()<<"4*"<<"\n";
	}*/		
	getReversePathSet(singlePath);
}

// 娣卞害浼樺厛鎼滅储
// path鐢ㄦ潵璁板綍璺緞
// visited 鐢ㄦ潵鏍囪鎼滅储杩囩殑鑺傜偣锛屽垵濮嬪寲鍏ㄩ儴涓篺alse
// v 褰撳墠鐨勮妭鐐?
// des 鐩殑鑺傜偣
// length 鐩墠宸茬粡寰楀埌鐨勮矾寰勭殑闀垮害

/*void SearchPaths(CFGNode* v, CFGNode* des, int length) { 
    if ((*visited)[v->bb]) return;
    singlePath->PathAddNode(v);
    if (v == des) {
	getReversePathSet(singlePath);
	return;
    } else {
        (*visited)[v->bb]= true;
	for(int i=0;i<PrevNumber(v);i++){			
		if(!(*visited)[index_Prev(i,v)->bb]){
			SearchPaths(index_Prev(i,v), des, length+1);
			singlePath->path.pop_back();
		}else if((*repeatvisit)[index_Prev(i,v)->bb]){
			if((*containdes)[index_Prev(i,v)->bb]==true)
				getReversePathSet(singlePath);
		}		
	}
	//(*visited)[v->bb]= false;
	(*repeatvisit)[v->bb]=true;
	
    }
}*/

void SearchPaths(CFGNode* v, CFGNode* des, int length) { 
    if ((*visited)[v->bb]) return;
    singlePath->PathAddNode(v);
    if (v == des) {
	getReversePathSet(singlePath);
	return;
    } else {
        (*visited)[v->bb]= true;
	for(int i=0;i<PrevNumber(v);i++){			
		if(!(*visited)[index_Prev(i,v)->bb]){
			SearchPaths(index_Prev(i,v), des, length+1);
			singlePath->path.pop_back();
		}		
	}
	
    }
}


void SearchPath(CFGNode* start, CFGNode* des,ReverseSinglePath* singlePath,std::map<BasicBlock*, bool> *visit){
	unsigned int top=1; unsigned int head=1;
	CFGNode* v=start;
	Prev* p=NULL;
	(*visit)[start->bb]=true;
	singlePath->path.push_back(start);
	singlePath->path.push_back(start);
	do{
		if(head==1){
			p=v->firstPrev;
			head=0;	
		}else
			p=p->nextPrev;
		if(p){
			if(!(*visit)[p->v->bb]){
				(*visit)[p->v->bb]= true;
				top++;
				singlePath->path.push_back(p->v);
				if(p->v==des){
					getReversePathSet(singlePath);
					(*visit)[des->bb]= false;
					top--;
					singlePath->path.pop_back();
					v=singlePath->path.back();
					head=0;
				}else{
					v=singlePath->path.back();
					head=1;
				}
			}		
		}else{
			top--;
			
			(*visit)[singlePath->path.back()->bb]= false;
			singlePath->path.pop_back();
			if(top){
				p=singlePath->path.back()->firstPrev;
				while(p->v!=v)
					p=p->nextPrev;
				v=singlePath->path.back();
				head=0;
			}
		}
	}while(top);
}

bool ContainNodeList(ReverseSinglePath* rpath,std::vector<CFGNode*> nodelist){
	int ContainTag=0;
	for(unsigned int i=0;i<nodelist.size();i++){
		for(unsigned int j=0;j<rpath->path.size();j++){
			if(nodelist.at(i)==rpath->path.at(j))
				{ContainTag++; break;}
		}
	}
	if(ContainTag==nodelist.size()) return true;
	else return false;
		
}



void PathSplice(){
	for(unsigned int i=0;i<AdjustedEntryToExitPathSet.size();i++){
		for(unsigned int j=0;j<AdjustedMainToEntryPathSet.size();j++){
			ReverseSinglePath* rpath=AdjustedEntryToExitPathSet.at(i);
			for(unsigned int k=1;k<AdjustedMainToEntryPathSet.at(j)->path.size();k++)
				rpath->PathAddNode(AdjustedMainToEntryPathSet.at(j)->path.at(k));
			CompletePathSet.push_back(rpath);
			
		}
	}
	
}

void FreeMemory(){
	for(unsigned int i=0;i<PathSet.size();i++)
		delete PathSet.at(i);
	PathSet.clear();
	AdjustedEntryToExitPathSet.clear();
	AdjustedMainToEntryPathSet.clear();
	CompletePathSet.clear();
}


bool TagClear(int mode){
	Path_Total_Number=0;
	std::map<BasicBlock*,bool>::iterator Iter;
	for (Iter=visited->begin( );Iter!=visited->end( );Iter++){
		if(Iter->second==true)
			Iter->second=false;
	}
	for (Iter=repeatvisit->begin( );Iter!=repeatvisit->end( );Iter++){
		if(Iter->second==true)
			Iter->second=false;
	}	
	for (Iter=containdes->begin( );Iter!=containdes->end( );Iter++){
		if(Iter->second==true)
			Iter->second=false;
	}
	delete singlePath;
	if(mode == 1){
		if(PathSet.size()==0){
				//errs()<<"Error!The target point is not reachable from the begin node 1"<<"\n";
				return false;
		}
	}
	singlePath=new ReverseSinglePath();	
	if(mode == 2){
			if(PathSet.size()==0){
				//errs()<<"Error!The target point is not reachable from the begin node 2"<<"\n";
			}
			else{
				if(AdjustedEntryToExitPathSet.size()==0){
					ReverseSinglePath* rpath=new ReverseSinglePath();
					for(unsigned int i=0;i<PathSet.at(0)->path.size();i++)
						rpath->PathAddNode(PathSet.at(0)->path.at(i));
					AdjustedEntryToExitPathSet.push_back(rpath);
				}
				//AdjustedEntryToExitPathSet.push_back(PathSet.at(0));
				else
					for(unsigned int i=0;i<PathSet.at(0)->path.size();i++)
						AdjustedEntryToExitPathSet.at(0)->PathAddNode(PathSet.at(0)->path.at(i));;
			}
			PathSet.clear();
		}
	if(mode == 3 ){
			if(PathSet.size()==0){
				//errs()<<"Error!The target point is not reachable from the begin node 3"<<"\n";
			}
			else{
				if(AdjustedEntryToExitPathSet.size()==0){
					ReverseSinglePath* rpath=new ReverseSinglePath();
					for(unsigned int i=PathSet.at(0)->path.size()-1;i>0;i--)
						rpath->PathAddNode(PathSet.at(0)->path.at(i));
					AdjustedEntryToExitPathSet.push_back(rpath);
				}
				else
					for(unsigned int i=PathSet.at(0)->path.size()-1;i>0;i--)
						AdjustedEntryToExitPathSet.at(0)->PathAddNode(PathSet.at(0)->path.at(i));
			}
			PathSet.clear();
	}
	return true;
			
}

//涓昏鐨勬墽琛屽嚱鏁?

void SearchReversePaths(BasicBlock* pathentry,BasicBlock* pathexit,BasicBlock* 	mainEntry,Module &m,int path_number){
	singlePath=new ReverseSinglePath();
	visited=new std::map<BasicBlock*,bool> ;
	repeatvisit=new std::map<BasicBlock*,bool> ;
	containdes=new std::map<BasicBlock*,bool> ;
	CFGNode* selectedbeginNode=NULL;
	CFGNode* selectedendNode=NULL;
	for(Module::iterator f = m.begin(); f != m.end(); f++)
		for(Function::iterator bb = f->begin(); bb != f->end(); bb++){
			visited->insert(make_pair(bb,false));
			repeatvisit->insert(make_pair(bb,false));
			containdes->insert(make_pair(bb,false));
			DetectSameNode(ConvertBBToNode(bb));
		}		

	for(Module::iterator f = m.begin(); f != m.end(); f++)
		for(Function::iterator bb = f->begin(); bb != f->end(); bb++){
			if(bb->getName().compare("return")==0 && f->getName().compare("main")==0)
			selectedbeginNode=ConvertBBToNode(bb);
			if(bb->getName().compare("entry")==0 && f->getName().compare("readtoken")==0)
			selectedendNode=ConvertBBToNode(bb);
		}

	CFGNode* pathEntry=ConvertBBToNode(pathentry);
	CFGNode* pathExit=ConvertBBToNode(pathexit);
    	CFGNode* mainentry=ConvertBBToNode(mainEntry);

	if(pathEntry==NULL || pathExit==NULL)
		return;
	else{
		SearchPaths(pathEntry,mainentry,0);
		if(TagClear(1)==false)
			return ;
		ChooseOptimalPath(pathEntry,mainentry,1);

		SearchPathToExit(pathExit);	
		TagClear(3);
		std::map<pair<CFGNode*,CFGNode*>,std::vector<CFGNode*> >::iterator it;
		it=FortifyPath->find(make_pair(pathEntry,pathExit));
		if((it->second).size()!=0){
			SearchPaths(pathExit,(it->second).back(),0);
			TagClear(2);
			for(unsigned int i=(it->second).size()-1;i>0;i--){
				SearchPaths((it->second).at(i),(it->second).at(i-1),0);
				TagClear(2);
			}
			SearchPaths((it->second).front(),pathEntry,0);
			TagClear(2);
		}
		else {
			SearchPaths(pathExit,pathEntry,0);
			TagClear(2);

		}
		PathSplice();//Splice path
		getGuideInfo(path_number);// get gudied information*/
	}
	FreeMemory();
	delete visited;	
}

