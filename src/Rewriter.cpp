#include <cstdio>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/Utils.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
//#include "clang/Rewrite/Rewriter.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;



class ASTVistorGenFuncPara : public RecursiveASTVisitor<ASTVistorGenFuncPara> {
public:
	ASTVistorGenFuncPara(SourceManager &S, SourceLocation s, SourceLocation e) : IsValid(true), TheSourceMgr(S), lStart(s), lEnd(e) {
		ParameterList.clear();
	}

	bool IsValid;

	bool VisitStmt(Stmt *s) {
		//printf("???");

		if (isa<DeclRefExpr>(s))
    	{
        	DeclRefExpr * dre = cast<DeclRefExpr>(s);
			ValueDecl * valueDecl = dre->getDecl();
			//if(isa<VarDecl>(valueDecl) || isa<FunctionDecl>(valueDecl) )
			if(isa<VarDecl>(valueDecl) )
			{
				//VarDecl * varDecl = cast<VarDecl>(valueDecl);
				QualType t = valueDecl->getType();		
	
				SourceLocation declLoc = valueDecl->getLocStart();

				BeforeThanCompare<SourceLocation> isBefore(TheSourceMgr);
			
				bool s_before_d = isBefore(lStart,declLoc);
				bool d_before_e = isBefore(declLoc,lEnd);
				bool isLocDecl = s_before_d && d_before_e;
				
				if(!isLocDecl)
				{
					//ParameterList.push_back(varDecl->getLocStart().printToString(TheSourceMgr));
					std::string mType = t.getAsString();
					std::string mName = dre->getNameInfo().getAsString();
					bool existed = false;
					for(int i = 1; i < ParameterList.size();i+=2)
					{
						if(ParameterList.at(i) == mName) 
						{
							existed = true;
							break;
						}
					}

					if(existed == false)
					{
						ParameterList.push_back(t.getAsString());
						ParameterList.push_back(dre->getNameInfo().getAsString());
					}
				}
			}						
			// printf("???\n");
		}

		if (isa<MemberExpr>(s))
		{
			MemberExpr * me = cast<MemberExpr>(s);	
			//ParameterList.push_back(me->getMemberNameInfo().getAsString());

		}

		if (isa<ReturnStmt>(s))
		{
			IsValid = false;
		}

		if (isa<CXXThisExpr>(s))
		{
			IsValid = false;
		}




		return true;
	}

	void cleanUp()
	{
		ParameterList.clear();
		IsValid = true;
	}

	void setLocationRange(SourceLocation s, SourceLocation e)
	{
		lStart = s;
		lEnd = e;
	}



	std::vector<std::string> ParameterList;
	SourceManager &TheSourceMgr;
	SourceLocation lStart;
	SourceLocation lEnd;


};



class ASTVistorModulizer : public RecursiveASTVisitor<ASTVistorModulizer> {
public:
	ASTVistorModulizer(SourceManager &S, Rewriter &R, SourceLocation loc) : IsValid(true), TheSourceMgr(S), TheRewriter(R), locDump(loc) {
		ParameterList.clear();
	}

	bool IsValid;

	bool VisitStmt(Stmt *s) {
		//printf("???");
		if (isa<CompoundStmt>(s))
		{
			CompoundStmt *cs = cast<CompoundStmt>(s);

			Stmt** ModuleStart = NULL;
			Stmt** ModuleEnd = NULL;

			
			ASTVistorGenFuncPara mChecker(TheSourceMgr,s->getLocStart(), s->getLocEnd());
	        
			//mChecker.TraverseStmt(FuncBody);


            for(Stmt** locals = cs->body_begin(); locals < cs->body_end(); locals++)
            {
                //printf("Locals %p \n",locals);
                if(locals!=NULL && (*locals) != NULL)
                {
					bool isValid = true;

					if(isa<DeclStmt>(*locals))
					{
						isValid = false;
					}
					else
					{
						mChecker.setLocationRange((*locals)->getLocStart(),(*locals)->getLocEnd());
						mChecker.TraverseStmt((*locals));

						if(mChecker.IsValid == true)
						{
							isValid = true;
							if(ModuleStart == NULL)
							{
								ModuleStart = locals;
							}	

							ModuleEnd = locals;

						}
						else
						{
							isValid = false;
							// Reclusive 
							ASTVistorModulizer mModulizer(TheSourceMgr, TheRewriter, locDump);
							mModulizer.TraverseStmt((*locals));			
		
						}

					}
	

					if(isValid == true && locals == (cs->body_end() -1))
					{
						isValid = false;
					}

					if(isValid == false)
					{
						if(ModuleStart == NULL) // Range Empty
						{
							// do nothing

						}
						else
						{				
							//TODO  Generate Module
							// Parameter List from mChecker
							// Code dumped from moduleStart to moduleEnd

							std::stringstream str;

							str << " /* ";
							str << "Module " << (*ModuleStart)->getLocStart().printToString(TheSourceMgr);
							str << " to " << (*ModuleEnd)->getLocEnd().printToString(TheSourceMgr);
							str << " */ \n" ;

							
							TheRewriter.InsertText(locDump, str.str(),true,true);


						}		
						ModuleStart = NULL;
						ModuleEnd = NULL;
						mChecker.cleanUp();

					}				
   				
                }
            }
			return false;	// Depth 1 only
		}



		return true;   
	}


