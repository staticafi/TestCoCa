#include <ast_instrumenter/condition_instrumenter.hpp>

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/Lexer.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

#include <memory>

namespace ast_instrumenter {

namespace {

class ConditionVisitor : public clang::RecursiveASTVisitor<ConditionVisitor> {
   public:
    ConditionVisitor(clang::ASTContext& context, clang::Rewriter& rewriter)
        : context_(context),
          rewriter_(rewriter),
          source_manager_(context.getSourceManager())
    {
    }

    bool TraverseIfStmt(clang::IfStmt* stmt)
    {
        bool value = false;
        if (constant_condition_value(stmt->getCond(), value)) {
            if (value) {
                return TraverseStmt(stmt->getThen());
            }
            return stmt->getElse() ? TraverseStmt(stmt->getElse()) : true;
        }
        wrap_condition(stmt->getCond());
        TraverseStmt(stmt->getCond());
        TraverseStmt(stmt->getThen());
        if (stmt->getElse()) {
            TraverseStmt(stmt->getElse());
        }
        return true;
    }

    bool TraverseWhileStmt(clang::WhileStmt* stmt)
    {
        bool value = false;
        bool is_constant = constant_condition_value(stmt->getCond(), value);
        if (!is_constant) {
            wrap_condition(stmt->getCond());
            TraverseStmt(stmt->getCond());
        } else if (!value) {
            return true;
        }
        return TraverseStmt(stmt->getBody());
    }

    bool TraverseForStmt(clang::ForStmt* stmt)
    {
        if (stmt->getInit()) {
            TraverseStmt(stmt->getInit());
        }
        clang::Expr* condition = stmt->getCond();
        bool value = false;
        bool is_constant = constant_condition_value(condition, value);
        if (condition != nullptr && !is_constant) {
            wrap_condition(condition);
            TraverseStmt(condition);
        }
        if (is_constant && !value) {
            return true;
        }
        if (stmt->getBody()) {
            TraverseStmt(stmt->getBody());
        }
        if (stmt->getInc()) {
            TraverseStmt(stmt->getInc());
        }
        return true;
    }

    bool TraverseDoStmt(clang::DoStmt* stmt)
    {
        TraverseStmt(stmt->getBody());
        bool value = false;
        if (!constant_condition_value(stmt->getCond(), value)) {
            wrap_condition(stmt->getCond());
            TraverseStmt(stmt->getCond());
        }
        return true;
    }

    bool TraverseConditionalOperator(clang::ConditionalOperator* op)
    {
        bool value = false;
        if (constant_condition_value(op->getCond(), value)) {
            return value ? TraverseStmt(op->getTrueExpr())
                         : TraverseStmt(op->getFalseExpr());
        }
        wrap_condition(op->getCond());
        TraverseStmt(op->getCond());
        TraverseStmt(op->getTrueExpr());
        TraverseStmt(op->getFalseExpr());
        return true;
    }

    bool TraverseSwitchStmt(clang::SwitchStmt* stmt)
    {
        handle_switch(stmt);
        return TraverseStmt(stmt->getBody());
    }

    unsigned goal_count() const
    {
        return goal_counter_;
    }

   private:
    bool is_rewritable(clang::SourceLocation location) const
    {
        return location.isValid() && location.isFileID() &&
               source_manager_.isWrittenInMainFile(location);
    }

    std::string source_text(clang::SourceRange range) const
    {
        clang::CharSourceRange char_range =
            clang::CharSourceRange::getTokenRange(range);
        return clang::Lexer::getSourceText(
                   char_range, source_manager_, context_.getLangOpts())
            .str();
    }

    clang::SourceLocation end_of_token(clang::SourceLocation location) const
    {
        return clang::Lexer::getLocForEndOfToken(
            location, 0, source_manager_, context_.getLangOpts());
    }

    bool constant_condition_value(clang::Expr* condition, bool& value) const
    {
        return condition != nullptr && !condition->HasSideEffects(context_) &&
               condition->EvaluateAsBooleanCondition(value, context_);
    }

    void wrap_condition(clang::Expr* condition)
    {
        if (condition == nullptr) {
            return;
        }
        clang::SourceLocation begin = condition->getBeginLoc();
        clang::SourceLocation end = condition->getEndLoc();
        if (!is_rewritable(begin) || !is_rewritable(end)) {
            return;
        }
        std::string id = std::to_string(goal_counter_);
        goal_counter_ += 2;
        rewriter_.InsertTextBefore(begin, "__testcoca_cond(" + id + ", (");
        rewriter_.InsertText(end_of_token(end), "))");
    }

    std::string case_predicate(const clang::CaseStmt* case_stmt,
                               const std::string& selector) const
    {
        std::string low = source_text(case_stmt->getLHS()->getSourceRange());
        if (case_stmt->getRHS() != nullptr) {
            std::string high =
                source_text(case_stmt->getRHS()->getSourceRange());
            return "(" + selector + " >= (" + low + ") && " + selector +
                   " <= (" + high + "))";
        }
        return "(" + selector + " == (" + low + "))";
    }

    void insert_guard(clang::SourceLocation colon, const std::string& text)
    {
        if (!is_rewritable(colon)) {
            return;
        }
        rewriter_.InsertText(end_of_token(colon), text);
    }

