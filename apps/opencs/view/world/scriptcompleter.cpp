#include "scriptcompleter.hpp"

#include "../../model/world/scriptcontext.hpp"
#include <components/compiler/nullerrorhandler.hpp>
#include <components/compiler/parser.hpp>
#include <components/compiler/extensions.hpp>
#include <components/compiler/lineparser.hpp>
#include <components/compiler/locals.hpp>
#include <components/compiler/literals.hpp>
#include <components/compiler/scanner.hpp>
#include <components/compiler/extensions0.hpp>
#include <QTextEdit>
#include <QTextBlock>
#include <memory>
#include <sstream>

namespace
{
    class CodeCompleteParser : public Compiler::LineParser
    {
            Compiler::NullErrorHandler mStubHandler;
            Compiler::Locals mStubLocals;
            Compiler::Literals mStubLiterals;
            std::vector<Interpreter::Type_Code> mStubCode;
            bool mContextAllowsLocal;
            bool mContextAllowsGlobal;
            bool mContextAllowsId;
            bool mContextAllowsInstruction;
            QStringList mKeywordCompletions;

        public:
            CodeCompleteParser(Compiler::Context& context);

            /// You should deduce context before querying other methods.
            void deduceContext (const std::string &line);
            void reset();
            void parseEOF (Compiler::Scanner& scanner);

            bool contextAllowsLocal() const;
            bool contextAllowsGlobal() const;
            bool contextAllowsId() const;
            bool contextAllowsInstruction() const;
            QStringList getKeywordCompletions() const;
    };
}

CodeCompleteParser::CodeCompleteParser(Compiler::Context &context)
    : Compiler::LineParser(mStubHandler, context, mStubLocals, mStubLiterals, mStubCode)
{
}

void CodeCompleteParser::deduceContext(const std::string &line)
{
    reset();
    std::istringstream stream (line);
    Compiler::Scanner scanner (mStubHandler, stream, getContext().getExtensions());

    try
    {
        scanner.scan (*this);
    }
    catch (...) {
    } // ignore syntax errors

    State state = getState();
    switch (state)
    {
        case SetState:
        {
            mContextAllowsLocal = true;
            mContextAllowsGlobal = true;
            break;
        }
        case ExplicitState:
        {
            mContextAllowsInstruction = true;
            mKeywordCompletions.append("enable");
            mKeywordCompletions.append("disable");
            mKeywordCompletions.append("StartScript");
            mKeywordCompletions.append("StopScript");
            mKeywordCompletions.append("ScriptRunning");
            break;
        }
        case BeginState:
        {
            mContextAllowsId = true;
            mContextAllowsInstruction = true;
            mKeywordCompletions.append("short");
            mKeywordCompletions.append("long");
            mKeywordCompletions.append("float");
            mKeywordCompletions.append("if");
            mKeywordCompletions.append("endif");
            mKeywordCompletions.append("while");
            mKeywordCompletions.append("endwhile");
            mKeywordCompletions.append("MessageBox");
            mKeywordCompletions.append("MenuMode");
            mKeywordCompletions.append("GetSquareRoot");
            mKeywordCompletions.append("set");
            mKeywordCompletions.append("return");
            mKeywordCompletions.append("Random");
            mKeywordCompletions.append("StartScript");
            mKeywordCompletions.append("StopScript");
            mKeywordCompletions.append("ScriptRunning");
            mKeywordCompletions.append("GetDistance");
            mKeywordCompletions.append("GetSecondsPassed");
            mKeywordCompletions.append("Enable");
            mKeywordCompletions.append("Disable");
            mKeywordCompletions.append("GetDisabled");
            break;
        }
        case SetLocalVarState:
        case SetGlobalVarState:
        case SetMemberVarState2:
        {
            mKeywordCompletions.append("to");
            break;
        }
        default:
            break;
    }
}

void CodeCompleteParser::reset()
{
    LineParser::reset();
    mContextAllowsLocal = false;
    mContextAllowsGlobal = false;
    mContextAllowsId = false;
    mContextAllowsInstruction = false;
    mKeywordCompletions.clear();
}

void CodeCompleteParser::parseEOF(Compiler::Scanner &)
{
}

bool CodeCompleteParser::contextAllowsLocal() const
{
    return mContextAllowsLocal;
}

bool CodeCompleteParser::contextAllowsGlobal() const
{
    return mContextAllowsGlobal;
}

bool CodeCompleteParser::contextAllowsId() const
{
    return mContextAllowsId;
}

bool CodeCompleteParser::contextAllowsInstruction() const
{
    return mContextAllowsInstruction;
}

QStringList CodeCompleteParser::getKeywordCompletions() const
{
    return mKeywordCompletions;
}

class CSVWorld::ScriptCompleter::Data
{
    public:
        Compiler::Extensions mExtensions;
        CSMWorld::ScriptContext mContext;
        QStringList mInstructions;
        CodeCompleteParser mParser;
        QTextEdit *mEditor;

        Data(const CSMWorld::Data& data)
            : mContext(data)
            , mParser(mContext)
            , mEditor(NULL)
        {
        }
};

CSVWorld::ScriptCompleter::ScriptCompleter(QTextEdit *editor, const CSMWorld::Data& data)
    : d(new ScriptCompleter::Data(data))
{
    d->mEditor = editor;
    // configure compiler
    Compiler::registerExtensions (d->mExtensions);
    d->mContext.setExtensions (&d->mExtensions);

    std::vector<std::string> keywords;
    d->mExtensions.listKeywords (keywords);
    for (int i = 0, n = keywords.size(); i < n; ++i)
    {
        d->mInstructions.append (QString::fromStdString(keywords[i]));
    }
}

namespace
{
    inline bool caseInsensitiveLessThan(const QString &s1, const QString &s2)
    {
        return s1.compare(s2, Qt::CaseInsensitive) < 0;
    }
}

QStringList CSVWorld::ScriptCompleter::codeComplete(const QTextCursor &cursor)
{
    QString line = cursor.block().text().left(cursor.positionInBlock());
    d->mParser.deduceContext (line.toUtf8().constData());

    QStringList completions = d->mParser.getKeywordCompletions();
    if (d->mParser.contextAllowsInstruction())
    {
        completions.append(d->mInstructions);
    }

    completions.removeDuplicates();
    qSort(completions.begin(), completions.end(), caseInsensitiveLessThan);

    return completions;
}

void CSVWorld::ScriptCompleter::invalidateIds()
{
    d->mContext.invalidateIds();
}

