using namespace clang;


class ASTVistorAddContextWrapper : public RecursiveASTVisitor<ASTVistorAddContextWrapper> {
public:
	ASTVistorAddContextWrapper(SourceManager &S, Rewriter &R) : TheSourceMgr(S), TheRewriter(R) {};

	bool VisitStmt(Stmt *s);

	
	bool isLocInRange(SourceLocation a, SourceLocation b, SourceLocation c); // is a in (b,c)
	
	SourceManager &TheSourceMgr;
	Rewriter &TheRewriter;
};