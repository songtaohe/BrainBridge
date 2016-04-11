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
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
public:
  MyASTVisitor(Rewriter &R) : TheRewriter(R) {}

  bool VisitStmt(Stmt *s) {
    // Only care about If statements.
    if (isa<IfStmt>(s)) {
      IfStmt *IfStatement = cast<IfStmt>(s);
      Stmt *Then = IfStatement->getThen();

      TheRewriter.InsertText(Then->getLocStart(), "// the 'if' part\n", true,
                             true);

      Stmt *Else = IfStatement->getElse();
      if (Else)
        TheRewriter.InsertText(Else->getLocStart(), "// the 'else' part\n",
                               true, true);
    }

    return true;
  }


  bool VisitFunctionDecl(FunctionDecl *f) {
    // Only function definitions (with bodies), not declarations.
    if (f->hasBody()) {
      Stmt *FuncBody = f->getBody();
      
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
      TheRewriter.InsertText(ST, SSBefore.str(), true, true);
      
      // And after
      std::stringstream SSAfter;
      SSAfter << "\n// End function " << FuncName;
      ST = FuncBody->getLocEnd().getLocWithOffset(1);
      TheRewriter.InsertText(ST, SSAfter.str(), true, true);
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
	lo.CPlusPlus = 1;

  // Initialize target info with the default triple for our platform.
	auto TO = std::make_shared<TargetOptions>();
	TO->Triple = llvm::sys::getDefaultTargetTriple();
	TargetInfo *TI =
      TargetInfo::CreateTargetInfo(TheCompInst.getDiagnostics(), TO);
	TheCompInst.setTarget(TI);

	TheCompInst.createFileManager();
	FileManager &FileMgr = TheCompInst.getFileManager();
	TheCompInst.createSourceManager(FileMgr);
	SourceManager &SourceMgr = TheCompInst.getSourceManager();
	TheCompInst.createPreprocessor(TU_Module);
	TheCompInst.createASTContext();



	fprintf(stderr,"%d\n",includeDirCount);	
	DirectoryEntry *ddir = NULL;

	for(int i = 0; i < includeDirCount; i++)
	{
		fprintf(stderr,"%s\n",includeDirStrList[i]);

		const DirectoryEntry* ddir =  FileMgr.getDirectory(includeDirStrList[i], false);
		if(ddir!=NULL)
		{
			DirectoryLookup hdir(FileMgr.getDirectory(includeDirStrList[i], false),clang::SrcMgr::C_System, false);
			TheCompInst.getPreprocessor().getHeaderSearchInfo().AddSearchPath(hdir,true);
		}
		else
		{
			fprintf(stderr,"DIR not found\n");
		}
	}

	fprintf(stderr,"include loaded\n");

	Rewriter TheRewriter;
	TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());


//	fprintf(stderr,"Mark1 %s\n",src);
	
  // Set the main file handled by the source manager to the input file.
	const FileEntry *FileIn = FileMgr.getFile(src);
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

	std::ofstream mout(dest);

  	const RewriteBuffer *RewriteBuf =
      	TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());

	fprintf(stderr,"RewriteBuffer? %p\n",RewriteBuf);
	if(RewriteBuf != NULL) 
	{
		fprintf(stderr,"RewriteBuffer Size %d\n",RewriteBuf->size());
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