    void handle_switch(clang::SwitchStmt* switch_stmt)
    {
        clang::Expr* selector_expr = switch_stmt->getCond();
        if (selector_expr == nullptr) {
            return;
        }
        if (!is_rewritable(selector_expr->getBeginLoc()) ||
            !is_rewritable(selector_expr->getEndLoc()) ||
            !is_rewritable(switch_stmt->getBeginLoc()) ||
            !is_rewritable(switch_stmt->getEndLoc())) {
            return;
        }

        std::string selector_text =
            source_text(selector_expr->getSourceRange());
        std::string selector_var =
            "__tc_sel_" + std::to_string(switch_counter_++);

        rewriter_.InsertTextBefore(
            switch_stmt->getBeginLoc(),
            "{ __typeof__(" + selector_text + ") " + selector_var + " = (" +
                selector_text + "); ");
        rewriter_.ReplaceText(selector_expr->getSourceRange(), selector_var);
        rewriter_.InsertText(end_of_token(switch_stmt->getEndLoc()), " }");

        llvm::SmallVector<clang::SwitchCase*, 8> cases;
        std::string default_predicate;
        for (clang::SwitchCase* switch_case =
                 switch_stmt->getSwitchCaseList();
             switch_case != nullptr;
             switch_case = switch_case->getNextSwitchCase()) {
            cases.push_back(switch_case);
            if (auto* case_stmt =
                    clang::dyn_cast<clang::CaseStmt>(switch_case)) {
                std::string predicate = case_predicate(case_stmt, selector_var);
                if (!default_predicate.empty()) {
                    default_predicate += " || ";
                }
                default_predicate += predicate;
            }
        }

        for (clang::SwitchCase* switch_case : cases) {
            if (auto* case_stmt =
                    clang::dyn_cast<clang::CaseStmt>(switch_case)) {
                std::string predicate = case_predicate(case_stmt, selector_var);
                insert_guard(case_stmt->getColonLoc(),
                             " if (" + predicate +
                                 ") __testcoca_process_goal(" +
                                 std::to_string(goal_counter_++) + ");");
            } else if (auto* default_stmt =
                           clang::dyn_cast<clang::DefaultStmt>(switch_case)) {
                std::string predicate = default_predicate.empty()
                                            ? "1"
                                            : "!(" + default_predicate + ")";
                insert_guard(default_stmt->getColonLoc(),
                             " if (" + predicate +
                                 ") __testcoca_process_goal(" +
                                 std::to_string(goal_counter_++) + ");");
            }
        }
    }

    clang::ASTContext& context_;
    clang::Rewriter& rewriter_;
    clang::SourceManager& source_manager_;
    unsigned switch_counter_ = 0;
    unsigned goal_counter_ = 0;
};

class ConditionASTConsumer : public clang::ASTConsumer {
   public:
    ConditionASTConsumer(clang::CompilerInstance& compiler,
                         std::string output_path)
        : compiler_(compiler), output_path_(std::move(output_path))
    {
        rewriter_.setSourceMgr(compiler_.getSourceManager(),
                               compiler_.getLangOpts());
    }

    void HandleTranslationUnit(clang::ASTContext& context) override
    {
        clang::SourceManager& source_manager = compiler_.getSourceManager();
        clang::SourceLocation file_start =
            source_manager.getLocForStartOfFile(source_manager.getMainFileID());
        rewriter_.InsertText(
            file_start,
            "extern void __testcoca_process_goal(unsigned int);\n"
            "extern _Bool __testcoca_cond(unsigned int, _Bool);\n");

        ConditionVisitor visitor(context, rewriter_);
        visitor.TraverseDecl(context.getTranslationUnitDecl());

        rewriter_.InsertText(
            file_start,
            "unsigned int __testcoca_goal_count = " +
                std::to_string(visitor.goal_count()) + ";\n");

        std::error_code error;
        llvm::raw_fd_ostream output(output_path_, error);
        if (error) {
            llvm::errs() << "ast_instrumenter: cannot open output file '"
                         << output_path_ << "': " << error.message() << "\n";
            return;
        }
        const clang::RewriteBuffer& buffer =
            rewriter_.getEditBuffer(source_manager.getMainFileID());
        buffer.write(output);
    }

   private:
    clang::CompilerInstance& compiler_;
    std::string output_path_;
    clang::Rewriter rewriter_;
};

class ConditionFrontendAction : public clang::ASTFrontendAction {
   public:
    explicit ConditionFrontendAction(std::string output_path)
        : output_path_(std::move(output_path))
    {
    }

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler,
        llvm::StringRef /*input_file*/) override
    {
        return std::make_unique<ConditionASTConsumer>(compiler, output_path_);
    }

   private:
    std::string output_path_;
};

class ConditionFrontendActionFactory
    : public clang::tooling::FrontendActionFactory {
   public:
    explicit ConditionFrontendActionFactory(std::string output_path)
        : output_path_(std::move(output_path))
    {
    }

    std::unique_ptr<clang::FrontendAction> create() override
    {
        return std::make_unique<ConditionFrontendAction>(output_path_);
    }

   private:
    std::string output_path_;
};

}  // namespace

bool instrument_conditions(const std::string& input_path,
                           const std::string& output_path,
                           const std::vector<std::string>& compiler_args)
{
    std::string directory = llvm::sys::path::parent_path(input_path).str();
    if (directory.empty()) {
        directory = ".";
    }

    llvm::SmallString<256> absolute_output(output_path);
    llvm::sys::fs::make_absolute(absolute_output);

    clang::tooling::FixedCompilationDatabase database(directory,
                                                       compiler_args);
    clang::tooling::ClangTool tool(database, {input_path});

    ConditionFrontendActionFactory factory(absolute_output.str().str());
    return tool.run(&factory) == 0;
}

}  // namespace ast_instrumenter
