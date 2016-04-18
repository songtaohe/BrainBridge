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

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
  MyASTVisitor(Rewriter &R) : TheRewriter(R) {}

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




    
      // Type name as string
      QualType QT = f->getReturnType();
      std::string TypeStr = QT.getAsString();
      
      // Function name
      DeclarationName DeclName = f->getNameInfo().getName();
      std::string FuncName = DeclName.getAsString();
      
      // Add comment before
      std::stringstream SSBefore;
      SSBefore << "// Begin function " << FuncName << " returning " << TypeStr
               << "\n";
      SourceLocation ST = f->getSourceRange().getBegin();
 //     TheRewriter.InsertText(ST, SSBefore.str(), true, true);
      
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
};

class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : Visitor(R) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
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
	MyASTConsumer TheConsumer(TheRewriter);

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
