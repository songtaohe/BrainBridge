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
#include "clang/AST/PrettyPrinter.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

//LangOptions &lo;


class ASTVistorGenFuncPara : public RecursiveASTVisitor<ASTVistorGenFuncPara> {
public:
	ASTVistorGenFuncPara(SourceManager &S, const LangOptions &L, SourceLocation s, SourceLocation e) : IsValid(true), TheSourceMgr(S), TheLangOpts(L), lStart(s), lEnd(e) {
		ParameterList.clear();
	}

	bool IsValid;

	bool VisitStmt(Stmt *s) {
		//printf("???");

		if (isa<MemberExpr>(s))
		{
			if((cast<MemberExpr>(s))->getFoundDecl()->getAccess() == AS_private) 
			{
				//fprintf(stderr, "A private member\n");
				IsValid = false;
			}
		}


		if (isa<DeclRefExpr>(s))
    	{
        	DeclRefExpr * dre = cast<DeclRefExpr>(s);
			ValueDecl * valueDecl = dre->getDecl();

			//if(dre->getFoundDecl()->getAccess() != AS_public)  // Avoid private type 
			//{
			//	IsValid = false;
			//}


			if(isa<CXXMethodDecl>(valueDecl)) 
			{
				// Check whether it is a static one
				if(cast<CXXMethodDecl>(valueDecl)->isStatic())
				{
					IsValid = false;
				}

			}
		
			if(isa<FunctionDecl>(valueDecl))
			{
				// Check whether it is a local one
				if(valueDecl->getLocStart().isValid() && dStart.isValid() && dEnd.isValid())
				{
					if(isLocInRange(valueDecl->getLocStart(), dStart,dEnd)) IsValid = false;
				}
				else
				{
					fprintf(stderr, "isLocInRange Location Invalid Bug \n");
					IsValid = false;
				}
				//IsValid = false;  // FIXME This filter is too relaxed
			}

			QualType t = valueDecl->getType();
            t = t.getNonReferenceType();
		
			if(t.getTypePtr()->getAsCXXRecordDecl() != NULL)
			{
				if(t.getTypePtr()->getAsCXXRecordDecl()->getAccess() == AS_private)
				{
					IsValid = false; // Filter out those nested private type decl 
				}
			}			


            std::string  typestr = t.getAsString();

            int cnt = std::count(typestr.begin(), typestr.end(),':');
            if(cnt > 2) IsValid = false; // FIXME  Need to indentity private type!   Too Complex .....




			//if(isa<VarDecl>(valueDecl) || isa<FunctionDecl>(valueDecl) )
			if(isa<VarDecl>(valueDecl) )
			{
				//VarDecl * varDecl = cast<VarDecl>(valueDecl);
				QualType t = valueDecl->getType();		
				t = t.getNonReferenceType();
	
				//std::string  typestr = t.getAsString(); 

				//int cnt = std::count(typestr.begin(), typestr.end(),':'); 
				//if(cnt > 2) IsValid = false;


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
					for(int i = 1; i < ParameterList.size();i+= 2)
					{
						if(ParameterList.at(i) == mName) 
						{
							existed = true;
							break;
						}
					}

					if(existed == false)
					{
						std::string s;
						llvm::raw_string_ostream mStr(s);
						//LangOptions lo;
						PrintingPolicy pp(TheLangOpts);
						//Twine mT("(&"+dre->getNameInfo().getAsString()+")");
						std::string mTypeStr = dre->getNameInfo().getAsString();
						std::string mRefTypeStr = "(&" + mTypeStr + ")";				
						Twine mT(mRefTypeStr);
						t.print(mStr,pp,mT,0);
	
						//TypePrinter mTypePrinter(TheLangOpts);
						//mTypePrinter.printBefore(t,mStr);

						//fprintf(stderr,"%s\n",mStr.str().c_str());
					
						//ParameterList.push_back(t.getAsString());
						ParameterList.push_back(mStr.str());
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
		

		// Following are filters

		if (isa<ReturnStmt>(s))
		{
			IsValid = false;
		}

		if (isa<CXXThisExpr>(s))
		{
			IsValid = false;
		}

		if (isa<BreakStmt>(s))
		{
			IsValid = false;
		}

		if (isa<ContinueStmt>(s))
		{
			IsValid = false;
		}
	
		if (isa<CaseStmt>(s))
		{
			IsValid = false;
		}

		if (isa<GotoStmt>(s))
		{
			IsValid = false;
		}

		if (isa<DefaultStmt>(s))
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

	void setDeclRange(SourceLocation s, SourceLocation e)
	{
		dStart = s;
		dEnd = e;
	}

	bool isLocInRange(SourceLocation a, SourceLocation b, SourceLocation c) // is a in (b,c)
	{
		BeforeThanCompare<SourceLocation> isBefore(TheSourceMgr);

        bool b_before_a = isBefore(b,a);
        bool a_before_c = isBefore(a,c);
        return b_before_a && a_before_c;
	}


	std::vector<std::string> ParameterList;
	SourceManager &TheSourceMgr;
	const LangOptions &TheLangOpts;
	SourceLocation lStart;
	SourceLocation lEnd;
	SourceLocation dStart;
	SourceLocation dEnd;

};



class ASTVistorModulizer : public RecursiveASTVisitor<ASTVistorModulizer> {
public:
	ASTVistorModulizer(SourceManager &S, Rewriter &R, SourceLocation loc, SourceLocation ds, SourceLocation de) : IsValid(true), TheSourceMgr(S), TheRewriter(R), locDump(loc), dStart(ds), dEnd(de) {
		ParameterList.clear();
	}

	bool IsValid;

	bool VisitStmt(Stmt *s) {
		//printf("???");
		if (isa<DeclStmt>(s))
		{
			return false; // Do not go into local decl, e.g. class;
		}

		if (isa<CompoundStmt>(s))
		{
			CompoundStmt *cs = cast<CompoundStmt>(s);

			Stmt** ModuleStart = NULL;
			Stmt** ModuleEnd = NULL;

			
			ASTVistorGenFuncPara mChecker(TheSourceMgr,TheRewriter.getLangOpts(), s->getLocStart(), s->getLocEnd());
	       	mChecker.cleanUp();
 			mChecker.setDeclRange(dStart,dEnd);

			//mChecker.TraverseStmt(FuncBody);


            for(Stmt** locals = cs->body_begin(); locals < cs->body_end(); locals++)
            {
                //printf("Locals %p \n",locals);
                if(locals!=NULL && (*locals) != NULL)
                {
					bool isValid = true;
					bool isLast = false;

					if(isa<DeclStmt>(*locals))
					{
						isValid = false;
						//ModuleStart = NULL;
                        //ModuleEnd = NULL;
                        //mChecker.cleanUp();
					}
					else
					{
						mChecker.setLocationRange((*locals)->getLocStart(),(*locals)->getLocEnd());

						std::vector<std::string> backupList = mChecker.ParameterList;  // There was a horrible bug! 
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
							mChecker.ParameterList = backupList;
							isValid = false;
							// Reclusive 
							if(!isa<DeclStmt>(*locals)) // No local record, class, structure and etc decl. 
							{
								ASTVistorModulizer mModulizer(TheSourceMgr, TheRewriter, locDump, dStart, dEnd);
								mModulizer.TraverseStmt((*locals));			
							}
						}

					}
	

					if(isValid == true && locals == (cs->body_end() -1))
					{
						isValid = false;
						isLast = true;
					}

					if(isValid == false)
					{
						if(ModuleStart == NULL) // Range Empty
						{
							// do nothing
							
						}
						else
						{				
							//TODO Here  Generate Module
							// Parameter List from mChecker
							// Code dumped from moduleStart to moduleEnd

							std::stringstream str;

							// Dump Module Position (TODO Debug Only)
							//str << " /* ";
							//str << "Module " << (*ModuleStart)->getLocStart().printToString(TheSourceMgr);
							//str << " to " << (*ModuleEnd)->getLocEnd().printToString(TheSourceMgr);
							//str << " */ \n" ;

							//str << "\nnamespace android {\n";
							str << "\nstatic void F_" << functionCounter++ ;  // FIXME add 'static' as a temperory fix; Need unique id!!!
							str << "(";
							// Paramter List
							for(int j = 0; j < mChecker.ParameterList.size();j+=2)
							{
								//str << mChecker.ParameterList.at(j) << " &" << mChecker.ParameterList.at(j+1);
								str << mChecker.ParameterList.at(j) ;
								if(j+2 != mChecker.ParameterList.size()) str << ",";
							}

							str << "){\n";
							// Stmts
							//for(Stmt ** tmpStmt = ModuleStart; tmpStmt <= ModuleEnd; tmpStmt++)
							//{
								//std::string mStr;	
								//llvm::raw_string_ostream rostream(mStr);
								//(*tmpStmt)->dump(rostream, TheSourceMgr);
								//str << rostream.str();
							//}

							SourceLocation tmpLocEnd;

							if(isLast == false)
							{
								tmpLocEnd = (*locals) -> getLocStart().getLocWithOffset(-1);
								if(tmpLocEnd.isValid())
								{

								}
								else
								{
									fprintf(stderr, "Can not copy code, location invalid 2\n");
								}

							}
							else
							{
								tmpLocEnd = cs -> getLocEnd().getLocWithOffset(-1);
								if(tmpLocEnd.isValid())
								{

								}
								else
								{
									fprintf(stderr, "Can not copy code, location invalid 1\n");
								}

							}


							tmpLocEnd = (*ModuleEnd)->getLocEnd();


							FullSourceLoc _fullLocStart((*ModuleStart)->getLocStart(), TheSourceMgr);
							FullSourceLoc _fullLocEnd(tmpLocEnd, TheSourceMgr);
							FullSourceLoc fullLocStart(_fullLocStart.getExpansionLoc(), TheSourceMgr);
                            FullSourceLoc fullLocEnd(_fullLocEnd.getExpansionLoc(), TheSourceMgr);


							std::string  tmpCodeStr;
						
							if(fullLocStart.getFileID() == TheSourceMgr.getMainFileID()
								&& fullLocEnd.getFileID() == TheSourceMgr.getMainFileID())
							{
	
							if(fullLocStart.getExpansionLoc().isValid() && fullLocEnd.getExpansionLoc().isValid())
							{

								fprintf(stderr, "From %s to %s\n",fullLocStart.getExpansionLoc().printToString(TheSourceMgr).c_str(),fullLocEnd.getExpansionLoc().printToString(TheSourceMgr).c_str());	
								tmpCodeStr =  TheRewriter.getRewrittenText(SourceRange(fullLocStart.getExpansionLoc(), fullLocEnd.getExpansionLoc()));
								Rewriter::RewriteOptions rangeOpts;
						        rangeOpts.IncludeInsertsAtBeginOfRange = false;
								int offset = TheRewriter.getRangeSize(SourceRange(fullLocEnd.getExpansionLoc(),fullLocEnd.getExpansionLoc()), rangeOpts);

								char t = *(TheSourceMgr.getCharacterData(fullLocEnd.getLocWithOffset(offset-1)));
								if(t != ';' && t != '}')
								{
								fprintf(stderr, "Offset %d \n",offset);
								while(1)
								{
										
       							    char t = *(TheSourceMgr.getCharacterData(fullLocEnd.getLocWithOffset(offset)));
									if(t!=';' && t!='}')
									{
										fprintf(stderr,"%c",t);
										tmpCodeStr += t;
									}
									else
									{
										if(t == ';') tmpCodeStr += t;
										break;
									}
									offset++;
								}
								}								
							}
							else
							{
								fprintf(stderr, "Can not copy code, location invalid\n");	
							}

							}
							else
							{
								fprintf(stderr, "Module Range Across File (a bug) \n");

							}



							const char * tmpCodeStrPtr = tmpCodeStr.c_str();
	

							bool getrid = false;
							for (int i = 0; i < strlen(tmpCodeStrPtr); i++) // Remove the macro define
							{	
								if(tmpCodeStrPtr[i] == '#') getrid = true;
								if(tmpCodeStrPtr[i] == '\n') getrid = false;

								if(getrid == false) str << tmpCodeStrPtr[i];
							}
							



							str << "\n}\n";
							

							TheRewriter.InsertText(locDump, str.str(),true,true);

							std::stringstream tag;
							tag << " /* HST_Module " << functionCounter << " */\n";
							TheRewriter.InsertText(fullLocStart, tag.str(),true,true);

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

	SourceLocation dStart;
	SourceLocation dEnd;

	static int functionCounter;
	static int totalModulizedLine;
};

int ASTVistorModulizer::functionCounter = 0;
int ASTVistorModulizer::totalModulizedLine = 0;


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
	return true;  // The following part is for debug only.

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

	bool VisitDecl(Decl *d)
	{
		if(isa<NamespaceDecl>(d))
		{
			std::string name = (cast<NamespaceDecl>(d))->getNameAsString();
			if(name == "android") return true; // Do not update!
		}
		updateScope(d);
		return true;
	}


  bool VisitFunctionDecl(FunctionDecl *f) {
	updateScope(f);
    // Only function definitions (with bodies), not declarations.
    if (f->hasBody()) {
      Stmt *FuncBody = f->getBody();
  		
		if(isa<CompoundStmt>(FuncBody))
		{
			//Start the modulizer !!! //TODO

			// BlackList
			 DeclarationName DeclName = f->getNameInfo().getName();
      		 std::string FuncName = DeclName.getAsString();

        	//if(strcmp("operator*",FuncName.c_str()) != 0 &&
			//	strcmp("doLogFrameDurations",FuncName.c_str()) !=0)  //FIXME temperory fix for private member access
        	//{
        
				ASTVistorModulizer mModulizer(TheSourceMgr, TheRewriter, scopeStart, scopeStart, scopeEnd);
				//ASTVistorModulizer mModulizer(TheSourceMgr, TheRewriter, f->getSourceRange().getBegin(), f->getLocStart(), f->getLocEnd());
				//mModulizer.dStart = f->getSourceRange().getBegin();
				//mModulizer.dEnd = f->getSourceRange().getEnd();
				mModulizer.TraverseStmt(FuncBody); 
			//}	
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

		ASTVistorGenFuncPara mGenFuncPara(TheSourceMgr,TheRewriter.getLangOpts(),FuncBody->getLocStart(), FuncBody->getLocEnd());
		//mGenFuncPara.TraverseStmt(FuncBody);
		
		char buf[1024*64];
		char * lp = buf;

		lp = lp + sprintf(lp, " /* (");	
	/*	
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
	*/
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
     
		if(strcmp("inverse",FuncName.c_str()) == 0)
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

	  //return false;

    } 
    
    return true;
  } 
  
private:
	Rewriter &TheRewriter;
	SourceManager &TheSourceMgr;
	ASTContext &TheASTContext;

	SourceLocation scopeStart; // the first {} from nothing...
	SourceLocation scopeEnd;
	
	bool isLocInRange(SourceLocation a, SourceLocation b, SourceLocation c) // is a in (b,c)
    {
        BeforeThanCompare<SourceLocation> isBefore(TheSourceMgr);

        bool b_before_a = isBefore(b,a);
        bool a_before_c = isBefore(a,c);
        return b_before_a && a_before_c;
    }


	void updateScope(Decl *s)
	{
		static int init = 0;		
		if(init == 0)
		{
			scopeStart = s->getSourceRange().getBegin();
			scopeEnd = s->getSourceRange().getEnd();
			init = 1;
		}
		else
		{
			if(!isLocInRange(s->getSourceRange().getBegin(),scopeStart,scopeEnd))
			{
				scopeStart = s->getSourceRange().getBegin();
	            scopeEnd = s->getSourceRange().getEnd();
			}		

		}

	} 



};

class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R,SourceManager &S, ASTContext &C) : Visitor(R,S,C),TheSourceMgr(S) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
    {  // Traverse the declaration using our AST visitor.
     	FullSourceLoc fLoc((*b)->getLocStart(),TheSourceMgr);
		if(TheSourceMgr.getMainFileID() == fLoc.getFileID())
		{
			Visitor.TraverseDecl(*b);
		
	  		//(*b)->dumpColor();
		}
	}  //(*b)->dumpColor();
    return true;
  }

private:
	MyASTVisitor Visitor;	
	SourceManager &TheSourceMgr;
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
//	lo = TheCompInst.getLangOpts();
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