	std::vector<std::string> ParameterList;
	SourceManager &TheSourceMgr;
	Rewriter &TheRewriter;
	SourceLocation locDump;

};





// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
  MyASTVisitor(Rewriter &R, SourceManager &S, ASTContext &C) : TheRewriter(R), TheSourceMgr(S), TheASTContext(C)  {}

	void InsertBraceAndString(Stmt *s, char* S)
	{
		return;
		Rewriter::RewriteOptions rangeOpts;
        rangeOpts.IncludeInsertsAtBeginOfRange = false; 
        unsigned int offset = TheRewriter.getRangeSize(SourceRange(s->getLocEnd(), s->getLocEnd()), rangeOpts);
        char str[1024];
		char str2[1024];
		SourceLocation sloc = s->getLocEnd().getLocWithOffset(offset);
		char t = *(TheRewriter.getSourceMgr().getCharacterData(sloc));
        int nextSemicolon = 0;


		if(offset == 0xffffffff) return;// This is a bad stmt, due to incomplete preprocessor... 

        while(t == ' ' || t == '\n' || t == '\t')
        {   
            nextSemicolon ++;
            t = *(TheRewriter.getSourceMgr().getCharacterData(sloc.getLocWithOffset(nextSemicolon)));
        }

        if(t == ';')
        {
            offset+=nextSemicolon;
        }

		//sprintf(str2,"}/* %d %c */ ",nextSemicolon, t);
		sprintf(str2,"}");

		TheRewriter.InsertText(s->getLocEnd().getLocWithOffset(offset+1),str2,true,true);

		//sprintf(str,"{/* %d %x %c */ %s",nextSemicolon,offset,t,S);
		sprintf(str,"{ %s",S);

		TheRewriter.InsertText(s->getLocStart(), str,true,true);
	}


  bool VisitStmt(Stmt *s) {
    // Only care about If statements.
	if(s->getLocEnd().isValid())
	{
//		TheRewriter.InsertText(s->getLocEnd()," /* ^^ Stmt End ^^ */ ",true,true);    
	}


	if (isa<DeclRefExpr>(s))
	{
		DeclRefExpr * dre = cast<DeclRefExpr>(s);
		std::string name = dre->getNameInfo().getAsString();

		if(dre->getLocStart().isValid())
		{
			char buf[1024];
			sprintf(buf," /* %s */ ",name.c_str());
			TheRewriter.InsertText(dre->getLocStart(),buf,true,true);

		}		

	}



	if (isa<IfStmt>(s)) {
      IfStmt *IfStatement = cast<IfStmt>(s);
      Stmt *Then = IfStatement->getThen();

	
		InsertBraceAndString(Then,"ALOGE(\"THEN\");");

      Stmt *Else = IfStatement->getElse();
      if (Else)
		{
			InsertBraceAndString(Else,"ALOGE(\"ELSE\");");
		}
    }
	
	if (isa<ForStmt>(s)){

		ForStmt * ForStatement = cast<ForStmt>(s);
		Stmt *Body = ForStatement->getBody();

		InsertBraceAndString(Body,"ALOGE(\"For\");");


	}



    return true;
  }


  bool VisitFunctionDecl(FunctionDecl *f) {
    // Only function definitions (with bodies), not declarations.
    if (f->hasBody()) {
      Stmt *FuncBody = f->getBody();
  		
		if(isa<CompoundStmt>(FuncBody))
		{
			//Start the modulizer !!! //TODO

			ASTVistorModulizer mModulizer(TheSourceMgr, TheRewriter, f->getSourceRange().getBegin());
			mModulizer.TraverseStmt(FuncBody); 




			
	
		}




		int counter = 0;
		if(isa<CompoundStmt>(FuncBody))
		{
			//printf("It is a compoundstmt\n");
			CompoundStmt *cs = cast<CompoundStmt>(FuncBody);

			for(Stmt** locals = cs->body_begin(); locals < cs->body_end(); locals++)
			{
				//printf("Locals %p \n",locals);
				if(locals!=NULL && (*locals) != NULL)
				{
				//printf("*Locals %p \n",*locals);
					if((*locals)->getLocStart().isValid())
					{
						char buf[1024];
						sprintf(buf," /* Stmt %d */ ",++counter);
						TheRewriter.InsertText((*locals)->getLocStart(),buf,true,true);
					}			
				}
			}
		}


		// Generate Parameter List
		
		bool isCXXFunc = false;

		if(isa<CXXMethodDecl>(f))
		{
			// We need replace 'this' 
			//isCXXFunc = true;			
		}



		if(FuncBody->getLocStart().isValid() && FuncBody->getLocEnd().isValid())
		{

		ASTVistorGenFuncPara mGenFuncPara(TheSourceMgr,FuncBody->getLocStart(), FuncBody->getLocEnd());
		mGenFuncPara.TraverseStmt(FuncBody);
		
		char buf[1024*64];
		char * lp = buf;

		lp = lp + sprintf(lp, " /* (");	
		
		if(isCXXFunc == true)
		{
			QualType t = (cast<CXXMethodDecl>(f))->getThisType(TheASTContext);  // This Function is dangerours for static function
			if(mGenFuncPara.ParameterList.size() == 0)
			{
				lp = lp + sprintf(lp," %s myTHIS ",t.getAsString());			
			}
			else
			{
				lp = lp + sprintf(lp," %s myTHIS , ",t.getAsString());			
			}
		}

			
		for(int i = 0;i<mGenFuncPara.ParameterList.size(); i+=2)
		{
			lp = lp + sprintf(lp, " %s ",mGenFuncPara.ParameterList.at(i).c_str());		
			if(i != mGenFuncPara.ParameterList.size() -2)		
				lp = lp + sprintf(lp, " %s, ",mGenFuncPara.ParameterList.at(i+1).c_str());
			else 				
				lp = lp + sprintf(lp, " %s ",mGenFuncPara.ParameterList.at(i+1).c_str());
		}
		lp = lp + sprintf(lp, " )*/ ");				
		
		if(FuncBody->getLocStart().isValid())
		{
			TheRewriter.InsertText(FuncBody->getLocStart(),buf,true,true);
		}

		}




    
      // Type name as string
      QualType QT = f->getReturnType();
      std::string TypeStr = QT.getAsString();
      
      // Function name
      DeclarationName DeclName = f->getNameInfo().getName();
      std::string FuncName = DeclName.getAsString();
     
		if(strcmp("createDisplay",FuncName.c_str()) == 0)
		{
			f->dumpColor();	
		}


 
      // Add comment before
      std::stringstream SSBefore;
      SSBefore << "// Begin function " << FuncName << " returning " << TypeStr
               << "\n";
      SourceLocation ST = f->getSourceRange().getBegin();
      TheRewriter.InsertText(ST, SSBefore.str(), true, true);
      
      // And after
      std::stringstream SSAfter;
      SSAfter << "\n// End function " << FuncName;
      ST = FuncBody->getLocEnd().getLocWithOffset(1);
//      TheRewriter.InsertText(ST, SSAfter.str(), true, true);


//		 InsertBraceAndString(FuncBody,"ALOGE(\"FUNCTION\");");



    } 
    
    return true;
  } 
  
