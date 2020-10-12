#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/Support/CommandLine.h"

using namespace clang::ast_matchers;

// Apply a custom category to all command-line options so that they are the only
// ones displayed.
static llvm::cl::OptionCategory MyToolCategory("My Clang Tool");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static llvm::cl::extrahelp
    CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);

// Matches for loops with initializers that are set to '0'
StatementMatcher LoopMatcher =
    forStmt(hasLoopInit(declStmt(
                hasSingleDecl(varDecl(hasInitializer(integerLiteral(equals(0))))
                                  .bind("initVarName")))),
            hasIncrement(unaryOperator(
                hasOperatorName("++"),
                hasUnaryOperand(declRefExpr(
                    to(varDecl(hasType(isInteger())).bind("incVarName")))))),
            hasCondition(binaryOperator(
                hasOperatorName("<"),
                hasLHS(ignoringParenImpCasts(declRefExpr(
                    to(varDecl(hasType(isInteger())).bind("condVarName"))))),
                hasRHS(expr(hasType(isInteger()))))))
        .bind("forLoop");

static bool areSameVariable(const clang::ValueDecl *First,
                            const clang::ValueDecl *Second) {
  return First && Second &&
         First->getCanonicalDecl() == Second->getCanonicalDecl();
}

static bool areSameExpr(clang::ASTContext *Context, const clang::Expr *First, const clang::Expr *Second) {
  if (!First || !Second)
    return false;
  llvm::FoldingSetNodeID FirstID, SecondID;
  First->Profile(FirstID, *Context, true);
  Second->Profile(SecondID, *Context, true);
  return FirstID == SecondID;
}

class LoopPrinter : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    clang::ASTContext *Context = Result.Context;

    const clang::ForStmt *FS =
        Result.Nodes.getNodeAs<clang::ForStmt>("forLoop");

    // We do not want to convert header files!
    if (!FS ||
        !Context->getSourceManager().isWrittenInMainFile(FS->getForLoc()))
      return;

    const clang::VarDecl *IncVar =
        Result.Nodes.getNodeAs<clang::VarDecl>("incVarName");
    const clang::VarDecl *CondVar =
        Result.Nodes.getNodeAs<clang::VarDecl>("condVarName");
    const clang::VarDecl *InitVar =
        Result.Nodes.getNodeAs<clang::VarDecl>("initVarName");

    // IncVar, CondVar, and InitVar must be the same
    if (!areSameVariable(IncVar, CondVar) || !areSameVariable(IncVar, CondVar))
      return;

    llvm::outs() << "Potential array-based loop discovered.\n";
  }
};

int main(int argc, const char *argv[]) {
  clang::tooling::CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(),
                                 OptionsParser.getSourcePathList());

  LoopPrinter Printer;
  MatchFinder Finder;
  Finder.addMatcher(LoopMatcher, &Printer);

  return Tool.run(clang::tooling::newFrontendActionFactory(&Finder).get());
}
