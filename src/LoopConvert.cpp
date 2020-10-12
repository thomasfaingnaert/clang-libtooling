#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/Support/CommandLine.h"

// Apply a custom category to all command-line options so that they are the only
// ones displayed.
static llvm::cl::OptionCategory MyToolCategory("My Clang Tool");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static llvm::cl::extrahelp
    CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);

int main(int argc, const char *argv[]) {
  clang::tooling::CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(),
                                 OptionsParser.getSourcePathList());
  return Tool.run(
      clang::tooling::newFrontendActionFactory<clang::SyntaxOnlyAction>()
          .get());

  return 0;
}
