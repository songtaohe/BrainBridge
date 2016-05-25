#include <cstdio>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
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
#include "clang/AST/PrettyPrinter.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

#include "Rewriter.h"
#include "ContextWrapper.h"
using namespace clang;

//LangOptions &lo;

bool ASTVistorAddContextWrapper::VisitStmt(Stmt *s) {

	if (isa<MemberExpr>(s))
	{
		//TODO filter out functions


		Expr* mBase = (cast<MemberExpr>(s))->getBase();
		Rewriter::RewriteOptions rangeOpts;
		rangeOpts.IncludeInsertsAtBeginOfRange = false;
		int offset = TheRewriter.getRangeSize(mBase->getSourceRange(), rangeOpts);

		fprintf(stderr, "Context Offset %d\n",offset);
		bool IsImplicitThis = false;		
		if(isa<CXXThisExpr>(mBase))
		{
			if(cast<CXXThisExpr>(mBase)->isImplicit()) IsImplicitThis = true;
		}

		bool IsArrow = (cast<MemberExpr>(s))->isArrow();




		TheRewriter.InsertText(mBase->getLocStart(), "(*((TYPE)(",true,true);
		
		if(IsImplicitThis) return true;

		if(IsArrow)
		{
			TheRewriter.InsertText(mBase->getLocStart(), "((void*)(",true,true);
		}
		else
		{
			TheRewriter.InsertText(mBase->getLocStart(), "(void*)(&(",true,true);
		}


		if(IsArrow)
		{
			std::string str = TheRewriter.getRewrittenText(SourceRange((cast<MemberExpr>(s))->getOperatorLoc(),(cast<MemberExpr>(s))->getExprLoc()));
			TheRewriter.ReplaceText(SourceRange((cast<MemberExpr>(s))->getOperatorLoc(),cast<MemberExpr>(s)->getExprLoc()),"+Offset)))");
			
			fprintf(stderr, "SSSS %s\n", str.c_str());
		}
		else
		{
			std::string str = TheRewriter.getRewrittenText(SourceRange((cast<MemberExpr>(s))->getOperatorLoc(),(cast<MemberExpr>(s))->getExprLoc()));
			TheRewriter.ReplaceText(SourceRange((cast<MemberExpr>(s))->getOperatorLoc(),cast<MemberExpr>(s)->getExprLoc()),"+Offset)))");
			
			fprintf(stderr, "SSSS %s\n", str.c_str());
		}


		if(IsImplicitThis)
		{
			TheRewriter.InsertText(mBase->getLocEnd(), "LOCALBASE))",true,true);
		}
		else
		{
			TheRewriter.InsertText(mBase->getLocStart().getLocWithOffset(offset), "))",true,true);
		}

		
		//
		
		
	}




	return true;
}


bool ASTVistorAddContextWrapper::isLocInRange(SourceLocation a, SourceLocation b, SourceLocation c) // is a in (b,c)
{
	BeforeThanCompare<SourceLocation> isBefore(TheSourceMgr);

    bool b_before_a = isBefore(b,a);
    bool a_before_c = isBefore(a,c);
    return b_before_a && a_before_c;
}