private:
	Rewriter &TheRewriter;
	SourceManager &TheSourceMgr;
	ASTContext &TheASTContext;

};

class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R,SourceManager &S, ASTContext &C) : Visitor(R,S,C) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
	  //(*b)->dumpColor();
    return true;
  }

private:
  MyASTVisitor Visitor;
};







extern "C"
{
int Rewrite(char* src, char* dest, char** includeDirStrList,
            int * includeDirType, int includeDirCount,
            char ** defineStrList, int defineCount)
{
	CompilerInstance TheCompInst;
	TheCompInst.createDiagnostics();

	LangOptions &lo = TheCompInst.getLangOpts();
	lo.CPlusPlus11 = 1;
	lo.CPlusPlus = 1;
	lo.Bool = 1;
	lo.C11 =1;
	lo.LineComment = 1;
	lo.Half = 1;
	lo.WChar = 1;
	lo.ImplicitInt = 0;

  // Initialize target info with the default triple for our platform.
	auto TO = std::make_shared<TargetOptions>();
	//TO->Triple = llvm::sys::getDefaultTargetTriple();
		
//	printf("Triple %s \n", llvm::sys::getDefaultTargetTriple().c_str());
//	printf("Triple %s \n", TO->CPU.c_str());
//	printf("Triple %s \n", TO->FPMath.c_str());
//	printf("Triple %s \n", TO->ABI.c_str());
//	printf("Triple %s \n", TO->EABIVersion.c_str());
	

	TO->Triple = std::string("armv7-unknow-linux-androideabi");
//	TO->CPU = std::string("armv7-a");
		

	TargetInfo *TI =
      TargetInfo::CreateTargetInfo(TheCompInst.getDiagnostics(), TO);
	TheCompInst.setTarget(TI);


	TheCompInst.getDiagnostics().setIgnoreAllWarnings(true);
	//TheCompInst.getDiagnostics().setSuppressAllDiagnostics(true);



	TheCompInst.createFileManager();
	FileManager &FileMgr = TheCompInst.getFileManager();
	TheCompInst.createSourceManager(FileMgr);
	SourceManager &SourceMgr = TheCompInst.getSourceManager();
	TheCompInst.createPreprocessor(TU_Module);
	TheCompInst.createASTContext();


	
TheCompInst.getPreprocessor().getBuiltinInfo().initializeBuiltins(TheCompInst.getPreprocessor().getIdentifierTable(),
						   TheCompInst.getLangOpts());




	//fprintf(stderr,"%d\n",includeDirCount);	
	DirectoryEntry *ddir = NULL;

	//for(int i = 0; i < includeDirCount; i++)
	for(int i = includeDirCount-1; i >= 0; i--)
	{
		//fprintf(stderr,"%s\n",includeDirStrList[i]);

		const DirectoryEntry* ddir =  FileMgr.getDirectory(includeDirStrList[i], false);
		if(ddir!=NULL)
		{
			DirectoryLookup hdir(FileMgr.getDirectory(includeDirStrList[i], false),clang::SrcMgr::C_System, false);
			TheCompInst.getPreprocessor().getHeaderSearchInfo().AddSearchPath(hdir,true);
		}
		else
		{
			//fprintf(stderr,"DIR not found\n");
		}
	}

	//fprintf(stderr,"include loaded\n");

	Rewriter TheRewriter;
	TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());


//	fprintf(stderr,"Mark1 %s\n",src);
	//Add Macro


	

	std::ifstream msrc1(src);
	dest[strlen(dest)-4] = 0;
	std::ofstream msrc2(dest);

	fprintf(stderr,"SRC %s\n",dest);

    std::string line;
	if (msrc1.is_open() && msrc2.is_open())
  	{
		for(int i = 0;i <defineCount;i++)
		{
			char buf[1024];
			char *p1 = NULL;
			char *p2 = NULL;


			p1 = defineStrList[i];

			int flag = 0;
			for(int j = 0; j<i;j++)
			{
				if(strcmp(defineStrList[i],defineStrList[j]) == 0)
				{
					flag = 1;
					break;
				}
			}

			if(flag == 1) continue;


			for(int j =0; j< strlen(defineStrList[i]); j++)
			{
				if(defineStrList[i][j] == '=')
				{
					defineStrList[i][j] = 0;
					p2 = &(defineStrList[i][j])+1;
					break;
				}
			}


			if(p2!=NULL)
				sprintf(buf,"#define %s %s\n",p1,p2);
			else
				sprintf(buf,"#define %s\n",p1);
			
			
			msrc2 << buf;		
		}


		msrc2 << std::string("#include \"/ssd/nexus6_lp/build/core/combo/include/arch/linux-arm/AndroidConfig.h\"");



    	while ( getline (msrc1,line) )
    	{
      		msrc2 << line << '\n';
    	}
    	msrc1.close();
		msrc2.close();
  	}




	
  // Set the main file handled by the source manager to the input file.
	const FileEntry *FileIn = FileMgr.getFile(dest);
	SourceMgr.setMainFileID(
    	SourceMgr.createFileID(FileIn, SourceLocation(), SrcMgr::C_User));
  	TheCompInst.getDiagnosticClient().BeginSourceFile(
		TheCompInst.getLangOpts(), &TheCompInst.getPreprocessor());

//	fprintf(stderr,"Mark1\n");
  // Create an AST consumer instance which is going to get called by
  // ParseAST.
	MyASTConsumer TheConsumer(TheRewriter, SourceMgr, TheCompInst.getASTContext());

//	fprintf(stderr,"Mark1\n");
  // Parse the file to AST, registering our consumer as the AST consumer.
	ParseAST(TheCompInst.getPreprocessor(), &TheConsumer,
           TheCompInst.getASTContext());

//	fprintf(stderr,"Mark1\n");
  // At this point the rewriter's buffer should be full with the rewritten
  // file contents.


	dest[strlen(dest)] = '.';
	std::ofstream mout(dest);

  	const RewriteBuffer *RewriteBuf =
      	TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());

	fprintf(stderr,"RewriteBuffer? %p\n",RewriteBuf);
	if(RewriteBuf != NULL) 
	{
		fprintf(stderr,"RewriteBuffer Size %d\n",RewriteBuf->size());
		mout<< std::string("#include <cutils/log.h>\n");
		mout<< std::string(RewriteBuf->begin(), RewriteBuf->end());
	}
	else
	{	  
		fprintf(stderr,"Dump Failed, can not get rewrite buffer for %s\n",src);

	}
    mout.close();


	if(RewriteBuf == 0) return 1;
	else return 0;
}
}
